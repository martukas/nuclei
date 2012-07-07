#include "SearchResultDataSource.h"

#include <QProgressDialog>
#include <limits>


SearchResultDataSource::SearchResultDataSource(const AbstractDataSource &baseDataSource,
                                               QObject *parent)
    : AbstractDataSource(parent),
      m_baseDataSource(baseDataSource),
      root(new AbstractTreeItem),
      pd(new QProgressDialog("Searching...", "Cancel", 0, 100)),
      sthread(0), searchFinished(false)
{
    pd->setMinimumDuration(0);
}

SearchResultDataSource::~SearchResultDataSource()
{
    delete pd;
    delete root;
    delete sthread;
}

void SearchResultDataSource::populate(SearchConstraints constraints)
{
    // save search options for later use
    m_constraints = constraints;

    // create search thread
    sthread = new SearchThread(constraints, root, m_baseDataSource);
    connect(sthread, SIGNAL(terminated()), this, SLOT(threadTerminated()));
    connect(sthread, SIGNAL(finished()), this, SLOT(threadFinished()));

    // prepare progress bar
    connect(sthread, SIGNAL(percentComplete(int)), pd, SLOT(setValue(int)));
    connect(pd, SIGNAL(canceled()), this, SLOT(cancelThread()));

    // start thread
    sthread->start();
}

AbstractTreeItem *SearchResultDataSource::rootItem() const
{
    if (searchFinished)
        return root;

    return 0;
}

QSharedPointer<Decay> SearchResultDataSource::decay(const AbstractTreeItem *item) const
{

}

void SearchResultDataSource::cancelThread()
{
    sthread->terminate();
    wait();
}

void SearchResultDataSource::threadTerminated()
{
    pd->reset();
    delete sthread;
    sthread = 0;

    delete root;
    root = new AbstractTreeItem;
}

void SearchResultDataSource::threadFinished()
{
    pd->reset();
    delete sthread;
    sthread = 0;
    searchFinished = true;
}


//######################################################################################


SearchThread::SearchThread(SearchConstraints constraints, AbstractTreeItem *root, const AbstractDataSource &baseDataSource)
    : m_constraints(constraints), resultRoot(root), m_baseDataSource(baseDataSource)
{
}

void SearchThread::run()
{
    // get base data source's root item
    AbstractTreeItem *baseRoot = m_baseDataSource.rootItem();


}

QList<AbstractTreeItem *> SearchThread::constraintConformingChildren(AbstractTreeItem *parent)
{
    if (parent->type() == AbstractTreeItem::DecayType)
        ;
}


//######################################################################################


SearchConstraints::SearchConstraints()
    : valid(false),
      minA(0), maxA(std::numeric_limits<unsigned int>::max()),
      minGammaIntensity(std::numeric_limits<double>::quiet_NaN()),
      minMu(std::numeric_limits<double>::quiet_NaN()),
      minQ(std::numeric_limits<double>::quiet_NaN()),
      minA22(std::numeric_limits<double>::quiet_NaN()),
      minA24(std::numeric_limits<double>::quiet_NaN()),
      minA42(std::numeric_limits<double>::quiet_NaN()),
      minA44(std::numeric_limits<double>::quiet_NaN())
{
}
