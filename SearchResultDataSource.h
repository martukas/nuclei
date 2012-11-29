#ifndef SEARCHRESULTDATASOURCE_H
#define SEARCHRESULTDATASOURCE_H

#include <QThread>

#include "AbstractDataSource.h"
#include "HalfLife.h"
#include "SearchConstraints.h"

class QProgressDialog;
class SearchThread;

class SearchTreeItem : public AbstractTreeItem
{
public:
    explicit SearchTreeItem(AbstractTreeItem::ItemType type = AbstractTreeItem::UnknownType, AbstractTreeItem *parent = 0);
    explicit SearchTreeItem(ItemType type, unsigned int A, const QList<QVariant> &data, bool selectable, AbstractTreeItem *parent = 0);
    explicit SearchTreeItem(AbstractTreeItem *original);
    virtual ~SearchTreeItem();

    void setOriginalItem(AbstractTreeItem *original);
    AbstractTreeItem * originalItem() const;
    void setCascade(const Decay::CascadeIdentifier &cascade);
    Decay::CascadeIdentifier cascade() const;

private:
    AbstractTreeItem *m_original;
    Decay::CascadeIdentifier m_cascade;
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
    virtual Decay::CascadeIdentifier cascade(const AbstractTreeItem *item) const;

signals:
    void resultAvailable(SearchResultDataSource *result);
    
public slots:

private slots:
    void cancelThread();
    void threadEnded(bool success);

private:
    const AbstractDataSource &m_baseDataSource;
    SearchConstraints m_constraints;

    SearchTreeItem *root;

    QProgressDialog *pd;
    SearchThread *sthread;
    bool searchFinished;
};


class SearchThread : public QThread
{
    Q_OBJECT

public:
    SearchThread(SearchConstraints constraints, AbstractTreeItem *root, const AbstractDataSource &baseDataSource);
    virtual ~SearchThread();

    void stopThread();

signals:
    void threadEnded(bool success);
    void percentComplete(int percent);

protected:
    virtual void run();
    AbstractTreeItem * getConstraintConformingSubtree(AbstractTreeItem *baseItem);

private slots:
    void processThreadEnd();

private:
    static bool isDeltaUsable(const UncertainDouble &delta);
    bool stop;
    SearchConstraints m_constraints;
    AbstractTreeItem *resultRoot;
    const AbstractDataSource &m_baseDataSource;
};


#endif // SEARCHRESULTDATASOURCE_H
