#ifndef GAMMATRANSITION_H
#define GAMMATRANSITION_H

#include <stdint.h>
#include <QFont>
#include <QPen>
#include "ClickableItem.h"

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
    enum DeltaState {
        UnknownDelta = 0x0,
        MagnitudeDefined = 0x1,
        SignDefined = 0x2,
        SignMagnitudeDefined = 0x3
    };

    GammaTransition(double energyKeV, double intensity,
                    const QString &multipol, double delta, DeltaState deltastate,
                    EnergyLevel *start, EnergyLevel *dest);

    double energyKeV() const;
    double intensity() const;
    QString multipolarity() const;
    double delta() const;
    DeltaState deltaState() const;

    QString intensityAsText() const;
    QString energyAsText() const;
    QString multipolarityAsText() const;
    QString deltaAsText() const;

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

    double m_e;
    double intens;
    QString m_mpol;
    double m_delta;
    DeltaState m_deltastate;
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
