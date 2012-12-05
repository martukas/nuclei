#include "ENSDFDownloader.h"
#include "ui_ENSDFDownloader.h"
#include "ui_ENSDFDownloaderSettings.h"

#include <QSettings>
#include <QDir>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QFtp>
#include <QRegExp>
#include <QDesktopServices>
#include <quazip/JlCompress.h>
#include <iostream>

ENSDFDownloader::ENSDFDownloader(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ENSDFDownloader),
    ftp(new QFtp(this)), pendinghandle(0), ftpOutFile(0)
{
    ui->setupUi(this);

    defaultPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    if (defaultPath.isEmpty())
        defaultPath = qApp->applicationDirPath();
    defaultPath.append("/ensdf");

    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->ftpCancelButton, SIGNAL(clicked()), this, SLOT(resetFtpTransfer()));

    connect(ui->downloadButton, SIGNAL(clicked()), this, SLOT(download()));
    connect(ui->localButton, SIGNAL(clicked()), this, SLOT(selectLocal()));
    connect(ui->setupButton, SIGNAL(clicked()), this, SLOT(setupDownload()));

    connect(ftp, SIGNAL(commandFinished(int,bool)), this, SLOT(ftpDispatcher(int,bool)));
    connect(ftp, SIGNAL(listInfo(QUrlInfo)), this, SLOT(ftpAppendListInfo(QUrlInfo)));
    connect(ftp, SIGNAL(stateChanged(int)), this, SLOT(ftpProcessState(int)));
    connect(ftp, SIGNAL(dataTransferProgress(qint64,qint64)), this, SLOT(ftpUpdateProgressBar(qint64,qint64)));

    QSettings s;
    // initialize ENSDF path if necessary
    if (!s.contains("ensdfPath")) {
        s.setValue("ensdfPath", defaultPath);
        s.sync();
    }
}

ENSDFDownloader::~ENSDFDownloader()
{
    delete ui;
}

void ENSDFDownloader::download()
{
    QSettings s;

    // check if destination folder exists
    dest.setPath(s.value("ensdfPath").toString());
    dest.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);

    while (!dest.exists()) {
        // try to create default path, if saved does not exist
        if (!dest.mkpath(dest.path())) {
            QString p(QFileDialog::getExistingDirectory(this, "Select Database Folder", dest.absolutePath()));
            if (p.isEmpty())
                return;
            dest.setPath(p);
        }
    }

    // check if destination folder is empty
    while (dest.count()) {
        if (QMessageBox::Cancel == QMessageBox::warning(this, "Folder not empty", "Please select an empty folder for the download of the ENSDF database!", QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok))
            return;

        QString p(QFileDialog::getExistingDirectory(this, "Select Database Folder", dest.absolutePath()));
        if (p.isEmpty())
            return;
        dest.setPath(p);
    }

    // check if destination folder is writeable
    QFile statfile;
    for (;;) {
        statfile.setFileName(dest.filePath("statfile.txt"));
        if (statfile.open(QIODevice::WriteOnly))
            break;

        if (QMessageBox::Cancel == QMessageBox::warning(this, "Folder not writeable", "Please select a writeable folder for the download of the ENSDF database!", QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok))
            return;

        QString p(QFileDialog::getExistingDirectory(this, "Select Database Folder", dest.absolutePath()));
        if (p.isEmpty())
            return;
        dest.setPath(p);
    }
    statfile.remove();

    // download and extract zip files
    ui->stackedWidget->setCurrentWidget(ui->progressPage);
    ui->downloadButton->setDisabled(true);

    pendinghandle = ftp->connectToHost("ftp.nndc.bnl.gov");
    ftpstate = Connecting;
}

void ENSDFDownloader::selectLocal()
{
    QSettings s;
    QString p(QFileDialog::getExistingDirectory(this, "Select Database Folder", s.value("ensdfPath").toString()));
    if (p.isEmpty())
        return;

    dest.setPath(p);
    saveENSDFLocation();
    accept();
}

void ENSDFDownloader::setupDownload()
{
    QDialog d;
    Ui::ENSDFDownloaderSettings setui;
    setui.setupUi(&d);
    connect(setui.okButton, SIGNAL(clicked()), &d, SLOT(accept()));
    connect(setui.cancelButton, SIGNAL(clicked()), &d, SLOT(reject()));
    if (d.exec() == QDialog::Accepted) {
        ftp->setTransferMode(setui.activeRadio->isChecked() ? QFtp::Active : QFtp::Passive);
        ftp->setProxy(setui.hostEdit->text(), setui.portEdit->text().toShort());
    }
}

void ENSDFDownloader::ftpDispatcher(int id, bool error)
{
    if (pendinghandle != id)
        return;

    if (error && (ftpstate != NotConnected)) {
        resetFtpTransfer();
        QMessageBox::warning(this, "Connection Error", "An error occured while downloading the database files. Please check your settings and internet connection and try again.", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    pendinghandle = -1;

    switch (ftpstate) {
    case Connecting:
        pendinghandle = ftp->login("bnlndc");
        ftpstate = LoggingIn;
        break;
    case LoggingIn:
        pendinghandle = ftp->cd("/outgoing/ensdf");
        ftpstate = ChangingDirectory;
        break;
    case ChangingDirectory:
        pendinghandle = ftp->list(".");
        ftpstate = RequestingList;
        break;
    case RequestingList:
        if (!ftpFiles.isEmpty()) {
            ftpFilesRemaining = ftpFiles;
            ftpstate = Downloading;
        }
        else {
            resetFtpTransfer();
            QMessageBox::warning(this, "Database files not found", "Please try again later or download ENSDF manually.", QMessageBox::Ok, QMessageBox::Ok);
            return;
        }
        // NO break here!
    case Downloading:
        if (ftpOutFile) {
            ftpOutFile->close();
            delete ftpOutFile;
            ftpOutFile = 0;
        }
        if (!ftpFilesRemaining.isEmpty()) {
            QString fn = ftpFilesRemaining.takeFirst();
            ui->progressLabel->setText(QString("Downloading File %1 of %2...").arg(ftpFiles.size()-ftpFilesRemaining.size()).arg(ftpFiles.size()));
            ftpOutFile = new QFile(dest.filePath(fn));
            ftpOutFile->open(QIODevice::WriteOnly);
            pendinghandle = ftp->get(fn, ftpOutFile);
        }
        else
            processZipFiles();
        break;
    default:
        break;
    }
}

void ENSDFDownloader::ftpAppendListInfo(const QUrlInfo &url)
{
    if (!url.isValid())
        return;
    if (!url.isFile() || !url.isReadable())
        return;

    QRegExp rexp("^ensdf_[0-9]{6,6}_[0-9]{3,3}.zip$");
    if (rexp.exactMatch(url.name())) {
        ensdfVersion = url.name().mid(6, 6);
        ftpFiles.append(url.name());
    }
}

void ENSDFDownloader::ftpProcessState(int state)
{
    switch(state) {
    case QFtp::Unconnected:
        ui->progressLabel->setText("Unconnected.");
        break;
    case QFtp::HostLookup:
        ui->progressLabel->setText("Looking up host...");
        break;
    case QFtp::Connecting:
        ui->progressLabel->setText("Connecting...");
        break;
    case QFtp::Connected:
        ui->progressLabel->setText("Connected.");
        break;
    case QFtp::LoggedIn:
        ui->progressLabel->setText("Logged in.");
        break;
    case QFtp::Closing:
        ui->progressLabel->setText("Closing down connection...");
        break;
    }
}

void ENSDFDownloader::ftpUpdateProgressBar(qint64 done, qint64 total)
{
    ui->progressBar->setMaximum(total);
    ui->progressBar->setValue(done);
}

void ENSDFDownloader::resetFtpTransfer()
{
    // update gui
    ftp->abort();
    ftp->close();
    ui->progressBar->setValue(0);
    ui->stackedWidget->setCurrentWidget(ui->buttonPage);
    ui->downloadButton->setEnabled(true);

    // remove downloaded files
    foreach (QString fn, ftpFiles)
        if (dest.exists(fn))
            dest.remove(fn);

    // reset state
    ftpstate = NotConnected;
    pendinghandle = -1;
    ftpFiles.clear();
    ftpFilesRemaining.clear();
    if (ftpOutFile)
        delete ftpOutFile;
    ftpOutFile = 0;
            ensdfVersion = "";

}

void ENSDFDownloader::processZipFiles()
{
    foreach (QString fn, ftpFiles) {
        ui->progressBar->setRange(0, 0);
        ui->progressLabel->setText(QString("Extracting files..."));
        JlCompress::extractDir(dest.filePath(fn), dest.absolutePath());
    }
    ui->progressBar->setRange(0, 100);
    saveENSDFLocation();
    resetFtpTransfer();
    accept();
}

void ENSDFDownloader::saveENSDFLocation()
{
    QSettings s;
    s.setValue("ensdfPath", dest.absolutePath());
    s.setValue("ensdfVersion", ensdfVersion.isEmpty() ? "user-selected" : ensdfVersion);
}



