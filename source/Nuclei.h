#pragma once

#include <QMainWindow>
#include <QPointer>
#include "ENSDFDataSource.h"

namespace Ui
{
class NucleiMainWindow;
class PreferencesDialog;
}

class QListWidgetItem;
class QModelIndex;
class QDoubleSpinBox;
class DecayCascadeItemModel;
class DecayCascadeFilterProxyModel;
class QLabel;

class Nuclei : public QMainWindow
{
  Q_OBJECT

public:
  explicit Nuclei(QWidget *parent = 0);
  ~Nuclei();

private slots:
  void initialize();

  void loadSelectedDecay(const QModelIndex &index);
  void loadSearchResultCascade(const QModelIndex &index);

  void on_decayOptionsButton_clicked();

protected:
  void closeEvent(QCloseEvent * event);

private:
  Ui::NucleiMainWindow *ui {nullptr};
  QDialog *preferencesDialog {nullptr};
  Ui::PreferencesDialog *preferencesDialogUi {nullptr};

  DecayCascadeItemModel *decaySelectionModel {nullptr};
  DecayCascadeItemModel *searchResultSelectionModel {nullptr};
  DecayCascadeFilterProxyModel *decayProxyModel {nullptr};
  DecayCascadeFilterProxyModel *searchProxyModel {nullptr};

  QPointer<ENSDFDataSource> data_source_;

  void reload_selection();
};
