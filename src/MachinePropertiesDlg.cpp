/*
Copyright (C) 2000,2001 Stefan Duffner

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

#include "MachinePropertiesDlg.h"
// #include <q3buttongroup.h>
#include <qfontdialog.h>
#include <qradiobutton.h>

// #include "ListBoxItems.h"

/**
 *  Constructs a MachinePropertiesDlhImpl which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
MachinePropertiesDlgImpl::MachinePropertiesDlgImpl(QWidget *parent,
                                                   const char *name, bool modal,
                                                   Qt::WindowFlags fl)
    : QDialog(parent, fl) {
  ui.setupUi(this);

  ui.cb_arrowType->addItems({"Line Arrow", "Filled Arrow", "White Arrow", "Filled Pointed", "White Pointed"});

  connect(ui.pb_ok, &QPushButton::clicked, this, &MachinePropertiesDlgImpl::accept);
  connect(ui.pb_cancel, &QPushButton::clicked, this, &MachinePropertiesDlgImpl::reject);
  connect(ui.pb_transfont, &QPushButton::clicked, this, &MachinePropertiesDlgImpl::chooseTFont);
  connect(ui.pb_statefont, &QPushButton::clicked, this, &MachinePropertiesDlgImpl::chooseSFont);
  connect(ui.cb_inittrans, &QCheckBox::clicked, this, &MachinePropertiesDlgImpl::drawITransClicked);
  connect(ui.rb_text, &QRadioButton::clicked, this, &MachinePropertiesDlgImpl::freeTextClicked);
  connect(ui.rb_binary, &QRadioButton::clicked, this, &MachinePropertiesDlgImpl::binaryClicked);
  connect(ui.rb_ascii, &QRadioButton::clicked, this, &MachinePropertiesDlgImpl::asciiClicked);

  // lb1 = new LBArrowLine(0);          // lbox_arrowtype);
  // lb2 = new LBArrowFilled(0);        // lbox_arrowtype);
  // lb3 = new LBArrowWhite(0);         // lbox_arrowtype);
  // lb4 = new LBArrowFilledPointed(0); // lbox_arrowtype);
  // lb5 = new LBArrowWhitePointed(0);  // lbox_arrowtype);

  // ui.lbox_arrowtype->insertItem(lb1);
  // ui.lbox_arrowtype->insertItem(lb2);
  // ui.lbox_arrowtype->insertItem(lb3);
  // ui.lbox_arrowtype->insertItem(lb4);
  // ui.lbox_arrowtype->insertItem(lb5);

  // ui.lbox_arrowtype->setVariableHeight(true);

  // connects
  /*  QObject::connect(pb_ok, SIGNAL(clicked()), MachinePropertiesDlg,
    SLOT(accept())); QObject::connect(pb_cancel, SIGNAL(clicked()),
    MachinePropertiesDlg, SLOT(reject())); QObject::connect(pb_statefont,
    SIGNAL(clicked()), MachinePropertiesDlg, SLOT(chooseSFont()));
    QObject::connect(pb_transfont, SIGNAL(clicked()), MachinePropertiesDlg,
    SLOT(chooseTFont())); QObject::connect(rb_binary, SIGNAL(clicked()),
    MachinePropertiesDlg, SLOT(binaryClicked())); QObject::connect(rb_ascii,
    SIGNAL(clicked()), MachinePropertiesDlg, SLOT(asciiClicked()));
    QObject::connect(rb_text, SIGNAL(clicked()), MachinePropertiesDlg,
    SLOT(freeTextClicked()));
    QObject::connect(cb_inittrans, SIGNAL(clicked()), MachinePropertiesDlg,
    SLOT(drawITransClicked()));*/
}

/**
 *  Destroys the object and frees any allocated resources
 */
MachinePropertiesDlgImpl::~MachinePropertiesDlgImpl() {
  // delete lb1;
  // delete lb2;
  // delete lb3;
  // delete lb4;
  // delete lb5;
}

/**
 * Lets you choose a font for the state names.
 * Opens a font dialog box and you can choose a font which will be used to draw
 * the state names.
 */
void MachinePropertiesDlgImpl::chooseSFont() {
  bool ok;
  QFont f;
  f = QFontDialog::getFont(&ok, sfont, this);

  if (ok) {
    ui.lb_statefont->setText(f.family());
    ui.lb_statefont->setFont(f);
    sfont = f;
  }
}

/**
 * Lets you choose a font for the conditions of the transitions.
 * Opens a font dialog box and you can choose a font which will be used to draw
 * the conditions for the transitions.
 */
void MachinePropertiesDlgImpl::chooseTFont() {
  bool ok;
  QFont f;
  f = QFontDialog::getFont(&ok, tfont, this);

  if (ok) {
    ui.lb_transfont->setText(f.family());
    ui.lb_transfont->setFont(f);
    tfont = f;
  }
}

/// Returns the type of the machine. 0: Binary / 1: ASCII
int MachinePropertiesDlgImpl::getType() {

  if (ui.rb_ascii->isChecked())
    return 1;
  if (ui.rb_text->isChecked())
    return 2;
  else
    return 0;

  //  return bg_type->selectedId();

  /*
  if (rb_binary->isChecked())
    return 0;
  else if
    return 1;
    */
}

/// Sets the type of the machine to @a t
void MachinePropertiesDlgImpl::setType(int t) {
  if (t == 1) {
    ui.rb_ascii->setChecked(true);
    asciiClicked();
  } else if (t == 0) {
    ui.rb_binary->setChecked(true);
    binaryClicked();
  } else {
    ui.rb_text->setChecked(true);
    freeTextClicked();
  }
}

void MachinePropertiesDlgImpl::enableType(bool et = true) {
  ui.bg_type->setEnabled(et);
}

/// Called when the 'Binary' radio button is clicked
void MachinePropertiesDlgImpl::binaryClicked() {
  ui.bg_moore->setEnabled(true);
  ui.bg_mealy->setEnabled(true);

  ui.sb_mooreout->setEnabled(true);
  ui.sb_mealyin->setEnabled(true);
  ui.sb_mealyout->setEnabled(true);
  ui.le_mealyin->setEnabled(true);
  ui.le_mealyout->setEnabled(true);
  ui.le_mooreout->setEnabled(true);
}

/// Called when the 'ASCII' radio button is clicked
void MachinePropertiesDlgImpl::asciiClicked() {
  ui.bg_moore->setEnabled(false);
  ui.bg_mealy->setEnabled(false);

  saved_numbits = ui.sb_mooreout->value();
  saved_numin = ui.sb_mealyin->value();
  saved_numout = ui.sb_mealyout->value();

  ui.sb_mealyin->setEnabled(false);
  ui.sb_mealyout->setEnabled(false);
  ui.sb_mooreout->setEnabled(false);
  ui.le_mealyin->setEnabled(false);
  ui.le_mealyout->setEnabled(false);
  ui.le_mooreout->setEnabled(false);

  ui.sb_mealyin->setValue(8);
  ui.sb_mealyout->setValue(8);
  ui.sb_mooreout->setValue(8);
}

void MachinePropertiesDlgImpl::freeTextClicked() {
  ui.bg_moore->setEnabled(false);
  ui.bg_mealy->setEnabled(false);

  ui.sb_mealyin->setValue(0);
  ui.sb_mealyout->setValue(0);
  ui.sb_mooreout->setValue(0);
}

void MachinePropertiesDlgImpl::drawITransClicked() {}
