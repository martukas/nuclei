#ifndef MIXINGRATIO_H
#define MIXINGRATIO_H

#include <QString>
#include <QMetaType>
#include <QDataStream>
#include "UncertainDouble.h"

class MixingRatio
{
public:
    MixingRatio();
    explicit MixingRatio(UncertainDouble delta);

    bool isValid() const;

    QString toString() const;

    UncertainDouble value() const;

    friend bool operator<(const MixingRatio &left, const MixingRatio &right);
    friend bool operator<(const MixingRatio &left, const UncertainDouble &right);
    friend bool operator>(const MixingRatio &left, const MixingRatio &right);
    friend bool operator>(const MixingRatio &left, const UncertainDouble &right);
    friend bool operator==(const MixingRatio &left, const MixingRatio &right);
    operator double() const;

    friend QDataStream & operator<<(QDataStream &out, const MixingRatio &delta);
    friend QDataStream & operator>>(QDataStream &in, MixingRatio &delta);

private:
    bool m_valid;
    UncertainDouble m_delta;
};

#endif // MIXINGRATIO_H
