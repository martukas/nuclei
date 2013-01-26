#include "SearchDialog.h"
#include "ui_SearchDialog.h"

#include <cmath>
#include <boost/math/special_functions/fpclassify.hpp>

#include "SearchResultDataSource.h"

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog),
    ds(0)
{
    ui->setupUi(this);
    ui->minIntHalfLife->setValue(1E-9);
    connect(ui->muValCheck, SIGNAL(toggled(bool)), this, SLOT(updateMuQAndOrButtons()));
    connect(ui->qValCheck, SIGNAL(toggled(bool)), this, SLOT(updateMuQAndOrButtons()));
    connect(ui->a22Check, SIGNAL(toggled(bool)), this, SLOT(updateAnisotropyAndOrButtons()));
    connect(ui->a24Check, SIGNAL(toggled(bool)), this, SLOT(updateAnisotropyAndOrButtons()));
    connect(ui->a42Check, SIGNAL(toggled(bool)), this, SLOT(updateAnisotropyAndOrButtons()));
    connect(ui->a44Check, SIGNAL(toggled(bool)), this, SLOT(updateAnisotropyAndOrButtons()));
}

SearchDialog::~SearchDialog()
{
    delete ui;
}

void SearchDialog::setDataSource(AbstractDataSource *datasource)
{
    ds = datasource;
}

SearchConstraints SearchDialog::searchConstraints() const
{
    SearchConstraints sc;

    sc.valid = true;

    sc.minA = ui->minA->value();
    sc.maxA = ui->maxA->value();
    if (ui->minParentHalfLifeCheck->isChecked())
        sc.minParentHl = HalfLife(ui->minParentHalfLife->value());
    if (ui->maxParentHalfLifeCheck->isChecked())
        sc.maxParentHl = HalfLife(ui->maxParentHalfLife->value());

    if (ui->minIntensCheck->isChecked())
        sc.minGammaIntensity = ui->minIntens->value();
    if (ui->minIntHalfLifeCheck->isChecked())
        sc.minLevelHl = HalfLife(ui->minIntHalfLife->value());
    if (ui->maxIntHalfLifeCheck->isChecked())
        sc.maxLevelHl = HalfLife(ui->maxIntHalfLife->value());
    if (ui->muValCheck->isChecked())
        sc.minMu = ui->muVal->value();
    if (ui->qValCheck->isChecked())
        sc.minQ = ui->qVal->value();

    sc.skipUnknownMu = ui->muValSkip->isChecked();
    sc.skipUnknownQ = ui->qValSkip->isChecked();
    sc.muAndQORCombined = ui->muQOr->isChecked();

    if (ui->a22Check->isChecked())
        sc.minA22 = ui->a22->value();
    if (ui->a24Check->isChecked())
        sc.minA24 = ui->a24->value();
    if (ui->a42Check->isChecked())
        sc.minA42 = ui->a42->value();
    if (ui->a44Check->isChecked())
        sc.minA44 = ui->a44->value();

    sc.skipUnknownAnisotropies = ui->anisotropySkip->isChecked();
    sc.anisotropiesORCombined = ui->anisotropyOr->isChecked();

    return sc;
}

void SearchDialog::setSearchConstraints(const SearchConstraints &sc)
{
    if (!sc.valid)
        return;

    ui->minA->setValue(sc.minA);
    ui->maxA->setValue(sc.maxA);

    ui->minParentHalfLifeCheck->setChecked(sc.minParentHl.isValid());
    if (sc.minParentHl.isValid())
        ui->minParentHalfLife->setValue(sc.minParentHl.seconds());
    ui->maxParentHalfLifeCheck->setChecked(sc.maxParentHl.isValid());
    if (sc.maxParentHl.isValid())
        ui->maxParentHalfLife->setValue(sc.maxParentHl.seconds());

    ui->minIntensCheck->setChecked(boost::math::isfinite(sc.minGammaIntensity));
    if (boost::math::isfinite(sc.minGammaIntensity))
        ui->minIntens->setValue(sc.minGammaIntensity);

    ui->minIntHalfLifeCheck->setChecked(sc.minLevelHl.isValid());
    if (sc.minLevelHl.isValid())
        ui->minIntHalfLife->setValue(sc.minLevelHl.seconds());
    ui->maxIntHalfLifeCheck->setChecked(sc.maxLevelHl.isValid());
    if (sc.maxLevelHl.isValid())
        ui->maxIntHalfLife->setValue(sc.maxLevelHl.seconds());

    ui->muValCheck->setChecked(boost::math::isfinite(sc.minMu));
    if (boost::math::isfinite(sc.minMu))
        ui->muVal->setValue(sc.minMu);
    ui->qValCheck->setChecked(boost::math::isfinite(sc.minQ));
    if (boost::math::isfinite(sc.minQ))
        ui->qVal->setValue(sc.minQ);

    ui->muValSkip->setChecked(sc.skipUnknownMu);
    ui->qValCheck->setChecked(sc.skipUnknownQ);
    if (sc.muAndQORCombined)
        ui->muQOr->setChecked(true);

    ui->a22Check->setChecked(boost::math::isfinite(sc.minA22));
    if (boost::math::isfinite(sc.minA22))
        ui->a22->setValue(sc.minA22);
    ui->a24Check->setChecked(boost::math::isfinite(sc.minA24));
    if (boost::math::isfinite(sc.minA24))
        ui->a24->setValue(sc.minA24);
    ui->a42Check->setChecked(boost::math::isfinite(sc.minA42));
    if (boost::math::isfinite(sc.minA42))
        ui->a42->setValue(sc.minA42);
    ui->a44Check->setChecked(boost::math::isfinite(sc.minA44));
    if (boost::math::isfinite(sc.minA44))
        ui->a44->setValue(sc.minA44);

    ui->anisotropySkip->setChecked(sc.skipUnknownAnisotropies);
    if (sc.anisotropiesORCombined)
        ui->anisotropyOr->setChecked(true);
}

void SearchDialog::accept()
{
    if (!ds)
        return;

    SearchResultDataSource *sds = new SearchResultDataSource(*ds, parent());
    connect(sds, SIGNAL(resultAvailable(SearchResultDataSource*)), this, SIGNAL(resultAvailable(SearchResultDataSource*)));

    sds->populate(searchConstraints());

    QDialog::accept();
}

void SearchDialog::updateAnisotropyAndOrButtons()
{
    int numchecked = 0;
    if (ui->a22Check->isChecked())
        numchecked++;
    if (ui->a24Check->isChecked())
        numchecked++;
    if (ui->a42Check->isChecked())
        numchecked++;
    if (ui->a44Check->isChecked())
        numchecked++;

    ui->anisotropyAnd->setEnabled(numchecked > 1);
    ui->anisotropyOr->setEnabled(numchecked > 1);
    ui->anisotropySkip->setEnabled(numchecked > 0);
}

void SearchDialog::updateMuQAndOrButtons()
{
    bool enable = ui->muValCheck->isChecked() && ui->qValCheck->isChecked();
    ui->muQAnd->setEnabled(enable);
    ui->muQOr->setEnabled(enable);
}
