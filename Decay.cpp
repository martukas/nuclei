#include "Decay.h"
#include <QDebug>
#include <QLocale>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsDropShadowEffect>
#include <QFontMetrics>
#include <QVector>
#include <cmath>
#include <boost/math/special_functions/fpclassify.hpp>
#include <algorithm>
#include <functional>
#include <Akk.h>
#include "EnergyLevel.h"
#include "ActiveGraphicsItemGroup.h"
#include "GammaTransition.h"
#include "GraphicsHighlightItem.h"

const double Decay::outerGammaMargin = 50.0;
const double Decay::outerLevelTextMargin = 4.0; // level lines extend beyond the beginning/end of the level texts by this value
const double Decay::maxExtraLevelDistance = 120.0;
const double Decay::levelToHalfLifeDistance = 10.0;
const double Decay::parentNuclideLevelLineLength = 110.0;
const double Decay::parentNuclideLevelLineExtraLength = 15.5;
const double Decay::parentNuclideMinSpinEnergyDistance = 15.0;
const double Decay::arrowHeadLength = 11.0;
const double Decay::arrowHeadWidth = 5.0;
const double Decay::arrowGap = 5.0;
const double Decay::parentNuclideToEnergyLevelsDistance = 30.0;
const double Decay::highlightWidth = 5.0;


Decay::Decay(const QString &name, Nuclide *parentNuclide, Nuclide *daughterNuclide, Type decayType, QObject *parent)
    : QObject(parent), m_name(name), pNuc(parentNuclide), dNuc(daughterNuclide), t(decayType),
      scene(0), m_lastFwhm(std::numeric_limits<double>::quiet_NaN()),
      m_upperSpectrumLimit(std::numeric_limits<double>::quiet_NaN()),
      pNucVerticalArrow(0), pNucHl(0),
      firstSelectedGamma(0), secondSelectedGamma(0), selectedEnergyLevel(0)
{
}

Decay::~Decay()
{
    delete pNuc;
    delete dNuc;
}

QString Decay::decayTypeAsText(Type type)
{
    switch (type) {
    case ElectronCapture:
        return "Electron Capture";
    case BetaPlus:
        return QString::fromUtf8("β+");
    case BetaMinus:
        return QString::fromUtf8("β-");
    case IsomericTransition:
        return "Isomeric Transition";
    case Alpha:
        return QString::fromUtf8("α");
    default:
        break;
    }
    return "";
}

QGraphicsScene * Decay::levelPlot()
{
    if (scene)
        return scene;

    scene = new QGraphicsScene(this);

    // decide if parent nuclide should be printed on the left side (beta-),
    // on the right side (EC, beta+, alpha) or not at all (isomeric)
    enum ParentPosition {
        NoParent,
        LeftParent,
        RightParent
    } parentpos = RightParent;
    if (t == IsomericTransition)
        parentpos = NoParent;
    else if (t == BetaMinus)
        parentpos = LeftParent;

    // create level items
    foreach (EnergyLevel *level, dNuc->levels()) {
        QFontMetrics stdBoldFontMetrics(stdBoldFont);

        level->item = new ActiveGraphicsItemGroup(level);
        connect(this, SIGNAL(enabledShadow(bool)), level->item, SLOT(setShadowEnabled(bool)));
        level->item->setActiveColor(QColor(224, 186, 100, 180));
        connect(level->item, SIGNAL(clicked(ClickableItem*)), this, SLOT(itemClicked(ClickableItem*)));

        level->graline = new QGraphicsLineItem(-outerGammaMargin, 0.0, outerGammaMargin, 0.0, level->item);
        level->graline->setPen(levelPen);
        // thick line for stable/isomeric levels
        if (level->halfLife().isStable() || level->isomerNum() > 0)
            level->graline->setPen(stableLevelPen);

        level->graclickarea = new QGraphicsRectItem(-outerGammaMargin, -0.5*stdBoldFontMetrics.height(), 2.0*outerGammaMargin, stdBoldFontMetrics.height());
        level->graclickarea->setPen(Qt::NoPen);
        level->graclickarea->setBrush(Qt::NoBrush);

        level->grahighlighthelper = new GraphicsHighlightItem(-outerGammaMargin, -0.5*highlightWidth, 2.0*outerGammaMargin, highlightWidth);
        level->grahighlighthelper->setOpacity(0.0);

        QString etext = level->energy().toString();
        level->graetext = new QGraphicsSimpleTextItem(etext, level->item);
        level->graetext->setFont(stdBoldFont);
        level->graetext->setPos(0.0, -stdBoldFontMetrics.height());

        QString spintext = level->spin().toString();
        level->graspintext = new QGraphicsSimpleTextItem(spintext, level->item);
        level->graspintext->setFont(stdBoldFont);
        level->graspintext->setPos(0.0, -stdBoldFontMetrics.height());

        QString hltext = level->halfLife().toString();
        level->grahltext = new QGraphicsSimpleTextItem(hltext, level->item);
        level->grahltext->setFont(stdFont);
        level->grahltext->setPos(0.0, -0.5*stdBoldFontMetrics.height());

        level->item->addHighlightHelper(level->grahighlighthelper);
        level->item->addToGroup(level->graline);
        level->item->addToGroup(level->graclickarea);
        level->item->addToGroup(level->graetext);
        level->item->addToGroup(level->graspintext);
        level->item->addToGroup(level->grahltext);
        scene->addItem(level->item);

        // plot level feeding arrow if necessary
        if (level->normalizedFeedIntensity().hasFiniteValue()) {
            // create line
            level->grafeedarrow = new QGraphicsLineItem;
            level->grafeedarrow->setPen((level->feedintens >= 10.0) ? intenseFeedArrowPen : feedArrowPen);
            scene->addItem(level->grafeedarrow);
            // create arrow head
            QPolygonF arrowpol;
            arrowpol << QPointF(0.0, 0.0);
            arrowpol << QPointF((parentpos == RightParent ? 1.0 : -1.0) * arrowHeadLength, 0.5*arrowHeadWidth);
            arrowpol << QPointF((parentpos == RightParent ? 1.0 : -1.0) * arrowHeadLength, -0.5*arrowHeadWidth);
            level->graarrowhead = new QGraphicsPolygonItem(arrowpol);
            level->graarrowhead->setBrush(QColor(level->grafeedarrow->pen().color()));
            level->graarrowhead->setPen(Qt::NoPen);
            scene->addItem(level->graarrowhead);
            // create intensity label
            level->grafeedintens = new QGraphicsSimpleTextItem(QString("%1 %").arg(level->normalizedFeedIntensity().toText()));
            level->grafeedintens->setFont(feedIntensityFont);
            scene->addItem(level->grafeedintens);
        }
    }

    // create gammas
    foreach (EnergyLevel *level, dNuc->levels()) {
        QList<GammaTransition*> levelgammas = level->depopulatingTransitions();
        foreach (GammaTransition *gamma, levelgammas) {
            ActiveGraphicsItemGroup *item = gamma->createGammaGraphicsItem(gammaFont, gammaPen, intenseGammaPen);
            connect(this, SIGNAL(enabledShadow(bool)), item, SLOT(setShadowEnabled(bool)));
            connect(item, SIGNAL(clicked(ClickableItem*)), this, SLOT(itemClicked(ClickableItem*)));
            scene->addItem(item);
        }
    }

    // create parent nuclide label and level(s)
    if (parentpos == LeftParent || parentpos == RightParent) {
        // create nuclide label
        QGraphicsItem *pNucGra = pNuc->createNuclideGraphicsItem(nucFont, nucIndexFont);
        scene->addItem(pNucGra);

        // create half-life label
        pNucHl = new QGraphicsSimpleTextItem(pNuc->halfLifeAsText());
        pNucHl->setFont(parentHlFont);
        scene->addItem(pNucHl);

        // create vertical arrow component
        pNucVerticalArrow = new QGraphicsLineItem;
        pNucVerticalArrow->setPen(feedArrowPen);
        scene->addItem(pNucVerticalArrow);

        // create level items
        foreach (EnergyLevel *level, pNuc->levels()) {
            QFontMetrics stdBoldFontMetrics(stdBoldFont);

            level->graline = new QGraphicsLineItem;
            level->graline->setPen(stableLevelPen);
            scene->addItem(level->graline);

            level->graetext = new QGraphicsSimpleTextItem(level->energy().toString());
            level->graetext->setFont(stdBoldFont);
            scene->addItem(level->graetext);

            level->graspintext = new QGraphicsSimpleTextItem(level->spin().toString());
            level->graspintext->setFont(stdBoldFont);
            scene->addItem(level->graspintext);
        }

    }

    // create daughter nuclide label
    QGraphicsItem *dNucGra = dNuc->createNuclideGraphicsItem(nucFont, nucIndexFont);
    scene->addItem(dNucGra);

    alignGraphicsItems();

    return scene;
}

void Decay::setShadowEnabled(bool enable)
{
    emit enabledShadow(enable);
}

QString Decay::name() const
{
    return m_name;
}

Decay::Type Decay::type() const
{
    return t;
}

Nuclide *Decay::parentNuclide() const
{
    return pNuc;
}

Nuclide *Decay::daughterNuclide() const
{
    return dNuc;
}

QVector<double> Decay::gammaSpectrumX(double fwhm) const
{
    if (fwhm == m_lastFwhm && !spectX.isEmpty())
        return spectX;

    // determine highest energy (updates m_lastFwhm as side effect)
    double max = upperSpectrumLimit(fwhm);

    const int numsamples = 1000;

    // create result vector
    spectX.resize(numsamples);

    const double interval = max / double(numsamples);
    for (int i=0; i<numsamples; i++)
        spectX[i] = interval*double(i)+0.5*interval;

    return spectX;
}

QVector<double> Decay::gammaSpectrumY(double fwhm) const
{
    const int numsamples = 1000;

    // determine highest energy (updates m_lastFwhm and clears spectX if necessary as side effect)
    double max = upperSpectrumLimit(fwhm);

    // create result vector
    QVector<double> yResult(numsamples);

    // add single gamma results
    foreach (EnergyLevel *level, dNuc->levels())
        foreach (GammaTransition *g, level->depopulatingTransitions())
            if (boost::math::isfinite(g->intensity())) {
                QVector<double> &spect = g->spectrum(fwhm, max, numsamples);
                std::transform(yResult.begin(), yResult.end(), spect.begin(), yResult.begin(), std::plus<double>());
            }

    return yResult;
}

QVector<double> Decay::firstSelectedGammaSpectrumY(double fwhm) const
{
    if (!firstSelectedGamma)
        return QVector<double>();

    // determine highest energy (updates m_lastFwhm and clears spectX if necessary as side effect)
    double max = upperSpectrumLimit(fwhm);

    const int numsamples = 1000;

    return firstSelectedGamma->spectrum(fwhm, max, numsamples);
}

QVector<double> Decay::secondSelectedGammaSpectrumY(double fwhm) const
{
    if (!secondSelectedGamma)
        return QVector<double>();

    // determine highest energy (updates m_lastFwhm and clears spectX if necessary as side effect)
    double max = upperSpectrumLimit(fwhm);

    const int numsamples = 1000;

    return secondSelectedGamma->spectrum(fwhm, max, numsamples);
}

Decay::CascadeIdentifier Decay::currentSelection() const
{
    CascadeIdentifier ci;
    if (firstSelectedGamma) {
        ci.start = firstSelectedGamma->depopulatedLevel()->energy();
        ci.pop = firstSelectedGamma->energy();
        ci.intermediate = firstSelectedGamma->populatedLevel()->energy();
    }
    if (selectedEnergyLevel) {
        ci.intermediate = selectedEnergyLevel->energy();
        ci.highlightIntermediate = true;
    }
    if (secondSelectedGamma) {
        ci.intermediate = secondSelectedGamma->depopulatedLevel()->energy();
        ci.depop = secondSelectedGamma->energy();
    }

    return ci;
}

void Decay::setCurrentSelection(const Decay::CascadeIdentifier &identifier)
{
    if (firstSelectedGamma)
        if (firstSelectedGamma->graphicsItem())
            firstSelectedGamma->graphicsItem()->setHighlighted(false);
    if (secondSelectedGamma)
        if (secondSelectedGamma->graphicsItem())
            secondSelectedGamma->graphicsItem()->setHighlighted(false);
    if (selectedEnergyLevel)
        if (selectedEnergyLevel->graphicsItem())
            selectedEnergyLevel->graphicsItem()->setHighlighted(false);

    EnergyLevel *start = 0;
    if (identifier.start.isValid())
        start = dNuc->levels().value(identifier.start);

    EnergyLevel *inter = 0;
    if (identifier.intermediate.isValid())
        inter = dNuc->levels().value(identifier.intermediate);

    if (start && identifier.pop) {
        const QList<GammaTransition*> &gammas = start->depopulatingTransitions();
        GammaTransition *gamma = 0;
        for (int i=0; i<gammas.size(); i++) {
            if (gammas.at(i)->energy() == identifier.pop) {
                gamma = gammas.at(i);
                break;
            }
        }
        if (gamma) {
            firstSelectedGamma = gamma;
            if (firstSelectedGamma->graphicsItem())
                firstSelectedGamma->graphicsItem()->setHighlighted(true);
        }
    }

    if (inter && identifier.depop) {
        const QList<GammaTransition*> &gammas = inter->depopulatingTransitions();
        GammaTransition *gamma = 0;
        for (int i=0; i<gammas.size(); i++) {
            if (gammas.at(i)->energy() == identifier.depop) {
                gamma = gammas.at(i);
                break;
            }
        }
        if (gamma) {
            secondSelectedGamma = gamma;
            if (secondSelectedGamma->graphicsItem())
                secondSelectedGamma->graphicsItem()->setHighlighted(true);
        }
    }

    if (inter && identifier.highlightIntermediate) {
        selectedEnergyLevel = inter;
        if (selectedEnergyLevel->graphicsItem())
            selectedEnergyLevel->graphicsItem()->setHighlighted(true);
    }

    triggerDecayDataUpdate();
}

Decay::DecayDataSet Decay::decayDataSet() const
{
    DecayDataSet dataset;

    // intermediate level
    if (selectedEnergyLevel) {
        dataset.intEnergy = selectedEnergyLevel->energy().toString();
        dataset.intHalfLife = selectedEnergyLevel->halfLife().toString();
        dataset.intSpin = selectedEnergyLevel->spin().toString();
        dataset.intMu = selectedEnergyLevel->mu().toText().replace("undefined", "?");
        dataset.intQ = selectedEnergyLevel->q().toText().replace("undefined", "?");
    }

    // gammas
    GammaTransition *pop = 0, *depop = 0;
    if (firstSelectedGamma) {
        if (firstSelectedGamma->depopulatedLevel() == selectedEnergyLevel)
            depop = firstSelectedGamma;
        else
            pop = firstSelectedGamma;
    }
    if (secondSelectedGamma) {
        if (secondSelectedGamma->depopulatedLevel() == selectedEnergyLevel)
            depop = secondSelectedGamma;
        else
            pop = secondSelectedGamma;
    }

    // populating
    if (pop) {
        dataset.popEnergy = pop->energy().toString();
        dataset.popIntensity = pop->intensityAsText();
        dataset.popMultipolarity = pop->multipolarityAsText();
        dataset.popMixing = pop->delta().toString().replace("undefined", "<i>unknown</i>");
    }

    // depopulating
    if (depop) {
        dataset.depopEnergy = depop->energy().toString();
        dataset.depopIntensity = depop->intensityAsText();
        dataset.depopMultipolarity = depop->multipolarityAsText();
        dataset.depopMixing = depop->delta().toString().replace("undefined", "<i>unknown</i>");
    }

    // start and end level
    if (firstSelectedGamma && secondSelectedGamma) {
        dataset.startEnergy = pop->depopulatedLevel()->energy().toString();
        dataset.startSpin = pop->depopulatedLevel()->spin().toString();
        dataset.endEnergy = depop->populatedLevel()->energy().toString();
        dataset.endSpin = depop->populatedLevel()->spin().toString();
    }

    // calculate anisotropies
    if (pop && depop && selectedEnergyLevel) {
        if (pop->depopulatedLevel()->spin().isValid() &&
            depop->populatedLevel()->spin().isValid() &&
            selectedEnergyLevel->spin().isValid() &&
            pop->delta().hasFiniteValue() &&
            depop->delta().hasFiniteValue()
           ) {
            // create list of sign combinations
            typedef QPair<double, double> SignCombination;
            QList< SignCombination > variants;
            QList< double > popvariants;
            if (pop->delta().sign() == UncertainDouble::MagnitudeDefined)
                popvariants << 1. << -1.;
            else
                popvariants << ((pop->delta() < 0.0) ? -1. : 1.);
            foreach (double popvariant, popvariants) {
                if (depop->delta().sign() == UncertainDouble::MagnitudeDefined)
                    variants << QPair<double, double>(popvariant, 1.) << SignCombination(popvariant, -1.);
                else
                    variants << SignCombination(popvariant, (depop->delta() < 0.0) ? -1. : 1.);
            }

            // compute possible results
            QStringList a22str, a24str, a42str, a44str;
            QVector<UncertainDouble> a22(variants.size()), a24(variants.size()), a42(variants.size()), a44(variants.size());
            Akk calc;
            calc.setInitialStateSpin(pop->depopulatedLevel()->spin().doubledSpin());
            calc.setIntermediateStateSpin(selectedEnergyLevel->spin().doubledSpin());
            calc.setFinalStateSpin(depop->populatedLevel()->spin().doubledSpin());

            for (int i=0; i<variants.size(); i++) {
                SignCombination variant = variants.at(i);
                UncertainDouble popdelta(pop->delta());
                UncertainDouble depopdelta(depop->delta());
                if (pop->delta().sign() == UncertainDouble::MagnitudeDefined)
                    popdelta.setValue(popdelta.value() * variant.first, UncertainDouble::SignMagnitudeDefined);
                if (depop->delta().sign() == UncertainDouble::MagnitudeDefined)
                    depopdelta.setValue(depopdelta.value() * variant.second, UncertainDouble::SignMagnitudeDefined);

                calc.setPopulatingGammaMixing(popdelta.value(), std::max(popdelta.lowerUncertainty(), popdelta.upperUncertainty()));
                calc.setDepopulatingGammaMixing(depopdelta.value(), std::max(depopdelta.lowerUncertainty(), depopdelta.upperUncertainty()));

                // determine uncertainty type of results (may only be SymmetricUncertainty or Approximately)
                UncertainDouble::UncertaintyType restype = UncertainDouble::SymmetricUncertainty;
                if (popdelta.uncertaintyType() == UncertainDouble::Approximately || depopdelta.uncertaintyType() == UncertainDouble::Approximately)
                    restype = UncertainDouble::Approximately;

                // determine prefix
                QString prfx;
                if (variants.size() > 1) {
                    prfx = QString::fromUtf8("%1: ");
                    prfx = prfx.arg(QString(variant.first < 0 ? "-" : "+") + QString(variant.second < 0 ? "-" : "+"));
                }

                a22[i].setValue(calc.a22(), UncertainDouble::SignMagnitudeDefined);
                a22[i].setUncertainty(calc.a22Uncertainty(), calc.a22Uncertainty(), restype);
                a24[i].setValue(calc.a24(), UncertainDouble::SignMagnitudeDefined);
                a24[i].setUncertainty(calc.a24Uncertainty(), calc.a24Uncertainty(), restype);
                a42[i].setValue(calc.a42(), UncertainDouble::SignMagnitudeDefined);
                a42[i].setUncertainty(calc.a42Uncertainty(), calc.a42Uncertainty(), restype);
                a44[i].setValue(calc.a44(), UncertainDouble::SignMagnitudeDefined);
                a44[i].setUncertainty(calc.a44Uncertainty(), calc.a44Uncertainty(), restype);

                a22str.append(QString("%1%2").arg(prfx).arg(a22[i].toText()));
                a24str.append(QString("%1%2").arg(prfx).arg(a24[i].toText()));
                a42str.append(QString("%1%2").arg(prfx).arg(a42[i].toText()));
                a44str.append(QString("%1%2").arg(prfx).arg(a44[i].toText()));
            }
            // replace strings if all values are equal
            if (variants.size() > 1) {
                if (a22.count(a22[0]) == (variants.size())) {
                    a22str.clear();
                    a22str.append(a22[0].toText());
                }
                if (a24.count(a24[0]) == (variants.size())) {
                    a24str.clear();
                    a24str.append(a24[0].toText());
                }
                if (a42.count(a42[0]) == (variants.size())) {
                    a42str.clear();
                    a42str.append(a42[0].toText());
                }
                if (a44.count(a44[0]) == (variants.size())) {
                    a44str.clear();
                    a44str.append(a44[0].toText());
                }
            }

            dataset.a22 = a22str.join(", ");
            dataset.a24 = a24str.join(", ");
            dataset.a42 = a42str.join(", ");
            dataset.a44 = a44str.join(", ");
        }
    }

    return dataset;
}

void Decay::itemClicked(ClickableItem *item)
{
    if (item->type() == ClickableItem::EnergyLevelType)
        clickedEnergyLevel(dynamic_cast<EnergyLevel*>(item));
    else if (item->type() == ClickableItem::GammaTransitionType)
        clickedGamma(dynamic_cast<GammaTransition*>(item));
}

void Decay::clickedGamma(GammaTransition *g)
{
    if (!g)
        return;

    // deselect if active level is clicked again
    if (g == firstSelectedGamma) {
        firstSelectedGamma->graphicsItem()->setHighlighted(false);
        firstSelectedGamma = secondSelectedGamma;
        secondSelectedGamma = 0;
    }
    else if (g == secondSelectedGamma) {
        secondSelectedGamma->graphicsItem()->setHighlighted(false);
        secondSelectedGamma = 0;
    }
    else {
        // deselect inappropriate level(s)
        bool firstok = false;
        if (firstSelectedGamma)
            firstok = g->populatedLevel() == firstSelectedGamma->depopulatedLevel() ||
                    g->depopulatedLevel() == firstSelectedGamma->populatedLevel();
        bool secondok = false;
        if (secondSelectedGamma)
            secondok = g->populatedLevel() == secondSelectedGamma->depopulatedLevel() ||
                    g->depopulatedLevel() == secondSelectedGamma->populatedLevel();

        if (firstok && secondok) {
            firstSelectedGamma->graphicsItem()->setHighlighted(false);
            firstSelectedGamma = secondSelectedGamma;
            secondSelectedGamma = 0;
        }
        else if (firstok) {
            if (secondSelectedGamma) {
                secondSelectedGamma->graphicsItem()->setHighlighted(false);
                secondSelectedGamma = 0;
            }
        }
        else if (secondok) {
            Q_ASSERT(firstSelectedGamma);
            firstSelectedGamma->graphicsItem()->setHighlighted(false);
            firstSelectedGamma = secondSelectedGamma;
            secondSelectedGamma = 0;
        }
        else {
            if (firstSelectedGamma) {
                firstSelectedGamma->graphicsItem()->setHighlighted(false);
                firstSelectedGamma = 0;
            }
            if (secondSelectedGamma) {
                secondSelectedGamma->graphicsItem()->setHighlighted(false);
                secondSelectedGamma = 0;
            }
        }

        // case: no gamma previously selected
        if (!firstSelectedGamma && !secondSelectedGamma) {
            firstSelectedGamma = g;
            firstSelectedGamma->graphicsItem()->setHighlighted(true);
            if (selectedEnergyLevel) {
                if (g->populatedLevel() != selectedEnergyLevel && g->depopulatedLevel() != selectedEnergyLevel) {
                    selectedEnergyLevel->graphicsItem()->setHighlighted(false);
                    selectedEnergyLevel = 0;
                }
            }
        }
        // case: one gamma previously selected, common level exists
        else {
            secondSelectedGamma = g;
            secondSelectedGamma->graphicsItem()->setHighlighted(true);
            // select intermediate level
            EnergyLevel *intermediate = firstSelectedGamma->populatedLevel();
            if (firstSelectedGamma->depopulatedLevel() == secondSelectedGamma->populatedLevel())
                intermediate = firstSelectedGamma->depopulatedLevel();
            // activate intermediate level
            if (selectedEnergyLevel)
                if (selectedEnergyLevel != intermediate)
                    selectedEnergyLevel->graphicsItem()->setHighlighted(false);
            selectedEnergyLevel = intermediate;
            selectedEnergyLevel->graphicsItem()->setHighlighted(true);
        }
    }

    triggerDecayDataUpdate();
}

void Decay::clickedEnergyLevel(EnergyLevel *e)
{
    if (!e)
        return;

    // deselect if clicked again
    if (e == selectedEnergyLevel) {
        selectedEnergyLevel->graphicsItem()->setHighlighted(false);
        selectedEnergyLevel = 0;
    }
    // select otherwise
    else {
        // deselect old level
        if (selectedEnergyLevel)
            selectedEnergyLevel->graphicsItem()->setHighlighted(false);
        selectedEnergyLevel = e;
        e->graphicsItem()->setHighlighted(true);
        // deselect gamma which is not connected to the level anymore
        if (firstSelectedGamma) {
            if (firstSelectedGamma->depopulatedLevel() != e && firstSelectedGamma->populatedLevel() != e) {
                firstSelectedGamma->graphicsItem()->setHighlighted(false);
                firstSelectedGamma = 0;
            }
        }
        if (secondSelectedGamma) {
            if (secondSelectedGamma->depopulatedLevel() != e && secondSelectedGamma->populatedLevel() != e) {
                secondSelectedGamma->graphicsItem()->setHighlighted(false);
                secondSelectedGamma = 0;
            }
        }
        if (secondSelectedGamma && !firstSelectedGamma) {
            firstSelectedGamma = secondSelectedGamma;
            secondSelectedGamma = 0;
        }
    }
    // prevent two gammas being active after the intermediate level was changed
    if (firstSelectedGamma && secondSelectedGamma) {
        firstSelectedGamma->graphicsItem()->setHighlighted(false);
        firstSelectedGamma = secondSelectedGamma;
        secondSelectedGamma = 0;
    }

    triggerDecayDataUpdate();
}

void Decay::triggerDecayDataUpdate()
{
    emit updatedDecayData(decayDataSet());
}

void Decay::alignGraphicsItems()
{
    QMap<Energy, EnergyLevel*> &levels(dNuc->levels());
    // decide if parent nuclide should be printed on the left side (beta-),
    // on the right side (EC, beta+, alpha) or not at all (isomeric)
    enum ParentPosition {
        NoParent,
        LeftParent,
        RightParent
    } parentpos = RightParent;
    if (t == IsomericTransition)
        parentpos = NoParent;
    else if (t == BetaMinus)
        parentpos = LeftParent;

    QFontMetrics stdFontMetrics(stdFont);
    QFontMetrics stdBoldFontMetrics(stdBoldFont);
    QFontMetrics parentHlFontMetrics(parentHlFont);
    QFontMetrics feedIntensityFontMetrics(feedIntensityFont);

    // determine size information
    double maxEnergyLabelWidth = 0.0;
    double maxSpinLabelWidth = 0.0;

    foreach (EnergyLevel *level, levels) {
        if (stdBoldFontMetrics.width(level->graspintext->text()) > maxSpinLabelWidth)
            maxSpinLabelWidth = stdBoldFontMetrics.width(level->graspintext->text());
        if (stdBoldFontMetrics.width(level->graetext->text()) > maxEnergyLabelWidth)
            maxEnergyLabelWidth = stdBoldFontMetrics.width(level->graetext->text());
    }

    // determine y coordinates for all levels
    double maxEnergyGap = 0.0;
    for (QMap<Energy, EnergyLevel*>::const_iterator i=levels.begin()+1; i!=levels.end(); i++) {
        double diff = i.key() - (i-1).key();
        maxEnergyGap = qMax(maxEnergyGap, diff);
    }
    for (QMap<Energy, EnergyLevel*>::const_iterator i=levels.begin()+1; i!=levels.end(); i++) {
        double minheight = 0.5*(i.value()->item->boundingRect().height() + (i-1).value()->item->boundingRect().height());
        double extraheight = maxExtraLevelDistance * (i.key() - (i-1).key()) / maxEnergyGap;
        i.value()->graYPos = std::floor((i-1).value()->graYPos - minheight - extraheight) + 0.5*i.value()->graline->pen().widthF();
    }

    // determine space needed for gammas
    double gammaspace = std::numeric_limits<double>::quiet_NaN();
    foreach (EnergyLevel *level, levels) {
        QList<GammaTransition*> levelgammas = level->depopulatingTransitions();
        foreach (GammaTransition *gamma, levelgammas) {
            if (boost::math::isnan(gammaspace))
                gammaspace = gamma->widthFromOrigin();
            else
                gammaspace += gamma->minimalXDistance();
        }
    }
    if (!boost::math::isfinite(gammaspace))
        gammaspace = 0.0;

    // set gamma positions
    double currentgammapos = 0.5*gammaspace;
    bool firstgamma = true;
    foreach (EnergyLevel *level, levels) {
        QList<GammaTransition*> levelgammas = level->depopulatingTransitions();
        foreach (GammaTransition *gamma, levelgammas) {
            if (firstgamma) {
                currentgammapos -= gamma->widthFromOrigin();
                firstgamma = false;
            }
            else {
                currentgammapos -= gamma->minimalXDistance();
            }
            gamma->updateArrow();
            gamma->graphicsItem()->setPos(std::floor(currentgammapos)+0.5*gamma->pen().widthF(), level->graYPos + 0.5*level->graline->pen().widthF());
        }
    }

    // determine line length for parent levels
    double pNucLineLength = parentNuclideLevelLineLength;
    if (parentpos == LeftParent || parentpos == RightParent) {
        pNucLineLength = qMax(parentNuclideLevelLineLength, pNuc->nuclideGraphicsItem()->boundingRect().width() + 20.0);
        foreach (EnergyLevel *level, pNuc->levels()) {
            pNucLineLength = qMax(pNucLineLength,
                                  level->graetext->boundingRect().width() +
                                  level->graspintext->boundingRect().width() +
                                  parentNuclideMinSpinEnergyDistance +
                                  2.0 * outerLevelTextMargin);
        }
    }
    pNucLineLength = std::ceil(pNucLineLength);

    // calculate length of level lines
    double leftlinelength = outerLevelTextMargin + maxSpinLabelWidth + outerGammaMargin + 0.5*gammaspace;
    double rightlinelength = outerLevelTextMargin + maxEnergyLabelWidth + outerGammaMargin + 0.5*gammaspace;

    // calculate start and end points of parent level lines
    double normalleft = std::floor((parentpos == RightParent) ? rightlinelength : -leftlinelength - pNucLineLength) - 0.5*feedArrowPen.widthF();
    double normalright = std::ceil((parentpos == RightParent) ? rightlinelength + pNucLineLength : -leftlinelength) + 0.5*feedArrowPen.widthF();
    double activeleft = std::floor((parentpos == RightParent) ? normalleft : normalleft - parentNuclideLevelLineExtraLength) - 0.5*feedArrowPen.widthF();
    double activeright = std::ceil((parentpos == RightParent) ? normalright + parentNuclideLevelLineExtraLength : normalright) + 0.5*feedArrowPen.widthF();

    // set level positions and sizes
    double arrowVEnd = std::numeric_limits<double>::quiet_NaN();
    foreach (EnergyLevel *level, levels) {

        // temporarily remove items from group (workaround)
        level->item->removeFromGroup(level->grahltext);
        level->item->removeFromGroup(level->graspintext);
        level->item->removeFromGroup(level->graetext);
        level->item->removeFromGroup(level->graclickarea);
        level->item->removeFromGroup(level->graline);
        level->item->removeHighlightHelper(level->grahighlighthelper);

        // rescale
        level->grahighlighthelper->setRect(-leftlinelength, -0.5*highlightWidth, leftlinelength+rightlinelength, highlightWidth);
        level->graline->setLine(-leftlinelength, 0.0, rightlinelength, 0.0);
        level->graclickarea->setRect(-leftlinelength, -0.5*stdBoldFontMetrics.height(), leftlinelength+rightlinelength, stdBoldFontMetrics.height());
        level->graspintext->setPos(-leftlinelength + outerLevelTextMargin, -stdBoldFontMetrics.height());
        level->graetext->setPos(rightlinelength - outerLevelTextMargin - stdBoldFontMetrics.width(level->graetext->text()), -stdBoldFontMetrics.height());
        double levelHlPos = 0.0;
        if (parentpos == RightParent) {
            levelHlPos = -leftlinelength - levelToHalfLifeDistance - stdFontMetrics.width(level->grahltext->text());
        }
        else {
            levelHlPos = rightlinelength + levelToHalfLifeDistance;
        }
        level->grahltext->setPos(levelHlPos, -0.5*stdBoldFontMetrics.height());

        // re-add items to group
        level->item->addHighlightHelper(level->grahighlighthelper);
        level->item->addToGroup(level->graline);
        level->item->addToGroup(level->graclickarea);
        level->item->addToGroup(level->graetext);
        level->item->addToGroup(level->graspintext);
        level->item->addToGroup(level->grahltext);

        level->item->setPos(0.0, level->graYPos); // add 0.5*pen-width to avoid antialiasing artifacts

        if (level->grafeedarrow) {
            double leftend = (parentpos == RightParent) ? rightlinelength + arrowGap + arrowHeadLength : activeleft;
            double rightend = (parentpos == RightParent) ? activeright : -leftlinelength - arrowGap - arrowHeadLength;
            double arrowY = level->graYPos;
            level->grafeedarrow->setLine(leftend, arrowY, rightend, arrowY);
            level->graarrowhead->setPos((parentpos == RightParent) ? rightlinelength + arrowGap : -leftlinelength - arrowGap, arrowY);
            if (boost::math::isnan(arrowVEnd))
                arrowVEnd = arrowY + 0.5*level->grafeedarrow->pen().widthF();
            level->grafeedintens->setPos(leftend + 15.0, arrowY - feedIntensityFontMetrics.height());
        }
    }

    // set position of daughter nuclide
    dNuc->nuclideGraphicsItem()->setPos(-0.5*dNuc->nuclideGraphicsItem()->boundingRect().width(), 0.3*dNuc->nuclideGraphicsItem()->boundingRect().height());

    // set position of parent nuclide
    if (parentpos == LeftParent || parentpos == RightParent) {
        double parentY = std::numeric_limits<double>::quiet_NaN();
        if (!levels.isEmpty())
            parentY = (levels.end()-1).value()->item->y() -
                    (levels.end()-1).value()->item->boundingRect().height() -
                    pNuc->nuclideGraphicsItem()->boundingRect().height() - parentNuclideToEnergyLevelsDistance;

        double parentcenter;
        if (parentpos == RightParent)
            parentcenter = rightlinelength + 0.5*(pNucLineLength+parentNuclideLevelLineExtraLength);
        else
            parentcenter = -leftlinelength - 0.5*(pNucLineLength+parentNuclideLevelLineExtraLength);

        pNuc->nuclideGraphicsItem()->setPos(parentcenter - 0.5*pNuc->nuclideGraphicsItem()->boundingRect().width(), parentY);

        // set position of parent levels
        double topMostLevel = 0.0;
        double y = qRound(parentY - 0.3*pNuc->nuclideGraphicsItem()->boundingRect().height()) + 0.5*stableLevelPen.widthF();
        foreach (EnergyLevel *level, pNuc->levels()) {
            double left = level->isFeedingLevel() ? activeleft : normalleft;
            double right = level->isFeedingLevel() ? activeright : normalright;

            level->graline->setLine(left, y, right, y);
            level->graetext->setPos((pNuc->levels().size() == 1 ? activeright : normalright) - outerLevelTextMargin - level->graetext->boundingRect().width(), y - level->graetext->boundingRect().height());
            level->graspintext->setPos((pNuc->levels().size() == 1 ? activeleft : normalleft) + outerLevelTextMargin, y - level->graetext->boundingRect().height());

            topMostLevel = y;

            // update y
            y -= qMax(level->graetext->boundingRect().height(), level->graspintext->boundingRect().height()) + 10.0;
        }

        double arrowVStart = topMostLevel - 0.5*stableLevelPen.widthF();
        double arrowX = (parentpos == RightParent) ? activeright : activeleft;
        if (boost::math::isfinite(arrowVStart) && boost::math::isfinite(arrowVEnd))
            pNucVerticalArrow->setLine(arrowX, arrowVStart, arrowX, arrowVEnd);

        pNucHl->setPos(parentcenter - 0.5*pNucHl->boundingRect().width(), topMostLevel - stdBoldFontMetrics.height() - parentHlFontMetrics.height() - 12.0);
    }
}

double Decay::upperSpectrumLimit(double fwhm) const
{
    if (fwhm == m_lastFwhm && boost::math::isfinite(m_upperSpectrumLimit))
        return m_upperSpectrumLimit;

    spectX.clear();
    m_lastFwhm = fwhm;

    // determine highest energy
    m_upperSpectrumLimit = 0.0;
    foreach (EnergyLevel *level, dNuc->levels())
        foreach (GammaTransition *g, level->depopulatingTransitions())
            if (boost::math::isfinite(g->intensity()))
                m_upperSpectrumLimit = qMax(m_upperSpectrumLimit, double(g->energy()));
    m_upperSpectrumLimit += 2*fwhm;

    return m_upperSpectrumLimit;
}

void Decay::setStyle(const QFont &fontfamily, unsigned int sizePx)
{
    // prepare fonts and their metrics
    stdFont = fontfamily;
    stdFont.setPixelSize(sizePx);
    stdBoldFont = fontfamily;
    stdBoldFont.setPixelSize(sizePx);
    stdBoldFont.setBold(true);
    nucFont = fontfamily;
    nucFont.setPixelSize(sizePx * 20 / 10);
    nucFont.setBold(true);
    nucIndexFont = fontfamily;
    nucIndexFont.setPixelSize(sizePx * 12 / 10);
    nucIndexFont.setBold(true);
    parentHlFont = fontfamily;
    parentHlFont.setPixelSize(sizePx * 12 / 10);
    feedIntensityFont = fontfamily;
    feedIntensityFont.setPixelSize(sizePx);
    feedIntensityFont.setItalic(true);
    gammaFont = fontfamily;
    gammaFont.setPixelSize(sizePx);

    // prepare pens
    levelPen.setWidthF(1.0);
    levelPen.setCapStyle(Qt::FlatCap);
    stableLevelPen.setWidthF(2.0);
    stableLevelPen.setCapStyle(Qt::FlatCap);
    feedArrowPen.setWidthF(1.0);
    feedArrowPen.setCapStyle(Qt::FlatCap);
    intenseFeedArrowPen.setWidthF(2.0);
    intenseFeedArrowPen.setColor(QColor(232, 95, 92));
    intenseFeedArrowPen.setCapStyle(Qt::FlatCap);
    gammaPen.setWidthF(1.0);
    gammaPen.setCapStyle(Qt::FlatCap);
    intenseGammaPen.setWidthF(2.0);
    intenseGammaPen.setColor(QColor(232, 95, 92));
    intenseGammaPen.setCapStyle(Qt::FlatCap);
}

Decay::CascadeIdentifier::CascadeIdentifier()
    : highlightIntermediate(false)
{
}

Decay::DecayDataSet::DecayDataSet()
    : startEnergy("? keV"), startSpin("/"),
      popEnergy("? keV"), popIntensity("? %"), popMultipolarity("<i>unknown</i>"), popMixing("<i>unknown</i>"),
      intEnergy("? keV"), intHalfLife("? ns"), intSpin("/"), intMu("?"), intQ("?"),
      depopEnergy("? keV"), depopIntensity("? %"), depopMultipolarity("<i>unknown</i>"), depopMixing("<i>unknown</i>"),
      endEnergy("? keV"), endSpin("/"),
      a22("?"), a24("?"), a42("?"), a44("?")
{
}

QDataStream & operator<<(QDataStream &out, const Decay::CascadeIdentifier &ident)
{
    out << ident.start << ident.pop << ident.intermediate << ident.depop << ident.highlightIntermediate;
    return out;
}

QDataStream & operator>>(QDataStream &in, Decay::CascadeIdentifier &ident)
{
    in >> ident.start >> ident.pop >> ident.intermediate >> ident.depop >> ident.highlightIntermediate;
    return in;
}

