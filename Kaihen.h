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
class QwtPlotCurve;
class QwtPlotZoomer;
class QDoubleSpinBox;

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

    void setPlotLin();
    void setPlotLog();

    void showPreferences();
    void showAbout();
    
private:
    Ui::KaihenMainWindow *ui;
    QDialog *pdd;
    Ui::PreferencesDialog *pd;

    ENSDFMassChain *currentMassChain;
    QSharedPointer<Decay> decay;

    QDoubleSpinBox *eres;
    QwtPlot *plot;
    QwtPlotZoomer *zoomer;
    QwtPlotCurve *curve;
};

#endif // KAIHEN_H
