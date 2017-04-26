#pragma once

#include <QGraphicsDropShadowEffect>

class GraphicsDropShadowEffect : public QGraphicsDropShadowEffect
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    explicit GraphicsDropShadowEffect(QObject *parent = 0);
    qreal opacity() const;
    void setOpacity(qreal opacity);
    
signals:
    
public slots:
    void setColor(const QColor &color);
    
private:
    qreal m_opacity;
    QColor m_color;
};
