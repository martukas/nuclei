#pragma once

#include <QFont>
#include <QPen>
#include <QSettings>

enum ParentPosition
{
  NoParent,
  LeftParent,
  RightParent
};

struct SchemeVisualSettings
{
    SchemeVisualSettings();
    static SchemeVisualSettings load(QSettings &s);
    void save(QSettings& s);

    void setStyle(const QFont&fontfamily, int sizePx);

    QFont font() const;
    int font_size() const;

    QColor inactive_color() const;
    QColor selected_color() const;
    QColor implicated_color() const;
    QColor hover_color() const;
    QColor nuclide_color() const;

    void set_inactive_color(QColor);
    void set_selected_color(QColor);
    void set_implicated_color(QColor);
    void set_hover_color(QColor);
    void set_nuclide_color(QColor);

    // style
    QFont stdFont() const;
    QFont stdBoldFont() const;
    QFont nucFont() const;
    QFont nucIndexFont() const;
    QFont parentHlFont() const;
    QFont feedIntensityFont() const;
    QFont gammaFont() const;

    QPen levelPen, stableLevelPen, feedArrowPen, gammaPen;

    double outerGammaMargin {50.0}; // margin between level texts (spin, energy) and gammas
    double outerLevelTextMargin {4.0}; // level lines extend beyond the beginning/end of the level texts by this value
    double maxExtraLevelDistance {120.0}; // maximal additional distance between two level lines
    double levelToHalfLifeDistance {10.0}; // distance between level line and half life text
    double parentNuclideLevelLineLength {110.0};
    double parentNuclideLevelLineExtraLength {15.5}; // additional length of the decaying level
    double parentNuclideMinSpinEnergyDistance {15.0}; // Minumal distance between spin and energy texts on parent level lines
    double feedingArrowLineLength {110.0}; // initial (minimal) line length of feeding arrows
    double feedingArrowHeadLength {11.0};
    double feedingArrowHeadWidth {5.0};
    double feedingArrowGap {5.0};
    double feedingArrowTextMargin {4.0}; // margin around the feeding intensity text
    double parentNuclideToEnergyLevelsDistance {30.0};
    double highlightWidth {5.0};

//    ParentPosition parentpos {NoParent};

  private:

    QFont font_;

    QColor inactive_color_ {Qt::black};
    QColor selected_color_ {QColor(224, 186, 100, 180)};
    QColor implicated_color_ {QColor(64, 166, 255, 180)};
//    QColor hover_color_ {QColor(64, 166, 255, 100)};
    QColor hover_color_ {QColor(178, 223, 150, 180)};
    QColor nuclide_color_ {QColor(64, 166, 255)};
};
