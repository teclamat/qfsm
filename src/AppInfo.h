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

#ifndef APPINFO_H
#define APPINFO_H

#include <QString>
#include <QWidget>

/// @namespace qfsm
namespace qfsm {

/// Provides general information about the application.
/// Stores version and author of the application and a method to display a info dialogbox.
class AppInfo {
 public:
  AppInfo() = delete;

  /// Gets major version info.
  /// @return Integer number representing major version number.
  static int getVersionMajor() { return s_versionMajor; };

  /// Gets minor version info.
  /// @return Integer number representing minor version number.
  static int getVersionMinor() { return s_versionMinor; };

  /// Gets version number as a single double value.
  /// @return Floating point number where minor version number is in fractional part.
  static double getVersionDouble();

  /// Gets version number formatted as text.
  /// @return Text with full version info.
  static QString getVersion() { return QString{ "%1.%2" }.arg(s_versionMajor).arg(s_versionMinor); };

  /// Gets current build date.
  /// @return Text with build date in `Y-m-d` format.
  static QString getDate() { return s_buildDate; };

  /// Gets author(s) information.
  /// @return Text with author information.
  static QString getAuthor() { return s_author; };

  /// Shows about message.
  /// @param a_targetWidget display target for showing about message.
  static void about(QWidget* a_targetWidget);

 private:
  static int s_versionMajor;
  static int s_versionMinor;
  static QString s_author;
  static QString s_buildDate;
};

} // namespace qfsm

#endif
