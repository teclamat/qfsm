#include "ExportStateTableDlg.h"

// #include <q3buttongroup.h>
#include <qcheckbox.h>
#include <QGroupBox>

#include "Options.h"

/*
 *  Constructs a ExportStateTableDlgImpl which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
ExportStateTableDlgImpl::ExportStateTableDlgImpl(QWidget *parent,
                                                 const char *name, bool modal,
                                                 Qt::WindowFlags fl)
    : QDialog(parent, fl) {
  ui.setupUi(this);
  connect(ui.cb_includeout, &QCheckBox::clicked, this, &ExportStateTableDlgImpl::includeOutputsClicked);
  connect(ui.pb_export, &QPushButton::clicked, this, &ExportStateTableDlgImpl::accept);
  connect(ui.pb_cancel, &QPushButton::clicked, this, &ExportStateTableDlgImpl::reject);
  connect(ui.bg_orientation, &QGroupBox::clicked, this, &ExportStateTableDlgImpl::orientationClicked);
  connect(ui.cb_resolve_invert, &QCheckBox::clicked, this, &ExportStateTableDlgImpl::resolveInvertedClicked);
}

/*
 *  Destroys the object and frees any allocated resources
 */
ExportStateTableDlgImpl::~ExportStateTableDlgImpl() {
  // no need to delete child widgets, Qt does it all for us
}

void ExportStateTableDlgImpl::includeOutputsClicked() {
  includeout = ui.cb_includeout->isChecked();
}

void ExportStateTableDlgImpl::resolveInvertedClicked() {
  resolve_inverted = ui.cb_resolve_invert->isChecked();
  qDebug("%d\n", resolve_inverted);
}

void ExportStateTableDlgImpl::orientationClicked(bool) {
  orientation = ui.rb_orientation1->isChecked();
}

/**
 * Initialises the dialog according to the current options @a opt.
 */
void ExportStateTableDlgImpl::init(Options *opt) {
  includeout = opt->getStateTableIncludeOut();
  ui.cb_includeout->setChecked(includeout);
  resolve_inverted = opt->getStateTableResolveInverted();
  ui.cb_resolve_invert->setChecked(resolve_inverted);
  orientation = opt->getStateTableOrientation();
  if (orientation) {
    ui.rb_orientation1->setChecked(true);
  } else {
    ui.rb_orientation0->setChecked(true);
  }
  // ui.bg_orientation->setButton(orientation);
}
