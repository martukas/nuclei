#include "Nuclei.h"
#include "ui_Nuclei.h"
#include "ui_PreferencesDialog.h"

#include <QSettings>
#include <QTimer>
#include <QDesktopWidget>

#include "DecayCascadeItemModel.h"
#include "DecayCascadeFilterProxyModel.h"

#include <qguiapplication.h>
#include <qscreen.h>
#include <util/logger.h>

Nuclei::Nuclei(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::NucleiMainWindow)
  , preferencesDialog(new QDialog(this))
  , preferencesDialogUi(new Ui::PreferencesDialog)
{
  CustomLogger::initLogger(spdlog::level::debug, "nuclei");

  ui->setupUi(this);
  setWindowTitle(QCoreApplication::applicationName() + QString(" ") + QCoreApplication::applicationVersion());

  preferencesDialogUi->setupUi(preferencesDialog);

  QAction *toggleSelector = ui->decaySelectorDock->toggleViewAction();
  toggleSelector->setToolTip(toggleSelector->toolTip().prepend("Show/Hide "));
  toggleSelector->setIcon(QIcon(":/toolbar/checkbox.png"));

  connect(ui->decayTreeCollapseButton, SIGNAL(clicked()), ui->decayTreeView, SLOT(collapseAll()));
  connect(ui->decayTreeExpandButton, SIGNAL(clicked()), ui->decayTreeView, SLOT(expandAll()));

  // separate init from constructor to avoid crash on cancel
  QTimer::singleShot(0, this, SLOT(initialize()));
}

Nuclei::~Nuclei()
{
  QSettings s;

  s.setValue("decayFilter", ui->decayFilterLineEdit->text());
  QModelIndex mi = ui->decayTreeView->currentIndex();
  QList<QVariant> selectionIndices;
  while (mi.isValid()) {
    selectionIndices.prepend(mi.row());
    mi = mi.parent();
  }
  s.setValue("decaySelection", selectionIndices);

  delete preferencesDialogUi;
  delete ui;
}

void Nuclei::initialize()
{
  QSettings s;

  restoreGeometry(s.value("geometry").toByteArray());
  restoreState(s.value("windowState").toByteArray());

  QSize wsize = size();
  if (wsize.width() > QGuiApplication::screens()[0]->geometry().width())
    wsize.setWidth(QGuiApplication::screens()[0]->geometry().width());
  if (wsize.height() > QGuiApplication::screens()[0]->geometry().height())
    wsize.setHeight(QGuiApplication::screens()[0]->geometry().width());
  this->resize(wsize);

  // initialize settings if necessary
  preferencesDialogUi->levelDiff->setValue(s.value("levelTolerance", 40.0).toDouble());
  preferencesDialogUi->gammaDiff->setValue(s.value("gammaTolerance", 5.0).toDouble());
  preferencesDialogUi->checkMergeAdopted->setChecked(s.value("mergeAdopted", false).toBool());
  preferencesDialogUi->buttonBox->setFocus();
  s.endGroup();

  data_source_ = new ENSDFDataSource(this);

  //  ENSDFDataSource *ds = new ENSDFDataSource(this);

  connect(preferencesDialogUi->resetCacheButton,
          SIGNAL(clicked()), data_source_, SLOT(deleteCache()));
  connect(preferencesDialogUi->resetDatabaseButton,
          SIGNAL(clicked()), data_source_, SLOT(deleteDatabaseAndCache()));

  decaySelectionModel = new DecayCascadeItemModel(data_source_, this);
  decayProxyModel = new DecayCascadeFilterProxyModel(this);
  decayProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
  connect(ui->decayFilterLineEdit, SIGNAL(textChanged(QString)), decayProxyModel, SLOT(setFilterWildcard(QString)));
  decayProxyModel->setSourceModel(decaySelectionModel);
  ui->decayTreeView->setModel(decayProxyModel);
  connect(ui->decayTreeView, SIGNAL(showItem(QModelIndex)), this, SLOT(loadSelectedDecay(QModelIndex)));

  searchResultSelectionModel = new DecayCascadeItemModel(0, this);
  searchProxyModel = new DecayCascadeFilterProxyModel(this);
  searchProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
  searchProxyModel->setSourceModel(searchResultSelectionModel);

  ui->decayFilterLineEdit->setText(s.value("decayFilter", "").toString());
  QList<QVariant> selectionIndices(s.value("decaySelection").toList());
  if (!selectionIndices.isEmpty()) {
    QModelIndex mi = decayProxyModel->index(selectionIndices.at(0).toInt(), 0);
    for (int i=1; i<selectionIndices.size(); i++)
      mi = decayProxyModel->index(selectionIndices.at(i).toInt(), 0, mi);
    ui->decayTreeView->setCurrentIndex(mi);
    loadSelectedDecay(mi);
  }
}

void Nuclei::loadSelectedDecay(const QModelIndex &index)
{
  if (!index.isValid())
    return;

  if (!decaySelectionModel)
    return;

  ui->nuclideBrowser->loadDecay(
        decaySelectionModel->decay(decayProxyModel->mapToSource(index),
                                   preferencesDialogUi->checkMergeAdopted->isChecked()));
}

void Nuclei::loadSearchResultCascade(const QModelIndex &index)
{
  if (!index.isValid())
    return;

  if (!searchResultSelectionModel)
    return;

  ui->nuclideBrowser->loadDecay(searchResultSelectionModel->decay(searchProxyModel->mapToSource(index),
                                                                  preferencesDialogUi->checkMergeAdopted->isChecked()));
}

void Nuclei::closeEvent(QCloseEvent *event)
{
  QSettings s;
  s.setValue("geometry", saveGeometry());
  s.setValue("windowState", saveState());
  QMainWindow::closeEvent(event);
}

void Nuclei::reload_selection()
{
  auto si = ui->decayTreeView->selectionModel()->selectedIndexes();
  if (si.size())
    loadSelectedDecay(*si.begin());
}

void Nuclei::on_decayOptionsButton_clicked()
{
  if (preferencesDialog->exec() == QDialog::Accepted)
  {
    DBG("Accepted");
    QSettings s;
    s.setValue("levelTolerance", preferencesDialogUi->levelDiff->value());
    s.setValue("gammaTolerance", preferencesDialogUi->gammaDiff->value());
    s.setValue("mergeAdopted", preferencesDialogUi->checkMergeAdopted->isChecked());
    s.endGroup();
    reload_selection();
  }
}
