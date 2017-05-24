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
  QGraphicsScene * levelPlot();

  void setShadowEnabled(bool enable);

  const DecayScheme& scheme() const;

  QString name() const;

  void triggerDataUpdate();

  std::set<Energy> selected_levels() const;
  std::set<Energy> selected_parent_levels() const;
  std::set<Energy> selected_transistions() const;

signals:
  void enabledShadow(bool enable);
  void selectionChanged();

private slots:
  void itemClicked(ClickableItem *item);

private:
  DecayScheme scheme_;

  void clickedGamma(TransitionItem *g);
  void clickedEnergyLevel(LevelItem *e);
  void alignGraphicsItems();


  QGraphicsScene *scene_ {nullptr};

  SchemeVisualSettings visual_settings_;

  NuclideItem parent_, daughter_;
  std::map<Energy, LevelItem*> levels_;
  std::map<Energy, LevelItem*> parent_levels_;
  std::map<Energy, TransitionItem*> transitions_;

  std::set<Energy> selected_levels_;
  std::set<Energy> selected_parent_levels_;
  std::set<Energy> selected_transitions_;

  void addLevel(Level level, SchemeVisualSettings vis);
  void addParentLevel(Level level, SchemeVisualSettings vis);
  void addTransition(Transition transition, SchemeVisualSettings vis);

  void deselect_all();
};
