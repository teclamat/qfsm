#include "OptPrintingDlg.h"

#include <QCheckBox>

/*
 *  Constructs a OptPrintingDlgImpl which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
OptPrintingDlgImpl::OptPrintingDlgImpl(QWidget *parent, const char *name,
                                       bool modal, Qt::WindowFlags fl)
    : QDialog(parent, fl) {
  ui.setupUi(this);
  connect(ui.cb_print_header, &QCheckBox::clicked, this, &OptPrintingDlgImpl::printHeaderClicked);
}

/*
 *  Destroys the object and frees any allocated resources
 */
OptPrintingDlgImpl::~OptPrintingDlgImpl() {
  // no need to delete child widgets, Qt does it all for us
}

void OptPrintingDlgImpl::init(Options *opt) {
  print_header = opt->getPrintHeader();
  //  cb_print_header->setChecked(print_header);
}

/// Called when the 'Print header' checkbox is clicked
void OptPrintingDlgImpl::printHeaderClicked() {
  print_header = ui.cb_print_header->isChecked();
}

/// Called when the 'Draw Box' checkbox is clicked
