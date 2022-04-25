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
#include "StatePropertiesDlg.h"

#include "gui/error.hpp"

#include <qcolor.h>
#include <qcolordialog.h>

#include "Convert.h"
#include "Machine.h"
#include "MainWindow.h"
#include "Project.h"
#include "StateOld.h"
#include "TransitionInfo.h"
#include "Utils.h"

/**
 *  Constructs a StatePropertiesDlgImpl which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
StatePropertiesDlgImpl::StatePropertiesDlgImpl(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : QDialog(parent, fl)
{
  ui.setupUi(this);

  valRadius = new QIntValidator(5, 500, this);
  valLineWidth = new QIntValidator(1, 10, this);

  ui.le_radius->setValidator(valRadius);
  ui.le_linewidth->setValidator(valLineWidth);
  // ui.lb_colorpreview->setAutoFillBackground(true);

  connect(ui.pb_ok, &QPushButton::clicked, this, &StatePropertiesDlgImpl::validate);
  connect(ui.pb_cancel, &QPushButton::clicked, this, &StatePropertiesDlgImpl::reject);
  connect(ui.pb_color, &QPushButton::clicked, this, &StatePropertiesDlgImpl::chooseFGColor);
}

/**
 *  Destroys the object and frees any allocated resources
 */
StatePropertiesDlgImpl::~StatePropertiesDlgImpl()
{
  delete valRadius;
  delete valLineWidth;
}

/**
 * Validates the state properties and displays an error box when invalid.
 * Displays and error box if no name is given, if the code is not in binary
 * format, if the values are out of range or if the code already exists
 */
void StatePropertiesDlgImpl::validate()
{
  QString r, l, c;
  int icode;
  Machine* m = main->project()->machine();
  Convert conv;
  int mtype = m->getType();

  r = ui.le_radius->text();
  l = ui.le_linewidth->text();
  c = ui.le_code->text();

  /* MEANINGLESS statements (icode set but unused) */
  if (mtype == Binary)
    icode = conv.binStrToInt(c);
  else
    icode = c.toInt();

  int pos = 0;

  if (ui.le_name->text().isEmpty())
    qfsm::gui::msg::info(tr("You must specify a name."));
  else if (!State::codeValid(m->getType(), c))
    qfsm::gui::msg::info(tr("Code is not in binary format."));
  else if (!State::mooreOutputValid(m->getType(), ui.le_mooreoutputs->text()))
    qfsm::gui::msg::info(tr("Moore outputs are not in the correct format."));
  // else if (!Utils::binStringValid(le_mooreoutputs->text()))
  // err.info(tr("Moore outputs are not in binary format."));
  else if (valRadius->validate(r, pos) != QValidator::Acceptable)
    qfsm::gui::msg::info(tr("Radius out of range."));
  else if (valLineWidth->validate(l, pos) != QValidator::Acceptable)
    qfsm::gui::msg::info(tr("Linewidth out of range."));
  else
    accept();
}

void StatePropertiesDlgImpl::setColor(const QColor& a_color)
{
  if (!a_color.isValid() || (m_outlineColor == a_color)) {
    return;
  }
  m_outlineColor = a_color;

  const QString colorText{ QVariant{ m_outlineColor }.toString() };
  QString styleSheetText{};
  if (m_outlineColor != Qt::black) {
    styleSheetText = QString{ "background-color:%1" }.arg(colorText);
  }

  ui.lb_colorpreview->setStyleSheet(styleSheetText);
  ui.lb_colorpreview->setText(colorText);
};

/**
 * Lets you choose a foreground color for the state
 */
void StatePropertiesDlgImpl::chooseFGColor()
{
  setColor(QColorDialog::getColor(m_outlineColor, this));
}
