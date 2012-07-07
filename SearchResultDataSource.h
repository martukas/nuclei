#ifndef SEARCHRESULTDATASOURCE_H
#define SEARCHRESULTDATASOURCE_H

#include <QThread>

#include "AbstractDataSource.h"
#include "HalfLife.h"

class QProgressDialog;
class SearchThread;

class SearchConstraints {

public:
    SearchConstraints();

    bool valid;

    unsigned int minA, maxA;
    HalfLife minParentHl, maxParentHl;

    double minGammaIntensity;

    HalfLife minLevelHl, maxLevelHl;
    double minMu;
    double minQ;

    double minA22, minA24, minA42, minA44;
};



class SearchResultDataSource : public AbstractDataSource
{
    Q_OBJECT
public:
    explicit SearchResultDataSource(const AbstractDataSource &baseDataSource, QObject *parent = 0);
    virtual ~SearchResultDataSource();

    virtual void populate(SearchConstraints constraints);
    
    virtual AbstractTreeItem * rootItem() const;
    virtual QSharedPointer<Decay> decay(const AbstractTreeItem *item) const;

signals:
    
public slots:

private slots:
    void cancelThread();
    void threadTerminated();
    void threadFinished();

private:
    const AbstractDataSource &m_baseDataSource;
    SearchConstraints m_constraints;

    AbstractTreeItem *root;

    QProgressDialog *pd;
    SearchThread *sthread;
    bool searchFinished;
};


class SearchThread : public QThread
{
    Q_OBJECT

public:
    SearchThread(SearchConstraints constraints, AbstractTreeItem *root, const AbstractDataSource &baseDataSource);

signals:
    void percentComplete(int percent);

protected:
    virtual void run();
    QList<AbstractTreeItem*> constraintConformingChildren(AbstractTreeItem *parent);

private:
    SearchConstraints m_constraints;
    AbstractTreeItem *resultRoot;
    const AbstractDataSource &m_baseDataSource;
};


#endif // SEARCHRESULTDATASOURCE_H
