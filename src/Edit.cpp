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

#include "Edit.h"

#include "Convert.h"
#include "Error.h"
#include "Machine.h"
#include "Project.h"
#include "Selection.h"
#include "TransitionInfo.h"
#include "TransitionInfoASCII.h"
#include "TransitionInfoBin.h"
#include "UndoBuffer.h"
#include "XMLHandler.h"

#include <QList>

namespace qfsm {

/**
 * Deletes all selected objects.
 * @param sel selection object containing all selected objects
 * @param m machine containing the selected objects
 */
void Edit::deleteSelection(Selection* sel, Machine* m)
{
  //  QListIterator<GState> si(sel->getSList());
  //  QListIterator<GTransition> ti(sel->getTList());
  QMutableListIterator<GState*> si(sel->getSList());
  QMutableListIterator<GTransition*> ti(sel->getTList());

  GState* s;
  GTransition* t;
  GObject* obj;
  int type = 0;

  // obj = NULL; // sel->getContextObject(type);
  // if (obj) {
  //   switch (type) {
  //     case StateT:
  //       s = (GState*)obj;
  //       m->getProject()->undoBuffer()->deleteState(s);

  //       if (s == m->getInitialState())
  //         sel->selectITrans(false);
  //       m->removeState(s);
  //       break;

  //     case TransitionT:
  //       t = (GTransition*)obj;
  //       m->getProject()->undoBuffer()->deleteTransition(t);

  //       s = (GState*)t->getStart();
  //       s->removeTransition(t);
  //       break;

  //     default:
  //       break;
  //   }
  // } else {
  m->getProject()->undoBuffer()->deleteSelection(&sel->getSList(), &sel->getTList());

  // delete Transitions
  for (; ti.hasNext();) {
    t = ti.next();
    s = (GState*)t->getStart();
    s->removeTransition(t);
  }

  // delete States
  for (; si.hasNext();) {
    s = si.next();
    if (s == m->getInitialState())
      sel->selectITrans(false);
    m->removeState(s);
  }

  sel->getSList().clear();
  sel->getTList().clear();
}

/**
 * Serializes the selected objects into a string (to be put in the clipboard).
 * The string is in the same format as a .fsm file (XML).
 *
 * @param p Pointer to the project that contains the objects to copy.
 * @param m Pointer to the machine that contains the objects to copy.
 * @param s String that contains the XML data
 * @returns true if successful
 */
QString Edit::copy(const Project* a_project)
{
  if (a_project && a_project->isValid()) {
    return a_project->copy();
  }
  return {};
}

/**
 * Pastes objects into the the current machine.
 *
 * @param sel Pointer to the selection object.
 * @param p Project to paste the objects into
 * @param m Machine to paste the objects into
 * @param data XML string (produced by Edit::copy())
 * @returns true if successful
 */
bool Edit::paste(Project* a_project, Selection* a_selection, const QString& a_data)
{
  if (!a_project || !a_project->isValid() || a_data.isEmpty()) {
    return false;
  }

  XMLHandler handler(a_project, a_selection, false, false);
  handler.addData(a_data);

  return handler.parse();
}

} // namespace qfsm
