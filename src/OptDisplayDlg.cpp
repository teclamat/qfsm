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
#include "OptDisplayDlg.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>

#include "Options.h"

/**
 *  Constructs a OptDisplayDlgImpl which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
OptDisplayDlgImpl::OptDisplayDlgImpl(QWidget *parent, const char *name,
                                     bool modal, Qt::WindowFlags fl)
    : QDialog(parent, fl) {

  ui.setupUi(this);
  ui.lb_gridcolor->setAutoFillBackground(true);
  ui.lb_shadowcolor->setAutoFillBackground(true);

  connect(ui.pb_shadowcolor, &QPushButton::clicked, this, &OptDisplayDlgImpl::chooseShadowColor);
  connect(ui.pb_gridcolor, &QPushButton::clicked, this, &OptDisplayDlgImpl::chooseGridColor);
  connect(ui.cb_tooltips, &QCheckBox::clicked, this, &OptDisplayDlgImpl::tooltipsClicked);
  connect(ui.cb_shadows, &QCheckBox::clicked, this, &OptDisplayDlgImpl::shadowsClicked);
  connect(ui.cb_ionames, &QCheckBox::clicked, this, &OptDisplayDlgImpl::ioNamesClicked);
  connect(ui.cb_iomark, &QCheckBox::clicked, this, &OptDisplayDlgImpl::ioMarkClicked);
  connect(ui.cb_drawbox, &QCheckBox::clicked, this, &OptDisplayDlgImpl::drawBoxClicked);
  connect(ui.le_start, &QLineEdit::textChanged, [this](QString) { startDescChanged(); });
  connect(ui.le_inversion, &QLineEdit::textChanged, [this](QString) { inversionDescChanged(); });
  connect(ui.le_default, &QLineEdit::textChanged, [this](QString) { defaultDescChanged(); });
  connect(ui.le_any, &QLineEdit::textChanged, [this](QString) { anyInputDescChanged(); });
}

/**
 *  Destroys the object and frees any allocated resources
 */
OptDisplayDlgImpl::~OptDisplayDlgImpl() {
  // no need to delete child widgets, Qt does it all for us
}

/**
 * Initialises the dialog according to the current options @a opt.
 */
void OptDisplayDlgImpl::init(Options *opt) {
  int gsize;
  QString stext;
  gridcolor = opt->getGridColor();
  // ui.lb_gridcolor->setBackgroundColor(gridcolor);

  gsize = opt->getGridSize();
  if (gsize < 3)
    gsize = 3;

  ui.sb_gridsize->setValue(gsize);

  shadowcolor = opt->getStateShadowColor();
  // ui.lb_shadowcolor->setBackgroundColor(shadowcolor);

  bshadows = opt->getStateShadows();
  ui.cb_shadows->setChecked(bshadows);

  btooltips = opt->getToolTips();
  ui.cb_tooltips->setChecked(btooltips);

  iomark = opt->getIOMark();
  ui.cb_iomark->setChecked(iomark);

  ionames = opt->getDisplayIONames();
  ui.cb_ionames->setChecked(opt->getDisplayIONames());

  bdrawbox = opt->getDrawBox();
  ui.cb_drawbox->setChecked(opt->getDrawBox());

  ui.le_start->setText(opt->getInitialDescriptor());
  ui.le_inversion->setText(opt->getInversionDescriptor());
  ui.le_any->setText(opt->getAnyInputDescriptor());
  ui.le_default->setText(opt->getDefaultTransitionDescriptor());
}

/**
 * Lets you choose a color used to draw the grid
 */
void OptDisplayDlgImpl::chooseGridColor() {
  QColor c;
  c = QColorDialog::getColor(gridcolor, this);

  if (c.isValid()) {
    /*
    QPalette pal;
    pal.setColor(lb_gridcolor->backgroundRole(), c);
    lb_gridcolor->setPalette(pal);
    */
    // ui.lb_gridcolor->setBackgroundColor(c);
    gridcolor = c;
  }
}

/**
 * Lets you choose a color used to draw the shadows
 */
void OptDisplayDlgImpl::chooseShadowColor() {
  QColor c;
  c = QColorDialog::getColor(shadowcolor, this);

  if (c.isValid()) {
    // ui.lb_shadowcolor->setBackgroundColor(c);
    shadowcolor = c;
  }
}
/// Called when the 'display shadows' checkbox is clicked
void OptDisplayDlgImpl::shadowsClicked() {
  bshadows = ui.cb_shadows->isChecked();
}

/// Called when the tooltips checkbox is clicked
void OptDisplayDlgImpl::tooltipsClicked() {
  btooltips = ui.cb_tooltips->isChecked();
}

/// Called when the IO-mark checkbox is clicked
void OptDisplayDlgImpl::ioMarkClicked() {
  iomark = ui.cb_iomark->isChecked();
}

/// Called when the start transition descriptor is changed
void OptDisplayDlgImpl::startDescChanged() {
  initial_descriptor = ui.le_start->text();
}

/// Called when the inversion descriptor is changed
void OptDisplayDlgImpl::inversionDescChanged() {
  inversion_descriptor = ui.le_inversion->text();
}

/// Called when the "any input" descriptor is changed
void OptDisplayDlgImpl::anyInputDescChanged() {
  any_input_descriptor = ui.le_any->text();
}

/// Called when the default transition descriptor is changed
void OptDisplayDlgImpl::defaultDescChanged() {
  default_descriptor = ui.le_default->text();
}

/// Called when the 'Display I/O names' checkbox is clicked
void OptDisplayDlgImpl::ioNamesClicked() {
  ionames = ui.cb_ionames->isChecked();
}

/// Called when the 'Draw Box' checkbox is clicked
void OptDisplayDlgImpl::drawBoxClicked() {
  bdrawbox = ui.cb_drawbox->isChecked();
}
