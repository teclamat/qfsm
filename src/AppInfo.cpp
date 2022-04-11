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

#include "AppInfo.h"

#include <QIcon>
#include <QMessageBox>
#include <QObject>
#include <QTextStream>

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <QtGlobal>
namespace Qt {
static auto endl = ::endl;
}
#endif

namespace qfsm {

int AppInfo::s_versionMajor = QFSM_VERSION_MAJOR;
int AppInfo::s_versionMinor = QFSM_VERSION_MINOR;
QString AppInfo::s_author = QStringLiteral("Stefan Duffner, Rainer Strobel, Aaron Erhardt");
QString AppInfo::s_buildDate = QStringLiteral(QFSM_BUILD_DATE);

double AppInfo::getVersionDouble()
{
  double version = static_cast<double>(s_versionMinor);
  while (version > 1.0) {
    version /= 10.0;
  }
  return version + static_cast<double>(s_versionMajor);
}

void AppInfo::about(QWidget* a_targetWidget)
{
  if (a_targetWidget == nullptr) {
    return;
  }

  QString text{};
  QTextStream stream{ &text };
  stream << QObject::tr("Qfsm - A graphical tool for designing and simulating finite state machines") << Qt::endl;
  stream << QObject::tr("Version ") << getVersion() << Qt::endl;
  stream << QObject::tr("Development version ") << s_buildDate << Qt::endl;
  stream << QObject::tr("Copyright 2000-2020 by ") << s_author << Qt::endl;
  stream << QObject::tr("email: qfsm@duffner-net.de");

  QMessageBox messageBox{ a_targetWidget };
  messageBox.setIconPixmap(a_targetWidget->windowIcon().pixmap(64, 64));
  messageBox.setWindowTitle(QObject::tr("About Qfsm"));
  messageBox.setText(text);
  messageBox.exec();
}

} // namespace qfsm
