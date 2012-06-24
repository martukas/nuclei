#ifndef DECAY_H
#define DECAY_H

#include <stdint.h>
#include <QObject>
#include <QStringList>
#include <QMap>
#include <QFont>
#include <QPen>
#include "Nuclide.h"
#include "SpinParity.h"

class QGraphicsScene;
class EnergyLevel;
class QGraphicsLineItem;
class QGraphicsSimpleTextItem;
class ClickableItem;
class GammaTransition;

namespace Ui {
class KaihenMainWindow;
}

class Decay : public QObject
{
    Q_OBJECT
public:
    enum Type {
        ElectronCapture,
        BetaPlus,
        BetaMinus,
        IsomericTransition,
        Alpha
    };

    explicit Decay(Nuclide parentNuclide, Nuclide daughterNuclide, Type decayType, QObject *parent = 0);
    explicit Decay(const QStringList &ensdfData, const QStringList &ensdfAdoptedLevels, QObject *parent = 0);

    ~Decay();

    QString decayTypeAsText() const;
    QGraphicsScene * levelPlot();
    void setUpdateableUi(Ui::KaihenMainWindow *updui);
    void setShadowEnabled(bool enable);

    QString toText() const;

    QVector<QPointF> gammaSpectrum(double fwhm) const;

signals:
    void enableShadow(bool enable);
    
private slots:
    void itemClicked(ClickableItem *item);

private:
    void clickedGamma(GammaTransition *g);
    void clickedEnergyLevel(EnergyLevel *e);
    void updateDecayDataLabels();
    void resetAnisotropyLabels();
    void alignGraphicsItems();
    void initializeStyle();
    void processENSDFLevels() const;
    int64_t parseEnsdfEnergy(QString estr) const;
    double gauss(const double x, const double sigma) const;

    Nuclide pNuc, dNuc;
    Type t;

    int64_t parentDecayStartEnergyEv;
    SpinParity parentDecayStartSpin;
    mutable double normalizeDecIntensToPercentParentDecay;
    mutable double normalizeGammaIntensToPercentParentDecay;


    QStringList ensdf, adopt;
    mutable QMap<int64_t, EnergyLevel*> levels;
    QGraphicsScene *scene;
    Ui::KaihenMainWindow *ui;

    // graphics items
    QGraphicsLineItem *pNucBaseLevel, *pNucStartLevel;
    QGraphicsLineItem *pNucVerticalArrow;
    QGraphicsSimpleTextItem *pNucHl, *pNucBaseEnergy, *pNucEnergy, *pNucSpin;

    // style
    QFont stdFont, stdBoldFont, nucFont, nucIndexFont, parentHlFont, feedIntensityFont, gammaFont;
    QPen levelPen, stableLevelPen, feedArrowPen, intenseFeedArrowPen, gammaPen, intenseGammaPen;

    // highlighted items
    GammaTransition *firstSelectedGamma, *secondSelectedGamma;
    EnergyLevel *selectedEnergyLevel;

    static const int primaryFontSize;
    static const double outerGammaMargin; // margin between level texts (spin, energy) and gammas
    static const double outerLevelTextMargin; // level lines extend beyond the beginning/end of the level texts by this value
    static const double maxExtraLevelDistance; // maximal additional distance between two level lines
    static const double levelToHalfLifeDistance; // distance between level line and half life text
    static const double parentNuclideLevelLineLength;
    static const double parentNuclideLevelLineExtraLength; // additional length of the decaying level
    static const double arrowHeadLength;
    static const double arrowHeadWidth;
    static const double arrowGap;
    static const double parentNuclideToEnergyLevelsDistance;
    static const double highlightWidth;
};

#endif // DECAY_H
