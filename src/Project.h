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

added #include <QFont>

Qt 5/6 port by Mateusz Tecław
*/

#ifndef PROJECT_H
#define PROJECT_H

#include <QFont>
#include <QObject>
#include <QXmlStreamWriter>

class Machine;
class MainWindow;
class UndoBuffer;
class GObject;
class GState;
class GTransition;

/// @namespace qfsm
namespace qfsm {

/// Stores project information.
class Project : public QObject {
  Q_OBJECT
 public:
  Project(QObject* a_parent = nullptr);

  ~Project() = default;

  void addMachine(QString, QString, QString, QString, int type, int numbits, QString, int, QString, int, QString, QFont,
                  QFont, int, bool draw_it);
  void addMachine(Machine* m);
  Machine* createMachine();
  void removeMachine();

  /// Returns true if the project has changed otherwise false
  bool hasChanged() const { return m_changed; };

  /// Sets project changed flag.
  /// @param a_isChanged changed flag, defaulted to `true`.
  void setChanged(bool a_isChanged = true) { m_changed = a_isChanged; };

  /// Returns the undo buffer
  UndoBuffer* undoBuffer() { return m_undoBuffer; };

  /// Returns the main window
  MainWindow* mainWindow() { return m_mainWindow; };

  /// Machine (projects can currently contain only a single machine)
  Machine* machine() { return m_machine; }

  QString copy() const;

  void saveTo(QIODevice* a_device, bool a_onlySelected = false) const;

  // QDomDocument getDomDocument(bool a_onlySelected = false, GObject* a_object = nullptr);

 private:
  void connectMachine();
  static void writeState(QXmlStreamWriter& a_xml, const GState* a_state);
  static void writeTransition(QXmlStreamWriter& a_xml, const GTransition* a_transition,
                              bool a_onlySelected, bool a_isPhantom = false);

 private:
  MainWindow* m_mainWindow;
  UndoBuffer* m_undoBuffer;
  Machine* m_machine;

  bool m_changed{ false };
};

} // namespace qfsm

#endif
