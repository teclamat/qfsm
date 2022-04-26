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

removed setAutoDelete calls from constructor
*/

#ifndef QXMLHANDLER_H
#define QXMLHANDLER_H

#include <QHash>
#include <QList>
#include <QObject>
#include <QXmlStreamReader>

#include "hash.hpp"

#include <cstddef>
#include <memory>

namespace qfsm {
class Machine;
class Project;
class State;
class Transition;
class InitialTransition;
struct PasteUndoData;
using StatePtr = std::shared_ptr<State>;
using TransitionPtr = std::shared_ptr<Transition>;
using InitialTransitionPtr = std::shared_ptr<InitialTransition>;
using PasteUndoDataPtr = std::unique_ptr<PasteUndoData>;
} // namespace qfsm

/**
 * @class XMLHandler
 * @brief XML parser that parses .fsm files
 */
class XMLHandler : public QObject, public QXmlStreamReader {
  Q_OBJECT
 public:
  XMLHandler(qfsm::Project* a_project, bool keepquiet = true, bool createnewmachine = true);
  ~XMLHandler() = default;
  bool parse();
  bool startDocument();
  bool startElement(qfsm::Hash a_tagHash, const QXmlStreamAttributes& atts);
  bool endElement(qfsm::Hash a_tagHash);
  bool characters(const QString& ch);

 private:
  bool parseMachineTag(const QXmlStreamAttributes& a_attributes);
  bool parseStateTag(const QXmlStreamAttributes& a_attributes);
  bool parseTransitionTag(const QXmlStreamAttributes& a_attributes);
  bool parseInitialTransitionTag(const QXmlStreamAttributes& a_attributes);
  bool parseFromState();
  bool parseToState();
  bool finishMachineTag();
  bool finishStateTag();
  bool finishTransitionTag();

 private:
  struct TransitionInfoData {
    bool inputAny;
    bool inputInvert;
    bool inputDefault;
    QString inputs;
    QString outputs;

    void clear()
    {
      inputAny = false;
      inputInvert = false;
      inputDefault = false;
      inputs.clear();
      outputs.clear();
    }
  };

  qfsm::Project* m_project;
  bool m_createMachine; /// If true a new machine is created, otherwise an existing one is used
  qfsm::Machine* m_machine;
  qfsm::PasteUndoDataPtr m_pasteUndoData;
  bool m_quiet; /// If true, no error messages are print (or dialog boxes opened) during parsing

  /// Processed state pointer
  qfsm::StatePtr m_state{};
  /// Processed transition pointer
  qfsm::TransitionPtr m_transition{};
  /// Processed initial transition
  qfsm::InitialTransitionPtr m_initialTransition{};
  /// Version of Qfsm that created the document
  double m_projectVersion{};
  /// Code of the initial state (that has to be saved during parsing)
  int m_initialStateCode{ -1 };

  TransitionInfoData m_transitionInfo{};

  QString m_textData{};
  /// true is the current state has a code
  bool m_stateHasCode;
  /// Transition tyoe
  int m_transitionType;
  /// Mapping of old state codes to new state codes
  QHash<int, int> m_stateCodesMapping;
  /// true if there is ann initial state
  bool m_hasInitialState;
};

#endif
