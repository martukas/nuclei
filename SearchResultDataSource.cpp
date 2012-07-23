#include "SearchResultDataSource.h"

#include <QProgressDialog>
#include <QMap>
#include <QVariant>
#include <limits>
#include <cmath>
#include <Akk.h>

#include <iostream>

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
    connect(sthread, SIGNAL(threadEnded(bool)), this, SLOT(threadEnded(bool)), Qt::QueuedConnection);

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
    sthread->stopThread();
}

void SearchResultDataSource::threadEnded(bool success)
{
    pd->reset();
    delete sthread;
    sthread = 0;

    if (success) {
        searchFinished = true;
        emit resultAvailable(this);
    }
    else {
        delete root;
        root = new SearchTreeItem(AbstractTreeItem::RootType);
    }
}


//######################################################################################


SearchThread::SearchThread(SearchConstraints constraints, AbstractTreeItem *root, const AbstractDataSource &baseDataSource)
    : stop(false), m_constraints(constraints), resultRoot(root), m_baseDataSource(baseDataSource)
{
    connect(this, SIGNAL(finished()), this, SLOT(processThreadEnd()));
    connect(this, SIGNAL(terminated()), this, SLOT(processThreadEnd()));
}

SearchThread::~SearchThread()
{
}

void SearchThread::stopThread()
{
    stop = true;
}

void SearchThread::run()
{
    stop = false;

    // get base data source's root item
    AbstractTreeItem *baseRoot = m_baseDataSource.rootItem();

    emit percentComplete(0);

    for (int i=0; i<baseRoot->childCount(); i++) {
        if (stop)
            return;

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
                int rejectMuQ = 0;
                // µ
                if (std::isfinite(m_constraints.minMu)) {
                    if (std::isfinite(intlevel->mu())) {
                        if (intlevel->mu() < m_constraints.minMu)
                            rejectMuQ++;
                    }
                    else {
                        if (!m_constraints.skipUnknownMu)
                            rejectMuQ++;
                    }
                }
                // Q
                if (std::isfinite(m_constraints.minQ)) {
                    if (std::isfinite(intlevel->q())) {
                        if (intlevel->q() < m_constraints.minQ)
                            rejectMuQ++;
                    }
                    else {
                        if (!m_constraints.skipUnknownQ)
                            rejectMuQ++;
                    }
                }
                if (rejectMuQ > 1 || (rejectMuQ == 1 && !m_constraints.muAndQORCombined))
                    continue;
                // intermediate state's half life
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

                        // initialize flags
                        bool a22ok = false, a24ok = false, a42ok = false, a44ok = false;

                        // keep cascade if necessary parameters are missing and skip field is checked
                        if (    !pop->depopulatedLevel()->spin().isValid() ||
                                !depop->populatedLevel()->spin().isValid() ||
                                !intlevel->spin().isValid() ||
                                !(pop->deltaState() & GammaTransition::SignMagnitudeDefined) ||
                                !(depop->deltaState() & GammaTransition::SignMagnitudeDefined)) {
                            if (m_constraints.skipUnknownAnisotropies) {
                                a22ok = true;
                                a24ok = true;
                                a42ok = true;
                                a44ok = true;
                            }
                        }
                        else {

                            Akk calc;
                            calc.setInitialStateSpin(pop->depopulatedLevel()->spin().doubledSpin());
                            calc.setIntermediateStateSpin(intlevel->spin().doubledSpin());
                            calc.setFinalStateSpin(depop->populatedLevel()->spin().doubledSpin());

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
                        }

                        if (m_constraints.anisotropiesORCombined) {
                            if (!a22ok && !a24ok && !a42ok && !a44ok)
                                continue;
                        }
                        else {
                            if (!(a22ok && a24ok && a42ok && a44ok))
                                continue;
                        }
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

#if defined(PRINT_SEARCH_RESULTS)
                    std::cout << dec->daughterNuclide()->name().toStdString() << "\t"
                              << dec->parentNuclide()->name().toStdString() << "\t"
                              << dec->parentNuclide()->halfLifeAsText().toStdString() << "\t"
                              << pop->depopulatedLevel()->energy() << "/"
                              << pop->populatedLevel()->energy() << "/"
                              << depop->populatedLevel()->energy() << "\t"
                              << pop->populatedLevel()->halfLife().toString().toStdString() << "\t"
                              << pop->populatedLevel()->qAsText().toStdString() << "\t"
                              << pop->populatedLevel()->muAsText().toStdString() << "\t"
                              << std::endl;
#endif
                }
            }

            li++;
        }

        // do not append subtree if no cascades were found
        if (cascades.size() == 0)
            return 0;

        // if this point is reached, the decay meets all criteria
        result = new SearchTreeItem(baseItem);
        //result->setSelectable(false);

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

void SearchThread::processThreadEnd()
{
    emit threadEnded(!stop);
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


