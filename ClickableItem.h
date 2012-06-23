#ifndef CLICKABLEITEM_H
#define CLICKABLEITEM_H

class ActiveGraphicsItemGroup;
class QGraphicsItem;

class ClickableItem
{
public:
    enum Type {
        EnergyLevelType,
        GammaTransitionType
    };

    ClickableItem(Type type);
    virtual Type type() const;
    virtual ActiveGraphicsItemGroup * graphicsItem() const;

protected:
    ActiveGraphicsItemGroup *item;

private:
    Type t;
};

#endif // CLICKABLEITEM_H
