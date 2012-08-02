#ifndef UPDATECHECK_H
#define UPDATECHECK_H

#include <QObject>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

class UpdateCheck : public QObject
{
    Q_OBJECT
public:
    explicit UpdateCheck(QObject *parent = 0);
    
signals:
    
private slots:
    void replyFinished(QNetworkReply *reply);
    void openWebsite();

private:
    QNetworkAccessManager *manager;
    QUrl url;
    int redirectcount;
};

#endif // UPDATECHECK_H
