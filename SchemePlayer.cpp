#include "SchemePlayer.h"
#include <QDebug>
#include <QLocale>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsDropShadowEffect>
#include <QTextDocument>
#include <QFontMetrics>
#include <QVector>
#include <cmath>
#include <boost/math/special_functions/fpclassify.hpp>
#include <algorithm>
#include <functional>
#include <Akk.h>
#include "ActiveGraphicsItemGroup.h"
#include "GraphicsHighlightItem.h"

#include "custom_logger.h"

SchemePlayer::SchemePlayer(XDecayPtr scheme, QObject *parent)
  : QObject(parent), scheme_(scheme),
    scene_(0)
{
}

QGraphicsScene * SchemePlayer::levelPlot()
{
  if (scene_)
    return scene_;

  scene_ = new QGraphicsScene(this);

  if (!scheme_)
    return scene_;

  // decide if parent nuclide should be printed on the left side (beta-),
  // on the right side (EC, beta+, alpha) or not at all (isomeric)
  ParentPosition parentpos = RightParent;
  if (scheme_->type() == XDecay::IsomericTransition)
    parentpos = NoParent;
  else if (scheme_->type() == XDecay::BetaMinus)
    parentpos = LeftParent;

  for (auto &level : scheme_->daughterNuclide()->levels()) {
    // create level
    LevelRendered *levrend = new LevelRendered(level.second, parentpos, vis, scene_);
    connect(this, SIGNAL(enabledShadow(bool)), levrend->graphicsItem(), SLOT(setShadowEnabled(bool)));
    connect(levrend->graphicsItem(), SIGNAL(clicked(ClickableItem*)), this, SLOT(itemClicked(ClickableItem*)));
    levels_.insert(levrend->energy_,levrend);
    // create gammas
    std::list<XGammaTransitionPtr> levelgammas = level.second->depopulatingTransitions();
    for (XGammaTransitionPtr gamma : levelgammas) {
      TransitionRendered *transrend = new TransitionRendered(gamma, vis, scene_);
//      ActiveGraphicsItemGroup *item = gamma->createGammaGraphicsItem(gammaFont, gammaPen, intenseGammaPen);
      connect(this, SIGNAL(enabledShadow(bool)), transrend->graphicsItem(), SLOT(setShadowEnabled(bool)));
      connect(transrend->graphicsItem(), SIGNAL(clicked(ClickableItem*)), this, SLOT(itemClicked(ClickableItem*)));
      transitions_.append(transrend);
    }
  }

  // create daughter nuclide label
  daughter_ = NuclideRendered(scheme_->daughterNuclide(), ClickableItem::DaughterNuclideType, vis, scene_);

  // create parent nuclide label and level(s)
  if (parentpos == LeftParent || parentpos == RightParent) {
    parent_ = NuclideRendered(scheme_->parentNuclide(), ClickableItem::ParentNuclideType, vis, scene_);

    for (auto &level : scheme_->parentNuclide()->levels()) {

      LevelRendered *levrend = new LevelRendered(level.second, NoParent, vis, scene_);
      parent_levels_.insert(levrend->energy_,levrend);


//      QFontMetrics stdBoldFontMetrics(vis.stdBoldFont);

//      level->graline = new QGraphicsLineItem;
//      level->graline->setPen(vis.stableLevelPen);
//      scene->addItem(level->graline);

//      level->graetext = new QGraphicsSimpleTextItem(QString::fromStdString(level->energy().to_string()));
//      level->graetext->setFont(vis.stdBoldFont);
//      scene->addItem(level->graetext);

//      level->graspintext = new QGraphicsSimpleTextItem(QString::fromStdString(level->spin().to_string()));
//      level->graspintext->setFont(vis.stdBoldFont);
//      scene->addItem(level->graspintext);
    }
  }

  alignGraphicsItems();
  return scene_;
}

void SchemePlayer::alignGraphicsItems()
{
  if (!scheme_)
    return;

//  QMap<Energy, EnergyLevel*> &levels(dNuc->levels());

//  DBG << "<SchemePlayer> levelsize " << levels.size();
  // decide if parent nuclide should be printed on the left side (beta-),
  // on the right side (EC, beta+, alpha) or not at all (isomeric)
  ParentPosition parentpos = RightParent;
  if (scheme_->type() == XDecay::IsomericTransition)
    parentpos = NoParent;
  else if (scheme_->type() == XDecay::BetaMinus)
    parentpos = LeftParent;

  QFontMetrics stdFontMetrics(vis.stdFont);
  QFontMetrics stdBoldFontMetrics(vis.stdBoldFont);
  QFontMetrics parentHlFontMetrics(vis.parentHlFont);
//  QFontMetrics feedIntensityFontMetrics(feedIntensityFont);

  // determine size information
  double maxEnergyLabelWidth = 0.0;
  double maxSpinLabelWidth = 0.0;

  foreach (LevelRendered *level, levels_) {
    if (stdBoldFontMetrics.width(level->graspintext->text()) > maxSpinLabelWidth)
      maxSpinLabelWidth = stdBoldFontMetrics.width(level->graspintext->text());
    if (stdBoldFontMetrics.width(level->graetext->text()) > maxEnergyLabelWidth)
      maxEnergyLabelWidth = stdBoldFontMetrics.width(level->graetext->text());
  }

  // determine y coordinates for all levels
  double maxEnergyGap = 0.0;
  if (!levels_.empty()) {
    for (QMap<Energy, LevelRendered*>::const_iterator i=levels_.begin()+1; i!=levels_.end(); i++) {
      double diff = i.key() - (i-1).key();
      maxEnergyGap = qMax(maxEnergyGap, diff);
    }
    for (QMap<Energy, LevelRendered*>::const_iterator i=levels_.begin()+1; i!=levels_.end(); i++) {
      double minheight = 0.5*(i.value()->graphicsItem()->boundingRect().height() + (i-1).value()->graphicsItem()->boundingRect().height());
      double extraheight = vis.maxExtraLevelDistance * (i.key() - (i-1).key()) / maxEnergyGap;
      i.value()->graYPos = std::floor((i-1).value()->graYPos - minheight - extraheight) + 0.5*i.value()->graline->pen().widthF();
    }
  }

  // determine space needed for gammas
  double gammaspace = std::numeric_limits<double>::quiet_NaN();
  for (auto &gamma : transitions_) {
    if (boost::math::isnan(gammaspace))
      gammaspace = gamma->widthFromOrigin();
    else
      gammaspace += gamma->minimalXDistance();
  }
  if (!boost::math::isfinite(gammaspace))
    gammaspace = 0.0;

  // set gamma positions
  double currentgammapos = 0.5*gammaspace;
  bool firstgamma = true;
  for (auto &gamma : transitions_) {
    if (firstgamma) {
      currentgammapos -= gamma->widthFromOrigin();
      firstgamma = false;
    }
    else {
      currentgammapos -= gamma->minimalXDistance();
    }

    if (levels_.contains(gamma->from_) && levels_.contains(gamma->to_)) {
      LevelRendered *from = levels_.value(gamma->from_);
      LevelRendered *to = levels_.value(gamma->to_);
      double arrowDestY = to->graYPos - from->graYPos;
      gamma->updateArrow(arrowDestY);
    }
    if (levels_.contains(gamma->from_)) {
      LevelRendered *from = levels_.value(gamma->from_);
      gamma->graphicsItem()->setPos(std::floor(currentgammapos)+0.5*gamma->pen().widthF(), from->graYPos + 0.5*from->graline->pen().widthF());
    }
  }

  // determine line length for parent levels
  double pNucLineLength = vis.parentNuclideLevelLineLength;
  if (parentpos == LeftParent || parentpos == RightParent) {
    pNucLineLength = qMax(vis.parentNuclideLevelLineLength, parent_.graphicsItem()->boundingRect().width() + 20.0);
    foreach (LevelRendered *level, parent_levels_) {
      pNucLineLength = qMax(pNucLineLength,
                            level->graetext->boundingRect().width() +
                            level->graspintext->boundingRect().width() +
                            vis.parentNuclideMinSpinEnergyDistance +
                            2.0 * vis.outerLevelTextMargin);
    }
  }
  pNucLineLength = std::ceil(pNucLineLength);

  // determine line length for feeding arrows
  double arrowLineLength = vis.feedingArrowLineLength;
  if (parentpos == LeftParent || parentpos == RightParent)
    foreach (LevelRendered *level, levels_)
      if (level->grafeedintens)
        arrowLineLength = qMax(arrowLineLength,
                               level->grafeedintens->boundingRect().width() +
                               vis.parentNuclideLevelLineExtraLength +
                               2.0 * vis.feedingArrowTextMargin);

  // calculate length of level lines
  double leftlinelength = vis.outerLevelTextMargin + maxSpinLabelWidth + vis.outerGammaMargin + 0.5*gammaspace;
  double rightlinelength = vis.outerLevelTextMargin + maxEnergyLabelWidth + vis.outerGammaMargin + 0.5*gammaspace;

  // calculate start and end points of parent level lines
  double arrowleft = std::floor((parentpos == RightParent) ? rightlinelength : -leftlinelength - arrowLineLength - vis.parentNuclideLevelLineExtraLength) - 0.5*vis.feedArrowPen.widthF();
  double arrowright = std::ceil((parentpos == RightParent) ? rightlinelength + arrowLineLength + vis.parentNuclideLevelLineExtraLength : -leftlinelength) + 0.5*vis.feedArrowPen.widthF();
  double activeleft = std::floor((parentpos == RightParent) ? arrowleft + arrowLineLength - pNucLineLength : arrowleft);
  double activeright = std::ceil((parentpos == RightParent) ? arrowright : arrowright - arrowLineLength + pNucLineLength);
  double normalleft = std::floor((parentpos == RightParent) ? activeleft : activeleft + vis.parentNuclideLevelLineExtraLength);
  double normalright = std::ceil((parentpos == RightParent) ? activeright - vis.parentNuclideLevelLineExtraLength : activeright);

  // set level positions and sizes
  double arrowVEnd = std::numeric_limits<double>::quiet_NaN();
  foreach (LevelRendered *level, levels_) {
    double newVEnd = level->align(leftlinelength, rightlinelength, arrowleft, arrowright, vis, parentpos);
    if (boost::math::isnan(arrowVEnd) && !boost::math::isnan(newVEnd))
      arrowVEnd = newVEnd;
  }

  // set position of daughter nuclide
  daughter_.graphicsItem()->setPos(-0.5*daughter_.graphicsItem()->boundingRect().width(), 0.3*daughter_.graphicsItem()->boundingRect().height());

  // set position of parent nuclide
  if (parentpos == LeftParent || parentpos == RightParent) {
    double parentY = std::numeric_limits<double>::quiet_NaN();
    if (!levels_.empty())
      parentY = (levels_.end()-1).value()->graphicsItem()->y() -
          (levels_.end()-1).value()->graphicsItem()->boundingRect().height() -
          parent_.graphicsItem()->boundingRect().height() - vis.parentNuclideToEnergyLevelsDistance;

    double parentcenter = (normalleft + normalright) / 2.0;

    parent_.graphicsItem()->setPos(parentcenter - 0.5*parent_.graphicsItem()->boundingRect().width(), parentY);

    // set position of parent levels
    double topMostLevel = 0.0;
    double y = qRound(parentY - 0.3*parent_.graphicsItem()->boundingRect().height()) + 0.5*vis.stableLevelPen.widthF();
    foreach (LevelRendered *level, parent_levels_) {
      bool feeding = false;
      if (scheme_->parentNuclide()->levels().count(level->energy_))
        feeding = scheme_->parentNuclide()->levels().at(level->energy_)->isFeedingLevel();
      double left = feeding ? activeleft : normalleft;
      double right = feeding ? activeright : normalright;

      level->graline->setLine(left, y, right, y);
      level->graetext->setPos((parent_levels_.size() == 1 ? activeright : normalright) - vis.outerLevelTextMargin - level->graetext->boundingRect().width(), y - level->graetext->boundingRect().height());
      level->graspintext->setPos((parent_levels_.size() == 1 ? activeleft : normalleft) + vis.outerLevelTextMargin, y - level->graetext->boundingRect().height());

      topMostLevel = y;

      // update y
      y -= qMax(level->graetext->boundingRect().height(), level->graspintext->boundingRect().height()) + 10.0;
    }

    double arrowVStart = topMostLevel - 0.5*vis.stableLevelPen.widthF();
    double arrowX = (parentpos == RightParent) ? activeright : activeleft;
    if (boost::math::isfinite(arrowVStart) && boost::math::isfinite(arrowVEnd))
      parent_.pNucVerticalArrow->setLine(arrowX, arrowVStart, arrowX, arrowVEnd);

    parent_.pNucHl->setPos(parentcenter - 0.5*parent_.pNucHl->boundingRect().width(),
                   topMostLevel - stdBoldFontMetrics.height() - parentHlFontMetrics.height() - 12.0);
  }
}


void SchemePlayer::setShadowEnabled(bool enable)
{
  emit enabledShadow(enable);
}

QString SchemePlayer::name() const
{
  return QString::fromStdString(scheme_->name());
}

SchemePlayer::CascadeIdentifier SchemePlayer::currentSelection() const
{
  CascadeIdentifier ci;
//  if (firstSelectedGamma.isValid()) {
//    ci.start = firstSelectedGamma->depopulatedLevel()->energy();
//    ci.pop = firstSelectedGamma->energy();
//    ci.intermediate = firstSelectedGamma->populatedLevel()->energy();
//  }
//  if (selectedEnergyLevel.isValid()) {
//    ci.intermediate = selectedEnergyLevel->energy();
//    ci.highlightIntermediate = true;
//  }
//  if (secondSelectedGamma.isValid()) {
//    ci.intermediate = secondSelectedGamma->depopulatedLevel()->energy();
//    ci.depop = secondSelectedGamma->energy();
//  }

  return ci;
}

void SchemePlayer::setCurrentSelection(const SchemePlayer::CascadeIdentifier &identifier)
{
//  if (firstSelectedGamma)
//    if (firstSelectedGamma->graphicsItem())
//      firstSelectedGamma->graphicsItem()->setHighlighted(false);
//  if (secondSelectedGamma)
//    if (secondSelectedGamma->graphicsItem())
//      secondSelectedGamma->graphicsItem()->setHighlighted(false);
//  if (selectedEnergyLevel)
//    if (selectedEnergyLevel->graphicsItem())
//      selectedEnergyLevel->graphicsItem()->setHighlighted(false);

//  EnergyLevel *start = 0;
//  if (identifier.start.isValid())
//    start = dNuc->levels().value(identifier.start);

//  EnergyLevel *inter = 0;
//  if (identifier.intermediate.isValid())
//    inter = dNuc->levels().value(identifier.intermediate);

//  if (start && identifier.pop) {
//    const QList<GammaTransition*> &gammas = start->depopulatingTransitions();
//    GammaTransition *gamma = 0;
//    for (int i=0; i<gammas.size(); i++) {
//      if (gammas.at(i)->energy() == identifier.pop) {
//        gamma = gammas.at(i);
//        break;
//      }
//    }
//    if (gamma) {
//      firstSelectedGamma = gamma;
//      if (firstSelectedGamma->graphicsItem())
//        firstSelectedGamma->graphicsItem()->setHighlighted(true);
//    }
//  }

//  if (inter && identifier.depop) {
//    const QList<GammaTransition*> &gammas = inter->depopulatingTransitions();
//    GammaTransition *gamma = 0;
//    for (int i=0; i<gammas.size(); i++) {
//      if (gammas.at(i)->energy() == identifier.depop) {
//        gamma = gammas.at(i);
//        break;
//      }
//    }
//    if (gamma) {
//      secondSelectedGamma = gamma;
//      if (secondSelectedGamma->graphicsItem())
//        secondSelectedGamma->graphicsItem()->setHighlighted(true);
//    }
//  }

//  if (inter && identifier.highlightIntermediate) {
//    selectedEnergyLevel = inter;
//    if (selectedEnergyLevel->graphicsItem())
//      selectedEnergyLevel->graphicsItem()->setHighlighted(true);
//  }

//  triggerSchemePlayerDataUpdate();
}

SchemePlayer::SchemePlayerDataSet SchemePlayer::decayDataSet() const
{
  SchemePlayerDataSet dataset;

//  // intermediate level
//  if (selectedEnergyLevel) {
//    dataset.intEnergy = QString::fromStdString(selectedEnergyLevel->energy().to_string());
//    dataset.intHalfLife = QString::fromStdString(selectedEnergyLevel->halfLife().to_string());
//    dataset.intSpin = QString::fromStdString(selectedEnergyLevel->spin().to_string());
//    dataset.intMu = QString::fromStdString(selectedEnergyLevel->mu().to_markup()).replace("undefined", "?");
//    dataset.intQ = QString::fromStdString(selectedEnergyLevel->q().to_markup()).replace("undefined", "?");
//  }

//  // gammas
//  GammaTransition *pop = 0, *depop = 0;
//  if (firstSelectedGamma) {
//    if (firstSelectedGamma->depopulatedLevel() == selectedEnergyLevel)
//      depop = firstSelectedGamma;
//    else
//      pop = firstSelectedGamma;
//  }
//  if (secondSelectedGamma) {
//    if (secondSelectedGamma->depopulatedLevel() == selectedEnergyLevel)
//      depop = secondSelectedGamma;
//    else
//      pop = secondSelectedGamma;
//  }

//  // populating
//  if (pop) {
//    dataset.popEnergy = QString::fromStdString(pop->energy().to_string());
//    dataset.popIntensity = pop->intensityAsText();
//    dataset.popMultipolarity = pop->multipolarityAsText();
//    dataset.popMixing = QString::fromStdString(pop->delta().to_string(true)).replace("undefined", "<i>unknown</i>");
//  }

//  // depopulating
//  if (depop) {
//    dataset.depopEnergy = QString::fromStdString(depop->energy().to_string());
//    dataset.depopIntensity = depop->intensityAsText();
//    dataset.depopMultipolarity = depop->multipolarityAsText();
//    dataset.depopMixing = QString::fromStdString(depop->delta().to_string(true)).replace("undefined", "<i>unknown</i>");
//  }

//  // start and end level
//  if (firstSelectedGamma && secondSelectedGamma) {
//    dataset.startEnergy = QString::fromStdString(pop->depopulatedLevel()->energy().to_string());
//    dataset.startSpin = QString::fromStdString(pop->depopulatedLevel()->spin().to_string());
//    dataset.endEnergy = QString::fromStdString(depop->populatedLevel()->energy().to_string());
//    dataset.endSpin = QString::fromStdString(depop->populatedLevel()->spin().to_string());
//  }

//  // calculate anisotropies
//  if (pop && depop && selectedEnergyLevel) {
//    if (pop->depopulatedLevel()->spin().valid() &&
//        depop->populatedLevel()->spin().valid() &&
//        selectedEnergyLevel->spin().valid() &&
//        pop->delta().hasFiniteValue() &&
//        depop->delta().hasFiniteValue()
//        ) {
//      // create list of sign combinations
//      typedef QPair<double, double> SignCombination;
//      QList< SignCombination > variants;
//      QList< double > popvariants;
//      if (pop->delta().sign() == UncertainDouble::MagnitudeDefined)
//        popvariants << 1. << -1.;
//      else
//        popvariants << ((pop->delta() < 0.0) ? -1. : 1.);
//      foreach (double popvariant, popvariants) {
//        if (depop->delta().sign() == UncertainDouble::MagnitudeDefined)
//          variants << QPair<double, double>(popvariant, 1.) << SignCombination(popvariant, -1.);
//        else
//          variants << SignCombination(popvariant, (depop->delta() < 0.0) ? -1. : 1.);
//      }

//      // compute possible results
//      QStringList a22str, a24str, a42str, a44str;
//      QVector<UncertainDouble> a22(variants.size()), a24(variants.size()), a42(variants.size()), a44(variants.size());
//      Akk calc;
//      calc.setInitialStateSpin(pop->depopulatedLevel()->spin().doubled_spin());
//      calc.setIntermediateStateSpin(selectedEnergyLevel->spin().doubled_spin());
//      calc.setFinalStateSpin(depop->populatedLevel()->spin().doubled_spin());

//      for (int i=0; i<variants.size(); i++) {
//        SignCombination variant = variants.at(i);
//        UncertainDouble popdelta(pop->delta());
//        UncertainDouble depopdelta(depop->delta());
//        if (pop->delta().sign() == UncertainDouble::MagnitudeDefined)
//          popdelta.setValue(popdelta.value() * variant.first, UncertainDouble::SignMagnitudeDefined);
//        if (depop->delta().sign() == UncertainDouble::MagnitudeDefined)
//          depopdelta.setValue(depopdelta.value() * variant.second, UncertainDouble::SignMagnitudeDefined);

//        calc.setPopulatingGammaMixing(popdelta.value(), std::max(popdelta.lowerUncertainty(), popdelta.upperUncertainty()));
//        calc.setDepopulatingGammaMixing(depopdelta.value(), std::max(depopdelta.lowerUncertainty(), depopdelta.upperUncertainty()));

//        // determine uncertainty type of results (may only be SymmetricUncertainty or Approximately)
//        UncertainDouble::UncertaintyType restype = UncertainDouble::SymmetricUncertainty;
//        if (popdelta.uncertaintyType() == UncertainDouble::Approximately || depopdelta.uncertaintyType() == UncertainDouble::Approximately)
//          restype = UncertainDouble::Approximately;

//        // determine prefix
//        QString prfx;
//        if (variants.size() > 1) {
//          prfx = QString::fromUtf8("%1: ");
//          prfx = prfx.arg(QString(variant.first < 0 ? "-" : "+") + QString(variant.second < 0 ? "-" : "+"));
//        }

//        a22[i].setValue(calc.a22(), UncertainDouble::SignMagnitudeDefined);
//        a22[i].setUncertainty(calc.a22Uncertainty(), calc.a22Uncertainty(), restype);
//        a24[i].setValue(calc.a24(), UncertainDouble::SignMagnitudeDefined);
//        a24[i].setUncertainty(calc.a24Uncertainty(), calc.a24Uncertainty(), restype);
//        a42[i].setValue(calc.a42(), UncertainDouble::SignMagnitudeDefined);
//        a42[i].setUncertainty(calc.a42Uncertainty(), calc.a42Uncertainty(), restype);
//        a44[i].setValue(calc.a44(), UncertainDouble::SignMagnitudeDefined);
//        a44[i].setUncertainty(calc.a44Uncertainty(), calc.a44Uncertainty(), restype);

//        a22str.append(QString("%1%2").arg(prfx).arg(QString::fromStdString(a22[i].to_markup())));
//        a24str.append(QString("%1%2").arg(prfx).arg(QString::fromStdString(a24[i].to_markup())));
//        a42str.append(QString("%1%2").arg(prfx).arg(QString::fromStdString(a42[i].to_markup())));
//        a44str.append(QString("%1%2").arg(prfx).arg(QString::fromStdString(a44[i].to_markup())));
//      }
//      // replace strings if all values are equal
//      if (variants.size() > 1) {
//        if (a22.count(a22[0]) == (variants.size())) {
//          a22str.clear();
//          a22str.append(QString::fromStdString(a22[0].to_markup()));
//        }
//        if (a24.count(a24[0]) == (variants.size())) {
//          a24str.clear();
//          a24str.append(QString::fromStdString(a24[0].to_markup()));
//        }
//        if (a42.count(a42[0]) == (variants.size())) {
//          a42str.clear();
//          a42str.append(QString::fromStdString(a42[0].to_markup()));
//        }
//        if (a44.count(a44[0]) == (variants.size())) {
//          a44str.clear();
//          a44str.append(QString::fromStdString(a44[0].to_markup()));
//        }
//      }

//      dataset.a22 = a22str.join(", ");
//      dataset.a24 = a24str.join(", ");
//      dataset.a42 = a42str.join(", ");
//      dataset.a44 = a44str.join(", ");
//    }
//  }

  return dataset;
}

void SchemePlayer::itemClicked(ClickableItem *item)
{
  if (item->type() == ClickableItem::EnergyLevelType)
    clickedEnergyLevel(dynamic_cast<LevelRendered*>(item));
  else if (item->type() == ClickableItem::GammaTransitionType)
    clickedGamma(dynamic_cast<TransitionRendered*>(item));
}

void SchemePlayer::clickedGamma(TransitionRendered *g)
{
  if (!g)
    return;

//  // deselect if active level is clicked again
//  if (g == firstSelectedGamma) {
//    firstSelectedGamma->graphicsItem()->setHighlighted(false);
//    firstSelectedGamma = secondSelectedGamma;
//    secondSelectedGamma = 0;
//  }
//  else if (g == secondSelectedGamma) {
//    secondSelectedGamma->graphicsItem()->setHighlighted(false);
//    secondSelectedGamma = 0;
//  }
//  else {
//    // deselect inappropriate level(s)
//    bool firstok = false;
//    if (firstSelectedGamma)
//      firstok = g->populatedLevel() == firstSelectedGamma->depopulatedLevel() ||
//          g->depopulatedLevel() == firstSelectedGamma->populatedLevel();
//    bool secondok = false;
//    if (secondSelectedGamma)
//      secondok = g->populatedLevel() == secondSelectedGamma->depopulatedLevel() ||
//          g->depopulatedLevel() == secondSelectedGamma->populatedLevel();

//    if (firstok && secondok) {
//      firstSelectedGamma->graphicsItem()->setHighlighted(false);
//      firstSelectedGamma = secondSelectedGamma;
//      secondSelectedGamma = 0;
//    }
//    else if (firstok) {
//      if (secondSelectedGamma) {
//        secondSelectedGamma->graphicsItem()->setHighlighted(false);
//        secondSelectedGamma = 0;
//      }
//    }
//    else if (secondok) {
//      Q_ASSERT(firstSelectedGamma);
//      firstSelectedGamma->graphicsItem()->setHighlighted(false);
//      firstSelectedGamma = secondSelectedGamma;
//      secondSelectedGamma = 0;
//    }
//    else {
//      if (firstSelectedGamma) {
//        firstSelectedGamma->graphicsItem()->setHighlighted(false);
//        firstSelectedGamma = 0;
//      }
//      if (secondSelectedGamma) {
//        secondSelectedGamma->graphicsItem()->setHighlighted(false);
//        secondSelectedGamma = 0;
//      }
//    }

//    // case: no gamma previously selected
//    if (!firstSelectedGamma && !secondSelectedGamma) {
//      firstSelectedGamma = g;
//      firstSelectedGamma->graphicsItem()->setHighlighted(true);
//      if (selectedEnergyLevel) {
//        if (g->populatedLevel() != selectedEnergyLevel && g->depopulatedLevel() != selectedEnergyLevel) {
//          selectedEnergyLevel->graphicsItem()->setHighlighted(false);
//          selectedEnergyLevel = 0;
//        }
//      }
//    }
//    // case: one gamma previously selected, common level exists
//    else {
//      secondSelectedGamma = g;
//      secondSelectedGamma->graphicsItem()->setHighlighted(true);
//      // select intermediate level
//      EnergyLevel *intermediate = firstSelectedGamma->populatedLevel();
//      if (firstSelectedGamma->depopulatedLevel() == secondSelectedGamma->populatedLevel())
//        intermediate = firstSelectedGamma->depopulatedLevel();
//      // activate intermediate level
//      if (selectedEnergyLevel)
//        if (selectedEnergyLevel != intermediate)
//          selectedEnergyLevel->graphicsItem()->setHighlighted(false);
//      selectedEnergyLevel = intermediate;
//      selectedEnergyLevel->graphicsItem()->setHighlighted(true);
//    }
//  }

  triggerDataUpdate();
}

void SchemePlayer::clickedEnergyLevel(LevelRendered *e)
{
  if (!e)
    return;

//  // deselect if clicked again
//  if (e == selectedEnergyLevel) {
//    selectedEnergyLevel->graphicsItem()->setHighlighted(false);
//    selectedEnergyLevel = 0;
//  }
//  // select otherwise
//  else {
//    // deselect old level
//    if (selectedEnergyLevel)
//      selectedEnergyLevel->graphicsItem()->setHighlighted(false);
//    selectedEnergyLevel = e;
//    e->graphicsItem()->setHighlighted(true);
//    // deselect gamma which is not connected to the level anymore
//    if (firstSelectedGamma) {
//      if (firstSelectedGamma->depopulatedLevel() != e && firstSelectedGamma->populatedLevel() != e) {
//        firstSelectedGamma->graphicsItem()->setHighlighted(false);
//        firstSelectedGamma = 0;
//      }
//    }
//    if (secondSelectedGamma) {
//      if (secondSelectedGamma->depopulatedLevel() != e && secondSelectedGamma->populatedLevel() != e) {
//        secondSelectedGamma->graphicsItem()->setHighlighted(false);
//        secondSelectedGamma = 0;
//      }
//    }
//    if (secondSelectedGamma && !firstSelectedGamma) {
//      firstSelectedGamma = secondSelectedGamma;
//      secondSelectedGamma = 0;
//    }
//  }
//  // prevent two gammas being active after the intermediate level was changed
//  if (firstSelectedGamma && secondSelectedGamma) {
//    firstSelectedGamma->graphicsItem()->setHighlighted(false);
//    firstSelectedGamma = secondSelectedGamma;
//    secondSelectedGamma = 0;
//  }

  triggerDataUpdate();
}

void SchemePlayer::triggerDataUpdate()
{
  emit updatedSchemePlayerData(decayDataSet());
}

void SchemePlayer::setStyle(const QFont &fontfamily, unsigned int sizePx)
{
  vis.setStyle(fontfamily, sizePx);
}

SchemePlayer::CascadeIdentifier::CascadeIdentifier()
  : highlightIntermediate(false)
{
}

SchemePlayer::SchemePlayerDataSet::SchemePlayerDataSet()
  : startEnergy("? keV"), startSpin("/"),
    popEnergy("? keV"), popIntensity("? %"), popMultipolarity("<i>unknown</i>"), popMixing("<i>unknown</i>"),
    intEnergy("? keV"), intHalfLife("? ns"), intSpin("/"), intMu("?"), intQ("?"),
    depopEnergy("? keV"), depopIntensity("? %"), depopMultipolarity("<i>unknown</i>"), depopMixing("<i>unknown</i>"),
    endEnergy("? keV"), endSpin("/"),
    a22("?"), a24("?"), a42("?"), a44("?")
{
}

//QDataStream & operator<<(QDataStream &out, const SchemePlayer::CascadeIdentifier &ident)
//{
//  out << ident.start << ident.pop << ident.intermediate << ident.depop << ident.highlightIntermediate;
//  return out;
//}

//QDataStream & operator>>(QDataStream &in, SchemePlayer::CascadeIdentifier &ident)
//{
//  in >> ident.start >> ident.pop >> ident.intermediate >> ident.depop >> ident.highlightIntermediate;
//  return in;
//}

