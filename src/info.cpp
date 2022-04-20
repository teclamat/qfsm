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

#include "info.hpp"

#include "literals.hpp"

#include <QIcon>
#include <QMessageBox>
#include <QObject>
#include <QTextStream>
#include <QWidget>

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <QtGlobal>
namespace Qt {
static auto endl = ::endl;
}
#endif

namespace qfsm::info {

constexpr int s_versionMajor = QFSM_VERSION_MAJOR;
constexpr int s_versionMinor = QFSM_VERSION_MINOR;
static const QString s_author = u"Stefan Duffner, Rainer Strobel, Aaron Erhardt"_qs;
static const QString s_buildDate = QStringLiteral(QFSM_BUILD_DATE);

/// Gets major version info.
/// @return Integer number representing major version number.
int getVersionMajor()
{
  return s_versionMajor;
};

/// Gets minor version info.
/// @return Integer number representing minor version number.
int getVersionMinor()
{
  return s_versionMinor;
};

double getVersionDouble()
{
  double version = static_cast<double>(s_versionMinor);
  while (version > 1.0) {
    version /= 10.0;
  }
  return version + static_cast<double>(s_versionMajor);
}

QString getVersion()
{
  return QString{ "%1.%2" }.arg(s_versionMajor).arg(s_versionMinor);
};

QString getDate()
{
  return s_buildDate;
};

QString getAuthor()
{
  return s_author;
};

void about(QWidget* a_targetWidget)
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
  messageBox.setIconPixmap(a_targetWidget->windowIcon().pixmap(48, 48));
  messageBox.setWindowTitle(QObject::tr("About Qfsm"));
  messageBox.setText(text);
  messageBox.exec();
}

} // namespace qfsm
