#pragma once

#include <QObject>
#include <QMap>
#include <QFont>
#include <QMetaType>
#include <NucData/DecayScheme.h>

#include "NuclideItem.h"

#include "ClickableItem.h"

#include "GraphicsScene.h"

class LevelItem;
class TransitionItem;
class FeedingArrow;

class SchemeGraphics : public QObject
{
  Q_OBJECT
public:

  explicit SchemeGraphics(DecayScheme scheme,
                        double min_intensity,
                        QObject *parent = 0);

  ~SchemeGraphics();

  void setStyle(const SchemeVisualSettings& vis);

  GraphicsScene* levelPlot();

  void setShadowEnabled(bool enable);

  const DecayScheme& scheme() const;

  QString name() const;

  void triggerDataUpdate();

  void clearSelection();

  std::set<Energy> selected_feedings(int level) const;
  std::set<Energy> selected_levels(int level) const;
  std::set<Energy> selected_parent_levels(int level) const;
  std::set<Energy> selected_transistions(int level) const;

  void select_levels(const std::set<Energy>&, int level=1);
  void select_feedings(const std::set<Energy>&, int level=1);
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
  ParentPosition parentpos_ {NoParent};

  NuclideItem* parent_ {nullptr};
  std::map<Energy, LevelItem*> parent_levels_;

  NuclideItem* daughter_ {nullptr};
  std::map<Energy, LevelItem*> daughter_levels_;
  std::map<Energy, FeedingArrow*> feeding_arrows_;
  std::list<TransitionItem*> transitions_;

  bool parent_selected_ {false};
  bool daughter_selected_ {false};
  bool highlight_cascade_ {false};

  double min_intensity_;

  void alignGraphicsItems();

  void addParent(Nuclide nuc);
  void addDaughter(Nuclide nuc);
  void addLevel(Level level);
  void addParentLevel(Level level);
  void addTransition(Transition transition);
  void connectItem(ClickableItem* item);

  void clickedGamma(TransitionItem *g);
  void clickedParentLevel(LevelItem *e);
  void clickedDaughterLevel(LevelItem *e);
  void clickedFeeding(FeedingArrow*);
  void clickedParent();
  void clickedDaughter();

  void deselect_all();
  void deselect_levels();
  void deselect_nuclides();
  void deselect_gammas();
  void deselect_feedings();

  void highlight_coincidences();
};
