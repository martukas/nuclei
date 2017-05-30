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

#include "ScrollZoomView.h"

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

  connect(ui->actionPreferences, SIGNAL(triggered()), this, SLOT(showPreferences()));

  ui->decayView->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

  ui->decayView->setInteractive(true);
  ui->decayView->setDragMode(QGraphicsView::ScrollHandDrag);
  ui->decayView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui->decayView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  Graphics_view_zoom* z = new Graphics_view_zoom(ui->decayView);
  z->set_modifiers(Qt::NoModifier);

  ui->textBrowser->setOpenLinks(true);
  ui->textBrowser->setOpenExternalLinks(true);

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
  ui->splitter->restoreState(s.value("splitter").toByteArray());

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

  if (decay_viewer_.isNull())
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
  svgGen.setTitle("Decay Level Scheme for the decay " + decay_viewer_->name());
  svgGen.setDescription(QString::fromUtf8("This scheme was created using Nuclei"));

  decay_viewer_->setShadowEnabled(false);
  QPainter painter(&svgGen);
  ui->decayView->scene()->render(&painter, inrect, inrect);
  decay_viewer_->setShadowEnabled(true);
}

void Nuclei::pdfExport()
{
  QSettings s;

  if (decay_viewer_.isNull())
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
  p.setDocName("Decay Level Scheme for the decay " + decay_viewer_->name());
  p.setCreator(QString("%1 %2 (%3)").arg(QCoreApplication::applicationName(), QCoreApplication::applicationVersion(), "NUCLEIURL"));

  decay_viewer_->setShadowEnabled(false);
  QPainter painter(&p);
  ui->decayView->scene()->render(&painter);
  decay_viewer_->setShadowEnabled(true);
}

void Nuclei::showAll()
{
  QGraphicsScene *scene = ui->decayView->scene();
  if (scene)
    ui->decayView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
  return;
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
  s.setValue("splitter", ui->splitter->saveState());
  QMainWindow::closeEvent(event);
}

void Nuclei::loadDecay(DecayScheme decay)
{
  QSettings s;
  s.setValue("preferences/levelTolerance", preferencesDialogUi->levelDiff->value());
  s.setValue("preferences/gammaTolerance", preferencesDialogUi->gammaDiff->value());
  s.sync();

  current_scheme_ = decay;
  decay_viewer_ = new SchemePlayer(decay, this);

  connect(decay_viewer_.data(), SIGNAL(selectionChanged()),
          this, SLOT(playerSelectionChanged()));
  decay_viewer_->setStyle(preferencesDialogUi->fontFamily->currentFont(), preferencesDialogUi->fontSize->value());
  QGraphicsScene *scene = decay_viewer_->levelPlot();
  ui->decayView->setScene(scene);
  ui->decayView->setSceneRect(scene->sceneRect().adjusted(-20, -20, 20, 20));

  // update plot
  decay_viewer_->triggerDataUpdate();
  showAll();
  playerSelectionChanged();
}

void Nuclei::playerSelectionChanged()
{
  if (!decay_viewer_)
    return;

  json comments;
  if (decay_viewer_->selected_levels().size())
  {
    auto nrg = *decay_viewer_->selected_levels().begin();
    auto levels = current_scheme_.daughterNuclide().levels();
    if (levels.count(nrg))
      comments = levels[nrg].text();
  }
  else if (decay_viewer_->selected_parent_levels().size())
  {
    auto nrg = *decay_viewer_->selected_parent_levels().begin();
    auto levels = current_scheme_.parentNuclide().levels();
    if (levels.count(nrg))
      comments = levels[nrg].text();
  }
  else if (decay_viewer_->selected_transistions().size())
  {
    auto nrg = *decay_viewer_->selected_transistions().begin();
    auto transitions = current_scheme_.daughterNuclide().transitions();
    if (!transitions.count(nrg))
      return;
    comments = transitions[nrg].text();
  }
  else if (decay_viewer_->parent_selected())
    comments = current_scheme_.parentNuclide().text();
  else if (decay_viewer_->daughter_selected())
    comments = current_scheme_.daughterNuclide().text();
  else
    comments = current_scheme_.text();

  set_text(comments);
}

void Nuclei::set_text(const json& jj)
{
  auto refs = current_scheme_.references();
  QString text;
  for (const auto& j : jj)
  {
    text += "<h3>" + QString::fromStdString(j["heading"]) + "</h3>";
    for (const auto& p : j["pars"])
      text += prep_comments(p, refs) + "<br>";
  }
  ui->textBrowser->setHtml(text);
}

QString Nuclei::prep_comments(const json& j,
                              const std::set<std::string>& refs)
{
  QString text;

  int refnum = 1;
  for (auto c : j)
  {
    auto newtext = c.get<std::string>();
    for (const auto& r : refs)
      if (boost::contains(newtext, r))
      {
        boost::replace_all(newtext, r,
                           make_reference_link(r, refnum++));
      }
    text += QString::fromStdString(newtext);
  }

  return text;
}


std::string Nuclei::make_reference_link(std::string ref, int num)
{
  return "<a href=\"https://www.nndc.bnl.gov/nsr/knum_act.jsp?ofrml=Normal&keylst="
      + ref + "%0D%0A&getkl=Search\"><small>"
      + std::to_string(num++)
      + "</small></a>";
}

