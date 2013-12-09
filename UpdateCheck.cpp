#include "UpdateCheck.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtDebug>
#include <QStringList>
#include <QRegExp>
#include <QDialog>
#include <QDesktopServices>
#include <ui_UpdateCheckDialog.h>
#include "version.h"

UpdateCheck::UpdateCheck(QObject *parent) :
    QObject(parent), url(NUCLEIVERSIONURL), redirectcount(0)
{
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("User-Agent", "Nuclei " VERSION);
    qDebug() << "Requesting " << url.toString();
    manager->get(request);
}

void UpdateCheck::replyFinished(QNetworkReply *reply)
{
    if (!reply)
        return;
    if (reply->error() != QNetworkReply::NoError)
        return;

    // process redirection
    QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (!redirectionTarget.isNull() && redirectcount < 5) {
        redirectcount++;

        QNetworkRequest request;
        url = url.resolved(redirectionTarget.toUrl());
        request.setUrl(url);
        request.setRawHeader("Nuclei", "Nuclei " VERSION);
        qDebug() << "Redirected to " << url.toString();

        reply->deleteLater();
        manager->get(request);
        return;
    }

    // process contents if HTTP 200 was replied
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
        QStringList versionfile = QString::fromUtf8(reply->readAll()).split("\n");

        // find current version in file
        int idx = versionfile.indexOf(QString("Version %1").arg(VERSION));
        if (idx < 0)
            return;
        idx = versionfile.indexOf(QRegExp("^Version (.*)$"), idx+1);
        if (idx < 0)
            return;
        for (int i=0; i<idx; i++)
            versionfile.removeFirst();

        QWidget *pwid = qobject_cast<QWidget*>(parent());
        QDialog *dialog = new QDialog(pwid);
        Ui::UpdateCheckDialog *ui = new Ui::UpdateCheckDialog();
        ui->setupUi(dialog);
        connect(dialog, SIGNAL(accepted()), this, SLOT(openWebsite()));

        // convert version info
        QString versiontext;
        bool infeatureblock = false;
        foreach (const QString &line, versionfile) {
            if (line.startsWith("Version ")) {
                if (infeatureblock) {
                    versiontext.append("</ul>");
                    infeatureblock = false;
                }
                versiontext.append("<h4>" + line + "</h4>\n");
            }
            else if (line.startsWith("    - ")) {
                if (!infeatureblock) {
                    versiontext.append("<ul>");
                    infeatureblock = true;
                }
                versiontext.append("<li>" + line.mid(6) + "</li>");
            }
        }
        if (infeatureblock)
            versiontext.append("</ul>");

        ui->changelog->setHtml(versiontext);

        dialog->show();
    }

    reply->deleteLater();
}

void UpdateCheck::openWebsite()
{
    QDesktopServices::openUrl(QUrl(NUCLEIURL));
}
