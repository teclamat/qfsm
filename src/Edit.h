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

/// Class providing various editing features.
class Edit {
 public:
  Edit() = delete;

  static void deleteSelection(Selection*, Machine*);
  static QString copy(const Project* a_project);
  static bool paste(Project* a_project, Selection* a_selection, const QString& a_data);
};

} // namespace qfsm

#endif
