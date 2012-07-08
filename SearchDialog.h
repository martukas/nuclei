#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include "SearchConstraints.h"

class SearchResultDataSource;
class AbstractDataSource;

namespace Ui {
class SearchDialog;
}

class SearchDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SearchDialog(QWidget *parent = 0);
    ~SearchDialog();

    void setDataSource(AbstractDataSource *datasource);

    SearchConstraints searchConstraints() const;
    void setSearchConstraints(const SearchConstraints &sc);

public slots:
    virtual void accept();

signals:
    void resultAvailable(SearchResultDataSource *result);
    
private:
    Ui::SearchDialog *ui;
    AbstractDataSource *ds;
};

#endif // SEARCHDIALOG_H
