#pragma once

#include <QObject>
#include <QMap>
#include <QFont>
#include <QMetaType>
#include "DecayScheme.h"

#include "NuclideItem.h"

#include "ClickableItem.h"

#include "GraphicsScene.h"

class LevelItem;
class TransitionItem;

class SchemePlayer : public QObject
{
  Q_OBJECT
public:

  explicit SchemePlayer(DecayScheme scheme,
                        double min_intensity,
                        QObject *parent = 0);

  void setStyle(const QFont &fontfamily, unsigned int sizePx);
  GraphicsScene* levelPlot();

  void setShadowEnabled(bool enable);

  const DecayScheme& scheme() const;

  QString name() const;

  void triggerDataUpdate();

  void clearSelection();

  std::set<Energy> selected_levels(int level) const;
  std::set<Energy> selected_parent_levels(int level) const;
  std::set<Energy> selected_transistions(int level) const;

  void select_levels(const std::set<Energy>&, int level=1);
  void select_parent_levels(const std::set<Energy>&, int level=1);
  void select_transistions(const std::set<Energy>&, int level=1);

  bool parent_selected() const;
  bool daughter_selected() const;

  void set_highlight_cascade(bool);

signals:
  void enabledShadow(bool enable);
  void selectionChanged();

private slots:
  void itemClicked(ClickableItem *item);
  void backgroundClicked();

private:
  DecayScheme scheme_;

  GraphicsScene *scene_ {nullptr};

  SchemeVisualSettings visual_settings_;

  NuclideItem* parent_ {nullptr};
  NuclideItem* daughter_ {nullptr};
  std::map<Energy, LevelItem*> levels_;
  std::map<Energy, LevelItem*> parent_levels_;
  std::list<TransitionItem*> transitions_;

  bool parent_selected_ {false};
  bool daughter_selected_ {false};
  bool highlight_cascade_ {false};

  double min_intensity_;

  void alignGraphicsItems();

  void addLevel(Level level, SchemeVisualSettings vis);
  void addParentLevel(Level level, SchemeVisualSettings vis);
  void addTransition(Transition transition, SchemeVisualSettings vis);

  void clickedGamma(TransitionItem *g);
  void clickedParentLevel(LevelItem *e);
  void clickedDaughterLevel(LevelItem *e);
  void clickedParent();
  void clickedDaughter();

  void deselect_all();
  void deselect_levels();
  void deselect_nuclides();
  void deselect_gammas();

  void highlight_coincidences();
};
