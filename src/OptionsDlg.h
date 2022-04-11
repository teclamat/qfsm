#ifndef SRC_OPTIONSDLG_H
#define SRC_OPTIONSDLG_H

#include <QDialog>
#include <QWidget>

#include "ui_OptionsDlg.h"

class OptionsDlg : public QDialog {
  Q_OBJECT

 public:
  OptionsDlg(QWidget* a_parent = nullptr);
  ~OptionsDlg() = default;
  Ui::OptionsDlg ui;
};

#endif // SRC_OPTIONSDLG_H
