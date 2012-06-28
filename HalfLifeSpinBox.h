#ifndef HALFLIFESPINBOX_H
#define HALFLIFESPINBOX_H

#include <QDoubleSpinBox>

class HalfLifeSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    explicit HalfLifeSpinBox(QWidget *parent = 0);

    virtual QString textFromValue(double value) const;
    virtual double valueFromText(const QString &text) const;

    virtual QValidator::State validate(QString &text, int &pos) const;

signals:
    
public slots:

private:
    double validateAndInterpret(QString &input, int &pos, QValidator::State &state) const;
    static double unitToFactor(const QString &unit);
    
};

#endif // HALFLIFESPINBOX_H
