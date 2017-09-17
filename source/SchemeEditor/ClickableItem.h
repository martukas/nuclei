#pragma once

class ActiveGraphicsItemGroup;
class QGraphicsItem;

class ClickableItem
{
public:
  enum Type
  {
    InvalidType,
    ParentNuclideType,
    ParentLevelType,
    DaughterNuclideType,
    DaughterLevelType,
    GammaTransitionType
  };

  ClickableItem() {}
  ClickableItem(Type type);
  virtual Type type() const;
  virtual ActiveGraphicsItemGroup * graphicsItem() const;

protected:
  ActiveGraphicsItemGroup *item {nullptr};
  Type t {InvalidType};
};
