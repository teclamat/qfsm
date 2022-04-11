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

#ifndef STATEPROPERTIESDLGIMPL_H
#define STATEPROPERTIESDLGIMPL_H

#include "ui_StatePropertiesDlg.h"
// #include <q3textedit.h>
#include <qcolor.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qvalidator.h>

#include "GState.h"

class MainWindow;

/**
 * @class StatePropertiesDlgImpl
 * @brief Dialog to change the properties of a state
 */
class StatePropertiesDlgImpl : public QDialog {
  Q_OBJECT

  Ui::StatePropertiesDlg ui;

public:
  StatePropertiesDlgImpl(QWidget *parent = 0, const char *name = 0,
                         bool modal = false, Qt::WindowFlags fl = {});
  ~StatePropertiesDlgImpl();

  /// Dialog mode
  enum DialogMode { StateAdd, StateEdit };

  /// Returns the string of the name field
  QString getName() { return ui.le_name->text(); };
  /// Sets the string of the name field
  void setName(const QString &s) { ui.le_name->setText(s); };
  /// Returns the string of the description field
  QString getDescription() { return ui.te_description->toPlainText(); };
  /// Sets the string of the description field
  void setDescription(QString s) {
    ui.te_description->setPlainText(s);
  };
  /// Returns the code
  QString getCode() { return ui.le_code->text(); };
  /// Sets the code
  void setCode(const QString &s) { ui.le_code->setText(s); };
  /// Returns the moore outputs
  QString getMooreOutputs() {
    return ui.le_mooreoutputs->text();
  };
  /// Sets the moore outputs
  void setMooreOutputs(const QString &s) {
    ui.le_mooreoutputs->setText(s);
  };
  /// Gets the entry actions
  QString getEntryActions() { return ui.le_entry->text(); };
  /// Sets the entry actions field
  void setEntryActions(const QString &s) {
    ui.le_entry->setText(s);
  };
  /// Gets the exit actions
  QString getExitActions() { return ui.le_exit->text(); };
  /// Sets the exit actions field
  void setExitActions(const QString &s) {
    ui.le_exit->setText(s);
  };
  /// Returns the radius
  QString getRadius() { return ui.le_radius->text(); };
  /// Sets the radius
  void setRadius(const QString &r) {
    ui.le_radius->setText(r);
  };
  /// Returns the line width
  QString getLineWidth() { return ui.le_linewidth->text(); };
  /// Sets the line width
  void setLineWidth(const QString &l) {
    ui.le_linewidth->setText(l);
  };
  /// Sets the pointer to the main window
  void setMainWindow(MainWindow *m) { main = m; };
  /// Sets the current mode (StateAdd, StateEdit)
  void setMode(int m) { mode = m; };
  /// Sets the state to edit
  void setState(GState *s) { state = s; };
  /// Sets the color of the the state
  void setColor(const QColor& a_color);
  /// Returns the color
  QColor getColor() { return m_outlineColor; };

  /// Sets the maximum length of the code
  void setCodeMaxLength(int l) { ui.le_code->setMaxLength(l); };
  /// Sets the maximum length of the moore outputs
  void setMooreOutputsMaxLength(int l) {
    ui.le_mooreoutputs->setMaxLength(l);
  };
  /// Sets the maximum length of the entry actions
  void setEntryMaxLength(int l) {
    ui.le_entry->setMaxLength(l);
  };
  /// Sets the maximum length of the exit actions
  void setExitMaxLength(int l) { ui.le_exit->setMaxLength(l); };

  /// Selects the first field of the dialog and sets the focus
  void selectFirst() {
    ui.le_name->selectAll();
    ui.le_name->setFocus();
  };

public slots:
  void chooseFGColor();
  void validate();

private:
  /// Mode of the dialog (see DialogMode)
  int mode;
  /// Pointer of the state to add/edit
  GState *state;
  /// Validator for the radius
  QIntValidator *valRadius;
  /// Validator for the line width
  QIntValidator *valLineWidth;
  /// Pointer to the main window
  MainWindow *main;
  /// Color
  QColor m_outlineColor;

};

#endif // STATEPROPERTIESDLGIMPL_H
