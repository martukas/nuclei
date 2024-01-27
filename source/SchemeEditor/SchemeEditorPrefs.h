#pragma once

#include <QDialog>
#include <QPointer>
#include "SchemeVisualSettings.h"

namespace Ui
{
class SchemeEditorPrefs;
}

class SchemeEditorPrefs : public QDialog
{
    Q_OBJECT

  public:
    explicit SchemeEditorPrefs(SchemeVisualSettings prefs, QWidget *parent = 0);
    ~SchemeEditorPrefs();

    SchemeVisualSettings prefs() const;

  private slots:

    void on_fontFamily_activated();

    void on_fontSize_editingFinished();

    void colChanged(QColor);

  private:
    Ui::SchemeEditorPrefs *ui {nullptr};
    SchemeVisualSettings prefs_;
};
