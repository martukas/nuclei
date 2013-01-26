#include "GammaTransition.h"

#include <QGraphicsTextItem>
#include <QFontMetrics>
#include <QList>
#include <QTextDocument>
#include <cmath>
#include <boost/math/special_functions/fpclassify.hpp>
#if !defined(M_PI)
#define M_PI        3.14159265358979323846264338327950288
#endif
#if !defined(M_LN2)
#define M_LN2       0.693147180559945309417232121458176568
#endif

#include "ActiveGraphicsItemGroup.h"
#include "EnergyLevel.h"
#include "GraphicsHighlightItem.h"

const double GammaTransition::textAngle = -60.0;
const double GammaTransition::arrowHeadLength = 11.0;
const double GammaTransition::arrowBaseLength = 7.0;
const double GammaTransition::arrowHeadWidth = 5.0;
const double GammaTransition::arrowBaseWidth = 5.0;
const double GammaTransition::highlightWidth = 5.0;
const QPolygonF GammaTransition::arrowHeadShape = initArrowHead();
const QPolygonF GammaTransition::arrowBaseShape = initArrowBase();


GammaTransition::GammaTransition(Energy energy, double intensity,
                                 const QString &multipol, UncertainDouble delta,
                                 EnergyLevel *start, EnergyLevel *dest)
    : ClickableItem(ClickableItem::GammaTransitionType),
      m_e(energy), intens(intensity), m_mpol(multipol), m_delta(delta), m_start(start), m_dest(dest),
      arrow(0), text(0), arrowhead(0), arrowbase(0), clickarea(0), highlightHelper(0), mindist(0.0),
      m_lastFwhm(std::numeric_limits<double>::quiet_NaN()),
      m_lastEmax(std::numeric_limits<double>::quiet_NaN()),
      m_lastSamples(std::numeric_limits<double>::quiet_NaN())
{
    start->m_depopulatingTransitions.append(this);
    dest->m_populatingTransitions.append(this);
}

GammaTransition::~GammaTransition()
{
}

Energy GammaTransition::energy() const
{
    return m_e;
}

double GammaTransition::intensity() const
{
    return intens;
}

QString GammaTransition::multipolarity() const
{
    return m_mpol;
}

const UncertainDouble & GammaTransition::delta() const
{
    return m_delta;
}

QString GammaTransition::intensityAsText() const
{
    QString intensstr;
    if (!boost::math::isnan(intens))
        intensstr = QString("%1 %").arg(intens, 0, 'g', 3);
    return intensstr;
}

QString GammaTransition::multipolarityAsText() const
{
    if (m_mpol.isEmpty())
        return "<i>unknown</i>";
    return m_mpol;
}

EnergyLevel *GammaTransition::depopulatedLevel() const
{
    return m_start;
}

EnergyLevel *GammaTransition::populatedLevel() const
{
    return m_dest;
}

QVector<double> & GammaTransition::spectrum(double fwhm, double emax, int samples) const
{
    if (fwhm == m_lastFwhm && emax == m_lastEmax && samples == m_lastSamples)
        return m_spectrum;

    m_spectrum.clear();
    m_lastFwhm = fwhm;
    m_lastEmax = emax;
    m_lastSamples = samples;

    // determine sigma @ 662 keV
    double sigma = fwhm / (2.0*sqrt(2.0*M_LN2));
    // determine local sigma
    double localstd = sqrt(m_e/662.0 * sigma*sigma);

    m_spectrum.resize(samples);

    const double interval = emax / double(samples);

    for (int i=0; i<m_spectrum.size(); i++)
        m_spectrum[i] = intens * gauss(interval*double(i)+0.5*interval - m_e, localstd);

    return m_spectrum;
}

ActiveGraphicsItemGroup *GammaTransition::createGammaGraphicsItem(const QFont &gammaFont, const QPen &gammaPen, const QPen &intenseGammaPen)
{
    if (item)
        return item;

    m_pen = gammaPen;
    if (intens >= 5.0)
        m_pen = intenseGammaPen;

    // group origin is set to the start level!
    item = new ActiveGraphicsItemGroup(this);
    item->setActiveColor(QColor(64, 166, 255, 180));

    arrowhead = new QGraphicsPolygonItem(arrowHeadShape);
    arrowhead->setBrush(QBrush(m_pen.color()));
    arrowhead->setPen(Qt::NoPen);

    arrowbase = new QGraphicsPolygonItem(arrowBaseShape);
    arrowbase->setBrush(QBrush(m_pen.color()));
    arrowbase->setPen(Qt::NoPen);
    arrowbase->setPos(0.0, 0.0);

    arrow = new QGraphicsLineItem;
    arrow->setPen(m_pen);

    QString intensstr = intensityAsText();
    if (!intensstr.isEmpty())
        intensstr += " ";
    QString textstr = QString("<html><body bgcolor=\"white\">%1<b>%2</b> %3</body></html>").arg(intensstr).arg(energy().toString()).arg(m_mpol);
    text = new QGraphicsTextItem;
    text->setFont(gammaFont);
    text->document()->setDocumentMargin(0.0);
    text->setHtml(textstr);
    //new QGraphicsRectItem(text->boundingRect(), text);
    double textheight = text->boundingRect().height();
    text->setPos(0.0, -textheight);
    text->setTransformOriginPoint(0.0, 0.5*textheight);
    text->setRotation(textAngle);
    mindist = std::abs(textheight / std::sin(-textAngle/180.0*M_PI));
    text->moveBy(0.5*textheight*std::sin(-textAngle/180.0*M_PI) - 0.5*mindist, 0.0);

    clickarea = new QGraphicsRectItem(-0.5*mindist, 0.0, mindist, 0.0);
    clickarea->setPen(Qt::NoPen);
    clickarea->setBrush(Qt::NoBrush);

    highlightHelper = new GraphicsHighlightItem(-0.5*highlightWidth, 0.0, highlightWidth, 0.0);

    item->addHighlightHelper(highlightHelper);
    item->addToGroup(arrowbase);
    item->addToGroup(text);
    item->addToGroup(arrow);
    item->addToGroup(arrowhead);
    item->addToGroup(clickarea);

    updateArrow();

    return item;
}

void GammaTransition::updateArrow()
{
    item->removeFromGroup(clickarea);
    item->removeFromGroup(arrowhead);
    item->removeFromGroup(arrowbase);
    item->removeFromGroup(arrow);
    item->removeHighlightHelper(highlightHelper);
    double arrowDestY = m_dest->graYPos - m_start->graYPos;
    arrowhead->setPos(0.0, arrowDestY);
    arrow->setLine(0.0, 0.0, 0.0, arrowDestY - arrowHeadLength);
    clickarea->setRect(-0.5*mindist, 0.0, mindist, arrowDestY);
    highlightHelper->setRect(-0.5*highlightWidth, 0.0, highlightWidth, arrowDestY - arrowHeadLength);
    item->addHighlightHelper(highlightHelper);
    item->addToGroup(arrow);
    item->addToGroup(arrowbase);
    item->addToGroup(arrowhead);
    item->addToGroup(clickarea);
}

double GammaTransition::minimalXDistance() const
{
    return mindist;
}

double GammaTransition::widthFromOrigin() const
{
    return item->childrenBoundingRect().right();
}

QPen GammaTransition::pen() const
{
    return m_pen;
}

double GammaTransition::gauss(const double x, const double sigma)
{
    double u = x / fabs(sigma);
    double p = (1 / (sqrt(2 * M_PI) * fabs(sigma))) * exp(-u * u / 2);
    return p;
}

QPolygonF GammaTransition::initArrowHead()
{
    QPolygonF arrowheadpol;
    arrowheadpol << QPointF(0.0, 0.0);
    arrowheadpol << QPointF(0.5*arrowHeadWidth, -arrowHeadLength);
    arrowheadpol << QPointF(-0.5*arrowHeadWidth, -arrowHeadLength);
    return arrowheadpol;
}

QPolygonF GammaTransition::initArrowBase()
{
    QPolygonF arrowbasepol;
    arrowbasepol << QPointF(0.0, arrowBaseLength);
    arrowbasepol << QPointF(0.5*arrowBaseWidth, 0.0);
    arrowbasepol << QPointF(-0.5*arrowBaseWidth, 0.0);
    return arrowbasepol;
}
