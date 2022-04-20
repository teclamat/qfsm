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

#ifndef SRC_INFO_HPP
#define SRC_INFO_HPP

#include <QString>

class QWidget;

/// Provides general information about the application.
/// @namespace qfsm::info
namespace qfsm::info {

/// Gets major version info.
/// @return Integer number representing major version number.
int getVersionMajor();

/// Gets minor version info.
/// @return Integer number representing minor version number.
int getVersionMinor();

/// Gets version number as a single double value.
/// @return Floating point number where minor version number is in fractional part.
double getVersionDouble();

/// Gets version number formatted as text.
/// @return Text with full version info.
QString getVersion();

/// Gets current build date.
/// @return Text with build date in `Y-m-d` format.
QString getDate();

/// Gets author(s) information.
/// @return Text with author information.
QString getAuthor();

/// Shows about message.
/// @param a_targetWidget display target for showing about message.
void about(QWidget* a_targetWidget);

} // namespace qfsm::info

#endif // SRC_INFO_HPP
