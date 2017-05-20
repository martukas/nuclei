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
  : QObject(parent)
  , scheme_(scheme)
{
  // decide if parent nuclide should be printed on the left side (beta-),
  // on the right side (EC, beta+, alpha) or not at all (isomeric)
  if (scheme_.mode().isomeric())
    visual_settings_.parentpos = NoParent;
  else if (scheme_.mode().beta_minus())
    visual_settings_.parentpos = LeftParent;
  else
    visual_settings_.parentpos = RightParent;

//  DBG << "Creating scheme player for:\n" << scheme_.to_string();
}

QGraphicsScene * SchemePlayer::levelPlot()
{
  if (scene_)
    return scene_;

  scene_ = new QGraphicsScene(this);

  if (!scheme_.mode().valid())
    return scene_;

  auto transitions = scheme_.daughterNuclide().transitions();
  for (auto &level : scheme_.daughterNuclide().levels())
  {
    addLevel(level.second, visual_settings_);
    for (auto gamma_nrg : level.second.depopulatingTransitions())
      addTransition(transitions.at(gamma_nrg), visual_settings_);
  }

  daughter_ = NuclideItem(scheme_.daughterNuclide(), ClickableItem::DaughterNuclideType, visual_settings_, scene_);

  if (visual_settings_.parentpos != NoParent)
  {
    SchemeVisualSettings parent_visual_settings = visual_settings_;
    parent_visual_settings.parentpos = NoParent;

    parent_ = NuclideItem(scheme_.parentNuclide(), ClickableItem::ParentNuclideType, parent_visual_settings, scene_);
    for (auto &level : scheme_.parentNuclide().levels())
      addParentLevel(level.second, parent_visual_settings);
  }

  alignGraphicsItems();
  return scene_;
}

void SchemePlayer::addLevel(Level level, SchemeVisualSettings vis)
{
  LevelItem *levrend = new LevelItem(level, vis, scene_);
  connect(this, SIGNAL(enabledShadow(bool)), levrend->graphicsItem(), SLOT(setShadowEnabled(bool)));
  connect(levrend->graphicsItem(), SIGNAL(clicked(ClickableItem*)), this, SLOT(itemClicked(ClickableItem*)));
  levels_[level.energy()] = levrend;
}

void SchemePlayer::addParentLevel(Level level, SchemeVisualSettings vis)
{
  LevelItem *levrend = new LevelItem(level, vis, scene_);
  connect(this, SIGNAL(enabledShadow(bool)), levrend->graphicsItem(), SLOT(setShadowEnabled(bool)));
  connect(levrend->graphicsItem(), SIGNAL(clicked(ClickableItem*)), this, SLOT(itemClicked(ClickableItem*)));
  parent_levels_[level.energy()] = levrend;
}

void SchemePlayer::addTransition(Transition transition, SchemeVisualSettings vis)
{
  TransitionItem *transrend = new TransitionItem(transition, vis, scene_);
  connect(this, SIGNAL(enabledShadow(bool)), transrend->graphicsItem(), SLOT(setShadowEnabled(bool)));
  connect(transrend->graphicsItem(), SIGNAL(clicked(ClickableItem*)), this, SLOT(itemClicked(ClickableItem*)));
  transitions_.append(transrend);
}

void SchemePlayer::alignGraphicsItems()
{
  if (!scheme_.mode().valid())
    return;

  QFontMetrics stdFontMetrics(visual_settings_.stdFont);
  QFontMetrics stdBoldFontMetrics(visual_settings_.stdBoldFont);
  QFontMetrics parentHlFontMetrics(visual_settings_.parentHlFont);
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
    if (!maxEnergyGap)
      maxEnergyGap = 1;
    auto prev_level = levels_.begin()->second;
    prev_energy = levels_.begin()->first;
    for (auto i : levels_)
    {
      double minheight = 0.5*(i.second->graphicsItem()->boundingRect().height() + prev_level->graphicsItem()->boundingRect().height());
      double extraheight = visual_settings_.maxExtraLevelDistance * (i.first - prev_energy) / maxEnergyGap;

      i.second->graYPos = std::floor(prev_level->graYPos - minheight - extraheight) + 0.5 * prev_level->graline->pen().widthF();
      prev_energy = i.first;
      prev_level = i.second;
    }
  }

  // determine space needed for gammas
  double gammaspace = std::numeric_limits<double>::quiet_NaN();
  double max_intensity = 0;
  for (auto &gamma : transitions_)
  {
    if (boost::math::isnan(gammaspace))
      gammaspace = gamma->widthFromOrigin();
    else
      gammaspace += gamma->minimalXDistance();
    if (gamma->transition_.intensity().hasFiniteValue())
      max_intensity = std::max(max_intensity, gamma->transition_.intensity().value());
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

    if (levels_.count(gamma->transition_.from()) && levels_.count(gamma->transition_.to()))
    {
      double arrowDestY = levels_.at(gamma->transition_.to())->graYPos
          - levels_.at(gamma->transition_.from())->graYPos;
      gamma->updateArrow(arrowDestY, max_intensity);
    }

    if (levels_.count(gamma->transition_.from()))
      gamma->graphicsItem()->setPos(std::floor(currentgammapos) + 0.5 * gamma->pen().widthF(),
                                    levels_.at(gamma->transition_.from())->graYPos + 0.5
                                    * levels_.at(gamma->transition_.from())->graline->pen().widthF());
  }

  // determine line length for parent levels
  double pNucLineLength = visual_settings_.parentNuclideLevelLineLength;
  if (visual_settings_.parentpos != NoParent)
  {
    pNucLineLength = qMax(visual_settings_.parentNuclideLevelLineLength, parent_.graphicsItem()->boundingRect().width() + 20.0);
    for (auto level : parent_levels_)
    {
      pNucLineLength = qMax(pNucLineLength,
                            level.second->graetext->boundingRect().width() +
                            level.second->graspintext->boundingRect().width() +
                            visual_settings_.parentNuclideMinSpinEnergyDistance +
                            2.0 * visual_settings_.outerLevelTextMargin);
    }
  }
  pNucLineLength = std::ceil(pNucLineLength);

  // determine line length for feeding arrows
  double arrowLineLength = visual_settings_.feedingArrowLineLength;
  if (visual_settings_.parentpos != NoParent)
    for (auto level : levels_)
      if (level.second->grafeedintens)
        arrowLineLength = qMax(arrowLineLength,
                               level.second->grafeedintens->boundingRect().width() +
                               visual_settings_.parentNuclideLevelLineExtraLength +
                               2.0 * visual_settings_.feedingArrowTextMargin);

  // calculate length of level lines
  double leftlinelength = visual_settings_.outerLevelTextMargin + maxSpinLabelWidth + visual_settings_.outerGammaMargin + 0.5*gammaspace;
  double rightlinelength = visual_settings_.outerLevelTextMargin + maxEnergyLabelWidth + visual_settings_.outerGammaMargin + 0.5*gammaspace;

  // calculate start and end points of parent level lines
  double arrowleft = std::floor((visual_settings_.parentpos == RightParent) ? rightlinelength : -leftlinelength - arrowLineLength - visual_settings_.parentNuclideLevelLineExtraLength) - 0.5*visual_settings_.feedArrowPen.widthF();
  double arrowright = std::ceil((visual_settings_.parentpos == RightParent) ? rightlinelength + arrowLineLength + visual_settings_.parentNuclideLevelLineExtraLength : -leftlinelength) + 0.5*visual_settings_.feedArrowPen.widthF();
  double activeleft = std::floor((visual_settings_.parentpos == RightParent) ? arrowleft + arrowLineLength - pNucLineLength : arrowleft);
  double activeright = std::ceil((visual_settings_.parentpos == RightParent) ? arrowright : arrowright - arrowLineLength + pNucLineLength);
  double normalleft = std::floor((visual_settings_.parentpos == RightParent) ? activeleft : activeleft + visual_settings_.parentNuclideLevelLineExtraLength);
  double normalright = std::ceil((visual_settings_.parentpos == RightParent) ? activeright - visual_settings_.parentNuclideLevelLineExtraLength : activeright);

  // set level positions and sizes
  double arrowVEnd = std::numeric_limits<double>::quiet_NaN();
  for (auto level : levels_)
  {
    double newVEnd = level.second->align(leftlinelength, rightlinelength, arrowleft, arrowright, visual_settings_);
    if (boost::math::isnan(arrowVEnd) && !boost::math::isnan(newVEnd))
      arrowVEnd = newVEnd;
  }

  // set position of daughter nuclide
  daughter_.graphicsItem()->setPos(-0.5*daughter_.graphicsItem()->boundingRect().width(),
                                   0.3*daughter_.graphicsItem()->boundingRect().height());

  // set position of parent nuclide
  if (visual_settings_.parentpos != NoParent)
  {
    double parentY = std::numeric_limits<double>::quiet_NaN();
    if (!levels_.empty())
      parentY = levels_.rbegin()->second->graphicsItem()->y() -
          levels_.rbegin()->second->graphicsItem()->boundingRect().height() -
          parent_.graphicsItem()->boundingRect().height() - visual_settings_.parentNuclideToEnergyLevelsDistance;

    double parentcenter = (normalleft + normalright) / 2.0;

    parent_.graphicsItem()->setPos(parentcenter - 0.5*parent_.graphicsItem()->boundingRect().width(), parentY);

    // set position of parent levels
    double topMostLevel = 0.0;
    double y = qRound(parentY - 0.3*parent_.graphicsItem()->boundingRect().height()) + 0.5*visual_settings_.stableLevelPen.widthF();
    for (auto level : parent_levels_)
    {
      bool feeding = false;
      if (scheme_.parentNuclide().levels().count(level.second->energy_))
        feeding = scheme_.parentNuclide().levels().at(level.second->energy_).isFeedingLevel();
      double left = feeding ? activeleft : normalleft;
      double right = feeding ? activeright : normalright;

      level.second->graline->setLine(left, y, right, y);
      level.second->graetext->setPos((parent_levels_.size() == 1 ? activeright : normalright) - visual_settings_.outerLevelTextMargin - level.second->graetext->boundingRect().width(), y - level.second->graetext->boundingRect().height());
      level.second->graspintext->setPos((parent_levels_.size() == 1 ? activeleft : normalleft) + visual_settings_.outerLevelTextMargin, y - level.second->graetext->boundingRect().height());

      topMostLevel = y;

      // update y
      y -= qMax(level.second->graetext->boundingRect().height(), level.second->graspintext->boundingRect().height()) + 10.0;
    }

    double arrowVStart = topMostLevel - 0.5*visual_settings_.stableLevelPen.widthF();
    double arrowX = (visual_settings_.parentpos == RightParent) ? activeright : activeleft;
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

  DBG << "Clicked transition " << g->transition_.energy().to_string();

  triggerDataUpdate();
}

void SchemePlayer::clickedEnergyLevel(LevelItem *e)
{
  if (!e)
    return;

  DBG << "Clicked level " << e->energy_.to_string();

  triggerDataUpdate();
}

void SchemePlayer::triggerDataUpdate()
{
  //  emit updatedData(decayDataSet());
}

void SchemePlayer::setStyle(const QFont &fontfamily, unsigned int sizePx)
{
  visual_settings_.setStyle(fontfamily, sizePx);
}
