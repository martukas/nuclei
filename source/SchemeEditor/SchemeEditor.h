#pragma once

#include <QWidget>
#include <QPointer>
#include "SchemeGraphics.h"

namespace Ui
{
class SchemeEditor;
}

class SchemeEditor : public QWidget
{
  Q_OBJECT

public:
  explicit SchemeEditor(QWidget *parent = 0);
  ~SchemeEditor();

  void loadDecay(DecayScheme decay);

private slots:
  void playerSelectionChanged();
  void on_checkFilterTransitions_clicked();
  void on_doubleTargetTransition_editingFinished();
  void on_doubleMinIntensity_editingFinished();

  void on_pushShowAll_clicked();
  void on_pushExportSvg_clicked();
  void on_pushExportPdf_clicked();
  void on_pushPrefs_clicked();

private:

  Ui::SchemeEditor *ui {nullptr};

  QPointer<SchemeGraphics> decay_viewer_;
  DecayScheme current_scheme_;

  std::string make_reference_link(std::string ref, int num);
  QString prep_comments(const json& j,
                        const std::set<std::string>& refs);
  void set_text(const json& jj);

  void refresh_scheme();
};
