#include "Nuclei.h"
#include "ui_Nuclei.h"
#include "ui_PreferencesDialog.h"

#include <QResizeEvent>
#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QSvgGenerator>
#include <QFileDialog>
#include <QPrinter>
#include <QDesktopWidget>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_intervalcurve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_series_data.h>
#include <qwt_scale_engine.h>
#include <qwt_text.h>
#include <qwt_plot_renderer.h>
#include <algorithm>
#include <functional>

#include "version.h"

#include "ENSDFDataSource.h"
#include "DecayCascadeItemModel.h"
#include "DecayCascadeFilterProxyModel.h"
#include "SearchDialog.h"
#include "SearchResultDataSource.h"
#include "UpdateCheck.h"

class PlotZoomer : public QwtPlotZoomer
{
public:
    PlotZoomer(QwtPlotCanvas *c)
        : QwtPlotZoomer(c)
    {
    }
protected:
    virtual QwtText trackerTextF(const QPointF &p) const
    {
        return QwtText(QString("%1 keV").arg(p.x(), 0, 'f', 1));
    }
};

Nuclei::Nuclei(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NucleiMainWindow),
    preferencesDialog(new QDialog(this)), preferencesDialogUi(new Ui::PreferencesDialog),
    searchDialog(new SearchDialog(this)),
    decaySelectionModel(0),
    searchResultSelectionModel(0),
    decayProxyModel(0),
    searchProxyModel(0),
    zoomer(0)
{
    ui->setupUi(this);
    setWindowTitle(QCoreApplication::applicationName() + QString(" ") + QCoreApplication::applicationVersion());

    QLabel *referto = new QLabel(PAPERSTATUSBAR, statusBar());
    referto->setAlignment(Qt::AlignRight);
    referto->setOpenExternalLinks(true);
    statusBar()->addPermanentWidget(referto);

    preferencesDialogUi->setupUi(preferencesDialog);

    // add toolbar widgets
    eres = new QDoubleSpinBox(ui->mainToolBar);
    eres->setSuffix(" %");
    eres->setRange(1.5, 100.0);
    eres->setSingleStep(0.2);
    eres->setToolTip("Energy resolution: FWHM in % at 662 keV");
    connect(eres, SIGNAL(valueChanged(double)), this, SLOT(updateEnergySpectrum()));
    ui->energySpectrumBar->addWidget(eres);

    QAction *toggleSelector = ui->decaySelectorDock->toggleViewAction();
    toggleSelector->setToolTip(toggleSelector->toolTip().prepend("Show/Hide "));
    toggleSelector->setIcon(QIcon(":/toolbar/checkbox.png"));
    ui->mainToolBar->insertAction(ui->actionZoom_In, toggleSelector);

    QAction *toggleInfo = ui->decayInfoDock->toggleViewAction();
    toggleInfo->setToolTip(toggleInfo->toolTip().prepend("Show/Hide "));
    toggleInfo->setIcon(QIcon(":/toolbar/view-list-text.png"));
    ui->mainToolBar->insertAction(ui->actionZoom_In, toggleInfo);

    ui->mainToolBar->insertSeparator(ui->actionZoom_In);

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(processTabSelection(int)));
    if (ui->tabWidget->currentWidget() != ui->energySpectrumTab)
        ui->energySpectrumBar->setDisabled(true);

    plot = new QwtPlot(ui->energySpectrumTab);
    plot->setCanvasBackground(Qt::white);
    ui->energySpectrumLayout->addWidget(plot);
    plot->setAxisTitle(QwtPlot::xBottom, "keV");
    plot->enableAxis(QwtPlot::yLeft, false);

    zoomer = new PlotZoomer(plot->canvas());
    zoomer->setTrackerMode(QwtPlotPicker::AlwaysOn);
    zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
    zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);

    new QwtPlotMagnifier(plot->canvas());

    QwtPlotPanner *panner = new QwtPlotPanner(plot->canvas());
    panner->setMouseButton(Qt::MidButton);

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->attach(plot);
    grid->setMajPen(QPen(Qt::gray));
    grid->setMinPen(QPen(QBrush(Qt::lightGray), 1.0, Qt::DotLine, Qt::SquareCap, Qt::BevelJoin));
    grid->enableXMin(true);
    grid->enableYMin(true);

    curve = new QwtPlotIntervalCurve;
    curve->attach(plot);
    curve->setStyle(QwtPlotIntervalCurve::Tube);
    curve->setPen(QPen(Qt::NoPen));
    curve->setBrush(QBrush(QColor(68, 68, 68)));
    curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

    g1curve = new QwtPlotIntervalCurve;
    g1curve->attach(plot);
    g1curve->setStyle(QwtPlotIntervalCurve::Tube);
    g1curve->setPen(QPen(Qt::NoPen));
    g1curve->setBrush(QBrush(QColor(126, 201, 80)));
    g1curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

    g2curve = new QwtPlotIntervalCurve;
    g2curve->attach(plot);
    g2curve->setStyle(QwtPlotIntervalCurve::Tube);
    g2curve->setPen(QPen(Qt::NoPen));
    g2curve->setBrush(QBrush(QColor(232, 95, 92)));
    g2curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

    connect(ui->decayTreeCollapseButton, SIGNAL(clicked()), ui->decayTreeView, SLOT(collapseAll()));
    connect(ui->decayTreeExpandButton, SIGNAL(clicked()), ui->decayTreeView, SLOT(expandAll()));

    connect(ui->searchTreeCollapseButton, SIGNAL(clicked()), ui->searchTreeView, SLOT(collapseAll()));
    connect(ui->searchTreeExpandButton, SIGNAL(clicked()), ui->searchTreeView, SLOT(expandAll()));

    connect(ui->actionSVG_Export, SIGNAL(triggered()), this, SLOT(svgExport()));
    connect(ui->actionPDF_Export, SIGNAL(triggered()), this, SLOT(pdfExport()));

    connect(ui->actionShow_all, SIGNAL(triggered()), this, SLOT(showAll()));
    connect(ui->actionOriginal_Size, SIGNAL(triggered()), this, SLOT(showOriginalSize()));
    connect(ui->actionZoom_In, SIGNAL(triggered()), this, SLOT(zoomIn()));
    connect(ui->actionZoom_Out, SIGNAL(triggered()), this, SLOT(zoomOut()));

    connect(ui->actionFind_Decay, SIGNAL(triggered()), this, SLOT(search()));

    connect(ui->actionLinear, SIGNAL(triggered()), this, SLOT(setPlotLin()));
    connect(ui->actionLogarithmic, SIGNAL(triggered()), this, SLOT(setPlotLog()));

    connect(ui->actionPreferences, SIGNAL(triggered()), this, SLOT(showPreferences()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));

    connect(searchDialog, SIGNAL(resultAvailable(SearchResultDataSource*)), this, SLOT(searchFinished(SearchResultDataSource*)));

    ui->decayView->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // separate init from constructor to avoid crash on cancel
    QTimer::singleShot(0, this, SLOT(initialize()));
}

Nuclei::~Nuclei()
{
    QSettings s;

    s.beginGroup("preferences");
    s.setValue("levelTolerance", preferencesDialogUi->levelDiff->value());
    s.setValue("gammaTolerance", preferencesDialogUi->gammaDiff->value());
    s.setValue("fontFamily", preferencesDialogUi->fontFamily->currentFont().family());
    s.setValue("fontSize", preferencesDialogUi->fontSize->value());
    s.endGroup();

    s.setValue("activeTab", (ui->tabWidget->currentWidget() == ui->decayCascadeTab) ? "decay" : "energy");

    s.setValue("energyScale", ui->actionLinear->isChecked() ? "lin" : "log");

    s.setValue("fwhmResolution", eres->value());

    s.setValue("decayFilter", ui->decayFilterLineEdit->text());
    QModelIndex mi = ui->decayTreeView->currentIndex();
    QList<QVariant> selectionIndices;
    while (mi.isValid()) {
        selectionIndices.prepend(mi.row());
        mi = mi.parent();
    }
    s.setValue("decaySelection", selectionIndices);

    if (m_decay)
        s.setValue("selectedCascade", QVariant::fromValue(m_decay->currentSelection()));

    s.setValue("searchSettings", QVariant::fromValue(searchDialog->searchConstraints()));

    delete preferencesDialogUi;
    delete ui;
}

void Nuclei::initialize()
{
    // load update checker
    new UpdateCheck(this);

    QSettings s;

    restoreGeometry(s.value("geometry").toByteArray());
    restoreState(s.value("windowState").toByteArray());

    QSize wsize = size();
    if (wsize.width() > QApplication::desktop()->availableGeometry().width())
        wsize.setWidth(QApplication::desktop()->availableGeometry().width());
    if (wsize.height() > QApplication::desktop()->availableGeometry().height())
        wsize.setHeight(QApplication::desktop()->availableGeometry().width());
    this->resize(wsize);

    // initialize settings if necessary
    if (!s.contains("exportDir"))
        s.setValue("exportDir", QDir::homePath());

    s.beginGroup("preferences");
    preferencesDialogUi->fontFamily->setCurrentFont(QFont(s.value("fontFamily", QFont().family()).toString()));
    preferencesDialogUi->fontSize->setValue(s.value("fontSize", 14).toInt());
    preferencesDialogUi->levelDiff->setValue(s.value("levelTolerance", 40.0).toDouble());
    preferencesDialogUi->gammaDiff->setValue(s.value("gammaTolerance", 5.0).toDouble());
    s.endGroup();

    ENSDFDataSource *ds = new ENSDFDataSource(this);

    connect(preferencesDialogUi->resetCacheButton, SIGNAL(clicked()), ds, SLOT(deleteCache()));
    connect(preferencesDialogUi->resetDatabaseButton, SIGNAL(clicked()), ds, SLOT(deleteDatabaseAndCache()));

    decaySelectionModel = new DecayCascadeItemModel(ds, this);
    decayProxyModel = new DecayCascadeFilterProxyModel(this);
    decayProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    connect(ui->decayFilterLineEdit, SIGNAL(textChanged(QString)), decayProxyModel, SLOT(setFilterWildcard(QString)));
    decayProxyModel->setSourceModel(decaySelectionModel);
    ui->decayTreeView->setModel(decayProxyModel);
    connect(ui->decayTreeView, SIGNAL(showItem(QModelIndex)), this, SLOT(loadSelectedDecay(QModelIndex)));

    searchDialog->setDataSource(ds);
    searchResultSelectionModel = new DecayCascadeItemModel(0, this);
    searchProxyModel = new DecayCascadeFilterProxyModel(this);
    searchProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    connect(ui->searchFilterLineEdit, SIGNAL(textChanged(QString)), searchProxyModel, SLOT(setFilterWildcard(QString)));
    searchProxyModel->setSourceModel(searchResultSelectionModel);
    ui->searchTreeView->setModel(searchProxyModel);
    connect(ui->searchTreeView, SIGNAL(showItem(QModelIndex)), this, SLOT(loadSearchResultCascade(QModelIndex)));

    // restore last session
    if (s.contains("searchSettings"))
        searchDialog->setSearchConstraints(s.value("searchSettings").value<SearchConstraints>());

    eres->setValue(s.value("fwhmResolution", 5.0).toDouble());

    if (s.value("energyScale", "lin").toString() == "log")
        ui->actionLogarithmic->trigger();

    if (s.value("activeTab", "decay").toString() == "decay")
        ui->tabWidget->setCurrentWidget(ui->decayCascadeTab);
    else
        ui->tabWidget->setCurrentWidget(ui->energySpectrumTab);

    ui->decayFilterLineEdit->setText(s.value("decayFilter", "").toString());
    QList<QVariant> selectionIndices(s.value("decaySelection").toList());
    if (!selectionIndices.isEmpty()) {
        QModelIndex mi = decayProxyModel->index(selectionIndices.at(0).toInt(), 0);
        for (int i=1; i<selectionIndices.size(); i++)
            mi = mi.child(selectionIndices.at(i).toInt(), 0);
        ui->decayTreeView->setCurrentIndex(mi);
        loadSelectedDecay(mi);
    }

    if (m_decay)
        m_decay->setCurrentSelection(s.value("selectedCascade", QVariant::fromValue(Decay::CascadeIdentifier())).value<Decay::CascadeIdentifier>());
}

void Nuclei::loadSelectedDecay(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    if (!decaySelectionModel)
        return;

    QSharedPointer<Decay> decay(decaySelectionModel->decay(decayProxyModel->mapToSource(index)));

    if (!decay)
        return;

    loadDecay(decay);
}

void Nuclei::loadSearchResultCascade(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    if (!searchResultSelectionModel)
        return;

    QSharedPointer<Decay> decay(searchResultSelectionModel->decay(searchProxyModel->mapToSource(index)));

    if (!decay)
        return;

    loadDecay(decay);

    Decay::CascadeIdentifier ci = searchResultSelectionModel->cascade(searchProxyModel->mapToSource(index));
    if (m_decay)
        m_decay->setCurrentSelection(ci);
}

void Nuclei::updateDecayData(Decay::DecayDataSet data)
{
    ui->startEnergy->setText(data.startEnergy);
    ui->startSpin->setText(data.startSpin);

    ui->popEnergy->setText(data.popEnergy);
    ui->popIntensity->setText(data.popIntensity);
    ui->popMultipolarity->setText(data.popMultipolarity);
    ui->popMixing->setText(data.popMixing);

    ui->intEnergy->setText(data.intEnergy);
    ui->intHalfLife->setText(data.intHalfLife);
    ui->intSpin->setText(data.intSpin);
    ui->intMu->setText(data.intMu + QString::fromUtf8(" µ<sub>N</sub>"));
    ui->intQ->setText(data.intQ + " eb");

    ui->depopEnergy->setText(data.depopEnergy);
    ui->depopIntensity->setText(data.depopIntensity);
    ui->depopMultipolarity->setText(data.depopMultipolarity);
    ui->depopMixing->setText(data.depopMixing);

    ui->endEnergy->setText(data.endEnergy);
    ui->endSpin->setText(data.endSpin);

    ui->a22->setText(data.a22);
    ui->a24->setText(data.a24);
    ui->a42->setText(data.a42);
    ui->a44->setText(data.a44);

    g1curve->setVisible(false);
    g2curve->setVisible(false);

    if (!m_decay.isNull())
        updateEnergySpectrum();
}

void Nuclei::updateEnergySpectrum()
{
    if (m_decay.isNull()) {
        curve->setVisible(false);
        g1curve->setVisible(false);
        g2curve->setVisible(false);
        return;
    }

    curve->setVisible(true);

    double fwhm = eres->value()/100.0 * 662.0;

    QVector<double> x(m_decay->gammaSpectrumX(fwhm));
    QVector<double> y(m_decay->gammaSpectrumY(fwhm));

    QVector<double> y1(x.size());
    QVector<double> y2(x.size());
    if (!m_decay.isNull()) {
        y1 = m_decay->firstSelectedGammaSpectrumY(fwhm);
        y2 = m_decay->secondSelectedGammaSpectrumY(fwhm);
        if (!y1.isEmpty()) {
            QVector<double> y1lower(x.size());
            g1curve->setSamples(mergeIntervalData(x, y1lower, y1));
            g1curve->setVisible(true);
        }
        else {
            y1.resize(x.size());
        }
        if (!y2.isEmpty()) {
            std::transform(y2.begin(), y2.end(), y1.begin(), y2.begin(), std::plus<double>());
            g2curve->setSamples(mergeIntervalData(x, y1, y2));
            g2curve->setVisible(true);
        }
        else {
            y2.resize(x.size());
        }
    }

    // create curve interval
    QVector<QwtIntervalSample> curvedata(mergeIntervalData(x, y2, y));

    curve->setSamples(curvedata);

    if (ui->actionLinear->isChecked())
        plot->setAxisAutoScale(QwtPlot::yLeft);
    else
        plot->setAxisScale(QwtPlot::yLeft, 1E-8, 10.0);
    if (!x.isEmpty())
        plot->setAxisScale(QwtPlot::xBottom, 0.0, x.last());
    zoomer->setZoomBase();
}

void Nuclei::svgExport()
{
    QSettings s;

    if (ui->tabWidget->currentWidget() == ui->decayCascadeTab) {
        if (m_decay.isNull())
            return;

        QString fn(QFileDialog::getSaveFileName(this, "Save As", s.value("exportDir").toString(), "Scalable Vector Graphics (*.svg)"));
        if (fn.isEmpty())
            return;

        QRectF inrect = ui->decayView->scene()->sceneRect();
        QRectF outrect = inrect.adjusted(-10.0, -10.0, 10.0, 10.0);

        QSvgGenerator svgGen;
        svgGen.setFileName(fn);
        svgGen.setSize(outrect.toRect().size());
        svgGen.setViewBox(outrect);
        svgGen.setTitle("Decay Level Scheme for the decay " + m_decay->name());
        svgGen.setDescription(QString::fromUtf8("This scheme was created using Nuclei (" NUCLEIURL ")"));

        m_decay->setShadowEnabled(false);
        QPainter painter(&svgGen);
        ui->decayView->scene()->render(&painter, inrect, inrect);
        m_decay->setShadowEnabled(true);
    }
    else {
        QString fn(QFileDialog::getSaveFileName(this, "Save As", s.value("exportDir").toString(), "Scalable Vector Graphics (*.svg)"));
        if (fn.isEmpty())
            return;

        QwtPlotRenderer r(this);
        r.setLayoutFlags(QwtPlotRenderer::KeepFrames);
        r.renderDocument(plot, fn, "svg", QSizeF(180.0, 120.0), 300);
    }
}

void Nuclei::pdfExport()
{
    QSettings s;

    if (ui->tabWidget->currentWidget() == ui->decayCascadeTab) {
        if (m_decay.isNull())
            return;

        QString fn(QFileDialog::getSaveFileName(this, "Save As", s.value("exportDir").toString(), "Portable Document Format (*.pdf)"));
        if (fn.isEmpty())
            return;

        const int scalefactor = 10;
        const double margin = 3.0;

        QRectF inrect = ui->decayView->scene()->sceneRect();
        QRectF outrect = inrect.adjusted(-margin*scalefactor, -margin*scalefactor, margin*scalefactor, margin*scalefactor);

        QPrinter p(QPrinter::HighResolution);
        p.setOutputFileName(fn);
        p.setPageMargins(margin, margin, margin, margin, QPrinter::Millimeter);
        p.setOutputFormat(QPrinter::PdfFormat);
        p.setPaperSize(outrect.toRect().size() / scalefactor, QPrinter::Millimeter);
        p.setDocName("Decay Level Scheme for the decay " + m_decay->name());
        p.setCreator(QString("%1 %2 (%3)").arg(QCoreApplication::applicationName(), QCoreApplication::applicationVersion(), NUCLEIURL));

        m_decay->setShadowEnabled(false);
        QPainter painter(&p);
        ui->decayView->scene()->render(&painter);
        m_decay->setShadowEnabled(true);
    }
    else {
        QString fn(QFileDialog::getSaveFileName(this, "Save As", s.value("exportDir").toString(), "Portable Document Format (*.pdf)"));
        if (fn.isEmpty())
            return;

        QwtPlotRenderer r(this);
        r.setLayoutFlags(QwtPlotRenderer::KeepFrames);
        r.renderDocument(plot, fn, "pdf", QSizeF(180.0, 120.0), 300);
    }
}

void Nuclei::showAll()
{
    if (ui->tabWidget->currentWidget() == ui->decayCascadeTab) {
        QGraphicsScene *scene = ui->decayView->scene();
        if (scene)
            ui->decayView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
        return;
    }
    else {
        zoomer->zoom(0);
        return;
    }
}

void Nuclei::showOriginalSize()
{
    ui->decayView->setTransform(QTransform());
}

void Nuclei::zoomIn()
{
    if (ui->tabWidget->currentWidget() == ui->decayCascadeTab) {
        ui->decayView->scale(1.25, 1.25);
    }
    else {
        if (int(zoomer->zoomRectIndex() + 1) >= zoomer->zoomStack().size()) {
            const QwtScaleDiv *xs = plot->axisScaleDiv(QwtPlot::xBottom);
            const QwtScaleDiv *ys = plot->axisScaleDiv(QwtPlot::yLeft);
            QRectF cr(xs->lowerBound(), ys->lowerBound(), xs->range(), ys->range());
            cr = cr.normalized();
            zoomer->zoom(QRectF(cr.left()+0.1*cr.width(), cr.top()+0.1*cr.height(), 0.8*cr.width(), 0.8*cr.height()));
        }
        else {
            zoomer->zoom(1);
        }
    }
}

void Nuclei::zoomOut()
{
    if (ui->tabWidget->currentWidget() == ui->decayCascadeTab)
        ui->decayView->scale(0.8, 0.8);
    else
        zoomer->zoom(-1);
}

void Nuclei::search()
{
    searchDialog->show();
}

void Nuclei::searchFinished(SearchResultDataSource *result)
{
    searchResultSelectionModel->setDataSource(result);
    ui->selectionTabWidget->setCurrentWidget(ui->searchResultTab);
    ui->searchTreeView->resizeColumnToContents(0);
}

void Nuclei::setPlotLin()
{
    zoomer->zoom(0);
    ui->actionLogarithmic->setChecked(false);
    plot->setAxisAutoScale(QwtPlot::yLeft);
    plot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);
    zoomer->setZoomBase();
    ui->tabWidget->setCurrentWidget(ui->energySpectrumTab);
}

void Nuclei::setPlotLog()
{
    zoomer->zoom(0);
    ui->actionLinear->setChecked(false);
    plot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
    plot->setAxisScale(QwtPlot::yLeft, 1E-8, 10.0);
    zoomer->setZoomBase();
    ui->tabWidget->setCurrentWidget(ui->energySpectrumTab);
}

void Nuclei::showPreferences()
{
    preferencesDialog->exec();

    if (m_decay) {
        Decay::CascadeIdentifier ci = m_decay->currentSelection();
        m_decay->setCurrentSelection(ci);
    }
}

void Nuclei::showAbout()
{
    QMessageBox::about(this,
                       QString("About: %1 %2").arg(QCoreApplication::applicationName(), QCoreApplication::applicationVersion()),
                       QString::fromUtf8(NUCLEIABOUT "<hr />" LIBAKKABOUT "<hr />" GPL)
                       );
}

void Nuclei::processTabSelection(int index)
{
    if (index == ui->tabWidget->indexOf(ui->energySpectrumTab))
        ui->energySpectrumBar->setEnabled(true);
    else
        ui->energySpectrumBar->setDisabled(true);
}

void Nuclei::closeEvent(QCloseEvent *event)
{
    QSettings s;
    s.setValue("geometry", saveGeometry());
    s.setValue("windowState", saveState());
    QMainWindow::closeEvent(event);
}

QVector<QwtIntervalSample> Nuclei::mergeIntervalData(const QVector<double> &x, const QVector<double> &y1, const QVector<double> &y2)
{
    QVector<QwtIntervalSample> result(x.size());
    // replace 0 with min() to avoid display errors in log plot
    for (int i=0; i<x.size(); i++)
        result[i] = QwtIntervalSample(x.at(i), qMax(y1.at(i), std::numeric_limits<double>::min()), qMax(y2.at(i), std::numeric_limits<double>::min()));
    return result;
}

void Nuclei::loadDecay(QSharedPointer<Decay> decay)
{
    QSettings s;
    s.setValue("preferences/levelTolerance", preferencesDialogUi->levelDiff->value());
    s.setValue("preferences/gammaTolerance", preferencesDialogUi->gammaDiff->value());
    s.sync();

    m_decay = decay;

    connect(m_decay.data(), SIGNAL(updatedDecayData(Decay::DecayDataSet)), this, SLOT(updateDecayData(Decay::DecayDataSet)));
    m_decay->setStyle(preferencesDialogUi->fontFamily->currentFont(), preferencesDialogUi->fontSize->value());
    QGraphicsScene *scene = m_decay->levelPlot();
    ui->decayView->setScene(scene);
    ui->decayView->setSceneRect(scene->sceneRect().adjusted(-20, -20, 20, 20));

    // update plot
    updateEnergySpectrum();
    m_decay->triggerDecayDataUpdate();
}

