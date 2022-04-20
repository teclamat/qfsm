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

in constructor of MainControl:
changed request whether ins null or empty to .isEmpty()

Qt 5/6 port by Mateusz Tecław
*/

#ifndef SRC_MAINCONTROL_HPP
#define SRC_MAINCONTROL_HPP

#include <QObject>
#include <QString>

class MainWindow;

/// @namespace qfsm
namespace qfsm {

/// Top level object that controls all open main windows.
/// It also holds a list of the most recently used files.
class MainControl : public QObject {
  Q_OBJECT
 public:
  /// Construct a new Main Control object.
  /// @param a_language UI language to be loaded by default.
  MainControl(const QString& a_language = {});

  /// Destroys Main Control object.
  ~MainControl() = default;

 public slots:
  /// Creates a new main window and opens it.
  void newWindow();

  /// Creates a new main window and opens the file named @a a_fileName in it.
  /// @param a_fileName file name from recent files list.
  void newWindow(const char* a_fileName);

  /// Closes the main window @a a_window.
  /// @param a_window window to close.
  void quitWindow(MainWindow* a_window);

 private:
  QString m_language;
};

} // namespace qfsm

#endif // SRC_MAINCONTROL_HPP
