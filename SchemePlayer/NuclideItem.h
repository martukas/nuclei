#ifndef NUCLIDE_RENDERED_H
#define NUCLIDE_RENDERED_H

#include <QString>
#include <QMap>
#include <QFont>
#include <QPair>
#include "ClickableItem.h"
#include "Nuclide.h"

class QGraphicsItem;
class QGraphicsItemGroup;
class QGraphicsScene;
class QGraphicsLineItem;
class QGraphicsSimpleTextItem;

class NuclideItem : public ClickableItem
{
public:
  NuclideItem();
  NuclideItem(const Nuclide& nuc, Type tp, SchemeVisualSettings vis, QGraphicsScene *scene);

  QGraphicsSimpleTextItem *pNucHl;
  QGraphicsLineItem *pNucVerticalArrow;

  NuclideId id_;

};

#endif // NUCLIDE_H
