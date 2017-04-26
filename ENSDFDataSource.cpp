#include "ENSDFDataSource.h"

#include <QFile>
#include <QSettings>
#include <QMessageBox>
#include <QApplication>
#include <QProgressDialog>
#include <QMutexLocker>
#include <QFileDialog>
#include <QDesktopServices>

#include "ENSDFParser.h"
#include "custom_logger.h"

const quint32 ENSDFDataSource::magicNumber = 0x4b616945;
const quint32 ENSDFDataSource::cacheVersion = 5;

ENSDFDataSource::ENSDFDataSource(QObject *parent)
  : QObject(parent)
  , root(new ENSDFTreeItem(ENSDFTreeItem::RootType))
  , mccache(0)
{
  // initialize cache path
  cachePath = QStandardPaths::displayName(QStandardPaths::DataLocation);
  if (cachePath.isEmpty())
    cachePath = qApp->applicationDirPath();

  // initialize ENSDF path if necessary
  defaultPath = QStandardPaths::displayName(QStandardPaths::DataLocation);
  if (defaultPath.isEmpty())
    defaultPath = qApp->applicationDirPath();
  defaultPath.append("/ensdf");
  QSettings s;
  if (!s.contains("ensdfPath"))
  {
    s.setValue("ensdfPath", defaultPath);
    s.sync();
  }

  // load decay cache
  if (!loadENSDFCache())
    createENSDFCache();
}

ENSDFDataSource::~ENSDFDataSource()
{
  delete root;
  delete mccache;
}

ENSDFTreeItem *ENSDFDataSource::rootItem() const
{
  return root;
}

DecayScheme ENSDFDataSource::decay(const ENSDFTreeItem *item) const
{
  QMutexLocker locker(&m);
  const ENSDFTreeItem *eitem = dynamic_cast<const ENSDFTreeItem*>(item);
  if (!eitem)
    return DecayScheme();

  if (!eitem->parent() || !eitem->isSelectable())
    return DecayScheme();
  if (mccache && mccache->aValue() == eitem->parent()->id().A()) {
    DecayScheme dec(mccache->decay(eitem->parent()->id(), eitem->data(0).toString().toStdString()));
    return dec;
  }

  delete mccache;
  QSettings s;
  QString dir =  s.value("ensdfPath", ".").toString();
  mccache = new ENSDFParser(eitem->parent()->id().A(), dir.toStdString());
  DecayScheme dec(mccache->decay(eitem->parent()->id(), eitem->data(0).toString().toStdString()));
  return dec;
}

void ENSDFDataSource::deleteDatabaseAndCache()
{
  QSettings s;
  deleteCache();

  // get A (mass number) strings
  QList<uint16_t> aList(getAvailableDataFileNumbers());
  if (aList.isEmpty())
    return;
  foreach (uint16_t a, aList) {
    QFile f(s.value("ensdfPath", ".").toString() + QString("/ensdf.%1").arg(a, int(3), int(10), QChar('0')));
    if (f.exists())
      f.remove();
  }
  QCoreApplication::exit(6000); // tells the code in main.cpp to restart the application!
}

void ENSDFDataSource::deleteCache()
{
  // return if cache file is missing
  QFile f(QDir(cachePath).absoluteFilePath("nuclei_ensdf.cache"));
  // delete file
  if (f.exists())
    f.remove();
  QCoreApplication::exit(6000); // tells the code in main.cpp to restart the application!
}

QList<uint16_t> ENSDFDataSource::getAvailableDataFileNumbers()
{
  QWidget *pwid = qobject_cast<QWidget*>(parent());

  QSettings s;
  QString dir =  s.value("ensdfPath", ".").toString();

  // get A (mass number) strings or exit application
  std::list<uint16_t> aList(ENSDFParser::aValues(dir.toStdString()));
  bool firsttry = true;
  while (aList.empty())
  {
    if (!firsttry)
      if (QMessageBox::Close ==
          QMessageBox::warning(pwid, "Selected Folder is Empty",
                               "Please select a folder containing the ENSDF database or use the download feature!",
                               QMessageBox::Ok | QMessageBox::Close, QMessageBox::Ok))
      {
        qApp->quit();
        return QList<uint16_t>();
      }

    dir = QFileDialog::getExistingDirectory(pwid, "Select Database Folder", dir);
    if (dir.isEmpty())
    {
      qApp->quit();
      return QList<uint16_t>();
    }
    dest.setPath(dir);
    s.setValue("ensdfPath", dest.absolutePath());

    aList = ENSDFParser::aValues(dir.toStdString());
    firsttry = false;
  }

  return QList<uint16_t>::fromStdList(aList);
}


bool ENSDFDataSource::loadENSDFCache()
{
  QSettings s;

  // return if cache file is missing
  QFile f(QDir(cachePath).absoluteFilePath("nuclei_ensdf.cache"));
  if (!f.open(QIODevice::ReadOnly))
    return false;

  // make sure ensdf files are available
  if (getAvailableDataFileNumbers().isEmpty())
    return false;

  QDataStream in(&f);
  quint32 magic;
  in >> magic;
  if (magic != magicNumber)
    return false;
  quint32 version;
  in >> version;
  if (version != cacheVersion)
    return false;
  qint32 qtversion;
  in >> qtversion;
  in.setVersion(qtversion);
  QString ensdfversion;
  in >> ensdfversion;
  if (ensdfversion != s.value("ensdfVersion").toString())
    return false;

  in >> (*root);

  return true;
}

void ENSDFDataSource::createENSDFCache()
{
  QWidget *pwid = qobject_cast<QWidget*>(parent());

  // get A (mass number) strings or exit application
  QList<uint16_t> aList(getAvailableDataFileNumbers());
  if (aList.isEmpty())
    return;

  // open cache file
  //  first try to read cache (path might be different than before)
  if (loadENSDFCache())
    return;

  QSettings s;
  QString ensdf_dir =  s.value("ensdfPath", ".").toString();
  //create directory if it does not exist
  QDir cacheDir(cachePath);
  if (!cacheDir.exists())
    cacheDir.mkpath(cachePath);
  QFile f(cacheDir.absoluteFilePath("nuclei_ensdf.cache"));
  while (!f.open(QIODevice::WriteOnly))
    if (QMessageBox::Close == QMessageBox::warning(pwid, "ENSDF Folder is not writeable!", "<p>The currently selected folder <br />" + cachePath + "<br /> is not writeable!</p><p>It must be writeable to create a cache file.</p>", QMessageBox::Close, QMessageBox::Close)) {
      qApp->quit();
      return;
    }

  QDataStream out(&f);
  out << magicNumber;
  out << cacheVersion;
  out << qint32(out.version());
  out << s.value("ensdfVersion").toString();

  // create cache structures
  QProgressDialog pd(pwid);
  pd.setWindowTitle("Nuclei Cache");
  pd.setLabelText("Creating Decay Cache...");
  pd.setMaximum(aList.size());
  pd.setWindowModality(Qt::WindowModal);
  pd.setCancelButton(0);

  std::set<std::string> unknown_decays;

  for (auto &a : aList) {
    ENSDFParser *mc = new ENSDFParser(a, ensdf_dir.toStdString());

    unknown_decays.insert(mc->unknown_decays.begin(), mc->unknown_decays.end());

    for (auto &daughter : mc->daughterNuclides()) {
      ENSDFTreeItem *d = new ENSDFTreeItem(ENSDFTreeItem::DaughterType,
                                           daughter,
                                           QList<QVariant>() << QString::fromStdString(daughter.symbolicName()).toUpper(),
                                           false,
                                           root);
      for (auto &decay : mc->decays(daughter))
      {
        QString st = QString::fromStdString(decay.first);
        new ENSDFTreeItem(ENSDFTreeItem::DecayType,
                          decay.second,
                          QList<QVariant>() << st,
                          true, d);
      }
    }

    delete mc;

    pd.setValue(a);
  }
  pd.setValue(aList.size());

  out << (*root);

  DBG << "Unknown decays:";
  for (auto d : unknown_decays)
    DBG << "   " << d;

}
