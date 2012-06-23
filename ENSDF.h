#ifndef ENSDF_H
#define ENSDF_H
#include <QStringList>
#include <QSharedPointer>

class Decay;

class ENSDF
{
public:
    explicit ENSDF(int A);

    QStringList name() const;

    static QStringList aValues();
    QStringList daughterNuclides() const;
    QList< QSharedPointer<Decay> > decays(const QString &daughterNuclide) const;

private:
    const int a;
    QStringList daughternuclides;

    QStringList contents;
};

#endif // ENSDF_H
