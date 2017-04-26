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

    QString name() const;

    void triggerDataUpdate();

signals:
    void enabledShadow(bool enable);
    
private slots:
    void itemClicked(ClickableItem *item);

private:
    DecayScheme scheme_;

    void clickedGamma(TransitionItem *g);
    void clickedEnergyLevel(LevelItem *e);
    void alignGraphicsItems();


    QGraphicsScene *scene_ {nullptr};

    // highlighted items
    Energy firstSelectedGamma, secondSelectedGamma;
    Energy selectedEnergyLevel;

    SchemeVisualSettings visual_settings_;

    NuclideItem parent_, daughter_;
    std::map<Energy,LevelItem*> levels_;
    std::map<Energy,LevelItem*> parent_levels_;
    QList<TransitionItem*> transitions_;

    void addLevel(Level level, SchemeVisualSettings vis);
    void addParentLevel(Level level, SchemeVisualSettings vis);
    void addTransition(Transition transition, SchemeVisualSettings vis);
};
