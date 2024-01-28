#include "SchemeEditor.h"
#include "ui_SchemeEditor.h"

#include "SchemeEditorPrefs.h"

#include <QSettings>
#include <QSvgGenerator>
#include <QFileDialog>
#include <QPrinter>
#include <boost/algorithm/string.hpp>
#include "ScrollZoomView.h"

#include <qpagelayout.h>
#include <qpagesize.h>
#include <util/logger.h>

SchemeEditor::SchemeEditor(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::SchemeEditor)
{
  ui->setupUi(this);

  ui->decayView->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
  ui->decayView->setInteractive(true);
  ui->decayView->setDragMode(QGraphicsView::ScrollHandDrag);
  ui->decayView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui->decayView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  Graphics_view_zoom* z = new Graphics_view_zoom(ui->decayView);
  z->set_modifiers(Qt::NoModifier);

  ui->textBrowser->setOpenLinks(true);
  ui->textBrowser->setOpenExternalLinks(true);

  QSettings s;
  s.beginGroup("SchemeEditor");
  ui->splitter->restoreState(s.value("splitter").toByteArray());
  ui->checkFilterTransitions->setChecked(s.value("filterTransitions", false).toBool());
}

SchemeEditor::~SchemeEditor()
{
  QSettings s;
  s.beginGroup("SchemeEditor");
  s.setValue("splitter", ui->splitter->saveState());
  s.setValue("filterTransitions", ui->checkFilterTransitions->isChecked());
  delete ui;
}

void SchemeEditor::on_pushShowAll_clicked()
{
  QGraphicsScene *scene = ui->decayView->scene();
  if (scene)
    ui->decayView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void SchemeEditor::on_pushExportSvg_clicked()
{
  if (decay_viewer_.isNull())
    return;

  QSettings s;
  s.beginGroup("SchemeEditor");

  QString fn(QFileDialog::getSaveFileName(this, "Save As", s.value("exportDir", QDir::homePath()).toString(), "Scalable Vector Graphics (*.svg)"));
  if (fn.isEmpty())
    return;

  QRectF inrect = ui->decayView->scene()->sceneRect();
  QRectF outrect = inrect.adjusted(-10.0, -10.0, 10.0, 10.0);

  QSvgGenerator svgGen;
  svgGen.setFileName(fn);
  svgGen.setSize(outrect.toRect().size());
  svgGen.setViewBox(outrect);
  svgGen.setTitle("Decay Level Scheme for the decay " + decay_viewer_->name());
  svgGen.setDescription(QString::fromUtf8("This scheme was created using SchemeEditor"));

  decay_viewer_->setShadowEnabled(false);
  QPainter painter(&svgGen);
  ui->decayView->scene()->render(&painter, inrect, inrect);
  decay_viewer_->setShadowEnabled(true);
}

void SchemeEditor::on_pushExportPdf_clicked()
{
  if (decay_viewer_.isNull())
    return;

  QSettings s;
  s.beginGroup("SchemeEditor");

  QString fn(QFileDialog::getSaveFileName(this, "Save As", s.value("exportDir", QDir::homePath()).toString(), "Portable Document Format (*.pdf)"));
  if (fn.isEmpty())
    return;

  const int scalefactor = 10;
  const double margin = 3.0;

  QRectF inrect = ui->decayView->scene()->sceneRect();
  QRectF outrect = inrect.adjusted(-margin*scalefactor, -margin*scalefactor, margin*scalefactor, margin*scalefactor);

  QPrinter p(QPrinter::HighResolution);
  p.setOutputFileName(fn);
  p.setPageMargins(QMarginsF(margin, margin, margin, margin), QPageLayout::Millimeter);
  p.setOutputFormat(QPrinter::PdfFormat);
  p.setPageSize(QPageSize(QSizeF(outrect.width() / scalefactor, outrect.height() / scalefactor), QPageSize::Millimeter));
  p.setDocName("Decay Level Scheme for the decay " + decay_viewer_->name());
  p.setCreator(QString("%1 %2 (%3)").arg(QCoreApplication::applicationName(), QCoreApplication::applicationVersion(), "SchemeEditorURL"));

  decay_viewer_->setShadowEnabled(false);
  QPainter painter(&p);
  ui->decayView->scene()->render(&painter);
  decay_viewer_->setShadowEnabled(true);
}

void SchemeEditor::loadDecay(DecayScheme decay)
{
  current_scheme_ = decay;
  refresh_scheme();
}

void SchemeEditor::refresh_scheme()
{
  QSettings s;
  auto prefs = SchemeVisualSettings::load(s);

  decay_viewer_ = new SchemeGraphics(current_scheme_,
                                   ui->doubleMinIntensity->value(), this);

  connect(decay_viewer_.data(), SIGNAL(selectionChanged()),
          this, SLOT(playerSelectionChanged()));
  decay_viewer_->setStyle(prefs);
  decay_viewer_->set_highlight_cascade(ui->checkFilterTransitions->isChecked());
  QGraphicsScene *scene = decay_viewer_->levelPlot();
  ui->decayView->setScene(scene);
  ui->decayView->setSceneRect(scene->sceneRect().adjusted(-20, -20, 20, 20));

  // update plot
  decay_viewer_->triggerDataUpdate();
  on_pushShowAll_clicked();
  playerSelectionChanged();
}

void SchemeEditor::playerSelectionChanged()
{
  if (!decay_viewer_)
    return;

  json comments;
  if (decay_viewer_->selected_levels(1).size())
  {
    auto nrg = *decay_viewer_->selected_levels(1).begin();
    auto levels = current_scheme_.daughterNuclide().levels();
    if (levels.count(nrg))
      comments = levels[nrg].text();
  }
  else if (decay_viewer_->selected_feedings(1).size())
  {
    auto nrg = *decay_viewer_->selected_feedings(1).begin();
    auto levels = current_scheme_.parentNuclide().levels();
    if (levels.count(nrg))
      comments = levels[nrg].text(); //should be something else
  }
  else if (decay_viewer_->selected_parent_levels(1).size())
  {
    auto nrg = *decay_viewer_->selected_parent_levels(1).begin();
    auto levels = current_scheme_.parentNuclide().levels();
    if (levels.count(nrg))
      comments = levels[nrg].text();
  }
  else if (decay_viewer_->selected_transistions(1).size())
  {
    auto nrg = *decay_viewer_->selected_transistions(1).begin();
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

void SchemeEditor::set_text(const json& jj)
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

QString SchemeEditor::prep_comments(const json& j,
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


std::string SchemeEditor::make_reference_link(std::string ref, int num)
{
  return "<a href=\"https://www.nndc.bnl.gov/nsr/knum_act.jsp?ofrml=Normal&keylst="
      + ref + "%0D%0A&getkl=Search\"><small>"
      + std::to_string(num)
      + "</small></a>";
}

void SchemeEditor::on_checkFilterTransitions_clicked()
{
  if (decay_viewer_)
    decay_viewer_->set_highlight_cascade(ui->checkFilterTransitions->isChecked());
}

void SchemeEditor::on_doubleTargetTransition_editingFinished()
{
  auto tr = current_scheme_.daughterNuclide().nearest_transition(ui->doubleTargetTransition->value());
  if (tr.energy().value().hasFiniteValue() && decay_viewer_)
  {
    decay_viewer_->clearSelection();
    decay_viewer_->select_transistions({tr.energy()}, 1);
    playerSelectionChanged();
  }
}

void SchemeEditor::on_doubleMinIntensity_editingFinished()
{
  refresh_scheme();
}

void SchemeEditor::on_pushPrefs_clicked()
{
  QSettings s;
  auto prefs = SchemeVisualSettings::load(s);

  auto prefsDalog = new SchemeEditorPrefs(prefs, this);
  if (prefsDalog->exec() == QDialog::Accepted)
  {
    prefsDalog->prefs().save(s);
    refresh_scheme();
  }
}
