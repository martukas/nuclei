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
#include "ActiveGraphicsItemGroup.h"
#include "GraphicsHighlightItem.h"

#include "custom_logger.h"

#include "LevelItem.h"
#include "TransitionItem.h"


SchemePlayer::SchemePlayer(DecayScheme scheme, QObject *parent)
  : QObject(parent), scheme_(scheme),
    scene_(0)
{
  // decide if parent nuclide should be printed on the left side (beta-),
  // on the right side (EC, beta+, alpha) or not at all (isomeric)
  if (scheme_.type() == DecayScheme::IsomericTransition)
    vis.parentpos = NoParent;
  else if (scheme_.type() == DecayScheme::BetaMinus)
    vis.parentpos = LeftParent;
}

QGraphicsScene * SchemePlayer::levelPlot()
{
  if (scene_)
    return scene_;

  scene_ = new QGraphicsScene(this);

  if (scheme_.type() == DecayScheme::Undefined)
    return scene_;

  auto transitions = scheme_.daughterNuclide().transitions();
  for (auto &level : scheme_.daughterNuclide().levels())
  {
    // create level
    LevelItem *levrend = new LevelItem(level.second, vis, scene_);
    connect(this, SIGNAL(enabledShadow(bool)), levrend->graphicsItem(), SLOT(setShadowEnabled(bool)));
    connect(levrend->graphicsItem(), SIGNAL(clicked(ClickableItem*)), this, SLOT(itemClicked(ClickableItem*)));
    levels_[levrend->energy_] = levrend;
    // create gammas
    for (auto gamma_nrg : level.second.depopulatingTransitions())
    {
      if (!transitions.count(gamma_nrg))
        continue;
      Transition gamma = transitions.at(gamma_nrg);
      TransitionItem *transrend = new TransitionItem(gamma, vis, scene_);
      //      ActiveGraphicsItemGroup *item = gamma->createGammaGraphicsItem(gammaFont, gammaPen, intenseGammaPen);
      connect(this, SIGNAL(enabledShadow(bool)), transrend->graphicsItem(), SLOT(setShadowEnabled(bool)));
      connect(transrend->graphicsItem(), SIGNAL(clicked(ClickableItem*)), this, SLOT(itemClicked(ClickableItem*)));
      transitions_.append(transrend);
    }
  }

  // create daughter nuclide label
  daughter_ = NuclideItem(scheme_.daughterNuclide(), ClickableItem::DaughterNuclideType, vis, scene_);

  // create parent nuclide label and level(s)
  if (vis.parentpos != NoParent) {
    parent_ = NuclideItem(scheme_.parentNuclide(), ClickableItem::ParentNuclideType, vis, scene_);
    for (auto &level : scheme_.parentNuclide().levels()) {
      SchemeVisualSettings vis_ovrd = vis;
      vis_ovrd.parentpos = NoParent;
      LevelItem *levrend = new LevelItem(level.second, vis, scene_);
      parent_levels_[levrend->energy_] = levrend;
    }
  }

  alignGraphicsItems();
  return scene_;
}

void SchemePlayer::alignGraphicsItems()
{
  if (scheme_.type() == DecayScheme::Undefined)
    return;

  //  DBG << "<SchemePlayer> levelsize " << levels.size();

  QFontMetrics stdFontMetrics(vis.stdFont);
  QFontMetrics stdBoldFontMetrics(vis.stdBoldFont);
  QFontMetrics parentHlFontMetrics(vis.parentHlFont);
  //  QFontMetrics feedIntensityFontMetrics(feedIntensityFont);

  // determine size information
  double maxEnergyLabelWidth = 0.0;
  double maxSpinLabelWidth = 0.0;

  for (auto level : levels_)
  {
    if (stdBoldFontMetrics.width(level.second->graspintext->text()) > maxSpinLabelWidth)
      maxSpinLabelWidth = stdBoldFontMetrics.width(level.second->graspintext->text());
    if (stdBoldFontMetrics.width(level.second->graetext->text()) > maxEnergyLabelWidth)
      maxEnergyLabelWidth = stdBoldFontMetrics.width(level.second->graetext->text());
  }

  // determine y coordinates for all levels
  double maxEnergyGap = 0.0;
  if (!levels_.empty())
  {
    Energy prev_energy = levels_.begin()->first;
    for (auto i : levels_)
    {
      double diff = i.first - prev_energy;
      maxEnergyGap = qMax(maxEnergyGap, diff);
      prev_energy = i.first;
    }
    auto prev_level = levels_.begin()->second;
    prev_energy = levels_.begin()->first;
    for (auto i : levels_)
    {
      double minheight = 0.5*(i.second->graphicsItem()->boundingRect().height() + prev_level->graphicsItem()->boundingRect().height());
      double extraheight = vis.maxExtraLevelDistance * (i.first - prev_energy) / maxEnergyGap;
      i.second->graYPos = std::floor(prev_level->graYPos - minheight - extraheight) + 0.5 * prev_level->graline->pen().widthF();
      prev_energy = i.first;
      prev_level = i.second;
    }
  }

  // determine space needed for gammas
  double gammaspace = std::numeric_limits<double>::quiet_NaN();
  for (auto &gamma : transitions_)
  {
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
  for (auto &gamma : transitions_)
  {
    if (firstgamma)
    {
      currentgammapos -= gamma->widthFromOrigin();
      firstgamma = false;
    }
    else
      currentgammapos -= gamma->minimalXDistance();

    if (levels_.count(gamma->from_) && levels_.count(gamma->to_))
    {
      double arrowDestY = levels_.at(gamma->to_)->graYPos - levels_.at(gamma->from_)->graYPos;
      gamma->updateArrow(arrowDestY);
    }

    if (levels_.count(gamma->from_))
      gamma->graphicsItem()->setPos(std::floor(currentgammapos) + 0.5 * gamma->pen().widthF(),
                                    levels_.at(gamma->from_)->graYPos + 0.5 * levels_.at(gamma->from_)->graline->pen().widthF());
  }

  // determine line length for parent levels
  double pNucLineLength = vis.parentNuclideLevelLineLength;
  if (vis.parentpos != NoParent)
  {
    pNucLineLength = qMax(vis.parentNuclideLevelLineLength, parent_.graphicsItem()->boundingRect().width() + 20.0);
    for (auto level : parent_levels_)
    {
      pNucLineLength = qMax(pNucLineLength,
                            level.second->graetext->boundingRect().width() +
                            level.second->graspintext->boundingRect().width() +
                            vis.parentNuclideMinSpinEnergyDistance +
                            2.0 * vis.outerLevelTextMargin);
    }
  }
  pNucLineLength = std::ceil(pNucLineLength);

  // determine line length for feeding arrows
  double arrowLineLength = vis.feedingArrowLineLength;
  if (vis.parentpos != NoParent)
    for (auto level : levels_)
      if (level.second->grafeedintens)
        arrowLineLength = qMax(arrowLineLength,
                               level.second->grafeedintens->boundingRect().width() +
                               vis.parentNuclideLevelLineExtraLength +
                               2.0 * vis.feedingArrowTextMargin);

  // calculate length of level lines
  double leftlinelength = vis.outerLevelTextMargin + maxSpinLabelWidth + vis.outerGammaMargin + 0.5*gammaspace;
  double rightlinelength = vis.outerLevelTextMargin + maxEnergyLabelWidth + vis.outerGammaMargin + 0.5*gammaspace;

  // calculate start and end points of parent level lines
  double arrowleft = std::floor((vis.parentpos == RightParent) ? rightlinelength : -leftlinelength - arrowLineLength - vis.parentNuclideLevelLineExtraLength) - 0.5*vis.feedArrowPen.widthF();
  double arrowright = std::ceil((vis.parentpos == RightParent) ? rightlinelength + arrowLineLength + vis.parentNuclideLevelLineExtraLength : -leftlinelength) + 0.5*vis.feedArrowPen.widthF();
  double activeleft = std::floor((vis.parentpos == RightParent) ? arrowleft + arrowLineLength - pNucLineLength : arrowleft);
  double activeright = std::ceil((vis.parentpos == RightParent) ? arrowright : arrowright - arrowLineLength + pNucLineLength);
  double normalleft = std::floor((vis.parentpos == RightParent) ? activeleft : activeleft + vis.parentNuclideLevelLineExtraLength);
  double normalright = std::ceil((vis.parentpos == RightParent) ? activeright - vis.parentNuclideLevelLineExtraLength : activeright);

  // set level positions and sizes
  double arrowVEnd = std::numeric_limits<double>::quiet_NaN();
  for (auto level : levels_)
  {
    double newVEnd = level.second->align(leftlinelength, rightlinelength, arrowleft, arrowright, vis);
    if (boost::math::isnan(arrowVEnd) && !boost::math::isnan(newVEnd))
      arrowVEnd = newVEnd;
  }

  // set position of daughter nuclide
  daughter_.graphicsItem()->setPos(-0.5*daughter_.graphicsItem()->boundingRect().width(), 0.3*daughter_.graphicsItem()->boundingRect().height());

  // set position of parent nuclide
  if (vis.parentpos != NoParent)
  {
    double parentY = std::numeric_limits<double>::quiet_NaN();
    if (!levels_.empty())
      parentY = levels_.rend()->second->graphicsItem()->y() -
          levels_.rend()->second->graphicsItem()->boundingRect().height() -
          parent_.graphicsItem()->boundingRect().height() - vis.parentNuclideToEnergyLevelsDistance;

    double parentcenter = (normalleft + normalright) / 2.0;

    parent_.graphicsItem()->setPos(parentcenter - 0.5*parent_.graphicsItem()->boundingRect().width(), parentY);

    // set position of parent levels
    double topMostLevel = 0.0;
    double y = qRound(parentY - 0.3*parent_.graphicsItem()->boundingRect().height()) + 0.5*vis.stableLevelPen.widthF();
    for (auto level : parent_levels_)
    {
      bool feeding = false;
      if (scheme_.parentNuclide().levels().count(level.second->energy_))
        feeding = scheme_.parentNuclide().levels().at(level.second->energy_).isFeedingLevel();
      double left = feeding ? activeleft : normalleft;
      double right = feeding ? activeright : normalright;

      level.second->graline->setLine(left, y, right, y);
      level.second->graetext->setPos((parent_levels_.size() == 1 ? activeright : normalright) - vis.outerLevelTextMargin - level.second->graetext->boundingRect().width(), y - level.second->graetext->boundingRect().height());
      level.second->graspintext->setPos((parent_levels_.size() == 1 ? activeleft : normalleft) + vis.outerLevelTextMargin, y - level.second->graetext->boundingRect().height());

      topMostLevel = y;

      // update y
      y -= qMax(level.second->graetext->boundingRect().height(), level.second->graspintext->boundingRect().height()) + 10.0;
    }

    double arrowVStart = topMostLevel - 0.5*vis.stableLevelPen.widthF();
    double arrowX = (vis.parentpos == RightParent) ? activeright : activeleft;
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
  return QString::fromStdString(scheme_.name());
}


void SchemePlayer::itemClicked(ClickableItem *item)
{
  if (item->type() == ClickableItem::EnergyLevelType)
    clickedEnergyLevel(dynamic_cast<LevelItem*>(item));
  else if (item->type() == ClickableItem::GammaTransitionType)
    clickedGamma(dynamic_cast<TransitionItem*>(item));
}

void SchemePlayer::clickedGamma(TransitionItem *g)
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

void SchemePlayer::clickedEnergyLevel(LevelItem *e)
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
//  emit updatedData(decayDataSet());
}

void SchemePlayer::setStyle(const QFont &fontfamily, unsigned int sizePx)
{
  vis.setStyle(fontfamily, sizePx);
}
