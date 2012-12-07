#ifndef NUCLEI_H
#define NUCLEI_H

#include <QMainWindow>
#include "Decay.h"

namespace Ui {
class NucleiMainWindow;
class PreferencesDialog;
}
class QListWidgetItem;
class QModelIndex;
class QwtPlot;
class QwtPlotIntervalCurve;
class QwtPlotZoomer;
class QwtIntervalSample;
class QDoubleSpinBox;
class SearchDialog;
class DecayCascadeItemModel;
class DecayCascadeFilterProxyModel;
class SearchResultDataSource;

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

    void updateDecayData(Decay::DecayDataSet data);
    void updateEnergySpectrum();

    void svgExport();
    void pdfExport();

    void showAll();
    void showOriginalSize();
    void zoomIn();
    void zoomOut();

    void search();
    void searchFinished(SearchResultDataSource *result);

    void setPlotLin();
    void setPlotLog();

    void showPreferences();
    void showAbout();
    
    void processTabSelection(int index);

protected:
    void closeEvent(QCloseEvent * event);

private:
    static QVector<QwtIntervalSample> mergeIntervalData(const QVector<double> &x, const QVector<double> &y1, const QVector<double> &y2);
    void loadDecay(QSharedPointer<Decay> decay);

    Ui::NucleiMainWindow *ui;
    QDialog *preferencesDialog;
    Ui::PreferencesDialog *preferencesDialogUi;

    SearchDialog *searchDialog;

    DecayCascadeItemModel *decaySelectionModel, *searchResultSelectionModel;
    DecayCascadeFilterProxyModel *decayProxyModel, *searchProxyModel;
    QSharedPointer<Decay> m_decay;

    QDoubleSpinBox *eres;
    QwtPlot *plot;
    QwtPlotZoomer *zoomer;
    QwtPlotIntervalCurve *curve, *g1curve, *g2curve;
};

#endif // NUCLEI_H
