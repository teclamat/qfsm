#include "ExportVVVVDlg.h"

#include "Options.h"

ExportVVVVDlgImpl::ExportVVVVDlgImpl(QWidget *parent, const char *name,
                                     bool modal, Qt::WindowFlags fl)
    : QDialog(parent, fl) {
  // load ui design for the current object
  ui.setupUi(this);

  ui.pb_update->setVisible(false);
  connect(ui.pb_close, &QPushButton::clicked, this, &ExportVVVVDlgImpl::reject);
  connect(ui.cb_reset, &QCheckBox::clicked, this, &ExportVVVVDlgImpl::resetClicked);
  connect(ui.le_reset_event, &QLineEdit::textChanged, [this](QString) { resetEventChanged(); });
  connect(ui.le_reset_action, &QLineEdit::textChanged, [this](QString) { resetActionChanged(); });

  // connect(ui.pb_update, SIGNAL(clicked()), this,
  // SIGNAL(updateClicked()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
ExportVVVVDlgImpl::~ExportVVVVDlgImpl() {
  // no need to delete child widgets, Qt does it all for us
}

void ExportVVVVDlgImpl::init(Options *opt, Machine *m) {
  options = opt;
  vvvv_reset = opt->getVVVVReset();
  reset_event = opt->getVVVVResetEvent();
  reset_action = opt->getVVVVResetAction();
  ui.cb_reset->setChecked(vvvv_reset);
  ui.le_reset_event->setEnabled(vvvv_reset);
  ui.le_reset_event->setText(opt->getVVVVResetEvent());
  ui.le_reset_action->setEnabled(vvvv_reset);
  ui.le_reset_action->setText(opt->getVVVVResetAction());
}

void ExportVVVVDlgImpl::resetClicked() {
  vvvv_reset = ui.cb_reset->isChecked();
  ui.le_reset_event->setEnabled(vvvv_reset);
  ui.le_reset_action->setEnabled(vvvv_reset);
  emit updateCode();
}

/*
void ExportVVVVDlgImpl::updateCode()
{
}
*/

void ExportVVVVDlgImpl::resetEventChanged() {
  reset_event = ui.le_reset_event->text();
  options->setVVVVResetEvent(reset_event);
  emit updateCode();
}

void ExportVVVVDlgImpl::resetActionChanged() {
  reset_action = ui.le_reset_action->text();
  options->setVVVVResetAction(reset_action);
  emit updateCode();
}

void ExportVVVVDlgImpl::setText(QString s) {
  ui.tb_output->setText(s);
}
