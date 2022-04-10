/*
Copyright (C) Stefan Duffner

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QFileDialog>
#include <QCheckBox>

#include "ExportVHDLDlg.h"
#include "Machine.h"
#include "Options.h"

/**
 *  Constructs a ExportVHDLDlgImpl which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
ExportVHDLDlgImpl::ExportVHDLDlgImpl(QWidget *parent, const char *name,
                                     bool modal, Qt::WindowFlags fl) {
  symbolic_states = false;
  inout_names = true;
  sync_reset = false;
  sync_enable = false;
  neg_reset = true;
  stdlogic = true;
  io_header = true;
  alliance = false;
  state_code = false;

  // load ui design for the current object
  ui.setupUi(this);

  connect(ui.pb_export, &QPushButton::clicked, this, &ExportVHDLDlgImpl::accept);
  connect(ui.pb_cancel, &QPushButton::clicked, this, &ExportVHDLDlgImpl::reject);
  // connects between ui elements and the current class must be done in the code
  connect(ui.cb_symbolicstates, SIGNAL(clicked()), this, SLOT(symbolicNamesClicked()));
  connect(ui.cb_inoutnames, SIGNAL(clicked()), this,
          SLOT(inOutNamesClicked()));
  connect(ui.cb_synchreset, SIGNAL(clicked()), this,
          SLOT(synchronousResetClicked()));
  connect(ui.cb_synchenable, SIGNAL(clicked()), this,
          SLOT(synchronousEnableClicked()));
  connect(ui.cb_negreset, SIGNAL(clicked()), this,
          SLOT(negResetClicked()));
  connect(ui.cb_stdlogic, SIGNAL(clicked()), this,
          SLOT(useStdLogicClicked()));
  connect(ui.cb_iodesc, SIGNAL(clicked()), this,
          SLOT(ioHeaderClicked()));
  connect(ui.cb_alliance, SIGNAL(clicked()), this,
          SLOT(allianceClicked()));
  connect(ui.cb_state_code, SIGNAL(clicked()), this,
          SLOT(stateCodeClicked()));
  connect(ui.bg_cond_notation, SIGNAL(clicked(bool)), this,
          SLOT(condNotationClicked(bool)));
  connect(ui.cb_sep_files, SIGNAL(clicked()), this,
          SLOT(sepFilesClicked()));
  connect(ui.cb_look_ahead, SIGNAL(clicked()), this,
          SLOT(lookAheadClicked()));
  connect(ui.le_architecture_name, SIGNAL(textChanged(QString)),
          this, SLOT(architectureNameChanged(QString)));
  connect(ui.le_architecture_file, SIGNAL(textChanged(QString)),
          this, SLOT(archtitecturePathChanged(QString)));
  connect(ui.le_entity_file, SIGNAL(textChanged(QString)), this,
          SLOT(entityPathChanged(QString)));
  connect(ui.pb_architecture_file, SIGNAL(clicked()), this,
          SLOT(selectArchitectureFileClicked()));
  connect(ui.pb_entity_file, SIGNAL(clicked()), this,
          SLOT(selectEntityFileClicked()));
  connect(ui.cb_debugstate, SIGNAL(clicked()), this,
          SLOT(debugStateClicked()));
}

/**
 *  Destroys the object and frees any allocated resources
 */
ExportVHDLDlgImpl::~ExportVHDLDlgImpl() {
  // no need to delete child widgets, Qt does it all for us
}

/**
 * public slot
 */
void ExportVHDLDlgImpl::symbolicNamesClicked() {
  symbolic_states = ui.cb_symbolicstates->isChecked();
}

void ExportVHDLDlgImpl::inOutNamesClicked() {
  inout_names = ui.cb_inoutnames->isChecked();
}

/**
 * public slot
 */
void ExportVHDLDlgImpl::synchronousResetClicked() {
  sync_reset = ui.cb_synchreset->isChecked();
}

/**
 * public slot
 */
void ExportVHDLDlgImpl::synchronousEnableClicked() {
  sync_enable = ui.cb_synchenable->isChecked();
}

/**
 * public slot
 */
void ExportVHDLDlgImpl::negResetClicked() {
  neg_reset = ui.cb_negreset->isChecked();
}

/**
 * public slot
 */
void ExportVHDLDlgImpl::useStdLogicClicked() {
  stdlogic = ui.cb_stdlogic->isChecked();
}

/**
 * public slot
 */
void ExportVHDLDlgImpl::ioHeaderClicked() {
  io_header = ui.cb_iodesc->isChecked();
}

/**
 * public slot
 */
void ExportVHDLDlgImpl::stateCodeClicked() {
  state_code = ui.cb_state_code->isChecked();
}

/**
 * public slot
 */
void ExportVHDLDlgImpl::allianceClicked() {
  alliance = ui.cb_alliance->isChecked();
  if (alliance) {
    ui.cb_symbolicstates->setEnabled(false);
    ui.cb_inoutnames->setEnabled(false);
    ui.cb_synchreset->setEnabled(false);
    ui.bg_cond_notation->setEnabled(false);
  } else {
    ui.cb_symbolicstates->setEnabled(true);
    ui.cb_inoutnames->setEnabled(true);
    ui.cb_synchreset->setEnabled(true);
    ui.bg_cond_notation->setEnabled(true);
  }
}

/**
 * Initialises the dialog according to the current options @a opt.
 */
void ExportVHDLDlgImpl::init(Options *opt, Machine *m) {
  symbolic_states = opt->getVHDLSymbolicStates();
  ui.cb_symbolicstates->setChecked(symbolic_states);
  inout_names = opt->getVHDLInOutNames();
  ui.cb_inoutnames->setChecked(inout_names);
  sync_reset = opt->getVHDLSyncReset();
  ui.cb_synchreset->setChecked(sync_reset);
  sync_enable = opt->getVHDLSyncEnable();
  ui.cb_synchenable->setChecked(sync_enable);
  neg_reset = opt->getVHDLNegReset();
  ui.cb_negreset->setChecked(neg_reset);
  stdlogic = opt->getVHDLStdLogic();
  ui.cb_stdlogic->setChecked(stdlogic);
  io_header = opt->getVHDLIOheader();
  ui.cb_iodesc->setChecked(io_header);
  alliance = opt->getVHDLAlliance();
  ui.cb_alliance->setChecked(alliance);
  cond_notation = opt->getVHDLCondNotation();
  state_code = opt->getVHDLStateCode();
  sep_files = opt->getVHDLSepFiles();
  path_entity = opt->getVHDLEntityPath();
  path_architecture = opt->getVHDLArchitecturePath();

  ui.cb_sep_files->setChecked(sep_files);
  sync_look_ahead = opt->getVHDLSyncLookAhead();
  ui.cb_look_ahead->setChecked(sync_look_ahead);
  architecture_name = opt->getVHDLArchitectureName();
  ui.le_architecture_name->setText(architecture_name);
  ui.cb_state_code->setChecked(state_code);
  if (cond_notation) {
    ui.rb_ifthen->setChecked(true);
  } else {
    ui.rb_casewhen->setChecked(true);
  }
  // ui.bg_cond_notation->setButton(cond_notation);
  setSepFiles(sep_files);

  if (m == NULL)
    machine_name = "";
  else
    machine_name = m->getName();
  if (path_entity.length() > 0) {
    if (sep_files) {
      path_entity = path_entity.left(path_entity.lastIndexOf("/") + 1) + "e_" +
                    machine_name + ".vhd";
      path_architecture =
          path_architecture.left(path_architecture.lastIndexOf("/") + 1) +
          "a_" + machine_name + "_" + architecture_name + ".vhd";
    } else
      path_entity = path_entity.left(path_entity.lastIndexOf("/") + 1) +
                    machine_name + ".vhd";
    ui.le_entity_file->setText(path_entity);
    ui.le_architecture_file->setText(path_architecture);
  } else
    ui.pb_export->setEnabled(false);

  if (m && m->getType() == 0) // Binary machine
  {
    ui.cb_alliance->setEnabled(true);
    ui.cb_inoutnames->setEnabled(true);
  } else {
    ui.cb_alliance->setEnabled(false);
    ui.cb_inoutnames->setEnabled(false);
  }

  if (alliance && m && m->getType() == 0) {
    ui.cb_symbolicstates->setEnabled(false);
    ui.cb_inoutnames->setEnabled(false);
    ui.cb_synchreset->setEnabled(false);
    ui.bg_cond_notation->setEnabled(false);
  } else {
    ui.cb_symbolicstates->setEnabled(true);
    // ui.cb_inoutnames->setEnabled(true);
    ui.cb_synchreset->setEnabled(true);
    ui.bg_cond_notation->setEnabled(true);
  }
}

/**
 * public slot
 */
void ExportVHDLDlgImpl::condNotationClicked(bool) { cond_notation = static_cast<int>(ui.rb_ifthen->isChecked()); }

void ExportVHDLDlgImpl::debugStateClicked() {
  debug_state = ui.cb_debugstate->isChecked();
}

void ExportVHDLDlgImpl::sepFilesClicked() {
  sep_files = ui.cb_sep_files->isChecked();
  setSepFiles(sep_files);
}

void ExportVHDLDlgImpl::lookAheadClicked() {
  sync_look_ahead = ui.cb_look_ahead->isChecked();
}

/**
 * public slot
 */
void ExportVHDLDlgImpl::architectureNameChanged(QString) {
  architecture_name = ui.le_architecture_name->text();
  qDebug() << "architecture name chaned:" << architecture_name;
}

/**
 * public slot
 * Show file dialog to get entity file name
 */
void ExportVHDLDlgImpl::selectArchitectureFileClicked() {

  if (path_architecture.length() == 0)
    path_architecture = "a_" + machine_name + "_" + architecture_name + ".vhd";

  QString path_architecture_temp = QFileDialog::getSaveFileName(
      this, "", path_architecture, "VHDL (*.vhd *.vhdl);;All Files (*)", 0,
      QFileDialog::DontConfirmOverwrite);

  if (path_architecture_temp.length() > 0) {
    ui.le_architecture_file->setText(path_architecture_temp);
    path_architecture = path_architecture_temp;
  }
}

/**
 * Public slot;
 * Show file dialog to get entity file name
 */
void ExportVHDLDlgImpl::selectEntityFileClicked() {
  if (path_entity.length() <= 0) {
    if (sep_files)
      path_entity = "e_" + machine_name + ".vhd";
    else
      path_entity = machine_name + ".vhd";
  }

  QString path_entity_temp = QFileDialog::getSaveFileName(
      this, "", path_entity, "VHDL (*.vhd *.vhdl);;All Files (*)", 0,
      QFileDialog::DontConfirmOverwrite);

  if (path_entity_temp.length() > 0) {
    ui.le_entity_file->setText(path_entity_temp);
    path_entity = path_entity_temp;
  }
}

/**
 * Public slot to update the entity export path
 */
void ExportVHDLDlgImpl::entityPathChanged(QString new_path) {
  path_entity = new_path;
  if (path_entity.length() > 0)
    ui.pb_export->setEnabled(true);
  else
    ui.pb_export->setEnabled(false);
}

/**
 * Public slot to update the architecture export path
 */
void ExportVHDLDlgImpl::archtitecturePathChanged(QString new_path) {
  path_architecture = new_path;
}

/// Set the flag for exporting the state machine in two files (architecture and
/// entity)
void ExportVHDLDlgImpl::setSepFiles(bool sep) {
  sep_files = sep;
  if (sep_files) {
    ui.l_entity_file->setText(tr("Entity File Name"));
    ui.le_architecture_file->setEnabled(true);
    ui.pb_architecture_file->setEnabled(true);
    ui.l_architecture_file->setEnabled(true);
  } else {
    ui.l_entity_file->setText(tr("Export File Name"));
    ui.le_architecture_file->setEnabled(false);
    ui.pb_architecture_file->setEnabled(false);
    ui.l_architecture_file->setEnabled(false);
  }
}
