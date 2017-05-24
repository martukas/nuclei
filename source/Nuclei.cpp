#include "Nuclei.h"
#include "ui_Nuclei.h"
#include "ui_PreferencesDialog.h"

#include <QResizeEvent>
#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QSvgGenerator>
#include <QFileDialog>
#include <QPrinter>
#include <QDesktopWidget>
#include <algorithm>
#include <functional>

#include "ENSDFDataSource.h"
#include "DecayCascadeItemModel.h"
#include "DecayCascadeFilterProxyModel.h"

#include "custom_logger.h"

Nuclei::Nuclei(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::NucleiMainWindow),
  preferencesDialog(new QDialog(this)), preferencesDialogUi(new Ui::PreferencesDialog),
  decaySelectionModel(0),
  searchResultSelectionModel(0),
  decayProxyModel(0),
  searchProxyModel(0)
{
  CustomLogger::initLogger(nullptr, "");

  ui->setupUi(this);
  setWindowTitle(QCoreApplication::applicationName() + QString(" ") + QCoreApplication::applicationVersion());

  preferencesDialogUi->setupUi(preferencesDialog);

  QAction *toggleSelector = ui->decaySelectorDock->toggleViewAction();
  toggleSelector->setToolTip(toggleSelector->toolTip().prepend("Show/Hide "));
  toggleSelector->setIcon(QIcon(":/toolbar/checkbox.png"));
  ui->mainToolBar->insertAction(ui->actionZoom_In, toggleSelector);

  ui->mainToolBar->insertSeparator(ui->actionZoom_In);

  connect(ui->decayTreeCollapseButton, SIGNAL(clicked()), ui->decayTreeView, SLOT(collapseAll()));
  connect(ui->decayTreeExpandButton, SIGNAL(clicked()), ui->decayTreeView, SLOT(expandAll()));

  connect(ui->actionSVG_Export, SIGNAL(triggered()), this, SLOT(svgExport()));
  connect(ui->actionPDF_Export, SIGNAL(triggered()), this, SLOT(pdfExport()));

  connect(ui->actionShow_all, SIGNAL(triggered()), this, SLOT(showAll()));
  connect(ui->actionOriginal_Size, SIGNAL(triggered()), this, SLOT(showOriginalSize()));
  connect(ui->actionZoom_In, SIGNAL(triggered()), this, SLOT(zoomIn()));
  connect(ui->actionZoom_Out, SIGNAL(triggered()), this, SLOT(zoomOut()));

  connect(ui->actionPreferences, SIGNAL(triggered()), this, SLOT(showPreferences()));

  ui->decayView->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

  // separate init from constructor to avoid crash on cancel
  QTimer::singleShot(0, this, SLOT(initialize()));
}

Nuclei::~Nuclei()
{
  QSettings s;

  s.beginGroup("preferences");
  s.setValue("levelTolerance", preferencesDialogUi->levelDiff->value());
  s.setValue("gammaTolerance", preferencesDialogUi->gammaDiff->value());
  s.setValue("fontFamily", preferencesDialogUi->fontFamily->currentFont().family());
  s.setValue("fontSize", preferencesDialogUi->fontSize->value());
  s.endGroup();

  s.setValue("decayFilter", ui->decayFilterLineEdit->text());
  QModelIndex mi = ui->decayTreeView->currentIndex();
  QList<QVariant> selectionIndices;
  while (mi.isValid()) {
    selectionIndices.prepend(mi.row());
    mi = mi.parent();
  }
  s.setValue("decaySelection", selectionIndices);

  delete preferencesDialogUi;
  delete ui;
}

void Nuclei::initialize()
{
  QSettings s;

  restoreGeometry(s.value("geometry").toByteArray());
  restoreState(s.value("windowState").toByteArray());

  QSize wsize = size();
  if (wsize.width() > QApplication::desktop()->availableGeometry().width())
    wsize.setWidth(QApplication::desktop()->availableGeometry().width());
  if (wsize.height() > QApplication::desktop()->availableGeometry().height())
    wsize.setHeight(QApplication::desktop()->availableGeometry().width());
  this->resize(wsize);

  // initialize settings if necessary
  if (!s.contains("exportDir"))
    s.setValue("exportDir", QDir::homePath());

  s.beginGroup("preferences");
  preferencesDialogUi->fontFamily->setCurrentFont(QFont(s.value("fontFamily", QFont().family()).toString()));
  preferencesDialogUi->fontSize->setValue(s.value("fontSize", 14).toInt());
  preferencesDialogUi->levelDiff->setValue(s.value("levelTolerance", 40.0).toDouble());
  preferencesDialogUi->gammaDiff->setValue(s.value("gammaTolerance", 5.0).toDouble());
  preferencesDialogUi->buttonBox->setFocus();
  s.endGroup();

  ENSDFDataSource *ds = new ENSDFDataSource(this);

  connect(preferencesDialogUi->resetCacheButton, SIGNAL(clicked()), ds, SLOT(deleteCache()));
  connect(preferencesDialogUi->resetDatabaseButton, SIGNAL(clicked()), ds, SLOT(deleteDatabaseAndCache()));

  decaySelectionModel = new DecayCascadeItemModel(ds, this);
  decayProxyModel = new DecayCascadeFilterProxyModel(this);
  decayProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
  connect(ui->decayFilterLineEdit, SIGNAL(textChanged(QString)), decayProxyModel, SLOT(setFilterWildcard(QString)));
  decayProxyModel->setSourceModel(decaySelectionModel);
  ui->decayTreeView->setModel(decayProxyModel);
  connect(ui->decayTreeView, SIGNAL(showItem(QModelIndex)), this, SLOT(loadSelectedDecay(QModelIndex)));

  searchResultSelectionModel = new DecayCascadeItemModel(0, this);
  searchProxyModel = new DecayCascadeFilterProxyModel(this);
  searchProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
  searchProxyModel->setSourceModel(searchResultSelectionModel);

  ui->decayFilterLineEdit->setText(s.value("decayFilter", "").toString());
  QList<QVariant> selectionIndices(s.value("decaySelection").toList());
  if (!selectionIndices.isEmpty()) {
    QModelIndex mi = decayProxyModel->index(selectionIndices.at(0).toInt(), 0);
    for (int i=1; i<selectionIndices.size(); i++)
      mi = mi.child(selectionIndices.at(i).toInt(), 0);
    ui->decayTreeView->setCurrentIndex(mi);
    loadSelectedDecay(mi);
  }
}

void Nuclei::loadSelectedDecay(const QModelIndex &index)
{
  if (!index.isValid())
    return;

  if (!decaySelectionModel)
    return;

  DecayScheme decay(decaySelectionModel->decay(decayProxyModel->mapToSource(index)));

//  if (!decay.valid())
//    return;

  loadDecay(decay);
}

void Nuclei::loadSearchResultCascade(const QModelIndex &index)
{
  if (!index.isValid())
    return;

  if (!searchResultSelectionModel)
    return;

  DecayScheme decay(searchResultSelectionModel->decay(searchProxyModel->mapToSource(index)));

//  if (!decay.valid())
//    return;

  loadDecay(decay);
}

void Nuclei::svgExport()
{
  QSettings s;

  if (m_decay.isNull())
    return;

  QString fn(QFileDialog::getSaveFileName(this, "Save As", s.value("exportDir").toString(), "Scalable Vector Graphics (*.svg)"));
  if (fn.isEmpty())
    return;

  QRectF inrect = ui->decayView->scene()->sceneRect();
  QRectF outrect = inrect.adjusted(-10.0, -10.0, 10.0, 10.0);

  QSvgGenerator svgGen;
  svgGen.setFileName(fn);
  svgGen.setSize(outrect.toRect().size());
  svgGen.setViewBox(outrect);
  svgGen.setTitle("Decay Level Scheme for the decay " + m_decay->name());
  svgGen.setDescription(QString::fromUtf8("This scheme was created using Nuclei"));

  m_decay->setShadowEnabled(false);
  QPainter painter(&svgGen);
  ui->decayView->scene()->render(&painter, inrect, inrect);
  m_decay->setShadowEnabled(true);
}

void Nuclei::pdfExport()
{
  QSettings s;

  if (m_decay.isNull())
    return;

  QString fn(QFileDialog::getSaveFileName(this, "Save As", s.value("exportDir").toString(), "Portable Document Format (*.pdf)"));
  if (fn.isEmpty())
    return;

  const int scalefactor = 10;
  const double margin = 3.0;

  QRectF inrect = ui->decayView->scene()->sceneRect();
  QRectF outrect = inrect.adjusted(-margin*scalefactor, -margin*scalefactor, margin*scalefactor, margin*scalefactor);

  QPrinter p(QPrinter::HighResolution);
  p.setOutputFileName(fn);
  p.setPageMargins(margin, margin, margin, margin, QPrinter::Millimeter);
  p.setOutputFormat(QPrinter::PdfFormat);
  p.setPaperSize(outrect.toRect().size() / scalefactor, QPrinter::Millimeter);
  p.setDocName("Decay Level Scheme for the decay " + m_decay->name());
  p.setCreator(QString("%1 %2 (%3)").arg(QCoreApplication::applicationName(), QCoreApplication::applicationVersion(), "NUCLEIURL"));

  m_decay->setShadowEnabled(false);
  QPainter painter(&p);
  ui->decayView->scene()->render(&painter);
  m_decay->setShadowEnabled(true);
}

void Nuclei::showAll()
{
  QGraphicsScene *scene = ui->decayView->scene();
  if (scene)
    ui->decayView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
  return;
}

void Nuclei::showOriginalSize()
{
  ui->decayView->setTransform(QTransform());
}

void Nuclei::zoomIn()
{
  ui->decayView->scale(1.25, 1.25);
}

void Nuclei::zoomOut()
{
  ui->decayView->scale(0.8, 0.8);
}

void Nuclei::showPreferences()
{
  preferencesDialog->exec();
}

void Nuclei::closeEvent(QCloseEvent *event)
{
  QSettings s;
  s.setValue("geometry", saveGeometry());
  s.setValue("windowState", saveState());
  QMainWindow::closeEvent(event);
}

void Nuclei::loadDecay(DecayScheme decay)
{
  QSettings s;
  s.setValue("preferences/levelTolerance", preferencesDialogUi->levelDiff->value());
  s.setValue("preferences/gammaTolerance", preferencesDialogUi->gammaDiff->value());
  s.sync();

  m_decay = QSharedPointer<SchemePlayer>(new SchemePlayer(decay, this));

  connect(m_decay.data(), SIGNAL(selectionChanged()),
          this, SLOT(playerSelectionChanged()));
  m_decay->setStyle(preferencesDialogUi->fontFamily->currentFont(), preferencesDialogUi->fontSize->value());
  QGraphicsScene *scene = m_decay->levelPlot();
  ui->decayView->setScene(scene);
  ui->decayView->setSceneRect(scene->sceneRect().adjusted(-20, -20, 20, 20));

  QString text;
  if (decay.comments.count("comments"))
  {
    text += "<h3>Comments</h3>";
    int refnum = 1;
    for (auto c : decay.comments["comments"])
    {
      auto newtext = c.get<std::string>();
      for (const auto& r : decay.references_)
        if (boost::contains(newtext, r.first))
        {
          boost::replace_all(newtext, r.first,
                             "<a href=\"https://www.nndc.bnl.gov/nsr/knum_act.jsp?ofrml=Normal&keylst="
                             + r.first + "%0D%0A&getkl=Search\">["
                             + std::to_string(refnum++)
                             + "]</a>");
        }
      text += QString::fromStdString(newtext) + "<br>";
    }
  }

  if (decay.comments.count("history"))
  {
    text += "<h3>History</h3>";
    for (auto j : decay.comments["history"])
    {
      for (json::iterator h = j.begin(); h != j.end(); ++h)
        text += "<b>" + QString::fromStdString(h.key()) + ":</b> "
            + QString::fromStdString(h.value().get<std::string>()) + "<br>";
      text += "<br>";
    }
  }

  ui->textBrowser->setOpenLinks(true);
  ui->textBrowser->setOpenExternalLinks(true);
  ui->textBrowser->setHtml(text);

  // update plot
  m_decay->triggerDataUpdate();
  showAll();
}

void Nuclei::playerSelectionChanged()
{
  if (!m_decay)
    return;

  if (m_decay->selected_levels().size())
  {
    auto nrg = *m_decay->selected_levels().begin();
    auto levels = m_decay->scheme().daughterNuclide().levels();
    if (!levels.count(nrg))
      return;
    const auto& level = levels[nrg];
    auto refs = m_decay->scheme().references_;

    QString text;

    text += "<h3>Level comments for "
        + QString::fromStdString(level.energy().to_string())
        + "</h3>";

    int refnum = 1;
    for (auto c : level.comments)
    {
      auto newtext = c;
      for (const auto& r : refs)
        if (boost::contains(newtext, r.first))
        {
          boost::replace_all(newtext, r.first,
                             "<a href=\"https://www.nndc.bnl.gov/nsr/knum_act.jsp?ofrml=Normal&keylst="
                             + r.first + "%0D%0A&getkl=Search\">["
                             + std::to_string(refnum++)
                             + "]</a>");
        }
      text += QString::fromStdString(newtext) + "<br>";
    }

    ui->textBrowser->setOpenLinks(true);
    ui->textBrowser->setOpenExternalLinks(true);
    ui->textBrowser->setHtml(text);
  }
}
