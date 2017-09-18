#include "SchemeVisualSettings.h"

#include "custom_logger.h"

SchemeVisualSettings::SchemeVisualSettings()
{
  // prepare pens
  levelPen.setWidthF(1.0);
  levelPen.setCapStyle(Qt::FlatCap);
  stableLevelPen.setWidthF(2.0);
  stableLevelPen.setCapStyle(Qt::FlatCap);
  feedArrowPen.setWidthF(1.0);
  feedArrowPen.setCapStyle(Qt::FlatCap);
  gammaPen.setWidthF(1.0);
  gammaPen.setCapStyle(Qt::FlatCap);
}

SchemeVisualSettings SchemeVisualSettings::load(QSettings& s)
{
  SchemeVisualSettings ret;

  s.beginGroup("SchemeVisualSettings");
  QFont font = QFont(s.value("fontFamily", QFont().family()).toString());
  int fontsize = s.value("fontSize", 14).toInt();
  ret.setStyle(font, fontsize);

  ret.inactive_color_ = s.value("inactiveColor", QColor(Qt::black).name(QColor::HexArgb)).toString();
  ret.selected_color_ = s.value("activeColor1", QColor(224, 186, 100, 180).name(QColor::HexArgb)).toString();
  ret.implicated_color_ = s.value("activeColor2", QColor(64, 166, 255, 180).name(QColor::HexArgb)).toString();
  ret.hover_color_ = s.value("hoverColor", QColor(178, 223, 150, 180).name(QColor::HexArgb)).toString();
  ret.nuclide_color_ = s.value("nuclideColor", QColor(64, 166, 255).name(QColor::HexArgb)).toString();
  s.endGroup();

  return ret;
}

void SchemeVisualSettings::save(QSettings& s)
{
  s.beginGroup("SchemeVisualSettings");
  s.setValue("fontFamily", font_.family());
  s.setValue("fontSize", font_.pixelSize());
  s.setValue("inactiveColor", inactive_color_.name(QColor::HexArgb));
  s.setValue("activeColor1", selected_color_.name(QColor::HexArgb));
  s.setValue("activeColor2", implicated_color_.name(QColor::HexArgb));
  s.setValue("hoverColor", hover_color_.name(QColor::HexArgb));
  s.setValue("nuclideColor", nuclide_color_.name(QColor::HexArgb));
  s.endGroup();
}

void SchemeVisualSettings::setStyle(const QFont &fontfamily,
                                    int sizePx)
{
  font_ = fontfamily;
  font_.setPixelSize(sizePx);
}

QColor SchemeVisualSettings::inactive_color() const
{
  return inactive_color_;
}

QColor SchemeVisualSettings::selected_color() const
{
  return selected_color_;
}

QColor SchemeVisualSettings::implicated_color() const
{
  return implicated_color_;
}

QColor SchemeVisualSettings::hover_color() const
{
  return hover_color_;
}

QColor SchemeVisualSettings::nuclide_color() const
{
  return nuclide_color_;
}

void SchemeVisualSettings::set_inactive_color(QColor c)
{
  inactive_color_ = c;
}

void SchemeVisualSettings::set_selected_color(QColor c)
{
  selected_color_ = c;
}

void SchemeVisualSettings::set_implicated_color(QColor c)
{
  implicated_color_ = c;
}

void SchemeVisualSettings::set_hover_color(QColor c)
{
  hover_color_ = c;
}

void SchemeVisualSettings::set_nuclide_color(QColor c)
{
  nuclide_color_ = c;
}


QFont SchemeVisualSettings::font() const
{
  return font_;
}

int SchemeVisualSettings::font_size() const
{
  return font_.pixelSize();
}

QFont SchemeVisualSettings::stdFont() const
{
  return font_;
}

QFont SchemeVisualSettings::gammaFont() const
{
  return font_;
}

QFont SchemeVisualSettings::stdBoldFont() const
{
  auto ret = font_;
  ret.setBold(true);
  return ret;
}

QFont SchemeVisualSettings::nucFont() const
{
  auto ret = font_;
  ret.setPixelSize(font_.pixelSize() * 2);
  ret.setBold(true);
  return ret;
}

QFont SchemeVisualSettings::nucIndexFont() const
{
  auto ret = font_;
  ret.setPixelSize(font_.pixelSize() * 1.2);
  ret.setBold(true);
  return ret;
}

QFont SchemeVisualSettings::parentHlFont() const
{
  auto ret = font_;
  ret.setPixelSize(font_.pixelSize() * 1.2);
  return ret;
}

QFont SchemeVisualSettings::feedIntensityFont() const
{
  auto ret = font_;
  ret.setItalic(true);
  return ret;
}
