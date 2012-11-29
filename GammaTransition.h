#ifndef GAMMATRANSITION_H
#define GAMMATRANSITION_H

#include <stdint.h>
#include <QFont>
#include <QPen>
#include "ClickableItem.h"
#include "Energy.h"
#include "MixingRatio.h"

class QGraphicsItem;
class ActiveGraphicsItemGroup;
class QGraphicsLineItem;
class QGraphicsTextItem;
class QGraphicsPolygonItem;
class QGraphicsRectItem;
class GraphicsHighlightItem;
class EnergyLevel;

class GammaTransition : public ClickableItem
{
public:
    GammaTransition(Energy energy, double intensity,
                    const QString &multipol, UncertainDouble delta,
                    EnergyLevel *start, EnergyLevel *dest);

    Energy energy() const;
    double intensity() const;
    QString multipolarity() const;
    const UncertainDouble &delta() const;

    QString intensityAsText() const;
    QString multipolarityAsText() const;

    EnergyLevel * depopulatedLevel() const;
    EnergyLevel * populatedLevel() const;

    QVector<double> & spectrum(double fwhm, double emax, int samples) const;

    ActiveGraphicsItemGroup * createGammaGraphicsItem(const QFont &gammaFont, const QPen &gammaPen, const QPen &intenseGammaPen);
    void updateArrow();
    double minimalXDistance() const;
    /**
     * Distance between origin and right edge of the bounding rect
     */
    double widthFromOrigin() const;

    QPen pen() const;

private:
    static double gauss(const double x, const double sigma);

    Energy m_e;
    double intens;
    QString m_mpol;
    UncertainDouble m_delta;
    EnergyLevel *m_start, *m_dest;

    QGraphicsLineItem *arrow;
    QGraphicsTextItem *text;
    QGraphicsPolygonItem *arrowhead, *arrowbase;
    QGraphicsRectItem *clickarea;
    GraphicsHighlightItem *highlightHelper;
    double mindist;
    QPen m_pen;

    mutable double m_lastFwhm;
    mutable double m_lastEmax;
    mutable int m_lastSamples;
    mutable QVector<double> m_spectrum;

    static const double textAngle;
    static const double arrowHeadLength;
    static const double arrowBaseLength;
    static const double arrowHeadWidth;
    static const double arrowBaseWidth;
    static const double highlightWidth;

    static const QPolygonF arrowHeadShape, arrowBaseShape;
    static QPolygonF initArrowHead();
    static QPolygonF initArrowBase();
};

#endif // GAMMATRANSITION_H
