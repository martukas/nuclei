#pragma once

#include <QObject>
#include <QStringList>
#include <QSharedPointer>
#include <QMetaType>
#include <QDataStream>

#include <NucData/DecayScheme.h>

class ENSDFTreeItem
{
public:
  enum ItemType
  {
    UnknownType,
    RootType,
    DaughterType,
    DecayType,
    CascadeType
  };

  explicit ENSDFTreeItem(ItemType type = UnknownType,
                         ENSDFTreeItem *parent = 0);
  explicit ENSDFTreeItem(ItemType type,
                         NuclideId id,
                         const QList<QVariant> &data,
                         bool selectable,
                         ENSDFTreeItem *parent = 0);
  /**
     * @brief This copy constructor creates a standalone copy without links to parent or children
     * @param original
     */
  explicit ENSDFTreeItem(const ENSDFTreeItem &original);
  virtual ~ENSDFTreeItem();

  ENSDFTreeItem *parent() const;
  ENSDFTreeItem *child(int row);

  int childCount() const;
  int columnCount() const;
  bool hasParent() const;

  void setParent(ENSDFTreeItem *parent);
  void setItemData(const QList<QVariant> &data);

  void setSelectable(bool selectable);
  bool isSelectable() const;

  QVariant data(int column) const;

  int row() const;

  NuclideId id() const { return nid; }
  ItemType type() const;

  friend QDataStream & operator<<(QDataStream &out, const ENSDFTreeItem &treeitem);
  friend QDataStream & operator>>(QDataStream &in, ENSDFTreeItem &treeitem);

protected:
  NuclideId nid;
  QList<ENSDFTreeItem*> childItems;
  QList<QVariant> itemData;
  ENSDFTreeItem *parentItem;
  bool m_isSelectable;
  ItemType m_type;
};

Q_DECLARE_METATYPE(ENSDFTreeItem)

