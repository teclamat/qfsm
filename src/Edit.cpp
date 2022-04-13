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

#include "GState.h"
#include "GTransition.h"
#include "Machine.h"
#include "Project.h"
#include "Selection.h"
#include "UndoBuffer.h"
#include "XMLHandler.h"

#include <QList>

namespace qfsm::edit {

bool deleteSelection(Project* a_project, Selection* a_selection)
{
  if (!a_project || !a_project->isValid() || !a_selection) {
    return false;
  }

  Machine* machine = a_project->machine();
  QList<GState*>& states = a_selection->getSList();
  QList<GTransition*>& transitions = a_selection->getTList();

  if (a_selection->hasContextItem()) {
    const ContextItem& contextItem = a_selection->contextItem();
    switch (contextItem.type) {
      case ObjectType::StateT: {
        GState* state = static_cast<GState*>(contextItem.item);
        if (state) {
          a_project->undoBuffer()->deleteState(state);
          if (state == machine->getInitialState()) {
            a_selection->selectITrans(false);
          }
          machine->removeState(state);
        }
        break;
      }
      case ObjectType::TransitionT: {
        GTransition* transition = static_cast<GTransition*>(contextItem.item);
        if (transition) {
          a_project->undoBuffer()->deleteTransition(transition);
          GState* state = static_cast<GState*>(transition->getStart());
          if (state) {
            state->removeTransition(transition);
          }
        }
        break;
      }
      default:
        break;
    }
  } else {
    a_project->undoBuffer()->deleteSelection(states, transitions);

    // Mark selected transitions as deleted in parent states.
    for (GTransition* transition : transitions) {
      if (transition) {
        GState* state = static_cast<GState*>(transition->getStart());
        if (state) {
          state->removeTransition(transition);
        }
      }
    }

    // Mark selected states as deleted in parent machine.
    for (GState* state : states) {
      if (state && (state == machine->getInitialState())) {
        a_selection->selectITrans(false);
      }
      machine->removeState(state);
    }
  }

  a_selection->clear();

  return true;
}

QString copy(const Project* a_project)
{
  if (a_project && a_project->isValid()) {
    return a_project->copy();
  }
  return {};
}

bool paste(Project* a_project, Selection* a_selection, const QString& a_data)
{
  if (!a_project || !a_project->isValid() || a_data.isEmpty()) {
    return false;
  }

  XMLHandler handler(a_project, a_selection, false, false);
  handler.addData(a_data);

  return handler.parse();
}

} // namespace qfsm::edit
