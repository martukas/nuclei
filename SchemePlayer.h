#ifndef SCHEME_PLAYER_H
#define SCHEME_PLAYER_H

#include <stdint.h>
#include <QObject>
#include <QStringList>
#include <QMap>
#include <QFont>
#include <QPen>
#include <QMetaType>
#include "DecayScheme.h"

#include "LevelRendered.h"
#include "TransitionRendered.h"
#include "NuclideRendered.h"

class QGraphicsScene;
class QGraphicsLineItem;
class QGraphicsSimpleTextItem;
class ClickableItem;

class SchemePlayer : public QObject
{
    Q_OBJECT
public:

    explicit SchemePlayer(DecaySchemePtr scheme, QObject *parent = 0);

    void setStyle(const QFont &fontfamily, unsigned int sizePx);
    QGraphicsScene * levelPlot();

    void setShadowEnabled(bool enable);

    QString name() const;

    struct CascadeIdentifier {
        CascadeIdentifier();
        Energy start;
        Energy pop;
        Energy intermediate;
        bool highlightIntermediate;
        Energy depop;
    };

    CascadeIdentifier currentSelection() const;
    void setCurrentSelection(const CascadeIdentifier &identifier);

    struct SchemePlayerDataSet {
        SchemePlayerDataSet();

        QString startEnergy;
        QString startSpin;

        QString popEnergy;
        QString popIntensity;
        QString popMultipolarity;
        QString popMixing;

        QString intEnergy;
        QString intHalfLife;
        QString intSpin;
        QString intMu;
        QString intQ;

        QString depopEnergy;
        QString depopIntensity;
        QString depopMultipolarity;
        QString depopMixing;

        QString endEnergy;
        QString endSpin;

        QString a22;
        QString a24;
        QString a42;
        QString a44;
    };

    SchemePlayerDataSet decayDataSet() const;
    void triggerDataUpdate();

signals:
    void enabledShadow(bool enable);
    void updatedSchemePlayerData(SchemePlayer::SchemePlayerDataSet data);
    
private slots:
    void itemClicked(ClickableItem *item);

private:
    DecaySchemePtr scheme_;

    void clickedGamma(TransitionRendered *g);
    void clickedEnergyLevel(LevelRendered *e);
    void alignGraphicsItems();


    QGraphicsScene *scene_;

    // highlighted items
    Energy firstSelectedGamma, secondSelectedGamma;
    Energy selectedEnergyLevel;

    SchemeVisualSettings vis;

    NuclideRendered parent_, daughter_;
    QMap<Energy,LevelRendered*> levels_;
    QMap<Energy,LevelRendered*> parent_levels_;
    QList<TransitionRendered*> transitions_;

};

Q_DECLARE_METATYPE(SchemePlayer::CascadeIdentifier)

//QDataStream & operator<<(QDataStream &out, const SchemePlayer::CascadeIdentifier &ident);
//QDataStream & operator>>(QDataStream &in, SchemePlayer::CascadeIdentifier &ident);

#endif // DECAY_H
