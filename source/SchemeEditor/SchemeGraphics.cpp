#include "SchemeGraphics.h"
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

SchemeGraphics::SchemeGraphics(DecayScheme scheme, double min_intensity, QObject *parent)
  : QObject(parent)
  , scheme_(scheme)
  , min_intensity_(min_intensity)
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

const DecayScheme& SchemeGraphics::scheme() const
{
  return scheme_;
}

bool SchemeGraphics::parent_selected() const
{
  return parent_selected_;
}

bool SchemeGraphics::daughter_selected() const
{
  return daughter_selected_;
}

GraphicsScene *SchemeGraphics::levelPlot()
{
  if (scene_)
    return scene_;

  scene_ = new GraphicsScene(this);
  connect(scene_, SIGNAL(clickedBackground()), this,
          SLOT(backgroundClicked()));

  //  scene_->setBackgroundBrush(QBrush(Qt::red, Qt::SolidPattern));

  if (!scheme_.valid())
    return scene_;

  addDaughter(scheme_.daughterNuclide());

  if (visual_settings_.parentpos != NoParent)
    addParent(scheme_.parentNuclide());

  alignGraphicsItems();

  return scene_;
}

void SchemeGraphics::addParent(Nuclide nuc)
{
  auto vis = visual_settings_;
  vis.parentpos = NoParent;

  parent_ = new NuclideItem(nuc, ClickableItem::ParentNuclideType,
                            vis, scene_);
  connectItem(parent_);

  for (auto &level : nuc.levels())
    addParentLevel(level.second);
}

void SchemeGraphics::addParentLevel(Level level)
{
  auto vis = visual_settings_;
  vis.parentpos = NoParent;
  LevelItem *levrend = new LevelItem(level,
                                     LevelItem::ParentLevelType,
                                     vis, scene_);
  connectItem(levrend);
  parent_levels_[level.energy()] = levrend;
}

void SchemeGraphics::addDaughter(Nuclide nuc)
{
  if (nuc.empty())
    return;

  daughter_ = new NuclideItem(nuc, ClickableItem::DaughterNuclideType,
                              visual_settings_, scene_);
  connectItem(daughter_);

  auto transitions = nuc.transitions();
  auto levels = nuc.levels();
  for (const auto& level : levels)
  {
    addLevel(level.second);
    auto depoptrans = level.second.depopulatingTransitions();

    for (const auto& it : depoptrans)
      addTransition(transitions.at(it));
  }
}

void SchemeGraphics::addLevel(Level level)
{
  LevelItem *levrend = new LevelItem(level,
                                     LevelItem::DaughterLevelType,
                                     visual_settings_, scene_);
  connectItem(levrend);
  levels_[level.energy()] = levrend;
}

void SchemeGraphics::addTransition(Transition transition)
{
  if (transition.intensity().value() < min_intensity_)
    return;
  TransitionItem *transrend
      = new TransitionItem(transition, visual_settings_, scene_);
  connectItem(transrend);
  transitions_.push_back(transrend);
}

void SchemeGraphics::connectItem(ClickableItem* item)
{
  connect(this, SIGNAL(enabledShadow(bool)),
          item->graphicsItem(), SLOT(setShadowEnabled(bool)));
  connect(item->graphicsItem(), SIGNAL(clicked(ClickableItem*)),
          this, SLOT(itemClicked(ClickableItem*)));
}

void SchemeGraphics::alignGraphicsItems()
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


void SchemeGraphics::setShadowEnabled(bool enable)
{
  emit enabledShadow(enable);
}

QString SchemeGraphics::name() const
{
  return QString::fromStdString(scheme_.name());
}

void SchemeGraphics::backgroundClicked()
{
  deselect_all();
  triggerDataUpdate();
}

void SchemeGraphics::itemClicked(ClickableItem *item)
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

void SchemeGraphics::clearSelection()
{
  deselect_all();
}

void SchemeGraphics::select_levels(const std::set<Energy>& s, int level)
{
  for (auto t : levels_)
    if (s.count(t.first))
      t.second->graphicsItem()->setHighlighted(level);
}

void SchemeGraphics::select_parent_levels(const std::set<Energy>& s, int level)
{
  for (auto t : parent_levels_)
    if (s.count(t.first))
      t.second->graphicsItem()->setHighlighted(level);
}

void SchemeGraphics::select_transistions(const std::set<Energy>& s, int level)
{
  for (auto t : transitions_)
    if (s.count(t->energy()))
      t->graphicsItem()->setHighlighted(level);
  highlight_coincidences();
}

void SchemeGraphics::clickedGamma(TransitionItem *g)
{
  if (!g)
    return;

  if (g->graphicsItem()->isHighlighted() == 1)
    g->graphicsItem()->setHighlighted(0);
  else
    g->graphicsItem()->setHighlighted(1);

  highlight_coincidences();

  triggerDataUpdate();
}

void SchemeGraphics::clickedParentLevel(LevelItem *e)
{
  if (!e)
    return;

  if (e->graphicsItem()->isHighlighted())
    e->graphicsItem()->setHighlighted(0);
  else
    e->graphicsItem()->setHighlighted(1);

  triggerDataUpdate();
}

void SchemeGraphics::clickedDaughterLevel(LevelItem *e)
{
  if (!e)
    return;

  if (e->graphicsItem()->isHighlighted())
    e->graphicsItem()->setHighlighted(0);
  else
    e->graphicsItem()->setHighlighted(1);

  triggerDataUpdate();
}

void SchemeGraphics::clickedParent()
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

void SchemeGraphics::clickedDaughter()
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

void SchemeGraphics::deselect_all()
{
  deselect_levels();
  deselect_nuclides();
  deselect_gammas();
}

void SchemeGraphics::deselect_levels()
{
  for (auto l : levels_)
    l.second->graphicsItem()->setHighlighted(0);
  for (auto l : parent_levels_)
    l.second->graphicsItem()->setHighlighted(0);
}

void SchemeGraphics::deselect_nuclides()
{
  daughter_selected_ = false;
  parent_selected_ = false;
}

void SchemeGraphics::deselect_gammas()
{
  for (auto l : transitions_)
    l->graphicsItem()->setHighlighted(0);
}

std::set<Energy> SchemeGraphics::selected_levels(int level) const
{
  std::set<Energy> str;
  for (auto t : levels_)
    if (t.second->graphicsItem()->isHighlighted() == level)
      str.insert(t.first);
  return str;
}

std::set<Energy> SchemeGraphics::selected_parent_levels(int level) const
{
  std::set<Energy> str;
  for (auto t : parent_levels_)
    if (t.second->graphicsItem()->isHighlighted() == level)
      str.insert(t.first);
  return str;
}

std::set<Energy> SchemeGraphics::selected_transistions(int level) const
{
  std::set<Energy> str;
  for (auto t : transitions_)
    if (t->graphicsItem()->isHighlighted() == level)
      str.insert(t->energy());
  return str;
}

void SchemeGraphics::triggerDataUpdate()
{
  emit selectionChanged();
}

void SchemeGraphics::setStyle(const QFont &fontfamily, unsigned int sizePx)
{
  visual_settings_.setStyle(fontfamily, sizePx);
}

void SchemeGraphics::set_highlight_cascade(bool h)
{
  highlight_cascade_ = h;
  highlight_coincidences();
}

void SchemeGraphics::highlight_coincidences()
{
  std::set<Energy> intersect;
  if (highlight_cascade_)
    intersect = scheme_.daughterNuclide().coincidences(selected_transistions(1));

  for (auto t : transitions_)
    if (t->graphicsItem()->isHighlighted() != 1)
      t->graphicsItem()->setHighlighted(
            intersect.count(t->energy()) ? 2 : 0);
}

