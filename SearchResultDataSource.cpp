#include "SearchResultDataSource.h"

#include <QProgressDialog>
#include <QMap>
#include <QVariant>
#include <limits>
#include <cmath>
#include <Akk.h>

#include "EnergyLevel.h"


SearchResultDataSource::SearchResultDataSource(const AbstractDataSource &baseDataSource,
                                               QObject *parent)
    : AbstractDataSource(parent),
      m_baseDataSource(baseDataSource),
      root(new SearchTreeItem(AbstractTreeItem::RootType)),
      pd(new QProgressDialog("Searching...", "Cancel", 0, 100)),
      sthread(0), searchFinished(false)
{
    pd->setWindowTitle("Cascade Search");
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
    if (!constraints.valid)
        return;

    // save search options for later use
    m_constraints = constraints;

    // create search thread
    sthread = new SearchThread(constraints, root, m_baseDataSource);
    connect(sthread, SIGNAL(terminated()), this, SLOT(threadTerminated()), Qt::QueuedConnection);
    connect(sthread, SIGNAL(finished()), this, SLOT(threadFinished()), Qt::QueuedConnection);

    // prepare progress bar
    connect(sthread, SIGNAL(percentComplete(int)), pd, SLOT(setValue(int)), Qt::QueuedConnection);
    connect(pd, SIGNAL(canceled()), this, SLOT(cancelThread()), Qt::QueuedConnection);

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
    const SearchTreeItem *sitem = dynamic_cast<const SearchTreeItem*>(item);
    if (!sitem)
        return QSharedPointer<Decay>(0);

    return m_baseDataSource.decay(sitem->originalItem());
}

Decay::CascadeIdentifier SearchResultDataSource::cascade(const AbstractTreeItem *item) const
{
    const SearchTreeItem *sitem = dynamic_cast<const SearchTreeItem*>(item);
    if (!sitem)
        return Decay::CascadeIdentifier();

    return sitem->cascade();
}

void SearchResultDataSource::cancelThread()
{
    sthread->terminate();
}

void SearchResultDataSource::threadTerminated()
{
    pd->reset();
    delete sthread;
    sthread = 0;

    delete root;
    root = new SearchTreeItem(AbstractTreeItem::RootType);
}

void SearchResultDataSource::threadFinished()
{
    pd->reset();
    delete sthread;
    sthread = 0;
    searchFinished = true;
    emit resultAvailable(this);
}


//######################################################################################


SearchThread::SearchThread(SearchConstraints constraints, AbstractTreeItem *root, const AbstractDataSource &baseDataSource)
    : m_constraints(constraints), resultRoot(root), m_baseDataSource(baseDataSource)
{
}

SearchThread::~SearchThread()
{
}

void SearchThread::run()
{
    // get base data source's root item
    AbstractTreeItem *baseRoot = m_baseDataSource.rootItem();

    emit percentComplete(0);

    for (int i=0; i<baseRoot->childCount(); i++) {
        AbstractTreeItem *child = baseRoot->child(i);

        // check if A matches selected interval
        unsigned int childA = child->A();
        if (m_constraints.maxA < childA || m_constraints.minA > childA)
            continue;

        // update progress bar
        emit percentComplete((childA - m_constraints.minA) * 100 / (m_constraints.maxA - m_constraints.minA + 1));

        // analyze subtree
        AbstractTreeItem *subtree = getConstraintConformingSubtree(child);
        if (subtree)
            subtree->setParent(resultRoot);
    }
    emit percentComplete(100);
}

AbstractTreeItem *SearchThread::getConstraintConformingSubtree(AbstractTreeItem *baseItem)
{
    if (!baseItem)
        return 0;

    AbstractTreeItem * result = 0;

    // stop condition
    if (baseItem->type() == AbstractTreeItem::DecayType) {

        // get decay
        QSharedPointer<Decay> dec(m_baseDataSource.decay(baseItem));

        // test constraints

        // test parent half life
        if (m_constraints.maxParentHl.isValid() || m_constraints.minParentHl.isValid()) {
            QList<HalfLife> phls = dec->parentNuclide()->halfLifes();
            bool minok = false;
            bool maxok = false;
            foreach (HalfLife hl, phls) {
                if (hl > m_constraints.minParentHl)
                    minok = true;
                if (hl < m_constraints.maxParentHl)
                    maxok = true;
            }
            if (m_constraints.minParentHl.isValid() && !minok)
                return 0;
            if (m_constraints.maxParentHl.isValid() && !maxok)
                return 0;
        }

        // find cascades
        const QMap<Energy, EnergyLevel*> levels = dec->daughterNuclide()->levels();
        QMap<Energy, EnergyLevel*>::const_iterator li = levels.begin() + 1;
        QMap<QString, Decay::CascadeIdentifier> cascades;
        while (li != levels.end()) {
            // get this level's gammas
            foreach (const GammaTransition *pop, li.value()->depopulatingTransitions()) {
                // check populating gamma's intensity
                if (std::isfinite(m_constraints.minGammaIntensity) && (pop->intensity() < m_constraints.minGammaIntensity || !std::isfinite(pop->intensity())))
                    continue;

                // check intermediate level
                const EnergyLevel *intlevel = pop->populatedLevel();
                if (std::isfinite(m_constraints.minMu) && (intlevel->mu() < m_constraints.minMu || !std::isfinite(intlevel->mu())))
                    continue;
                if (std::isfinite(m_constraints.minQ) && (intlevel->q() < m_constraints.minQ || !std::isfinite(intlevel->q())))
                    continue;
                if (m_constraints.minLevelHl.isValid() && (intlevel->halfLife() < m_constraints.minLevelHl || !intlevel->halfLife().isValid()))
                    continue;
                if (m_constraints.maxLevelHl.isValid() && (intlevel->halfLife() > m_constraints.maxLevelHl || !intlevel->halfLife().isValid()))
                    continue;

                // iterate over depopulating gammas
                foreach (const GammaTransition *depop, intlevel->depopulatingTransitions()) {
                    // check depopulating gamma's intensity
                    if (std::isfinite(m_constraints.minGammaIntensity) && (depop->intensity() < m_constraints.minGammaIntensity || !std::isfinite(pop->intensity())))
                        continue;

                    // check anisotropy-limits
                    if (std::isfinite(m_constraints.minA22) ||
                            std::isfinite(m_constraints.minA24) ||
                            std::isfinite(m_constraints.minA42) ||
                            std::isfinite(m_constraints.minA44)) {

                        // discard cascade if necessary parameters are missing
                        if (!pop->depopulatedLevel()->spin().isValid() ||
                                !depop->populatedLevel()->spin().isValid() ||
                                !intlevel->spin().isValid() ||
                                !(pop->deltaState() & GammaTransition::SignMagnitudeDefined) ||
                                !(depop->deltaState() & GammaTransition::SignMagnitudeDefined))
                            continue;

                        Akk calc;
                        calc.setInitialStateSpin(pop->depopulatedLevel()->spin().doubledSpin());
                        calc.setIntermediateStateSpin(intlevel->spin().doubledSpin());
                        calc.setFinalStateSpin(depop->populatedLevel()->spin().doubledSpin());

                        // initialize flags
                        bool a22ok = false, a24ok = false, a42ok = false, a44ok = false;

                        // compute Akk for all possible sign combinations
                        QList<double> popvariants, depopvariants;
                        popvariants << 1.0;
                        depopvariants << 1.0;
                        if (pop->deltaState() == GammaTransition::MagnitudeDefined)
                            popvariants << -1.0;
                        if (pop->deltaState() == GammaTransition::MagnitudeDefined)
                            depopvariants << -1.0;

                        foreach (double popvariant, popvariants) {
                            foreach (double depopvariant, depopvariants) {
                                calc.setPopulatingGammaMixing(pop->delta() * popvariant);
                                calc.setDepopulatingGammaMixing(depop->delta() * depopvariant);
                                if (!std::isfinite(m_constraints.minA22) || qAbs(calc.a22()) >= m_constraints.minA22)
                                    a22ok = true;
                                if (!std::isfinite(m_constraints.minA24) || qAbs(calc.a24()) >= m_constraints.minA24)
                                    a24ok = true;
                                if (!std::isfinite(m_constraints.minA42) || qAbs(calc.a42()) >= m_constraints.minA42)
                                    a42ok = true;
                                if (!std::isfinite(m_constraints.minA44) || qAbs(calc.a44()) >= m_constraints.minA44)
                                    a44ok = true;
                            }
                        }

                        if (!(a22ok && a24ok && a42ok && a44ok))
                            continue;
                    }

                    // create cascade identifier
                    Decay::CascadeIdentifier cid;
                    cid.highlightIntermediate = true;
                    cid.start = li.value()->energy();
                    cid.pop = pop->energy();
                    cid.intermediate = intlevel->energy();
                    cid.depop = depop->energy();

                    // create cascade name
                    QString cname(QString::fromUtf8("%1 → %2 → %3"));
                    cname = cname.arg(li.value()->energy().toString())
                                 .arg(intlevel->energy().toString())
                                 .arg(depop->populatedLevel()->energy().toString());

                    cascades.insert(cname, cid);
                }
            }

            li++;
        }

        // do not append subtree if no cascades were found
        if (cascades.size() == 0)
            return 0;

        // if this point is reached, the decay meets all criteria
        result = new SearchTreeItem(baseItem);
        result->setSelectable(false);

        // append cascades
        QMap<QString, Decay::CascadeIdentifier>::const_iterator cascadeit = cascades.begin();
        while (cascadeit != cascades.end()) {

            SearchTreeItem *cascadeItem = new SearchTreeItem(AbstractTreeItem::CascadeType,
                                                             baseItem->A(),
                                                             QList<QVariant>() << cascadeit.key(),
                                                             true,
                                                             result
                                                             );
            cascadeItem->setCascade(cascadeit.value());
            cascadeItem->setOriginalItem(baseItem);

            cascadeit++;
        }

        return result;
    }

    // recursion
    for (int i=0; i<baseItem->childCount(); i++) {
        AbstractTreeItem *child = baseItem->child(i);
        AbstractTreeItem *subtree = getConstraintConformingSubtree(child);
        if (subtree) {
            // copy baseItem to populate root
            if (!result) {
                result = new SearchTreeItem(baseItem);
            }
            subtree->setParent(result);
        }
    }

    return result;
}


//######################################################################################

SearchTreeItem::SearchTreeItem(AbstractTreeItem::ItemType type, AbstractTreeItem *parent)
    : AbstractTreeItem(type, parent), m_original(0)
{
}

SearchTreeItem::SearchTreeItem(AbstractTreeItem::ItemType type, unsigned int A, const QList<QVariant> &data, bool selectable, AbstractTreeItem *parent)
    : AbstractTreeItem(type, A, data, selectable, parent)
{
}

SearchTreeItem::SearchTreeItem(AbstractTreeItem *original)
    : AbstractTreeItem(*original), m_original(original)
{
}

SearchTreeItem::~SearchTreeItem()
{
}

void SearchTreeItem::setOriginalItem(AbstractTreeItem *original)
{
    m_original = original;
}

AbstractTreeItem *SearchTreeItem::originalItem() const
{
    return m_original;
}

void SearchTreeItem::setCascade(const Decay::CascadeIdentifier &cascade)
{
    m_cascade = cascade;
}

Decay::CascadeIdentifier SearchTreeItem::cascade() const
{
    return m_cascade;
}


