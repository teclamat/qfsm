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

added code parts for Windows, which were only enabled, when
the code is compiled for a Windows system.
There, the additional include windows.h is needed and
the WinMain function is used instead of main.

Qt 5/6 port by Mateusz Tecław
*/

#include "MainControl.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QLocale>
#include <QSettings>
#include <QString>
#include <QTextStream>
#include <QTranslator>

QString loadLanguage(QTranslator* trans);

int main(int argc, char** argv)
{
  QApplication qfsmApp{ argc, argv };
  QApplication::setOrganizationName(QStringLiteral("qfsm"));

  // QTranslator trans(0);
  // QString lang = loadLanguage(&trans);
  // qfsm.installTranslator(&trans);

  qfsm::MainControl control{};

  if (argc <= 1) {
    control.newWindow();
  } else {
    for (int i = 1; i < argc; ++i) {
      control.newWindow(argv[i]);
    }
  }

  return qfsmApp.exec();
}

/*
void loadLanguage(QTranslator* trans)
{
  QDir dir;
  QStringList flist, list;
  QString fn;
//  int len;

  dir.cd("i18n");
  dir.setNameFilter("*.qm");
  flist = dir.entryList();

  QStringList::Iterator it;

  for (it=flist.begin(); it!=flist.end(); ++it)
  {
    fn = *it;
    if (!trans->load(fn, "i18n"))
      qDebug("Language "+fn+" not loaded.");
//    len = fn.length();
//    fn = fn.left(len-3);
//    list.append(fn);
  }

//  cmb_language->insertStringList(list);
}
*/

QString loadLanguage(QTranslator* trans)
{
  QString qfsmpath;
  QDir dir = QDir::home();
  QDir qfsmdir(dir.absolutePath() + "/.qfsm");
  if (!qfsmdir.exists()) {
    if (!dir.mkdir(".qfsm"))
      qDebug() << ".qfsm not created";
  }

  QFile file(qfsmdir.absolutePath() + "/language");
  QTextStream fin(&file);

  if (file.isOpen()) {
    qDebug() << "language file already open";
    file.close();
  }

  QString s;

  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "language file could not be opened.";

    QLocale loc = QLocale::system();
    switch (loc.language()) {
      case QLocale::German:
        s = "German";
        break;
      case QLocale::French:
        s = "French";
        break;
      default:
        s = "English";
        break;
    }
    qDebug() << "Most appropriate system locale:" << s;
  } else {
    fin >> s;
    qDebug() << "Locale by language file:" << s;
  }

  qfsmpath = getenv("QFSMDIR");
  //  QDir dir; //(qfsmpath);
  if (qfsmpath.isEmpty()) {
    qDebug("$QFSMDIR not set!");
#ifdef Q_OS_WIN
    QString tmppath;
    QSettings settings("HKEY_LOCAL_MACHINE\\Software\\Qfsm", QSettings::NativeFormat);
    tmppath = settings.value("Install_Dir", QVariant("-1")).toString();
    qDebug() << "tmppath:" << tmppath;
    if (tmppath == "-1")
      dir = QDir::current();
    else
      dir.cd(tmppath);
    //	dir.cdUp();

    dir.cd(QFSM_LANGUAGE_DIR);
    qfsmpath = dir.absolutePath();
#else
    qfsmpath = QFSM_LANGUAGE_DIR;
    dir = QDir(qfsmpath); // QDir::current();
#endif
    qDebug() << "Looking for language files in" << qfsmpath;
    //	QMessageBox::information(NULL, "debug info", qfsmpath);
  } else {
    dir = QDir(qfsmpath); // QDir::current();
    if (!dir.cd("po"))
      dir.cd(QFSM_LANGUAGE_DIR);
  }

  if (dir.exists())
    trans->load(s + ".qm", dir.absolutePath());

  file.close();

  return s;
}
