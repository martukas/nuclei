#include "ENSDFDataSource.h"

#include <QFile>
#include <QSettings>
#include <QMessageBox>
#include <QApplication>
#include <QProgressDialog>

#include "ENSDFDownloader.h"
#include "ENSDFMassChain.h"

const quint32 ENSDFDataSource::magicNumber = 0x4b616945;
const quint32 ENSDFDataSource::cacheVersion = 1;

ENSDFDataSource::ENSDFDataSource(QObject *parent)
    : AbstractDataSource(parent), root(new ENSDFTreeItem(0)), mccache(0)
{
    // load decay cache
    if (!loadENSDFCache())
        createENSDFCache();
}

ENSDFDataSource::~ENSDFDataSource()
{
    delete root;
}

AbstractTreeItem *ENSDFDataSource::rootItem() const
{
    return root;
}

QSharedPointer<Decay> ENSDFDataSource::decay(const AbstractTreeItem *item) const
{
    const ENSDFTreeItem *eitem = dynamic_cast<const ENSDFTreeItem*>(item);
    if (!eitem)
        return QSharedPointer<Decay>();

    if (!eitem->parent() || !eitem->isSelectable())
        return QSharedPointer<Decay>();

    if (mccache && mccache->aValue() == eitem->A())
        return mccache->decay(eitem->parent()->data(0).toString(), eitem->data(0).toString());

    delete mccache;
    mccache = new ENSDFMassChain(eitem->A());
    return mccache->decay(eitem->parent()->data(0).toString(), eitem->data(0).toString());
}


bool ENSDFDataSource::loadENSDFCache()
{
    QSettings s;
    QFile f(s.value("ensdfPath", ".").toString() + "/kaihen_ensdf.cache");
    if (!f.open(QIODevice::ReadOnly))
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
    QList<unsigned int> aList(ENSDFMassChain::aValues());
    bool firsttry = true;
    while (aList.isEmpty()) {
        if (!firsttry)
            if (QMessageBox::Close == QMessageBox::warning(pwid, "Selected Folder is Empty", "Please select a folder containing the ENSDF database or use the download feature!", QMessageBox::Ok | QMessageBox::Close, QMessageBox::Ok))
                qApp->quit();
        ENSDFDownloader downloader(pwid);
        if (downloader.exec() != QDialog::Accepted) {
            qApp->quit();
            return;
        }

        aList = ENSDFMassChain::aValues();
        firsttry = false;
    }

    // open cache file
    //  first try to read cache (path might be different than before)
    if (loadENSDFCache())
        return;

    QSettings s;
    QFile f(s.value("ensdfPath", ".").toString() + "/kaihen_ensdf.cache");
    while (!f.open(QIODevice::WriteOnly))
        if (QMessageBox::Close == QMessageBox::warning(pwid, "ENSDF Folder is not writeable!", "<p>The currently selected folder <br />" + s.value("ensdfPath", ".").toString() + "<br /> is not writeable!</p><p>It must be writeable to create a cache file.</p>", QMessageBox::Ok | QMessageBox::Close, QMessageBox::Retry))
            qApp->quit();

    QDataStream out(&f);
    out << magicNumber;
    out << cacheVersion;
    out << qint32(out.version());
    out << s.value("ensdfVersion").toString();

    // create cache structures
    QProgressDialog pd(pwid);
    pd.setLabelText("Creating Decay Cache...");
    pd.setMaximum(aList.size());
    pd.setWindowModality(Qt::WindowModal);
    pd.setCancelButton(0);
    foreach (unsigned int a, aList) {
        ENSDFMassChain *mc = new ENSDFMassChain(a);

        foreach (const QString &daughter, mc->daughterNuclides()) {
            ENSDFTreeItem *d = new ENSDFTreeItem(QList<QVariant>() << daughter, a, false, root);
            foreach (const QString decay, mc->decays(daughter))
                new ENSDFTreeItem(QList<QVariant>() << decay, a, true, d);
        }

        delete mc;

        pd.setValue(a);
    }
    pd.setValue(aList.size());

    out << (*root);
}




ENSDFTreeItem::ENSDFTreeItem(AbstractTreeItem *parent)
    : AbstractTreeItem(parent), m_A(0)
{
}

ENSDFTreeItem::ENSDFTreeItem(const QList<QVariant> &data, unsigned int A, bool isdecay, AbstractTreeItem *parent)
    : AbstractTreeItem(data, isdecay, parent), m_A(A)
{
}

unsigned int ENSDFTreeItem::A() const
{
    return m_A;
}

QDataStream & operator <<(QDataStream &out, const ENSDFTreeItem &treeitem)
{
    out << treeitem.itemData;
    out << treeitem.m_A;
    out << treeitem.m_isSelectable;
    out << quint32(treeitem.childItems.size());
    foreach (const AbstractTreeItem *it, treeitem.childItems) {
        const ENSDFTreeItem *eit = dynamic_cast<const ENSDFTreeItem*>(it);
        if (eit)
            out << (*eit);
    }
    return out;
}


QDataStream & operator >>(QDataStream &in, ENSDFTreeItem &treeitem)
{
    in >> treeitem.itemData;
    in >> treeitem.m_A;
    in >> treeitem.m_isSelectable;
    quint32 numchildren;
    in >> numchildren;
    for (quint32 i=0; i<numchildren; i++) {
        ENSDFTreeItem *childitem = new ENSDFTreeItem(&treeitem);
        in >> (*childitem);
    }
    return in;
}

