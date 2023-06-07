#ifndef ENSDFDOWNLOADER_H
#define ENSDFDOWNLOADER_H

#include <QDialog>
#include <QDir>

namespace Ui {
class ENSDFDownloader;
}
class QFtp;
class QUrlInfo;

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
    void ftpAppendListInfo(const QUrlInfo &url);
    void ftpProcessState(int state);
    void ftpUpdateProgressBar(qint64 done, qint64 total);
    void resetFtpTransfer();
    
private:
    void processZipFiles();
    void saveENSDFLocation();
    Ui::ENSDFDownloader *ui;
    QString defaultPath;
    QFtp *ftp;

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
