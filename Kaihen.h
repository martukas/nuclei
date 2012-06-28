#ifndef KAIHEN_H
#define KAIHEN_H

#include <QMainWindow>
#include "Decay.h"

namespace Ui {
class KaihenMainWindow;
class PreferencesDialog;
}
class ENSDFMassChain;
class QListWidgetItem;
class QwtPlot;
class QwtPlotIntervalCurve;
class QwtPlotZoomer;
class QwtIntervalSample;
class QDoubleSpinBox;
class SearchDialog;

class Kaihen : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Kaihen(QWidget *parent = 0);
    ~Kaihen();

private slots:
    void initialize();

    void selectedA(const QString &aName);
    void selectedNuclide(const QString &nuclideName);
    void selectedDecay(const QString &decayName);

    void updateDecayData(Decay::DecayDataSet data);
    void updateEnergySpectrum();

    void svgExport();
    void pdfExport();

    void showAll();
    void showOriginalSize();
    void zoomIn();
    void zoomOut();

    void search();

    void setPlotLin();
    void setPlotLog();

    void showPreferences();
    void showAbout();
    
protected:
    void closeEvent(QCloseEvent * event);

private:
    static QVector<QwtIntervalSample> mergeIntervalData(const QVector<double> &x, const QVector<double> &y1, const QVector<double> &y2);

    Ui::KaihenMainWindow *ui;
    QDialog *pdd;
    Ui::PreferencesDialog *pd;

    SearchDialog *m_search;

    ENSDFMassChain *currentMassChain;
    QSharedPointer<Decay> decay;

    QDoubleSpinBox *eres;
    QwtPlot *plot;
    QwtPlotZoomer *zoomer;
    QwtPlotIntervalCurve *curve, *g1curve, *g2curve;
};

#endif // KAIHEN_H
