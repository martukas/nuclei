#pragma once

#include <QObject>
#include <QMap>
#include <QFont>
#include <QMetaType>
#include "DecayScheme.h"

#include "NuclideItem.h"

#include "ClickableItem.h"

class QGraphicsScene;
class LevelItem;
class TransitionItem;

class SchemePlayer : public QObject
{
  Q_OBJECT
public:

  explicit SchemePlayer(DecayScheme scheme, QObject *parent = 0);

  void setStyle(const QFont &fontfamily, unsigned int sizePx);
  QGraphicsScene* levelPlot();

  void setShadowEnabled(bool enable);

  const DecayScheme& scheme() const;

  QString name() const;

  void triggerDataUpdate();

  void clearSelection();

  std::set<Energy> selected_levels() const;
  std::set<Energy> selected_parent_levels() const;
  std::set<Energy> selected_transistions() const;

  bool parent_selected() const;
  bool daughter_selected() const;

signals:
  void enabledShadow(bool enable);
  void selectionChanged();

private slots:
  void itemClicked(ClickableItem *item);

private:
  DecayScheme scheme_;

  QGraphicsScene *scene_ {nullptr};

  SchemeVisualSettings visual_settings_;

  NuclideItem* parent_ {nullptr};
  NuclideItem* daughter_ {nullptr};
  std::map<Energy, LevelItem*> levels_;
  std::map<Energy, LevelItem*> parent_levels_;
  std::list<TransitionItem*> transitions_;

  std::set<Energy> selected_levels_;
  std::set<Energy> selected_parent_levels_;
  std::set<Energy> selected_transitions_;
  bool parent_selected_ {false};
  bool daughter_selected_ {false};

  void alignGraphicsItems();

  void addLevel(Level level, SchemeVisualSettings vis);
  void addParentLevel(Level level, SchemeVisualSettings vis);
  void addTransition(Transition transition, SchemeVisualSettings vis);

  void clickedGamma(TransitionItem *g);
  void clickedEnergyLevel(LevelItem *e);
  void clickedParent();
  void clickedDaughter();

  void deselect_all();
};
