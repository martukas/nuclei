#ifndef ENSDFDOWNLOADER_H
#define ENSDFDOWNLOADER_H

#include <QDialog>
#include <QDir>
#include <QUrl>
#include <QNetworkAccessManager>

namespace Ui {
class ENSDFDownloader;
}

class ENSDFDownloader : public QDialog
{
    Q_OBJECT
    
public:
    explicit ENSDFDownloader(QWidget *parent = 0);
    ~ENSDFDownloader();

private slots:
    void download();
    void selectLocal();
    void setupDownload();

    void ftpDispatcher(int id, bool error);
    void ftpAppendListInfo(const QUrl &url);
    void ftpProcessState(int state);
    void ftpUpdateProgressBar(qint64 done, qint64 total);
    void resetFtpTransfer();

//    void downloadFinished(QNetworkReply *reply);
    
private:
    void processZipFiles();
    void saveENSDFLocation();
    Ui::ENSDFDownloader *ui;
    QString defaultPath;
    QNetworkAccessManager manager;
//    QFtp *ftp;

    QDir dest;
    enum FtpState {
        NotConnected,
        Connecting,
        LoggingIn,
        ChangingDirectory,
        RequestingList,
        Downloading,
        Finished,
        Error
    } ftpstate;
    int pendinghandle;
    QStringList ftpFiles, ftpFilesRemaining;
    QFile *ftpOutFile;
    QString ensdfVersion;

};

#endif // ENSDFDOWNLOADER_H
