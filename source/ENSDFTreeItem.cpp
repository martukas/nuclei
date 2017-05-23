#include "ENSDFTreeItem.h"

#include <QVariant>

ENSDFTreeItem::ENSDFTreeItem(ItemType type, ENSDFTreeItem *parent)
  : parentItem(parent), m_isSelectable(false), m_type(type)
{
  itemData.append(QVariant());
  setParent(parent);
}

ENSDFTreeItem::ENSDFTreeItem(ItemType type,
                             NuclideId id,
                             const QList<QVariant> &data,
                             bool selectable,
                             ENSDFTreeItem *parent)
  : nid(id), itemData(data),
    parentItem(parent), m_isSelectable(selectable),
    m_type(type)
{
  setParent(parent);
}

ENSDFTreeItem::ENSDFTreeItem(const ENSDFTreeItem &original)
  : nid(original.nid), itemData(original.itemData), m_isSelectable(original.m_isSelectable), m_type(original.m_type)
{
}

ENSDFTreeItem::~ENSDFTreeItem()
{
  qDeleteAll(childItems);
}

ENSDFTreeItem *ENSDFTreeItem::parent() const
{
  return parentItem;
}

ENSDFTreeItem *ENSDFTreeItem::child(int row)
{
  return childItems.value(row);
}

int ENSDFTreeItem::childCount() const
{
  return childItems.count();
}

int ENSDFTreeItem::columnCount() const
{
  return itemData.count();
}

bool ENSDFTreeItem::hasParent() const
{
  return parentItem;
}

void ENSDFTreeItem::setParent(ENSDFTreeItem *parent)
{
  parentItem = parent;
  if (parent)
    parent->childItems.append(this);
}

void ENSDFTreeItem::setItemData(const QList<QVariant> &data)
{
  itemData = data;
}

void ENSDFTreeItem::setSelectable(bool selectable)
{
  m_isSelectable = selectable;
}

bool ENSDFTreeItem::isSelectable() const
{
  return m_isSelectable;
}

QVariant ENSDFTreeItem::data(int column) const
{
  return itemData.value(column);
}

int ENSDFTreeItem::row() const
{
  if (parentItem)
    return parentItem->childItems.indexOf(const_cast<ENSDFTreeItem*>(this));
  return 0;
}

ENSDFTreeItem::ItemType ENSDFTreeItem::type() const
{
  return m_type;
}

QDataStream & operator <<(QDataStream &out, const ENSDFTreeItem &treeitem)
{
  out << treeitem.itemData;
  out << treeitem.nid.A();
  out << treeitem.nid.Z();
  out << treeitem.m_isSelectable;
  out << int(treeitem.m_type);
  out << quint32(treeitem.childItems.size());
  foreach (const ENSDFTreeItem *it, treeitem.childItems)
  {
    const ENSDFTreeItem *eit = dynamic_cast<const ENSDFTreeItem*>(it);
    if (eit)
      out << (*eit);
  }
  return out;
}


QDataStream & operator >>(QDataStream &in, ENSDFTreeItem &treeitem)
{
  uint16_t a, z;
  in >> treeitem.itemData;
  in >> a;
  in >> z;
  treeitem.nid = NuclideId::fromAZ(a, z);
  in >> treeitem.m_isSelectable;
  int type;
  in >> type;
  treeitem.m_type = ENSDFTreeItem::ItemType(type);
  quint32 numchildren;
  in >> numchildren;
  for (quint32 i=0; i<numchildren; i++)
  {
    ENSDFTreeItem *childitem = new ENSDFTreeItem(ENSDFTreeItem::UnknownType, &treeitem);
    in >> (*childitem);
  }
  return in;
}

