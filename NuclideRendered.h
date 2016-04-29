#ifndef NUCLIDE_RENDERED_H
#define NUCLIDE_RENDERED_H

#include <QString>
#include <QMap>
#include <QFont>
#include <QPair>
#include "ClickableItem.h"
#include "XNuclide.h"

class QGraphicsItem;
class QGraphicsItemGroup;
class QGraphicsScene;
class QGraphicsLineItem;
class QGraphicsSimpleTextItem;

class NuclideRendered : public ClickableItem
{
public:
  NuclideRendered();
  NuclideRendered(XNuclidePtr nuc, Type tp, SchemeVisualSettings vis, QGraphicsScene *scene);

  QGraphicsSimpleTextItem *pNucHl;
  QGraphicsLineItem *pNucVerticalArrow;

  NuclideId id_;

};

#endif // NUCLIDE_H
