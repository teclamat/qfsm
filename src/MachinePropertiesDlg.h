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
/*
Qt 4 Port by Rainer Strobel

in method getType:
changed the code which checks the current type so that it works
under windows
*/

#ifndef MACHINEPROPERTIESDLHIMPL_H
#define MACHINEPROPERTIESDLHIMPL_H

#include "ui_MachinePropertiesDlg.h"

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>

// class LBArrowLine;
// class LBArrowFilled;
// class LBArrowWhite;
// class LBArrowFilledPointed;
// class LBArrowWhitePointed;

/**
 * @class MachinePropertiesDlgImpl
 * @brief Dialog to set the properties of the current machine.
 */
class MachinePropertiesDlgImpl : public QDialog {
  Q_OBJECT
  Ui::MachinePropertiesDlg ui;

public:
  MachinePropertiesDlgImpl(QWidget *parent = 0, const char *name = 0,
                           bool modal = false, Qt::WindowFlags fl = {});
  ~MachinePropertiesDlgImpl();

  /// Returns the name of the machine.
  QString getName() { return ui.le_name->text(); };
  /// Sets the name of the machine.
  void setName(QString n) { ui.le_name->setText(n); };
  /// Returns the version of the machine.
  QString getVersion() { return ui.le_version->text(); };
  /// Sets the version of the machine.
  void setVersion(QString v) { ui.le_version->setText(v); };
  int getType();
  void setType(int t);
  void enableType(bool et);

  /// Returns the number of bits the states are coded with, i.e. the number of
  /// moore outputs
  int getNumMooreOutputs() {
    return ui.sb_mooreout->value();
  };
  /// Sets the number of bits the states are coded with, i.e. the number of
  /// moore outputs.
  void setNumMooreOutputs(int num) {
    ui.sb_mooreout->setValue(num);
    saved_numbits = num;
  };
  /// Returns the names of the moore output bits.
  QString getMooreOutputNames() {
    return ui.le_mooreout->text();
  };
  /// Sets the names of the moore output bits.
  void setMooreOutputNames(QString s) {
    ui.le_mooreout->setText(s);
  };

  /// Returns the number of mealy input bits
  int getNumInputs() { return ui.sb_mealyin->value(); };
  /// Sets the number of mealy input bits
  void setNumInputs(int num) {
    ui.sb_mealyin->setValue(num);
    saved_numin = num;
  };
  /// Returns the names of mealy input bits.
  QString getMealyInputNames() {
    return ui.le_mealyin->text();
  };
  /// Sets the names of mealy input bits.
  void setMealyInputNames(QString s) {
    ui.le_mealyin->setText(s);
  };

  /// Returns the number of mealy output bits
  int getNumOutputs() { return ui.sb_mealyout->value(); };
  /// Sets the number of mealy output bits
  void setNumOutputs(int num) {
    ui.sb_mealyout->setValue(num);
    saved_numout = num;
  };
  /// Returns the names of mealy output bits.
  QString getMealyOutputNames() {
    return ui.le_mealyout->text();
  };
  /// Sets the names of mealy output bits.
  void setMealyOutputNames(QString s) {
    ui.le_mealyout->setText(s);
  };

  /// Returns the font used for the state names
  QFont getSFont() { return sfont; };
  /// Sets the font used for the state names
  void setSFont(QFont f) {
    sfont = f;
    ui.lb_statefont->setText(f.family());
    ui.lb_statefont->setFont(f);
  };
  /// Returns the font used for the transition names
  QFont getTFont() { return tfont; };
  /// Sets the font used for the transition names
  void setTFont(QFont f) {
    tfont = f;
    ui.lb_transfont->setText(f.family());
    ui.lb_transfont->setFont(f);
  };
  /// Returns the arrow type used for the transitions
  int getArrowType() {
    return ui.cb_arrowType->currentIndex();
    // return ui.lbox_arrowtype->currentItem();
  };
  /// Sets the arrow type used for the transitions
  void setArrowType(int at) {
    ui.cb_arrowType->setCurrentIndex(at);
    // ui.lbox_arrowtype->setCurrentItem(at);
  };
  /// Returns the string of the desription field
  QString getDescription() { return ui.te_description->toPlainText(); };
  /// Sets the string of the description field
  void setAuthor(QString s) { ui.le_author->setText(s); };
  /// Returns the string of the desription field
  QString getAuthor() { return ui.le_author->text(); };
  /// Sets the string of the description field
  void setDescription(QString s) {
    ui.te_description->setText(s);
  };

  bool getDrawITrans() {
    return ui.cb_inittrans->isChecked();
  };
  void setDrawITrans(bool di) {
    ui.cb_inittrans->setChecked(di);
  };

  /// Selects first field in dialog and sets the focus.
  void selectFirst() {
    ui.le_name->selectAll();
    ui.le_name->setFocus();
  };

private:
  /// Line arrow list box item
  // LBArrowLine *lb1;
  // /// Filled arrow list box item
  // LBArrowFilled *lb2;
  // /// White arrow list box item
  // LBArrowWhite *lb3;
  // /// Filled pointed arrow list box item
  // LBArrowFilledPointed *lb4;
  // /// White pointed arrow list box item
  // LBArrowWhitePointed *lb5;

  /// State font
  QFont sfont;
  /// Transition font
  QFont tfont;

  /// Previous number of moore output bits
  int saved_numbits;
  /// Previous number of input bits
  int saved_numin;
  /// Previous number of output bits
  int saved_numout;

public slots:
  void chooseSFont();
  void chooseTFont();
  void binaryClicked();
  void asciiClicked();
  void freeTextClicked();
  void drawITransClicked();
};

#endif // MACHINEPROPERTIESDLHIMPL_H
