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

#include "XMLHandler.h"

#include "gui/error.hpp"
#include "info.hpp"
#include "initialtransition.hpp"
#include "literals.hpp"
#include "machine.hpp"
#include "state.hpp"
#include "transition.hpp"

#include "Convert.h"
#include "FileIO.h"
#include "IOInfoASCII.h"
#include "IOInfoBin.h"
#include "IOInfoText.h"
#include "Project.h"
#include "TransitionInfo.h"
#include "TransitionInfoASCII.h"
#include "TransitionInfoBin.h"
#include "TransitionInfoText.h"
#include "UndoBuffer.h"

#include <QMessageBox>

constexpr qfsm::Hash PROJECT_TAG = "qfsmproject"_hash;
constexpr qfsm::Hash MACHINE_TAG = "machine"_hash;
constexpr qfsm::Hash STATE_TAG = "state"_hash;
constexpr qfsm::Hash TRANSITION_TAG = "transition"_hash;
constexpr qfsm::Hash INIT_TRANS_TAG = "itransition"_hash;
constexpr qfsm::Hash TRANS_INPUTS_TAG = "inputs"_hash;
constexpr qfsm::Hash TRANS_OUTPUTS_TAG = "outputs"_hash;
constexpr qfsm::Hash TRANS_FROM_STATE = "from"_hash;
constexpr qfsm::Hash TRANS_TO_STATE = "to"_hash;
constexpr qfsm::Hash MOORE_OUT_NAMES = "outputnames_moore"_hash;
constexpr qfsm::Hash MEALY_IN_NAMES = "inputnames"_hash;
constexpr qfsm::Hash MEALY_OUT_NAMES = "outputnames"_hash;

struct qfsm::PasteUndoData {
  StatePtr prevInitialState;
  InitialTransitionPtr prevInitialTransition;
  StatePtr newInitialState;
  InitialTransitionPtr newInitialTransition;
  QList<StatePtr> undoStates;
  QList<TransitionPtr> undoTransitions;
  int prevMooreOutputsCount;
  int prevMealyInputsCount;
  int prevMealyOutputsCount;
};

XMLHandler::XMLHandler(qfsm::Project* a_project, bool a_quiet, bool a_createMachine)
  : QObject{ a_project }
  , QXmlStreamReader{}
  , m_project{ a_project }
  , m_createMachine{ !a_project->isValid() || a_createMachine }
  , m_machine{ m_createMachine ? m_project->createMachine() : m_project->machine() }
  , m_pasteUndoData{ std::make_unique<qfsm::PasteUndoData>() }
  , m_quiet{ a_quiet }
{
  m_projectVersion = 1;
  m_initialStateCode = 0;
  m_hasInitialState = true;
  m_transitionType = 0;
}

bool XMLHandler::parse()
{
  bool result = true;

  while (result && !atEnd()) {
    readNext();
    const QStringView name = qualifiedName();
    switch (tokenType()) {
      case TokenType::StartDocument: {
        startDocument();
        break;
      }
      case TokenType::StartElement: {
        result = startElement(qfsm::hash(name.utf16(), name.utf16() + name.size()), attributes());
        break;
      }
      case TokenType::EndElement: {
        endElement(qfsm::hash(name.utf16(), name.utf16() + name.size()));
        break;
      }
      case TokenType::Characters: {
        if (!isCDATA() && !isWhitespace()) {
          m_textData = text().toString();
        }
        break;
      }
    }
  }

  return result && !hasError();
}

bool XMLHandler::startDocument()
{
  m_projectVersion = qfsm::info::getVersionDouble();
  m_initialStateCode = -1;
  m_initialTransition = nullptr;
  m_textData.clear();
  m_stateCodesMapping.clear();

  return true;
}

bool XMLHandler::startElement(qfsm::Hash a_tagHash, const QXmlStreamAttributes& a_attributes)
{
  bool result = true;

  switch (a_tagHash) {
    case PROJECT_TAG: {
      if (a_attributes.hasAttribute("version")) {
        m_projectVersion = a_attributes.value("version").toDouble();
      }
      break;
    }
    case MACHINE_TAG: {
      result = parseMachineTag(a_attributes);
      break;
    }
    case STATE_TAG: {
      result = parseStateTag(a_attributes);
      break;
    }
    case TRANSITION_TAG: {
      result = parseTransitionTag(a_attributes);
      break;
    }
    case INIT_TRANS_TAG: {
      parseInitialTransitionTag(a_attributes);
      break;
    }
    case TRANS_INPUTS_TAG: {
      m_transitionInfo.inputAny = a_attributes.value("any").toInt() != 0;
      m_transitionInfo.inputInvert = a_attributes.value("invert").toInt() != 0;
      m_transitionInfo.inputDefault = a_attributes.value("default").toInt() != 0;
      break;
    }
  }

  return result;
}

bool XMLHandler::endElement(qfsm::Hash a_tagName)
{
  switch (a_tagName) {
    case TRANS_FROM_STATE: {
      parseFromState();
      break;
    }
    case TRANS_TO_STATE: {
      parseToState();
      break;
    }
    case MACHINE_TAG: {
      finishMachineTag();
      break;
    }
    case STATE_TAG: {
      finishStateTag();
      break;
    }
    case TRANSITION_TAG: {
      finishTransitionTag();
      break;
    }
    case MOORE_OUT_NAMES: {
      m_machine->setMooreOutputNames(m_machine->mooreOutputsCount(), m_textData);
      break;
    }
    case MEALY_IN_NAMES: {
      m_machine->setMealyInputNames(m_machine->mealyInputsCount(), m_textData);
      break;
    }
    case MEALY_OUT_NAMES: {
      m_machine->setMealyOutputNames(m_machine->mealyOutputsCount(), m_textData);
      break;
    }
    case TRANS_INPUTS_TAG: {
      m_transitionInfo.inputs = m_textData;
      break;
    }
    case TRANS_OUTPUTS_TAG: {
      m_transitionInfo.outputs = m_textData;
      break;
    }
  }

  m_textData.clear();

  return true;
}

bool XMLHandler::parseMachineTag(const QXmlStreamAttributes& a_attributes)
{
  m_hasInitialState = m_machine->hasInitialState();
  m_initialTransition = nullptr;
  m_stateCodesMapping.clear();

  if (a_attributes.hasAttribute("initialstate")) {
    m_initialStateCode = a_attributes.value("initialstate").toInt();
  } else {
    m_initialStateCode = -1;
  }

  const int mooreOutputsCount = a_attributes.value((m_projectVersion <= 0.41) ? "numbits" : "nummooreout").toInt();
  const int mealyInputsCount = a_attributes.value("numin").toInt();
  const int mealyOutputsCount = a_attributes.value("numout").toInt();

  if (m_createMachine) {
    m_machine->setName(a_attributes.value("name").toString());
    m_machine->setVersion(a_attributes.value("version").toString());
    m_machine->setAuthor(a_attributes.value("author").toString());
    m_machine->setDescription(a_attributes.value("description").toString());
    m_machine->setMooreOutputsCount(mooreOutputsCount);
    m_machine->setMealyInputsCount(mealyInputsCount);
    m_machine->setMealyOutputsCount(mealyOutputsCount);

    if (a_attributes.hasAttribute("type")) {
      m_machine->setType(static_cast<qfsm::Machine::Type>(a_attributes.value("type").toInt()));
    }
    if (a_attributes.hasAttribute("arrowtype")) {
      m_machine->setArrowType(static_cast<qfsm::ArrowType>(a_attributes.value("arrowtype").toInt()));
    }
    if (a_attributes.hasAttribute("draw_it")) {
      m_machine->setDrawInitialTransition(a_attributes.value("draw_it").toInt() != 0);
    }

    QFont& stateFont = m_machine->stateFont();
    if (a_attributes.hasAttribute("statefont")) {
      stateFont.setFamily(a_attributes.value("statefont").toString());
    }
    if (a_attributes.hasAttribute("statefontsize")) {
      stateFont.setPointSize(a_attributes.value("statefontsize").toInt());
    }
    if (a_attributes.hasAttribute("statefontweight")) {
      stateFont.setWeight(static_cast<QFont::Weight>(a_attributes.value("statefontweight").toInt()));
    }
    if (a_attributes.hasAttribute("statefontitalic")) {
      stateFont.setItalic(a_attributes.value("statefontitalic").toInt() != 0);
    }

    QFont& transitionFont = m_machine->transitionFont();
    if (a_attributes.hasAttribute("transfont")) {
      transitionFont.setFamily(a_attributes.value("transfont").toString());
    }
    if (a_attributes.hasAttribute("transfontsize")) {
      transitionFont.setPointSize(a_attributes.value("transfontsize").toInt());
    }
    if (a_attributes.hasAttribute("transfontweight")) {
      transitionFont.setWeight(static_cast<QFont::Weight>(a_attributes.value("transfontweight").toInt()));
    }
    if (a_attributes.hasAttribute("transfontitalic")) {
      transitionFont.setItalic(a_attributes.value("transfontitalic").toInt() != 0);
    }
  } else {
    const auto machineType = static_cast<qfsm::Machine::Type>(a_attributes.value("type").toInt());
    if (!a_attributes.hasAttribute("type") || (machineType != m_machine->type())) {
      qfsm::gui::msg::warn(tr("The types of the two machines are not compatible."));
      return false;
    }

    m_pasteUndoData->prevMooreOutputsCount = m_machine->mooreOutputsCount();
    m_pasteUndoData->prevMealyInputsCount = m_machine->mealyInputsCount();
    m_pasteUndoData->prevMealyOutputsCount = m_machine->mealyOutputsCount();
    m_pasteUndoData->prevInitialState = m_machine->initialState();
    m_pasteUndoData->prevInitialTransition = m_machine->initialTransition();
    m_pasteUndoData->undoStates.clear();
    m_pasteUndoData->undoTransitions.clear();

    if (!m_quiet) {
      if (mooreOutputsCount > m_machine->mooreOutputsCount()) {
        if (qfsm::gui::msg::warn(tr("The number of moore outputs exceeds the limit of this machine.\nDo you want to "
                                    "increase the number of moore outputs of the machine?"),
                                 qfsm::gui::msg::Button::Ok | qfsm::gui::msg::Button::Cancel) == QMessageBox::Ok) {
          m_machine->setMooreOutputsCount(mooreOutputsCount);
        }
      }
      if (mealyInputsCount > m_machine->mealyInputsCount()) {
        if (qfsm::gui::msg::warn(tr("The number of mealy inputs exceeds the limit of this machine.\nDo you want to "
                                    "increase the number of mealy inputs of the machine?"),
                                 qfsm::gui::msg::Button::Ok | qfsm::gui::msg::Button::Cancel) == QMessageBox::Ok) {
          m_machine->setMealyInputsCount(mealyInputsCount);
        }
      }
      if (mealyOutputsCount > m_machine->mealyOutputsCount()) {
        if (qfsm::gui::msg::warn(tr("The number of mealy outputs exceeds the limit of this machine.\nDo you want to "
                                    "increase the number of mealy outputs of the machine?"),
                                 qfsm::gui::msg::Button::Ok | qfsm::gui::msg::Button::Cancel) == QMessageBox::Ok) {
          m_machine->setMealyOutputsCount(mealyOutputsCount);
        }
      }
    }
  }
}

bool XMLHandler::parseStateTag(const QXmlStreamAttributes& a_attributes)
{
  if (m_machine == nullptr) {
    return false;
  }

  m_state = std::make_shared<qfsm::State>();

  if (a_attributes.hasAttribute("code")) {
    int code = a_attributes.value("code").toInt();
    if (m_machine->hasState(code)) {
      code = m_stateCodesMapping.insert(code, m_machine->getNewCode()).value();
    }

    m_state->setCode(code);

    if (m_projectVersion <= 0.41) {
      m_state->setMooreOutputs(std::make_unique<IOInfoBin>(IO_MooreOut, code, m_machine->mooreOutputsCount()));
    }
  } else {
    m_state->setCode(m_machine->getNewCode());
  }

  if (a_attributes.hasAttribute("moore_outputs")) {
    std::unique_ptr<IOInfo> iotmp{};
    if ((m_machine->type() == qfsm::Machine::Type::Binary) || (m_projectVersion <= 0.41)) {
      iotmp = std::make_unique<IOInfoBin>(IO_MooreOut);
      iotmp->setBin(a_attributes.value("moore_outputs").toString(), m_machine->mooreOutputsCount());
    } else if (m_machine->type() == qfsm::Machine::Type::Ascii) {
      iotmp = std::make_unique<IOInfoASCII>(IO_MooreOut, a_attributes.value("moore_outputs").toString());
    } else {
      iotmp = std::make_unique<IOInfoText>(IO_MooreOut, a_attributes.value("moore_outputs").toString());
    }
    m_state->setMooreOutputs(std::move(iotmp));
  }

  m_state->setDescription(a_attributes.value("description").toString());
  m_state->setPosition({ a_attributes.value("xpos").toDouble(), a_attributes.value("ypos").toDouble() });
  m_state->setRadius(a_attributes.value("radius").toInt());
  m_state->setColor(a_attributes.value("pencolor").toUInt());
  m_state->setLineWidth(a_attributes.value("linewidth").toInt());
  m_state->setEntryActions(a_attributes.value("entry_actions").toString());
  m_state->setExitActions(a_attributes.value("exit_actions").toString());
  if (a_attributes.hasAttribute("finalstate")) {
    m_state->setFinal(a_attributes.value("finalstate").toInt() != 0);
  } else if (a_attributes.hasAttribute("endstate")) {
    m_state->setFinal(a_attributes.value("endstate").toInt() != 0);
  }
}

bool XMLHandler::parseTransitionTag(const QXmlStreamAttributes& a_attributes)
{
  if (m_machine == nullptr) {
    return false;
  }

  m_transitionType = a_attributes.hasAttribute("type") ? a_attributes.value("type").toInt() : 1;

  m_transition = std::make_shared<qfsm::Transition>();
  m_transition->setDescription(a_attributes.value("description").toString());
  m_transition->setStartPosition({ a_attributes.value("xpos").toDouble(), a_attributes.value("ypos").toDouble() });
  m_transition->setEndPosition({ a_attributes.value("endx").toDouble(), a_attributes.value("endy").toDouble() });
  m_transition->setControlPoint1({ a_attributes.value("c1x").toDouble(), a_attributes.value("c1y").toDouble() });
  m_transition->setControlPoint2({ a_attributes.value("c2x").toDouble(), a_attributes.value("c2y").toDouble() });
  m_transition->setStraight(a_attributes.value("straight").toInt() != 0);
}

bool XMLHandler::parseInitialTransitionTag(const QXmlStreamAttributes& a_attributes)
{
  m_initialTransition = std::make_shared<qfsm::InitialTransition>();
  m_initialTransition->setStartState(m_machine->phantomState());
  m_initialTransition->setStartPosition(
      { a_attributes.value("xpos").toDouble(), a_attributes.value("ypos").toDouble() });
  m_initialTransition->setEndPosition({ a_attributes.value("endx").toDouble(), a_attributes.value("endy").toDouble() });
}

bool XMLHandler::parseFromState()
{
  bool success = false;
  const int parsedCode = m_textData.toInt(&success);

  if (success) {
    const int code = m_stateCodesMapping.value(parsedCode, parsedCode);
    m_transition->setStartState(m_machine->state(code));
  }

  return success;
}

bool XMLHandler::parseToState()
{
  bool success = false;
  const int parsedCode = m_textData.toInt(&success);

  if (success) {
    const int code = m_stateCodesMapping.value(parsedCode, parsedCode);
    m_transition->setEndState(m_machine->state(code));
  }

  return success;
}

bool XMLHandler::finishMachineTag()
{
  m_initialStateCode = m_stateCodesMapping.value(m_initialStateCode, m_initialStateCode);

  if (!m_hasInitialState && (m_initialStateCode >= 0)) {
    qfsm::InitialTransitionPtr initialTransition = m_machine->initialTransition();
    if (initialTransition) {
      qfsm::StatePtr initialState = m_machine->state(m_initialStateCode);
      if (initialState) {
        m_machine->setInitialState(initialState);
        initialTransition->setEndState(initialState);
      }
      if (m_initialTransition) {
        m_initialTransition->setEndState(initialState);
        m_machine->setInitialTransition(m_initialTransition);
        m_machine->attachInitialTransition();
        m_initialTransition = nullptr;
      }
    }
  }

  m_initialTransition = nullptr;

  if (!m_createMachine) {
    m_pasteUndoData->newInitialState = m_machine->initialState();
    m_pasteUndoData->newInitialTransition = m_machine->initialTransition();

    m_project->undoBuffer()->paste(&undostatelist, &undotranslist, m_prevInitialState, m_machine->initialState(),
                                   m_prevInitialTransition, m_machine->initialTransition(), m_prevMooreOutputsCount,
                                   m_prevMealyInputsCount, m_prevMealyOutputsCount);
  }
}

bool XMLHandler::finishStateTag()
{
  m_state->setName(m_textData);
  m_machine->addState(m_state);
  if (!m_createMachine) {
    m_pasteUndoData->undoStates.push_back(m_state);
  }
}

bool XMLHandler::finishTransitionTag()
{
  if (m_machine) {
    qfsm::Transition::InfoPtr info{};
    // GState* sfrom; <-- state from
    Convert conv{};

    if (m_transitionType == Binary) {
      IOInfoBin bin{ IO_MealyIn };
      IOInfoBin bout{ IO_MealyOut };

      bin = conv.binStrToX10(m_machine->mealyInputsCount(), m_transitionInfo.inputs, IO_MealyIn);
      bout = conv.binStrToX10(m_machine->mealyOutputsCount(), m_transitionInfo.outputs, IO_MealyOut);
      bin.setInvert(m_transitionInfo.inputInvert);
      bin.setAnyInput(m_transitionInfo.inputAny);
      bin.setDefault(m_transitionInfo.inputDefault);

      info = std::make_unique<TransitionInfoBin>(bin, bout);
    } else if (m_transitionType == Ascii) {
      IOInfoASCII ain{ IO_MealyIn, m_transitionInfo.inputs };
      IOInfoASCII aout{ IO_MealyOut, m_transitionInfo.outputs };
      ain.setInvert(m_transitionInfo.inputInvert);
      ain.setAnyInput(m_transitionInfo.inputAny);
      ain.setDefault(m_transitionInfo.inputDefault);

      info = std::make_unique<TransitionInfoASCII>(ain, aout);
    } else {
      IOInfoText tin{ IO_MealyIn, m_transitionInfo.inputs };
      IOInfoText tout{ IO_MealyOut, m_transitionInfo.outputs };
      tin.setAnyInput(m_transitionInfo.inputAny);
      tin.setDefault(m_transitionInfo.inputDefault);

      info = std::make_unique<TransitionInfoText>(tin, tout);
    }

    info->setType(m_transitionType);

    m_transition->setInfo(std::move(info));

    if (!m_transition->hasStartState()) {
      m_transition->setStartState(m_machine->phantomState());
    }
    sfrom->addTransition(m_project, transition, false);

    if (transition->isStraight())
      transition->straighten();

    if (!m_createMachine) {
      m_pasteUndoData->undoTransitions.push_back(m_transition);
    }
  }

  m_transitionInfo.clear();
}
