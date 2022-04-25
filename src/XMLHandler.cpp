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

#include <qmessagebox.h>

#include "gui/error.hpp"
#include "info.hpp"
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
#include "Selection.h"
#include "TransitionInfo.h"
#include "TransitionInfoASCII.h"
#include "TransitionInfoBin.h"
#include "TransitionInfoText.h"
#include "UndoBuffer.h"
#include "XMLHandler.h"

constexpr qfsm::Hash PROJECT_TAG = "qfsmproject"_hash;
constexpr qfsm::Hash MACHINE_TAG = "machine"_hash;
constexpr qfsm::Hash STATE_TAG = "state"_hash;
constexpr qfsm::Hash TRANSITION_TAG = "transition"_hash;
constexpr qfsm::Hash INIT_TRANS_TAG = "itransition"_hash;
constexpr qfsm::Hash INPUTS_TAG = "inputs"_hash;
constexpr qfsm::Hash OUTPUTS_TAG = "outputs"_hash;
constexpr qfsm::Hash TRANS_FROM_STATE = "from"_hash;
constexpr qfsm::Hash TRANS_TO_STATE = "to"_hash;

/// Constructor
XMLHandler::XMLHandler(qfsm::Project* a_project, Selection* sel /*=nullptr*/, bool a_quiet /*=true*/,
                       bool a_createMachine)
  : QObject{ a_project }
  , QXmlStreamReader{}
{
  m_project = a_project;
  m_createMachine = !m_project->isValid() || a_createMachine;
  m_machine = m_createMachine ? m_project->createMachine() : m_project->machine();
  m_quiet = a_quiet;

  m_state = nullptr;
  m_transition = nullptr;

  m_prevInitialState = nullptr;
  m_prevInitialTransition = nullptr;
  m_prevMooreOutputsCount = 0;
  m_prevMealyInputsCount = 0;
  m_prevMealyOutputsCount = 0;
  m_projectVersion = 1;
  m_initialStateCode = 0;
  m_hasInitialState = true;
  m_stateHasCode = true;
  m_mooreOutputsCount = 0;
  m_mealyInputsCount = 0;
  m_mealyOutputsCount = 0;
  m_transitionType = 0;

  selection = sel;
  itransition = nullptr;
  inamescont = false;
  onamescont = false;
  monamescont = false;
  snamecont = false;
  invert = false;
  any = false;
  def = false;
  fromcont = false;
  tocont = false;
  tincont = false;
  toutcont = false;
  hasfrom = false;
  hasto = false;
  newinitialstate = nullptr;
  newinitialtrans = nullptr;
  state_code_size = 1;
  //  undostatelist.setAutoDelete(false);
  //  undotranslist.setAutoDelete(false);
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

/// Starts a new document
bool XMLHandler::startDocument()
{
  m_projectVersion = qfsm::info::getVersionDouble();
  m_initialStateCode = -1;
  m_stateHasCode = false;
  m_textData.clear();
  m_stateCodesMapping.clear();

  itransition = nullptr;
  inamescont = false;
  onamescont = false;
  monamescont = false;
  snamecont = false;
  fromcont = false;
  tocont = false;
  tincont = false;
  toutcont = false;
  hasfrom = false;
  hasto = false;
  rstatelist.clear();

  return true;
}

/// Called when a start element was parsed
bool XMLHandler::startElement(qfsm::Hash a_tagHash, const QXmlStreamAttributes& a_attributes)
{
  bool result = true;

  switch (a_tagHash) {
    case PROJECT_TAG: {
      m_projectVersion = a_attributes.value("version").toDouble();
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
  }

  if (!result) {
    return false;
  }

  if (qName == "outputnames_moore") {
    monames = "";
    monamescont = true;
  } else if (qName == "inputnames") {
    inames = "";
    inamescont = true;
  } else if (qName == "outputnames") {
    onames = "";
    onamescont = true;
    // } else if (qName == "state") {
    // } else if (qName == "transition") {
    // } else if (qName == "from") {
    //   hasfrom = true;
    //   from = "";
    //   fromcont = true;
  } else if (qName == "to") {
    hasto = true;
    to = "";
    tocont = true;
  } else if (qName == "inputs") {
    iinfo = "";
    invert = false;
    any = false;
    def = false;
    // int len = a_attributes.length();
    // for (int i = 0; i < len; i++) {
    // aname = a_attributes.qName(i);
    // if (aname == "invert")
    invert = (bool)a_attributes.value("invert").toInt();
    // else if (aname == "any")
    any = (bool)a_attributes.value("any").toInt();
    // else if (aname == "default")
    def = (bool)a_attributes.value("default").toInt();
    // }
    tincont = true;
  } else if (qName == "outputs") {
    oinfo = "";
    toutcont = true;
  } else if (qName == "itransition") {
    if (m_machine) {
      itransition = new GITransition{};
      itransition->setStart(m_machine->getPhantomState());
      itransition->setXPos(a_attributes.value("xpos").toDouble());
      itransition->setYPos(a_attributes.value("ypos").toDouble());
      itransition->setEndPosX(a_attributes.value("endx").toDouble());
      itransition->setEndPosY(a_attributes.value("endy").toDouble());
    }
  }
  return true;
}

/// Called when a closing tag was parsed
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
  }

  if (qName == "machine") {
  } else if (qName == "inputnames") {
    m_machine->setMealyInputNames(m_machine->getNumInputs(), inames);
    inames = "";
    inamescont = false;
  } else if (qName == "outputnames") {
    if (m_machine)
      m_machine->setMealyOutputNames(m_machine->getNumOutputs(), onames);
    onames = "";
    onamescont = false;
  } else if (qName == "outputnames_moore") {
    if (m_machine)
      m_machine->setMooreOutputNames(m_machine->getNumMooreOutputs(), monames);
    monames = "";
    monamescont = false;
    // } else if (qName == "state") {
    // } else if (qName == "transition") {
    // else if (qName == "from")
    //   fromcont = false;
    // else if (qName == "to")
    //   tocont = false;
  } else if (qName == "inputs")
    tincont = false;
  else if (qName == "outputs")
    toutcont = false;

  m_textData.clear();

  return true;
}

bool XMLHandler::parseMachineTag(const QXmlStreamAttributes& a_attributes)
{
  m_hasInitialState = m_machine->hasInitialState();

  if (!m_createMachine) {
    m_prevMooreOutputsCount = m_machine->mooreOutputsCount();
    m_prevMealyInputsCount = m_machine->mealyInputsCount();
    m_prevMealyOutputsCount = m_machine->mealyOutputsCount();
    m_prevInitialState = m_machine->initialState();
    m_prevInitialTransition = m_machine->initialTransition();

    undostatelist.clear();
    undotranslist.clear();
  }
  m_initialStateCode = -1;
  inamescont = false;
  onamescont = false;
  monamescont = false;
  snamecont = false;
  itransition = nullptr;
  rstatelist.clear();
  m_stateCodesMapping.clear();

  if (m_createMachine) {
    m_machine->setName(a_attributes.value("name").toString());
    m_machine->setVersion(a_attributes.value("version").toString());
    m_machine->setAuthor(a_attributes.value("author").toString());
    m_machine->setDescription(a_attributes.value("description").toString());

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
  }

  m_mooreOutputsCount = a_attributes.value((m_projectVersion <= 0.41) ? "numbits" : "nummooreout").toInt();
  m_mealyInputsCount = a_attributes.value("numin").toInt();
  m_mealyOutputsCount = a_attributes.value("numout").toInt();
  m_initialStateCode = a_attributes.value("initialstate").toInt();

  if (!m_quiet && (m_mooreOutputsCount > m_machine->mooreOutputsCount())) {
    if (qfsm::gui::msg::warn(tr("The number of moore outputs exceeds the limit of this machine.\nDo you want to "
                                "increase the number of moore outputs of the machine?"),
                             qfsm::gui::msg::Button::Ok | qfsm::gui::msg::Button::Cancel) == QMessageBox::Ok) {
      m_machine->setMooreOutputsCount(m_mooreOutputsCount);
    }
  } else if (m_createMachine) {
    m_machine->setMooreOutputsCount(m_mooreOutputsCount);
  }

  if (!m_quiet && (m_mealyInputsCount > m_machine->mealyInputsCount())) {
    if (qfsm::gui::msg::warn(tr("The number of mealy inputs exceeds the limit of this machine.\nDo you want to "
                                "increase the number of mealy inputs of the machine?"),
                             qfsm::gui::msg::Button::Ok | qfsm::gui::msg::Button::Cancel) == QMessageBox::Ok) {
      m_machine->setMealyInputsCount(m_mealyInputsCount);
    }
  } else if (m_createMachine) {
    m_machine->setMealyInputsCount(m_mealyInputsCount);
  }

  if (!m_quiet && (m_mealyOutputsCount > m_machine->mealyOutputsCount())) {
    if (qfsm::gui::msg::warn(tr("The number of mealy outputs exceeds the limit of this machine.\nDo you want to "
                                "increase the number of mealy outputs of the machine?"),
                             qfsm::gui::msg::Button::Ok | qfsm::gui::msg::Button::Cancel) == QMessageBox::Ok) {
      m_machine->setMealyOutputsCount(m_mealyOutputsCount);
    }
  } else if (m_createMachine) {
    m_machine->setMealyOutputsCount(m_mealyOutputsCount);
  }
}

bool XMLHandler::parseStateTag(const QXmlStreamAttributes& a_attributes)
{
  if (m_machine == nullptr) {
    return false;
  }

  m_state = std::make_shared<qfsm::State>();
  m_stateHasCode = false;

  if (a_attributes.hasAttribute("code")) {
    int code = a_attributes.value("code").toInt();
    if (m_machine->hasState(code)) {
      code = m_stateCodesMapping.insert(code, m_machine->getNewCode()).value();
    }

    m_state->setCode(code);
    m_stateHasCode = true;

    if (m_projectVersion <= 0.41) {
      m_state->setMooreOutputs(std::make_unique<IOInfoBin>(IO_MooreOut, code, m_machine->mooreOutputsCount()));
    }
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

  sname = "";
  snamecont = true;
}

bool XMLHandler::parseTransitionTag(const QXmlStreamAttributes& a_attributes)
{
  if (m_machine == nullptr) {
    return false;
  }
  iinfo = "";
  oinfo = "";
  tincont = false;
  toutcont = false;
  fromcont = false;
  tocont = false;
  hasfrom = false;
  hasto = false;

  m_transitionType = a_attributes.hasAttribute("type") ? a_attributes.value("type").toInt() : 1;

  m_transition = std::make_shared<qfsm::Transition>();
  m_transition->setDescription(a_attributes.value("description").toString());
  m_transition->setStartPosition({ a_attributes.value("xpos").toDouble(), a_attributes.value("ypos").toDouble() });
  m_transition->setEndPosition({ a_attributes.value("endx").toDouble(), a_attributes.value("endy").toDouble() });
  m_transition->setControlPoint1({ a_attributes.value("c1x").toDouble(), a_attributes.value("c1y").toDouble() });
  m_transition->setControlPoint2({ a_attributes.value("c2x").toDouble(), a_attributes.value("c2y").toDouble() });
  m_transition->setStraight(a_attributes.value("straight").toInt() != 0);
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
    it = m_machine->getInitialTransition();

    if (it) {
      is = m_machine->getState(m_initialStateCode);
      if (is) {
        m_machine->setInitialState(is);
        it->setEnd(is);
      }
      if (itransition) {
        delete it;
        itransition->setEnd(is);
        m_machine->setInitialTransition(itransition);
        m_machine->attachInitialTransition();
        itransition = nullptr;
      }
    }
  }

  if (itransition) {
    delete itransition;
    itransition = nullptr;
  }

  if (m_createMachine)
    m_project->addMachine(m_machine);

  if (!m_createMachine) {
    newinitialstate = m_machine->getInitialState();
    newinitialtrans = m_machine->getInitialTransition();

    m_project->undoBuffer()->paste(&undostatelist, &undotranslist, m_prevInitialState, newinitialstate,
                                   m_prevInitialTransition, newinitialtrans, m_prevMooreOutputsCount,
                                   m_prevMealyInputsCount, m_prevMealyOutputsCount);
  }
}

bool XMLHandler::finishStateTag()
{
  m_state->setName(m_textData);
  m_machine->addState(state, false);
  undostatelist.append(state);
  if (selection)
    selection->select(state, false);
  if (!m_stateHasCode)
    m_state->setCode(m_machine->getNewCode());
  sname = "";
  snamecont = false;
}

bool XMLHandler::finishTransitionTag()
{
  if (m_machine) {
    TransitionInfo* info;
    GState* sfrom;
    Convert conv;

    if (m_transitionType == Binary) {
      IOInfoBin bin(IO_MealyIn), bout(IO_MealyOut);

      bin = conv.binStrToX10(m_machine->getNumInputs(), iinfo, IO_MealyIn);
      bout = conv.binStrToX10(m_machine->getNumOutputs(), oinfo, IO_MealyOut);
      bin.setInvert(invert);
      bin.setAnyInput(any);
      bin.setDefault(def);

      info = new TransitionInfoBin(bin, bout);
    } else if (m_transitionType == Ascii) {
      m_transitionType = Ascii;
      IOInfoASCII ain(IO_MealyIn, iinfo), aout(IO_MealyOut, oinfo);
      ain.setInvert(invert);
      ain.setAnyInput(any);
      ain.setDefault(def);

      info = new TransitionInfoASCII(ain, aout);
    } else {
      m_transitionType = Text;
      IOInfoText tin(IO_MealyIn, iinfo), tout(IO_MealyOut, oinfo);
      tin.setAnyInput(any);
      tin.setDefault(def);

      info = new TransitionInfoText(tin, tout);
    }

    info->setType(m_transitionType);
    transition->setInfo(info);

    // QMap<int, int>::Iterator mit;

    // if (hasto && !rstatelist.contains(to.toInt())) {
    //   int ito;
    //   ito = to.toInt();
    //   mit = m_stateCodesMapping.find(ito);
    //   if (mit != m_stateCodesMapping.end())
    //     ito = mit.value();
    //   transition->setEnd(m_machine->getState(ito));
    // } else
    //   transition->setEnd(nullptr);

    // if (hasfrom && !rstatelist.contains(from.toInt())) {
    //   int ifrom = from.toInt();
    //   mit = m_stateCodesMapping.find(ifrom);
    //   if (mit != m_stateCodesMapping.end())
    //     ifrom = mit.value();
    //   sfrom = m_machine->getState(ifrom);
    //   if (!sfrom)
    //     sfrom = m_machine->getPhantomState();
    // } else
    //   sfrom = m_machine->getPhantomState();
    // transition->setStart(sfrom);

    if (!m_transition->hasStartState()) {
      m_transition->setStartState(m_machine->phantomState());
    }

    if (transition->isStraight())
      transition->straighten();
    sfrom->addTransition(m_project, transition, false);
    undotranslist.append(transition);
    // if (selection)
    //   selection->select(transition, false);
  }
  iinfo = "";
  oinfo = "";
  fromcont = tocont = tincont = toutcont = false;
}
