#pragma once

#include <QMainWindow>
#include <QPointer>
#include "SchemePlayer.h"
#include "ENSDFDataSource.h"

namespace Ui {
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

    void svgExport();
    void pdfExport();

    void showAll();

    void showPreferences();

    void playerSelectionChanged();
    
    void on_actionMerge_adopted_triggered();

    void on_checkFilterTransition_clicked();

    void on_doubleTargetTransition_editingFinished();

protected:
    void closeEvent(QCloseEvent * event);

private:
    void loadDecay(DecayScheme decay);

    Ui::NucleiMainWindow *ui {nullptr};
    QDialog *preferencesDialog {nullptr};
    Ui::PreferencesDialog *preferencesDialogUi {nullptr};

    DecayCascadeItemModel *decaySelectionModel {nullptr};
    DecayCascadeItemModel *searchResultSelectionModel {nullptr};
    DecayCascadeFilterProxyModel *decayProxyModel {nullptr};
    DecayCascadeFilterProxyModel *searchProxyModel {nullptr};

    QPointer<ENSDFDataSource> data_source_;

    QPointer<SchemePlayer> decay_viewer_;
    DecayScheme current_scheme_;

    std::string make_reference_link(std::string ref, int num);
    QString prep_comments(const json& j,
                          const std::set<std::string>& refs);
    void set_text(const json& jj);

    void reload_selection();
};
