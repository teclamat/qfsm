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
#include "SimulatorDlg.h"

// Added by qt3to4:
#include "../pics/greenlight.xpm"
#include "../pics/greylight.xpm"
#include "../pics/redlight.xpm"
#include "Const.h"
#include "Convert.h"
#include "Machine.h"
#include "MainWindow.h"
#include "Project.h"
#include "TransitionInfo.h"

#include <QTimer>
#include <QCloseEvent>
#include <QPixmap>

/**
 *  Constructs a SimulatorDlgImpl which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
SimulatorDlgImpl::SimulatorDlgImpl(QWidget *parent, const char *name,
                                   bool modal, Qt::WindowFlags fl)
    : QDialog(parent, fl) {
  ui.setupUi(this);

  main = (MainWindow *)parent;

  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &SimulatorDlgImpl::inputSent);

  redlight = new QPixmap((const char **)redlight_xpm);
  greenlight = new QPixmap((const char **)greenlight_xpm);
  greylight = new QPixmap((const char **)greylight_xpm);

  connect(ui.pb_send, &QPushButton::clicked, this, &SimulatorDlgImpl::send);
  connect(ui.pb_reset, &QPushButton::clicked, this, &SimulatorDlgImpl::reset);
  connect(ui.pb_clock, &QPushButton::clicked, this, &SimulatorDlgImpl::clockPressed);
  connect(ui.pb_cancel, &QPushButton::clicked, this, &SimulatorDlgImpl::closeSim);
  connect(ui.pb_bin0, &QPushButton::released, this, &SimulatorDlgImpl::button0Pressed);
  connect(ui.pb_bin1, &QPushButton::released, this, &SimulatorDlgImpl::button1Pressed);
  connect(ui.pb_bin2, &QPushButton::released, this, &SimulatorDlgImpl::button2Pressed);
  connect(ui.pb_bin3, &QPushButton::released, this, &SimulatorDlgImpl::button3Pressed);
  connect(ui.pb_bin4, &QPushButton::released, this, &SimulatorDlgImpl::button4Pressed);
  connect(ui.pb_bin5, &QPushButton::released, this, &SimulatorDlgImpl::button5Pressed);
  connect(ui.pb_bin6, &QPushButton::released, this, &SimulatorDlgImpl::button6Pressed);
  connect(ui.pb_bin7, &QPushButton::released, this, &SimulatorDlgImpl::button7Pressed);
  connect(ui.pb_bin8, &QPushButton::released, this, &SimulatorDlgImpl::button8Pressed);
  connect(ui.pb_bin9, &QPushButton::released, this, &SimulatorDlgImpl::button9Pressed);
  connect(ui.pb_bin10, &QPushButton::released, this, &SimulatorDlgImpl::button10Pressed);
  connect(ui.pb_bin11, &QPushButton::released, this, &SimulatorDlgImpl::button11Pressed);
  connect(ui.pb_bin12, &QPushButton::released, this, &SimulatorDlgImpl::button12Pressed);
  connect(ui.pb_bin13, &QPushButton::released, this, &SimulatorDlgImpl::button13Pressed);
  connect(ui.pb_bin14, &QPushButton::released, this, &SimulatorDlgImpl::button14Pressed);
  connect(ui.pb_bin15, &QPushButton::released, this, &SimulatorDlgImpl::button15Pressed);
  connect(ui.rb_ohex, &QRadioButton::clicked, this, &SimulatorDlgImpl::oHexPressed);
  connect(ui.rb_obin, &QRadioButton::clicked, this, &SimulatorDlgImpl::oBinPressed);
  connect(ui.rb_oascii, &QRadioButton::clicked, this, &SimulatorDlgImpl::oASCIIPressed);
  connect(ui.rb_ihex, &QRadioButton::clicked, this, &SimulatorDlgImpl::iHexPressed);
  connect(ui.rb_ibin, &QRadioButton::clicked, this, &SimulatorDlgImpl::iBinPressed);
  connect(ui.rb_iascii, &QRadioButton::clicked, this, &SimulatorDlgImpl::iASCIIPressed);
  connect(ui.le_inputs, &QLineEdit::textChanged, this, &SimulatorDlgImpl::inputsChanged);
  connect(ui.le_inputs, &QLineEdit::returnPressed, this, &SimulatorDlgImpl::returnPressedInputs);
}

/**
 *  Destroys the object and frees any allocated resources
 */
SimulatorDlgImpl::~SimulatorDlgImpl() {
  delete redlight;
  delete greenlight;
  delete greylight;
}

/// Initialises the dialog
void SimulatorDlgImpl::init() {
  resetButtons();

  if (isIBinChecked())
    ui.le_inputs->setMaxLength(
        main->project()->machine()->getNumInputs());
  else if (isIHexChecked())
    ui.le_inputs->setMaxLength(
        int((double)main->project()->machine()->getNumInputs() / 4 + 0.9));
  else
    ui.le_inputs->setMaxLength(
        int((double)main->project()->machine()->getNumInputs() / 8 + 0.9) * 4);

  setInputs("");
  ui.le_inputs->setFocus();
  setState(Off);
}

/**
 * Close the simulator.
 * Emits the closing signal and hides the dialog. Is invoked when the users
 * clicks the Close button.
 */
void SimulatorDlgImpl::closeSim() {
  emit closing();
  hide();
}

/**
 * Is invoked when the user closes the window.
 * Emits the closing signal and hides the dialog.
 */
void SimulatorDlgImpl::closeEvent(QCloseEvent *e) {
  emit closing();
  e->accept();
}

/**
 * User has clicked the Send button.
 */
void SimulatorDlgImpl::send() { emit inputSent(); }

/**
 * User has clicked the Reset button.
 */
void SimulatorDlgImpl::reset() { emit resetPressed(); }

/// Stops the timer for the clock.
void SimulatorDlgImpl::stopTimer() { timer->stop(); }

/// Selects the contents of the 'inputs' field and sets the focus
void SimulatorDlgImpl::selectFirst() {
  ui.le_inputs->selectAll();
  ui.le_inputs->setFocus();
}

void SimulatorDlgImpl::clearInput() {
  ui.le_inputs->setText("");
  ui.le_inputs->setFocus();
}

/**
 * Is invoked when the text in the inputs field changes.
 * The buttons for the bits 0-15 are changed according to the string @a in.
 */
void SimulatorDlgImpl::inputsChanged(const QString &in) {
  Convert conv;
  QString bin;
  //  char* bininv;
  IOInfoBin io(IO_MealyIn), ioinv(IO_MealyIn);

  int len = main->project()->machine()->getNumInputs();
  //  bin = new char[len+1];
  //  bininv = new char[len+1];
  if (isIBinChecked())
    ioinv = conv.binStrToX10(len, in, IO_MealyIn);
  else if (isIHexChecked())
    ioinv = conv.hexStrToX10(len, in, IO_MealyIn);
  else {
    unsigned char ascii[MAX_CHARARRAY_LENGTH];
    int alen;

    conv.resolveEscapes(in, ascii, MAX_CHARARRAY_LENGTH, alen);
    ioinv = conv.asciiToX10(len, ascii, alen, IO_MealyIn);
  }

  //  conv.invertX10(len, bininv, bin);
  io = ioinv;
  io.invertBits();
  bin = io.getInfo();

  if (ui.pb_bin0->isEnabled())
    ui.pb_bin0->setChecked(bin[0] == '1');
  if (ui.pb_bin1->isEnabled())
    ui.pb_bin1->setChecked(bin[1] == '1');
  if (ui.pb_bin2->isEnabled())
    ui.pb_bin2->setChecked(bin[2] == '1');
  if (ui.pb_bin3->isEnabled())
    ui.pb_bin3->setChecked(bin[3] == '1');
  if (ui.pb_bin4->isEnabled())
    ui.pb_bin4->setChecked(bin[4] == '1');
  if (ui.pb_bin5->isEnabled())
    ui.pb_bin5->setChecked(bin[5] == '1');
  if (ui.pb_bin6->isEnabled())
    ui.pb_bin6->setChecked(bin[6] == '1');
  if (ui.pb_bin7->isEnabled())
    ui.pb_bin7->setChecked(bin[7] == '1');
  if (ui.pb_bin8->isEnabled())
    ui.pb_bin8->setChecked(bin[8] == '1');
  if (ui.pb_bin9->isEnabled())
    ui.pb_bin9->setChecked(bin[9] == '1');
  if (ui.pb_bin10->isEnabled())
    ui.pb_bin10->setChecked(bin[10] == '1');
  if (ui.pb_bin11->isEnabled())
    ui.pb_bin11->setChecked(bin[11] == '1');
  if (ui.pb_bin12->isEnabled())
    ui.pb_bin12->setChecked(bin[12] == '1');
  if (ui.pb_bin13->isEnabled())
    ui.pb_bin13->setChecked(bin[13] == '1');
  if (ui.pb_bin14->isEnabled())
    ui.pb_bin14->setChecked(bin[14] == '1');
  if (ui.pb_bin15->isEnabled())
    ui.pb_bin15->setChecked(bin[15] == '1');

  //  delete [] bin;
  //  delete [] bininv;
}

/// Called when return is pressed in the 'inputs' field
void SimulatorDlgImpl::returnPressedInputs() {
  /*
  if (pb_send->isEnabled())
    send();
    */
}

/// Button 0 was pressed
void SimulatorDlgImpl::button0Pressed() { updateInputsStr(); }

/// Button 1 was pressed
void SimulatorDlgImpl::button1Pressed() { updateInputsStr(); }

/// Button 2 was pressed
void SimulatorDlgImpl::button2Pressed() { updateInputsStr(); }

/// Button 3 was pressed
void SimulatorDlgImpl::button3Pressed() { updateInputsStr(); }

/// Button 4 was pressed
void SimulatorDlgImpl::button4Pressed() { updateInputsStr(); }

/// Button 5 was pressed
void SimulatorDlgImpl::button5Pressed() { updateInputsStr(); }

/// Button 6 was pressed
void SimulatorDlgImpl::button6Pressed() { updateInputsStr(); }

/// Button 7 was pressed
void SimulatorDlgImpl::button7Pressed() { updateInputsStr(); }

/// Button 8 was pressed
void SimulatorDlgImpl::button8Pressed() { updateInputsStr(); }

/// Button 9 was pressed
void SimulatorDlgImpl::button9Pressed() { updateInputsStr(); }

/// Button 10 was pressed
void SimulatorDlgImpl::button10Pressed() { updateInputsStr(); }

/// Button 11 was pressed
void SimulatorDlgImpl::button11Pressed() { updateInputsStr(); }

/// Button 12 was pressed
void SimulatorDlgImpl::button12Pressed() { updateInputsStr(); }

/// Button 13 was pressed
void SimulatorDlgImpl::button13Pressed() { updateInputsStr(); }

/// Button 14 was pressed
void SimulatorDlgImpl::button14Pressed() { updateInputsStr(); }

/// Button 15 was pressed
void SimulatorDlgImpl::button15Pressed() { updateInputsStr(); }

/**
 * Updates the string in the inputs field according to the state of the buttons.
 */
void SimulatorDlgImpl::updateInputsStr() {
  int len;
  QString s;
  Convert conv;
  QString bin;
  IOInfoBin io(IO_MealyIn), ioinv(IO_MealyIn);

  len = main->project()->machine()->getNumInputs();
  s = ui.le_inputs->text();

  if (isIBinChecked())
    ioinv = conv.binStrToX10(len, s, IO_MealyIn);
  else if (isIHexChecked())
    ioinv = conv.hexStrToX10(len, s, IO_MealyIn);
  else {
    unsigned char ascii[MAX_CHARARRAY_LENGTH];
    int alen;

    conv.resolveEscapes(s, ascii, MAX_CHARARRAY_LENGTH, alen);
    ioinv = conv.asciiToX10(len, ascii, alen, IO_MealyIn);
  }

  io = ioinv;
  io.invertBits();
  bin = io.getInfo();
  bin.resize(len);

  if (len > 0)
    bin[0] = QChar{ ui.pb_bin0->isChecked() + '0' };
  if (len > 1)
    bin[1] = QChar{ ui.pb_bin1->isChecked() + '0' };
  if (len > 2)
    bin[2] = QChar{ ui.pb_bin2->isChecked() + '0' };
  if (len > 3)
    bin[3] = QChar{ ui.pb_bin3->isChecked() + '0' };
  if (len > 4)
    bin[4] = QChar{ ui.pb_bin4->isChecked() + '0' };
  if (len > 5)
    bin[5] = QChar{ ui.pb_bin5->isChecked() + '0' };
  if (len > 6)
    bin[6] = QChar{ ui.pb_bin6->isChecked() + '0' };
  if (len > 7)
    bin[7] = QChar{ ui.pb_bin7->isChecked() + '0' };
  if (len > 8)
    bin[8] = QChar{ ui.pb_bin8->isChecked() + '0' };
  if (len > 9)
    bin[9] = QChar{ ui.pb_bin9->isChecked() + '0' };
  if (len > 10)
    bin[10] = QChar{ ui.pb_bin10->isChecked() + '0' };
  if (len > 11)
    bin[11] = QChar{ ui.pb_bin11->isChecked() + '0' };
  if (len > 12)
    bin[12] = QChar{ ui.pb_bin12->isChecked() + '0' };
  if (len > 13)
    bin[13] = QChar{ ui.pb_bin13->isChecked() + '0' };
  if (len > 14)
    bin[14] = QChar{ ui.pb_bin14->isChecked() + '0' };
  if (len > 15)
    bin[15] = QChar{ ui.pb_bin15->isChecked() + '0' };

  io.setInfo(bin);

  //  io.setInfo(bin);
  ioinv = io;
  ioinv.invertBits();

  if (isIBinChecked())
    s = conv.X10ToBinStr(ioinv);
  else if (isIHexChecked())
    s = conv.X10ToHexStr(ioinv);
  else {
    unsigned char ascii[MAX_CHARARRAY_LENGTH];
    int alen;

    if (!conv.X10ToASCII(ioinv, ascii, MAX_CHARARRAY_LENGTH, alen))
      s = "";
    else
      s = conv.asciiToReadableStr(ascii, alen);
  }

  ui.le_inputs->setText(s);
}

/// Called when the 'Binary' button for the inputs is pressed.
void SimulatorDlgImpl::iBinPressed() {
  QString tmp;
  Convert conv;

  if (isIHexChecked()) {
    tmp = conv.hexStrToBinStr(main->project()->machine()->getNumInputs(),
                              ui.le_inputs->text());
    //    le_inputs->setText(tmp);
    //    rb_ibin->setChecked(true);
    //    inputsChanged(tmp);
  } else if (isIASCIIChecked()) {
    unsigned char ascii[MAX_CHARARRAY_LENGTH];
    int alen;

    conv.resolveEscapes(ui.le_inputs->text(), ascii,
                        MAX_CHARARRAY_LENGTH, alen);
    tmp =
        conv.asciiToBinStr(main->project()->machine()->getNumInputs(), ascii, alen);
    //    le_inputs->setText(tmp);
    //    rb_ibin->setChecked(true);
    //    inputsChanged(tmp);
  }
  ui.le_inputs->setMaxLength(main->project()->machine()->getNumInputs());
  ui.le_inputs->setText(tmp);
  ui.rb_ibin->setChecked(true);
  inputsChanged(tmp);
}

/// Called when the 'Hexadecimal' button for the inputs is pressed.
void SimulatorDlgImpl::iHexPressed() {
  QString tmp;
  Convert conv;
  int maxlen;

  if (isIBinChecked()) {
    tmp = conv.binStrToHexStr(ui.le_inputs->text());
    //    le_inputs->setText(tmp);
    //    rb_ihex->setChecked(true);
    //    inputsChanged(tmp);
  } else if (isIASCIIChecked()) {
    unsigned char ascii[MAX_CHARARRAY_LENGTH];
    int alen;

    conv.resolveEscapes(ui.le_inputs->text(), ascii,
                        MAX_CHARARRAY_LENGTH, alen);
    tmp = conv.asciiToHexStr(ascii, alen);
    //    le_inputs->setText(tmp);
    //    rb_ihex->setChecked(true);
    //    inputsChanged(tmp);
  }
  maxlen = int((double)main->project()->machine()->getNumInputs() / 4 + 0.9);
  tmp = tmp.right(maxlen);
  ui.le_inputs->setMaxLength(maxlen);
  ui.le_inputs->setText(tmp);
  ui.rb_ihex->setChecked(true);
  inputsChanged(tmp);
}

/// Called when the 'ASCII' button for the inputs is pressed
void SimulatorDlgImpl::iASCIIPressed() {
  QString tmp;
  Convert conv;

  if (isIBinChecked()) {
    unsigned char ascii[MAX_CHARARRAY_LENGTH];
    int alen;

    conv.binStrToASCII(ui.le_inputs->text(), ascii,
                       MAX_CHARARRAY_LENGTH, alen);
    tmp = conv.asciiToReadableStr(ascii, alen);
    //    le_inputs->setText(tmp);
    //    rb_iascii->setChecked(true);
    //    inputsChanged(tmp);
  } else if (isIHexChecked()) {
    unsigned char ascii[MAX_CHARARRAY_LENGTH];
    unsigned char single;
    int alen;

    conv.hexStrToASCII(ui.le_inputs->text(), ascii,
                       MAX_CHARARRAY_LENGTH, alen);
    if (alen > 0) {
      single = ascii[alen - 1];
      tmp = conv.asciiToReadableStr(&single, 1);
    }
    //    le_inputs->setText(tmp);
    //    rb_iascii->setChecked(true);
    //    inputsChanged(tmp);
  }
  ui.le_inputs->setMaxLength(
      int((double)main->project()->machine()->getNumInputs() / 8 + 0.9) * 4);
  ui.le_inputs->setText(tmp);
  ui.rb_iascii->setChecked(true);
  inputsChanged(tmp);
}

/// Called when the 'Binary' button for the outputs is pressed.
void SimulatorDlgImpl::oBinPressed() {
  QString tmp;
  Convert conv;

  if (isOHexChecked()) {
    tmp = conv.hexStrToBinStr(main->project()->machine()->getNumOutputs(),
                              ui.lb_outputs->text());
    ui.lb_outputs->setText(tmp);
  } else if (isOASCIIChecked()) {
    unsigned char ascii[MAX_CHARARRAY_LENGTH];
    int alen;
    conv.resolveEscapes(ui.lb_outputs->text(), ascii,
                        MAX_CHARARRAY_LENGTH, alen);
    tmp = conv.asciiToBinStr(main->project()->machine()->getNumOutputs(), ascii,
                             alen);
    ui.lb_outputs->setText(tmp);
  }
}

/// Called when the 'Hexadecimal' button for the outputs is pressed.
void SimulatorDlgImpl::oHexPressed() {
  QString tmp;
  Convert conv;

  if (isOBinChecked()) {
    tmp = conv.binStrToHexStr(ui.lb_outputs->text());
    ui.lb_outputs->setText(tmp);
  } else if (isOASCIIChecked()) {
    unsigned char ascii[MAX_CHARARRAY_LENGTH];
    int alen;
    conv.resolveEscapes(ui.lb_outputs->text(), ascii,
                        MAX_CHARARRAY_LENGTH, alen);
    tmp = conv.asciiToHexStr(ascii, alen);
    ui.lb_outputs->setText(tmp);
  }
}

/// Called when the 'ASCII' button for the outputs is pressed.
void SimulatorDlgImpl::oASCIIPressed() {
  QString tmp;
  Convert conv;

  if (isOBinChecked()) {
    unsigned char ascii[MAX_CHARARRAY_LENGTH];
    int alen;

    conv.binStrToASCII(ui.lb_outputs->text(), ascii,
                       MAX_CHARARRAY_LENGTH, alen, false);
    tmp = conv.asciiToReadableStr(ascii, alen);
    ui.lb_outputs->setText(tmp);
  } else if (isOHexChecked()) {
    unsigned char ascii[MAX_CHARARRAY_LENGTH];
    int alen;

    conv.hexStrToASCII(ui.lb_outputs->text(), ascii,
                       MAX_CHARARRAY_LENGTH, alen, false);
    tmp = conv.asciiToReadableStr(ascii, alen);
    ui.lb_outputs->setText(tmp);
  }
}

/// Sets all button states to Off
void SimulatorDlgImpl::resetButtons() {
  if (main->project()->machine()) {
    if (main->project()->machine()->getType() == Binary) {
      ui.rb_ibin->setChecked(true);
      ui.rb_obin->setChecked(true);
    } else {
      ui.rb_iascii->setChecked(true);
      ui.rb_oascii->setChecked(true);
    }
  }

  resetBits();
  ui.pb_clock->setChecked(false);
}

void SimulatorDlgImpl::resetBits() {
  ui.pb_bin0->setChecked(false);
  ui.pb_bin1->setChecked(false);
  ui.pb_bin2->setChecked(false);
  ui.pb_bin3->setChecked(false);
  ui.pb_bin4->setChecked(false);
  ui.pb_bin5->setChecked(false);
  ui.pb_bin6->setChecked(false);
  ui.pb_bin7->setChecked(false);
  ui.pb_bin8->setChecked(false);
  ui.pb_bin9->setChecked(false);
  ui.pb_bin10->setChecked(false);
  ui.pb_bin11->setChecked(false);
  ui.pb_bin12->setChecked(false);
  ui.pb_bin13->setChecked(false);
  ui.pb_bin14->setChecked(false);
  ui.pb_bin15->setChecked(false);
}

/// Called when the 'clock' button is pressed
void SimulatorDlgImpl::clockPressed() {
  double hz;
  double msec;
  hz = ui.frequency->value();
  //  hz /= 10;
  msec = 1000 / hz;

  if (!ui.pb_clock->isChecked()) {
    timer->start((int)msec);
    ui.lb_frequency->setEnabled(false);
    ui.frequency->setEnabled(false);
    ui.pb_send->setEnabled(false);
    resetBits();
  } else // if (timer->isActive())
  {
    timer->stop();
    ui.lb_frequency->setEnabled(true);
    ui.frequency->setEnabled(true);
    ui.pb_send->setEnabled(true);
  }
}

/**
 * Sets the state of the machine to @a s
 * @param s State of the machine
 *    - Busy: Machine hasn't reached an end state (red light)
 *    - Finished: Machine has reached an end state (green light)
 *    - Otherwise undefined (grey light)
 */
void SimulatorDlgImpl::setState(int s) {
  switch (s) {
  case Busy:
    ui.pm_end->setPixmap(*redlight);
    break;
  case Finished:
    ui.pm_end->setPixmap(*greenlight);
    break;
  default:
    ui.pm_end->setPixmap(*greylight);
    break;
  }
}
