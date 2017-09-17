#include "ClickableItem.h"
#include "ActiveGraphicsItemGroup.h"

ClickableItem::ClickableItem(Type type)
  :t(type) {}

ClickableItem::Type ClickableItem::type() const
{
  return t;
}

ActiveGraphicsItemGroup *ClickableItem::graphicsItem() const
{
  return item;
}
