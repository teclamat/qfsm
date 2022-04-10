#ifndef OPTPRINTINGDLGIMPL_H
#define OPTPRINTINGDLGIMPL_H
#include "ui_OptPrintingDlg.h"

#include "Options.h"

class OptPrintingDlgImpl : public QDialog {
  Q_OBJECT

  Ui::OptPrintingDlg ui;

public:
  OptPrintingDlgImpl(QWidget *parent = 0, const char *name = 0,
                     bool modal = false, Qt::WindowFlags fl = {});
  ~OptPrintingDlgImpl();

  bool getPrintHeader() { return print_header; };
  void setPrintHeader(bool ph) { print_header = ph; };

  void init(Options *opt);

public slots:
  void printHeaderClicked();

private:
  bool print_header;
};

#endif // OPTPRINTINGDLGIMPL_H
