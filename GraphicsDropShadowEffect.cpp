#include "GraphicsDropShadowEffect.h"
#include <QPainter>
#include <QPaintDevice>
#include <QSvgGenerator>

GraphicsDropShadowEffect::GraphicsDropShadowEffect(QObject *parent)
    : QGraphicsDropShadowEffect(parent), m_opacity(1.0)
{
}

qreal GraphicsDropShadowEffect::opacity() const
{
    return m_opacity;
}

void GraphicsDropShadowEffect::setColor(const QColor &color)
{
    m_color = color;
    setOpacity(m_opacity);
}

void GraphicsDropShadowEffect::setOpacity(qreal opacity)
{
    QColor c(m_color);
    c.setAlphaF(c.alphaF()*opacity);
    QGraphicsDropShadowEffect::setColor(c);
    update();
}
