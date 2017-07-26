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
  if (scheme_.decay_info().mode.isomeric() ||
      !scheme_.decay_info().valid())
    visual_settings_.parentpos = NoParent;
  else if (scheme_.decay_info().mode.beta_minus())
    visual_settings_.parentpos = LeftParent;
  else
    visual_settings_.parentpos = RightParent;

  //  DBG << "Creating scheme player for:\n" << scheme_.to_string();
}

const DecayScheme& SchemePlayer::scheme() const
{
  return scheme_;
}

bool SchemePlayer::parent_selected() const
{
  return parent_selected_;
}

bool SchemePlayer::daughter_selected() const
{
  return daughter_selected_;
}

QGraphicsScene* SchemePlayer::levelPlot()
{
  if (scene_)
    return scene_;

  scene_ = new QGraphicsScene(this);

  if (!scheme_.valid())
    return scene_;

  auto transitions = scheme_.daughterNuclide().transitions();
  auto levels = scheme_.daughterNuclide().levels();
  for (auto level = levels.begin(); level != levels.end(); ++level)
  {
    addLevel(level->second, visual_settings_);
    auto depoptrans = level->second.depopulatingTransitions();

    for (auto it = depoptrans.begin(); it != depoptrans.end(); ++it)
      addTransition(transitions.at(*it), visual_settings_);
  }

  if (!scheme_.daughterNuclide().empty())
  {
    daughter_ = new NuclideItem(scheme_.daughterNuclide(),
                                ClickableItem::DaughterNuclideType,
                                visual_settings_, scene_);
    connect(this, SIGNAL(enabledShadow(bool)),
            daughter_->graphicsItem(), SLOT(setShadowEnabled(bool)));
    connect(daughter_->graphicsItem(), SIGNAL(clicked(ClickableItem*)), this, SLOT(itemClicked(ClickableItem*)));
  }

  if (visual_settings_.parentpos != NoParent)
  {
    SchemeVisualSettings parent_visual_settings = visual_settings_;
    parent_visual_settings.parentpos = NoParent;

    parent_ = new NuclideItem(scheme_.parentNuclide(),
                              ClickableItem::ParentNuclideType,
                              parent_visual_settings, scene_);

    connect(this, SIGNAL(enabledShadow(bool)),
            parent_->graphicsItem(), SLOT(setShadowEnabled(bool)));
    connect(parent_->graphicsItem(), SIGNAL(clicked(ClickableItem*)),
            this, SLOT(itemClicked(ClickableItem*)));

    for (auto &level : scheme_.parentNuclide().levels())
      addParentLevel(level.second, parent_visual_settings);
  }

  alignGraphicsItems();

  return scene_;
}

void SchemePlayer::addLevel(Level level, SchemeVisualSettings vis)
{
  LevelItem *levrend = new LevelItem(level,
                                     LevelItem::DaughterLevelType,
                                     vis, scene_);
  connect(this, SIGNAL(enabledShadow(bool)), levrend->graphicsItem(), SLOT(setShadowEnabled(bool)));
  connect(levrend->graphicsItem(), SIGNAL(clicked(ClickableItem*)), this, SLOT(itemClicked(ClickableItem*)));
  levels_[level.energy()] = levrend;
}

void SchemePlayer::addParentLevel(Level level, SchemeVisualSettings vis)
{
  LevelItem *levrend = new LevelItem(level,
                                     LevelItem::ParentLevelType,
                                     vis, scene_);
  connect(this, SIGNAL(enabledShadow(bool)), levrend->graphicsItem(), SLOT(setShadowEnabled(bool)));
  connect(levrend->graphicsItem(), SIGNAL(clicked(ClickableItem*)), this, SLOT(itemClicked(ClickableItem*)));
  parent_levels_[level.energy()] = levrend;
}

void SchemePlayer::addTransition(Transition transition, SchemeVisualSettings vis)
{
  TransitionItem *transrend = new TransitionItem(transition, vis, scene_);
  connect(this, SIGNAL(enabledShadow(bool)), transrend->graphicsItem(), SLOT(setShadowEnabled(bool)));
  connect(transrend->graphicsItem(), SIGNAL(clicked(ClickableItem*)), this, SLOT(itemClicked(ClickableItem*)));
  transitions_.push_back(transrend);
}

void SchemePlayer::alignGraphicsItems()
{
  if (!scheme_.valid())
    return;

  QFontMetrics stdFontMetrics(visual_settings_.stdFont);
  QFontMetrics stdBoldFontMetrics(visual_settings_.stdBoldFont);
  QFontMetrics parentHlFontMetrics(visual_settings_.parentHlFont);
  //  QFontMetrics feedIntensityFontMetrics(feedIntensityFont);

  // determine size information
  int maxEnergyLabelWidth {0};
  int maxSpinLabelWidth {0};

  for (auto level : levels_)
  {
    maxSpinLabelWidth
        = qMax(maxSpinLabelWidth, level.second->spin_width());
    maxEnergyLabelWidth
        = qMax(maxEnergyLabelWidth, level.second->energy_width());
  }

  // determine y coordinates for all levels
  if (!levels_.empty())
  {
    double maxEnergyGap = 0.0;
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
      double minheight
          = 0.5*(i.second->graphicsItem()->boundingRect().height()
                 + prev_level->graphicsItem()->boundingRect().height());
      double extraheight
          = visual_settings_.maxExtraLevelDistance
          * (i.first - prev_energy) / maxEnergyGap;

      i.second->set_ypos(prev_level->above_ypos(minheight + extraheight));
      prev_energy = i.first;
      prev_level = i.second;
    }
  }

  // determine space needed for gammas
  double gammaspace {0};
  double max_intensity {0};
  for (auto &gamma : transitions_)
  {
    if (!gammaspace)
      gammaspace = gamma->widthFromOrigin();
    gammaspace += gamma->minimalXDistance();
    max_intensity = qMax(max_intensity, gamma->intensity());
  }

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

    if (levels_.count(gamma->from())
        && levels_.count(gamma->to()))
    {
      double arrowDestY =
          levels_.at(gamma->to())->ypos()
          - levels_.at(gamma->from())->ypos();
      gamma->updateArrow(arrowDestY, max_intensity);
    }

    if (levels_.count(gamma->from()))
      gamma->graphicsItem()->setPos(
            std::floor(currentgammapos) + 0.5 * gamma->pen().widthF(),
            levels_.at(gamma->from())->bottom_ypos());
  }

  // determine line length for parent levels
  double pNucLineLength = visual_settings_.parentNuclideLevelLineLength;
  if (parent_ && (visual_settings_.parentpos != NoParent))
  {
    pNucLineLength
        = qMax(visual_settings_.parentNuclideLevelLineLength,
               parent_->graphicsItem()->boundingRect().width() + 20.0);
    for (auto level : parent_levels_)
      pNucLineLength
          = qMax(pNucLineLength, level.second->nuc_line_width()
                 + visual_settings_.parentNuclideMinSpinEnergyDistance
                 + 2.0 * visual_settings_.outerLevelTextMargin);
  }
  pNucLineLength = std::ceil(pNucLineLength);

  // determine line length for feeding arrows
  double arrowLineLength = visual_settings_.feedingArrowLineLength;
  if (visual_settings_.parentpos != NoParent)
    for (auto level : levels_)
      arrowLineLength = qMax(arrowLineLength,
                             level.second->feed_intensity_width()
                             + visual_settings_.parentNuclideLevelLineExtraLength
                             + 2.0 * visual_settings_.feedingArrowTextMargin);

  // calculate length of level lines
  double leftlinelength = visual_settings_.outerLevelTextMargin
      + maxSpinLabelWidth + visual_settings_.outerGammaMargin
      + 0.5*gammaspace;
  double rightlinelength = visual_settings_.outerLevelTextMargin
      + maxEnergyLabelWidth + visual_settings_.outerGammaMargin
      + 0.5*gammaspace;

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
    double newVEnd
        = level.second->align(leftlinelength, rightlinelength,
                              arrowleft, arrowright,
                              visual_settings_);
    if (boost::math::isnan(arrowVEnd) && !boost::math::isnan(newVEnd))
      arrowVEnd = newVEnd;
  }

  // set position of daughter nuclide
  if (daughter_ && !scheme_.daughterNuclide().empty())
    daughter_->graphicsItem()->setPos(-0.5*daughter_->graphicsItem()->boundingRect().width(),
                                      0.3*daughter_->graphicsItem()->boundingRect().height());

  // set position of parent nuclide
  if (parent_ && (visual_settings_.parentpos != NoParent))
  {
    double parentY = std::numeric_limits<double>::quiet_NaN();
    if (!levels_.empty())
      parentY = levels_.rbegin()->second->graphicsItem()->y() -
          levels_.rbegin()->second->graphicsItem()->boundingRect().height() -
          parent_->graphicsItem()->boundingRect().height() - visual_settings_.parentNuclideToEnergyLevelsDistance;

    double parentcenter = (normalleft + normalright) / 2.0;

    parent_->graphicsItem()->setPos(parentcenter - 0.5*parent_->graphicsItem()->boundingRect().width(), parentY);

    // set position of parent levels
    double topMostLevel = 0.0;
    double y = qRound(parentY - 0.3*parent_->graphicsItem()->boundingRect().height()) + 0.5*visual_settings_.stableLevelPen.widthF();
    for (auto level : parent_levels_)
    {
      bool feeding = false;
      if (scheme_.parentNuclide().levels().count(level.second->energy()))
        feeding = scheme_.parentNuclide().levels().at(level.second->energy()).isFeedingLevel();
      double left = feeding ? activeleft : normalleft;
      double right = feeding ? activeright : normalright;

      level.second->set_funky_position(left, right, y, visual_settings_);
      level.second->set_funky2_position(
            (parent_levels_.size() == 1 ? activeright : normalright)
            - visual_settings_.outerLevelTextMargin,
            (parent_levels_.size() == 1 ? activeleft : normalleft)
            + visual_settings_.outerLevelTextMargin,
            y);

      topMostLevel = y;

      // update y
      y -= level.second->max_y_height() + 10.0;
    }

    double arrowVStart = topMostLevel -
        0.5 * visual_settings_.stableLevelPen.widthF();
    double arrowX = (visual_settings_.parentpos == RightParent)
        ? activeright : activeleft;
    parent_->position_arrow(arrowX, arrowVStart, arrowVEnd);
    parent_->position_text(parentcenter,
                           topMostLevel
                           - stdBoldFontMetrics.height()
                           - parentHlFontMetrics.height());
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
  if (item->type() == ClickableItem::DaughterLevelType)
    clickedDaughterLevel(dynamic_cast<LevelItem*>(item));
  else if (item->type() == ClickableItem::ParentLevelType)
    clickedParentLevel(dynamic_cast<LevelItem*>(item));
  else if (item->type() == ClickableItem::GammaTransitionType)
    clickedGamma(dynamic_cast<TransitionItem*>(item));
  else if (item && (item->type() == ClickableItem::ParentNuclideType))
    clickedParent();
  else if (item && (item->type() == ClickableItem::DaughterNuclideType))
    clickedDaughter();
  else
  {
    deselect_all();
    triggerDataUpdate();
  }
}

void SchemePlayer::clearSelection()
{
  deselect_all();
}

void SchemePlayer::select_levels(const std::set<Energy>& s)
{
  selected_levels_ = s;
  triggerDataUpdate();
}

void SchemePlayer::select_parent_levels(const std::set<Energy>& s)
{
  selected_parent_levels_ = s;
  triggerDataUpdate();
}

void SchemePlayer::select_transistions(const std::set<Energy>& s, int level)
{
  for (auto t : transitions_)
    if (s.count(t->energy()))
      t->graphicsItem()->setHighlighted(level);
}

void SchemePlayer::clickedGamma(TransitionItem *g)
{
  if (!g)
    return;

  if (g->graphicsItem()->isHighlighted())
  {
    deselect_all();
    g->graphicsItem()->setHighlighted(0);
  }
  else
  {
    deselect_all();
    g->graphicsItem()->setHighlighted(1);
  }

  triggerDataUpdate();
}

void SchemePlayer::clickedParentLevel(LevelItem *e)
{
  if (!e)
    return;

  if (e->graphicsItem()->isHighlighted())
    e->graphicsItem()->setHighlighted(0);
  else
  {
    deselect_all();
    e->graphicsItem()->setHighlighted(1);
    selected_parent_levels_.insert(e->energy());
  }

  triggerDataUpdate();
}

void SchemePlayer::clickedDaughterLevel(LevelItem *e)
{
  if (!e)
    return;

  if (e->graphicsItem()->isHighlighted())
    e->graphicsItem()->setHighlighted(0);
  else
  {
    deselect_all();
    e->graphicsItem()->setHighlighted(1);
    selected_levels_.insert(e->energy());
  }

  triggerDataUpdate();
}

void SchemePlayer::clickedParent()
{
  if (parent_selected_)
    parent_selected_ = false;
  else
  {
    deselect_all();
    parent_selected_ = true;
  }
  triggerDataUpdate();
}

void SchemePlayer::clickedDaughter()
{
  if (daughter_selected_)
    daughter_selected_ = false;
  else
  {
    deselect_all();
    daughter_selected_ = true;
  }
  triggerDataUpdate();
}

void SchemePlayer::deselect_all()
{
  deselect_levels();
  deselect_nuclides();
  deselect_gammas();
}

void SchemePlayer::deselect_levels()
{
  for (auto l : levels_)
    l.second->graphicsItem()->setHighlighted(0);
  for (auto l : parent_levels_)
    l.second->graphicsItem()->setHighlighted(0);
  selected_levels_.clear();
  selected_parent_levels_.clear();
}

void SchemePlayer::deselect_nuclides()
{
  daughter_selected_ = false;
  parent_selected_ = false;
}

void SchemePlayer::deselect_gammas()
{
  for (auto l : transitions_)
    l->graphicsItem()->setHighlighted(0);
}

std::set<Energy> SchemePlayer::selected_levels() const
{
  return selected_levels_;
}

std::set<Energy> SchemePlayer::selected_parent_levels() const
{
  return selected_parent_levels_;
}

std::set<Energy> SchemePlayer::selected_transistions(int level) const
{
  std::set<Energy> str;
  for (auto t : transitions_)
    if (t->graphicsItem()->isHighlighted() == level)
      str.insert(t->energy());
  return str;
}

void SchemePlayer::triggerDataUpdate()
{
  emit selectionChanged();
}

void SchemePlayer::setStyle(const QFont &fontfamily, unsigned int sizePx)
{
  visual_settings_.setStyle(fontfamily, sizePx);
}

void SchemePlayer::set_transition_filter(Energy e)
{

}
