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

#ifndef GUI_ERROR_HPP
#define GUI_ERROR_HPP

#include <QMessageBox>
#include <QString>

/// Provides an easy interface for error messageboxes.
/// @namespace qfsm::gui::error
namespace qfsm::gui::error {

/// Enumerates possible messagebox buttons.
/// @typedef Button
using Button = QMessageBox::StandardButton;

/// Displays information message box.
/// @param a_message text to be displayed.
/// @param a_buttons buttons to show in message box, defaults to OK.
/// @return Return value of message box.
int info(const QString& a_message, QMessageBox::StandardButtons a_buttons = {});

/// Displays warning message box.
/// @param a_message text to be displayed.
/// @param a_buttons buttons to show in message box, defaults to OK.
/// @return Return value of message box.
int warn(const QString& a_message, QMessageBox::StandardButtons a_buttons = {});

} // namespace qfsm::gui::error

#endif // GUI_ERROR_HPP
