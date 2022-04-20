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

#include "gui/error.hpp"

#include <qradiobutton.h>

#include "TransitionPropertiesDlg.h"

#include "Const.h"
#include "Transition.h"
#include "TransitionInfo.h"

#include <QPushButton>

/**
 *  Constructs a TransitionPropertiesDlgImpl which is a child of 'parent', with
 * the name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
TransitionPropertiesDlgImpl::TransitionPropertiesDlgImpl(QWidget* parent, const char* name, bool modal,
                                                         Qt::WindowFlags fl)
  : QDialog(parent, fl)
{
  ui.setupUi(this);

  connect(ui.pb_ok, &QPushButton::clicked, this, &TransitionPropertiesDlgImpl::validate);
  connect(ui.pb_cancel, &QPushButton::clicked, this, &TransitionPropertiesDlgImpl::reject);
  connect(ui.rb_ascii, &QRadioButton::clicked, this, &TransitionPropertiesDlgImpl::asciiClicked);
  connect(ui.rb_bin, &QRadioButton::clicked, this, &TransitionPropertiesDlgImpl::binaryClicked);
  connect(ui.rb_text, &QRadioButton::clicked, this, &TransitionPropertiesDlgImpl::freeTextClicked);
  connect(ui.cb_invert, &QCheckBox::clicked, this, &TransitionPropertiesDlgImpl::invertClicked);
  connect(ui.cb_default, &QCheckBox::clicked, this, &TransitionPropertiesDlgImpl::defaultClicked);
  connect(ui.cb_any, &QCheckBox::clicked, this, &TransitionPropertiesDlgImpl::anyClicked);
}

/**
 *  Destroys the object and frees any allocated resources
 */
TransitionPropertiesDlgImpl::~TransitionPropertiesDlgImpl() {}

/// Validates the strings entered as the transition inputs and outputs
void TransitionPropertiesDlgImpl::validate()
{
  QString in, out;
  int type;
  int cres;

  //  in = ui.le_input->text();
  in = getInputs();
  out = ui.le_output->text();
  type = getType();

  cres = Transition::conditionValid(type, in);
  if (cres) {
    switch (cres) {
      case 1:
        qfsm::gui::error::info(tr("Input condition is not in binary format."));
        break;
      case 2:
        qfsm::gui::error::info(tr("Incomplete escape sequence."));
        break;
      case 3:
        qfsm::gui::error::info(tr("'-' has to define a range."));
        break;
      case 4:
        qfsm::gui::error::info(tr("Output must be only one character."));
        break;
      case 5:
        qfsm::gui::error::info(
            tr("Invalid escape sequence.\nThe format is \\0xx, where xx is "
               "a\nhexadecimal value"));
        break;
      default:
        qfsm::gui::error::info(tr("Input condition is not in the correct format."));
        break;
    }
    return;
  } else {
    cres = Transition::conditionValid(type, out, false);
    if (cres) {
      switch (cres) {
        case 1:
          qfsm::gui::error::info(tr("Output condition is not in binary format."));
          break;
        case 2:
          qfsm::gui::error::info(tr("Incomplete escape sequence."));
          break;
        case 3:
          qfsm::gui::error::info(tr("'-' has to define a range."));
          break;
        case 4:
          qfsm::gui::error::info(tr("Output must be only one character."));
          break;
        case 5:
          qfsm::gui::error::info(
              tr("Invalid escape sequence.\nThe format is \\0xx, where xx "
                 "is a\nhexadecimal value"));
          break;
        default:
          qfsm::gui::error::info(tr("Output condition is not in the correct format."));
          break;
      }
      return;
    } else
      accept();
  }
}

/// Returns the type of the transition
int TransitionPropertiesDlgImpl::getType()
{
  if (ui.rb_ascii->isChecked())
    return 1;
  if (ui.rb_text->isChecked())
    return 2;
  else
    return 0;

  //  return bg_type->selectedId();

  /*
  if (rb_ascii->isChecked())
    return Ascii;
  else
    return Binary;
    */
}

void TransitionPropertiesDlgImpl::resetFields()
{
  ui.cb_default->setEnabled(false);
  ui.cb_invert->setEnabled(false);
  ui.cb_any->setEnabled(false);
  ui.le_input->setEnabled(true);
  ui.le_input->clear();
  ui.le_output->clear();
}

/// Sets the type of the transition
void TransitionPropertiesDlgImpl::setType(int t)
{
  // qDebug("setType %d", t);
  if (t == Binary) {
    ui.rb_bin->setEnabled(true);
    ui.rb_ascii->setEnabled(true);
    ui.rb_text->setEnabled(false);
    ui.rb_bin->setChecked(true);
    binaryClicked();
  }
  if (t == Ascii) {
    ui.rb_bin->setEnabled(true);
    ui.rb_ascii->setEnabled(true);
    ui.rb_text->setEnabled(false);

    ui.rb_ascii->setChecked(true);
    // ui.cb_default->setEnabled(true); // not needed -
    // this is done in asciiClicked() below
    asciiClicked();
  } else if (t == Text) {
    ui.rb_bin->setEnabled(false);
    ui.rb_ascii->setEnabled(false);
    ui.rb_text->setEnabled(true);

    ui.rb_text->setChecked(true);
    /*
     // not needed - this is done in freeTextClicked() below
    ui.cb_default->setEnabled(false);
    ui.cb_default->setChecked(false);
    ui.cb_any->setEnabled(false);
    ui.cb_any->setChecked(false);
    ui.cb_invert->setEnabled(false);
    ui.cb_invert->setChecked(false);
    */
    freeTextClicked();
  }
  if (ui.cb_default->isEnabled())
    defaultClicked();
  if (ui.cb_any->isEnabled())
    anyClicked();
  if (ui.cb_invert->isEnabled())
    invertClicked();
}

/// Called when the 'Binary' button is clicked
void TransitionPropertiesDlgImpl::binaryClicked()
{
  // qDebug("binaryClicked()");
  ui.le_input->setMaxLength(binmax_in);
  ui.le_output->setMaxLength(binmax_out);

  ui.cb_default->setEnabled(true);
  ui.cb_any->setEnabled(true);
  ui.cb_invert->setEnabled(true);

  if (ui.cb_default->isChecked()) {
    ui.le_input->setEnabled(false);
    ui.cb_any->setEnabled(false);
    ui.cb_invert->setEnabled(false);

  } else if (ui.cb_any->isChecked()) {
    ui.cb_default->setEnabled(false);
    ui.le_input->setEnabled(false);
    ui.cb_invert->setEnabled(false);
  } else if (ui.cb_invert->isChecked()) {
    ui.cb_default->setEnabled(false);
    ui.cb_any->setEnabled(false);
  }
}

/// Called when the 'ASCII' button is clicked
void TransitionPropertiesDlgImpl::asciiClicked()
{
  // qDebug("ASCIIClicked()");
  ui.le_input->setMaxLength(MAX_ASCII_INPUTLENGTH);
  ui.le_output->setMaxLength(MAX_ASCII_OUTPUTLENGTH);

  ui.cb_default->setEnabled(true);
  ui.cb_any->setEnabled(true);
  ui.cb_invert->setEnabled(true);

  if (ui.cb_default->isChecked()) {
    ui.le_input->setEnabled(false);
    ui.cb_any->setEnabled(false);
    ui.cb_invert->setEnabled(false);

  } else if (ui.cb_any->isChecked()) {
    ui.cb_default->setEnabled(false);
    ui.le_input->setEnabled(false);
    ui.cb_invert->setEnabled(false);
  } else if (ui.cb_invert->isChecked()) {
    ui.cb_default->setEnabled(false);
    ui.cb_any->setEnabled(false);
  }
}

void TransitionPropertiesDlgImpl::freeTextClicked()
{
  // qDebug("freeTextClicked()");
  ui.le_input->setMaxLength(MAX_TEXT_INPUTLENGTH);
  ui.le_output->setMaxLength(MAX_TEXT_OUTPUTLENGTH);

  ui.cb_default->setEnabled(true);
  ui.cb_any->setEnabled(false);
  ui.cb_any->setChecked(false);
  ui.cb_invert->setEnabled(false);
  ui.cb_invert->setChecked(false);
  /*
if (!cb_any->isChecked() && !cb_invert->isChecked())
  cb_default->setEnabled(true);
else
  cb_default->setEnabled(false);
defaultClicked();
anyClicked();
invertClicked();
*/
  if (ui.cb_default->isChecked()) {
    ui.le_input->setEnabled(false);
  }
  if (ui.cb_default->isEnabled())
    defaultClicked();
  /*
  // no ANY or INVERT in text conditions
  if (ui.cb_any->isEnabled())
    anyClicked();
  if (ui.cb_invert->isEnabled())
    invertClicked();
    */
}

void TransitionPropertiesDlgImpl::anyClicked()
{
  if (ui.cb_any->isChecked()) {
    ui.cb_invert->setEnabled(false);
    ui.cb_default->setEnabled(false);
    ui.le_input->setEnabled(false);
  } else {
    ui.cb_invert->setEnabled(true);
    ui.cb_default->setEnabled(true);
    ui.le_input->setEnabled(true);
  }
}

void TransitionPropertiesDlgImpl::defaultClicked()
{
  if (ui.cb_default->isChecked()) {
    ui.cb_invert->setEnabled(false);
    ui.cb_any->setEnabled(false);
    ui.le_input->setEnabled(false);
    ui.le_input->setText("");
  } else {
    if (getType() != Text) {
      ui.cb_invert->setEnabled(true);
      ui.cb_any->setEnabled(true);
    }
    ui.le_input->setEnabled(true);
  }
}

void TransitionPropertiesDlgImpl::invertClicked()
{
  if (ui.cb_invert->isChecked()) {
    ui.le_input->setEnabled(true);
    ui.cb_default->setEnabled(false);
    ui.cb_any->setEnabled(false);
  } else {
    ui.cb_default->setEnabled(true);
    ui.cb_any->setEnabled(true);
    ui.cb_default->setEnabled(true);
  }
}

/// Returns the input condition string
QString TransitionPropertiesDlgImpl::getInputs()
{
  QString inputs = ui.le_input->text().simplified();
  inputs.replace(" | ", "|");
  inputs.replace(" |", "|");
  inputs.replace("| ", "|");
  return inputs;
}

/// Sets the input condition string
void TransitionPropertiesDlgImpl::setInputs(QString s)
{
  ui.le_input->setText(s.replace("|", " | "));
}
