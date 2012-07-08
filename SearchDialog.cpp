#include "SearchDialog.h"
#include "ui_SearchDialog.h"

#include <cmath>

#include "SearchResultDataSource.h"

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog),
    ds(0)
{
    ui->setupUi(this);
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

    if (ui->a22Check->isChecked())
        sc.minA22 = ui->a22->value();
    if (ui->a24Check->isChecked())
        sc.minA24 = ui->a24->value();
    if (ui->a42Check->isChecked())
        sc.minA42 = ui->a42->value();
    if (ui->a44Check->isChecked())
        sc.minA44 = ui->a44->value();

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

    ui->minIntensCheck->setChecked(std::isfinite(sc.minGammaIntensity));
    if (std::isfinite(sc.minGammaIntensity))
        ui->minIntens->setValue(sc.minGammaIntensity);

    ui->minIntHalfLifeCheck->setChecked(sc.minLevelHl.isValid());
    if (sc.minLevelHl.isValid())
        ui->minIntHalfLife->setValue(sc.minLevelHl.seconds());
    ui->maxIntHalfLifeCheck->setChecked(sc.maxLevelHl.isValid());
    if (sc.maxLevelHl.isValid())
        ui->maxIntHalfLife->setValue(sc.maxLevelHl.seconds());

    ui->muValCheck->setChecked(std::isfinite(sc.minMu));
    if (std::isfinite(sc.minMu))
        ui->muVal->setValue(sc.minMu);
    ui->qValCheck->setChecked(std::isfinite(sc.minQ));
    if (std::isfinite(sc.minQ))
        ui->qVal->setValue(sc.minQ);

    ui->a22Check->setChecked(std::isfinite(sc.minA22));
    if (std::isfinite(sc.minA22))
        ui->a22->setValue(sc.minA22);
    ui->a24Check->setChecked(std::isfinite(sc.minA24));
    if (std::isfinite(sc.minA24))
        ui->a24->setValue(sc.minA24);
    ui->a42Check->setChecked(std::isfinite(sc.minA42));
    if (std::isfinite(sc.minA42))
        ui->a42->setValue(sc.minA42);
    ui->a44Check->setChecked(std::isfinite(sc.minA44));
    if (std::isfinite(sc.minA44))
        ui->a44->setValue(sc.minA44);
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
