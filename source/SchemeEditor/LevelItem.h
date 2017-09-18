#pragma once

#include "ClickableItem.h"
#include "SchemeVisualSettings.h"
#include "Level.h"

class ActiveGraphicsItemGroup;
class QGraphicsLineItem;
class QGraphicsSimpleTextItem;
class QGraphicsTextItem;
class QGraphicsPolygonItem;
class GraphicsHighlightItem;
class QGraphicsRectItem;
class QGraphicsScene;
class QGraphicsItemGroup;

class LevelItem : public ClickableItem
{
public:
  LevelItem() {}
  LevelItem(Level level, Type type, ParentPosition parentpos,
            SchemeVisualSettings vis,
            QGraphicsScene *scene);

  //returns feeding arrow height, if any
  double align(double leftlinelength, double rightlinelength,
               double arrowleft, double arrowright,
               ParentPosition parentpos,
               SchemeVisualSettings vis);

  Energy energy() const;

  int energy_width() const;
  int spin_width() const;

  double above_ypos(double offset);
  void set_ypos(double new_ypos);
  double ypos() const;
  double bottom_ypos() const;
  double nuc_line_width() const;
  double feed_intensity_width() const;

  void set_funky_position(double left, double right, double y, SchemeVisualSettings vis);
  void set_funky2_position(double xe, double xspin, double y);

  double max_y_height() const;

private:
  QGraphicsLineItem *line_ {nullptr};
  QGraphicsLineItem *feedarrow_ {nullptr};
  QGraphicsPolygonItem *arrowhead_ {nullptr};
  QGraphicsSimpleTextItem *etext_ {nullptr};
  QGraphicsSimpleTextItem *spintext_ {nullptr};
  QGraphicsSimpleTextItem *hltext_ {nullptr};
  QGraphicsTextItem *feedintens_ {nullptr};
  QGraphicsRectItem *click_area_ {nullptr};
  GraphicsHighlightItem *highlight_helper_ {nullptr};
  double ypos_ {0.0};

  Energy energy_;
};
