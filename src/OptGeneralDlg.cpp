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

#include <qdir.h>
#include <qsettings.h>
#include <qstringlist.h>
#include <stdlib.h>
#include <QDebug>

#include "OptGeneralDlg.h"

/**
 *  Constructs a OptGeneralDlgImpl which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
OptGeneralDlgImpl::OptGeneralDlgImpl(QWidget *parent)
    : QWidget(parent) {
  ui.setupUi(this);
}

/**
 *  Destroys the object and frees any allocated resources
 */
OptGeneralDlgImpl::~OptGeneralDlgImpl() {}

/// Returns the language set in the combo box
QString OptGeneralDlgImpl::getLanguage() {
  QString tmp;
  if (ui.cmb_language->count() == 0)
    return "English";

  tmp = ui.cmb_language->currentText();
  return tmp;
}

/**
 * Sets the language @a s in the combo box.
 */
void OptGeneralDlgImpl::setLanguage(QString s) {
  QString tmp;

  for (int i = 0; i < ui.cmb_language->count(); i++) {
    tmp = ui.cmb_language->itemText(i);
    if (tmp == s) {
      ui.cmb_language->setCurrentIndex(i);
      break;
    }
  }
}

/**
 * Initialises the dialog with the current language
 */
void OptGeneralDlgImpl::init() {
  QDir dir;
  QStringList flist, list;
  QString fn;
  int len;

  QString apppath = getenv("QFSMDIR");
  if (apppath.isEmpty()) {
    qDebug() << "$QFSMDIR not set!";
#ifdef Q_OS_WIN
    QString tmppath;
    QSettings settings("HKEY_LOCAL_MACHINE\\Software\\Qfsm",
                       QSettings::NativeFormat);
    tmppath = settings.value("Install_Dir", QVariant("-1")).toString();
    qDebug() << "tmppath:" << tmppath;
    if (tmppath == "-1")
      dir = QDir::current();
    else
      dir.cd(tmppath);
    //	dir.cdUp();
    dir.cd(QFSM_LANGUAGE_DIR);
    apppath = dir.absolutePath();
#else
    apppath = QFSM_LANGUAGE_DIR;
    dir = QDir(apppath); // QDir::current();
#endif
    qDebug() << "Looking for language files in" << apppath;
  } else {
    dir = QDir(apppath); // QDir::current();
    if (!dir.cd("po"))
      dir.cd(QFSM_LANGUAGE_DIR);
  }
  dir.setNameFilters({"*.qm"});
  flist = dir.entryList();

  ui.cmb_language->clear();

  QStringList::Iterator it;

  for (it = flist.begin(); it != flist.end(); ++it) {
    fn = *it;
    len = fn.length();
    fn = fn.left(len - 3);
    list.append(fn);
    qDebug() << fn;
  }

  if (list.count() > 0) ui.cmb_language->addItems(list);
}
