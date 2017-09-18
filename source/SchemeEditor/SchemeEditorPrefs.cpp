#include "SchemeEditorPrefs.h"
#include "ui_SchemeEditorPrefs.h"

#include <QSettings>

#include "custom_logger.h"

SchemeEditorPrefs::SchemeEditorPrefs(SchemeVisualSettings prefs, QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::SchemeEditorPrefs)
  , prefs_ (prefs)
{
  ui->setupUi(this);

  ui->colorInactive->setDisplayMode(color_widgets::ColorPreview::AllAlpha);
  ui->colorHover->setDisplayMode(color_widgets::ColorPreview::AllAlpha);
  ui->colorActive1->setDisplayMode(color_widgets::ColorPreview::AllAlpha);
  ui->colorActive2->setDisplayMode(color_widgets::ColorPreview::AllAlpha);
  ui->colorNuclide->setDisplayMode(color_widgets::ColorPreview::AllAlpha);

  ui->fontFamily->setCurrentFont(prefs_.font().toString());
  ui->fontSize->setValue(prefs_.font_size());  
  ui->colorInactive->setColor(prefs_.inactive_color());
  ui->colorHover->setColor(prefs_.hover_color());
  ui->colorActive1->setColor(prefs_.selected_color());
  ui->colorActive2->setColor(prefs_.implicated_color());
  ui->colorNuclide->setColor(prefs_.nuclide_color());

  connect(ui->colorInactive, SIGNAL(colorChanged(QColor)), this, SLOT(colChanged(QColor)));
  connect(ui->colorHover, SIGNAL(colorChanged(QColor)), this, SLOT(colChanged(QColor)));
  connect(ui->colorActive1, SIGNAL(colorChanged(QColor)), this, SLOT(colChanged(QColor)));
  connect(ui->colorActive2, SIGNAL(colorChanged(QColor)), this, SLOT(colChanged(QColor)));
  connect(ui->colorNuclide, SIGNAL(colorChanged(QColor)), this, SLOT(colChanged(QColor)));
}

SchemeEditorPrefs::~SchemeEditorPrefs()
{
  delete ui;
}

SchemeVisualSettings SchemeEditorPrefs::prefs() const
{
  return prefs_;
}

void SchemeEditorPrefs::on_fontFamily_activated(const QString &arg1)
{
  prefs_.setStyle(ui->fontFamily->currentFont().family(),
                  ui->fontSize->value());
}

void SchemeEditorPrefs::on_fontSize_editingFinished()
{
  prefs_.setStyle(ui->fontFamily->currentFont().family(),
                  ui->fontSize->value());
}

void SchemeEditorPrefs::colChanged(QColor)
{
  prefs_.set_inactive_color(ui->colorInactive->color());
  prefs_.set_hover_color(ui->colorHover->color());
  prefs_.set_selected_color(ui->colorActive1->color());
  prefs_.set_implicated_color(ui->colorActive2->color());
  prefs_.set_nuclide_color(ui->colorNuclide->color());
}

