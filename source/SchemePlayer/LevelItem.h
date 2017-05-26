#pragma once

#include "ClickableItem.h"
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
  LevelItem();
  LevelItem(Level level,
            SchemeVisualSettings vis,
            QGraphicsScene *scene);

  //returns feeding arrow height, if any
  double align(double leftlinelength, double rightlinelength,
               double arrowleft, double arrowright,
               SchemeVisualSettings vis);

  Energy energy() const;

  QString energy_text() const;
  QString spin_text() const;

  double above_ypos(double offset);
  void set_ypos(double new_ypos);
  double ypos() const;
  double bottom_ypos() const;
  double nuc_line_width() const;
  double feed_intensity_width() const;

  void set_funky_position(double left, double right, double y);
  void set_funky2_position(double xe, double xspin, double y);

  double max_y_height() const;

private:
  QGraphicsLineItem *line_ {nullptr}; //
  QGraphicsLineItem *feedarrow_ {nullptr};
  QGraphicsPolygonItem *arrowhead_ {nullptr};
  QGraphicsSimpleTextItem *etext_ {nullptr}; //
  QGraphicsSimpleTextItem *spintext_ {nullptr}; //
  QGraphicsSimpleTextItem *hltext_ {nullptr};
  QGraphicsTextItem *feedintens_ {nullptr}; //
  QGraphicsRectItem *clickarea_ {nullptr};
  GraphicsHighlightItem *highlighthelper_ {nullptr};
  double ypos_ {0.0}; //

  Energy energy_;
};
