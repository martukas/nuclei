#ifndef MIXINGRATIO_H
#define MIXINGRATIO_H

#include <QString>
#include <QMetaType>
#include <QDataStream>

class MixingRatio
{
public:
    enum State {
        UnknownDelta = 0x0,
        MagnitudeDefined = 0x1,
        SignDefined = 0x2,
        SignMagnitudeDefined = 0x3
    };

    MixingRatio();
    explicit MixingRatio(double delta, State state = SignMagnitudeDefined);

    bool isValid() const;

    QString toString() const;

    double value() const;
    State state() const;

    friend bool operator<(const MixingRatio &left, const MixingRatio &right);
    friend bool operator<(const MixingRatio &left, const double &right);
    friend bool operator>(const MixingRatio &left, const MixingRatio &right);
    friend bool operator>(const MixingRatio &left, const double &right);
    friend bool operator==(const MixingRatio &left, const MixingRatio &right);
    operator double() const;

    friend QDataStream & operator<<(QDataStream &out, const MixingRatio &delta);
    friend QDataStream & operator>>(QDataStream &in, MixingRatio &delta);

private:
    bool m_valid;
    double m_delta;
    State m_state;
};

#endif // MIXINGRATIO_H
