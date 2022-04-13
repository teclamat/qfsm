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

removed constructor of struct dtlist, which disables the AutoDelete feature
of QList. In Qt 4, this feature does no longer exist.
replaced Qt 3 interator through Qt 4 iterators
*/

#ifndef EDIT_H
#define EDIT_H

#include <QString>

class Selection;
class Machine;

namespace qfsm {
class Project;
}

/// Provides editing features.
/// @namespace qfsm::edit
namespace qfsm::edit {

/// Deletes selected items.
/// @param a_project current project.
/// @param a_selection items selection.
/// @retval true delete operation succeeded.
/// @return false otherwise.
bool deleteSelection(Project* a_project, Selection* a_selection);

/// Serializes selected items into string (to be put into clipboard or used as drag'n'drop data).
/// Resulting string os of same format as qfsm project file (XML).
/// @param a_project current project.
/// @return String in XML format containing selected items. If operation fails, returned string is empty.
QString copy(const Project* a_project);

/// Pastes items into current project.
/// @param a_project current project.
/// @param a_selection current selection.
/// @param a_data data to be put into project (as Qfsm project XML format).
/// @retval true data processed properly.
/// @retval false otjerwise.
bool paste(Project* a_project, Selection* a_selection, const QString& a_data);

} // namespace qfsm::edit

#endif
