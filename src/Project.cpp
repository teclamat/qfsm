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

#include "Project.h"
#include "AppInfo.h"
#include "DrawArea.h"
#include "GObject.h"
#include "GState.h"
#include "Machine.h"
#include "MainWindow.h"
#include "TransitionInfo.h"
#include "UndoBuffer.h"

#include <QBuffer>

namespace qfsm {

Project::Project(QObject* a_parent)
  : QObject{ a_parent }
  , m_mainWindow{ qobject_cast<MainWindow*>(a_parent) }
  , m_undoBuffer{ new UndoBuffer{ this } }
  , m_machine{ nullptr }
{
}

/**
 * Adds a m_machine to the project.
 * @param n m_machine name
 * @param nb number of bits to code the states
 * @param ni number of input bits
 * @param no number of output bits
 * @param sf font used for drawing the state names
 * @param tf font used for drawing the transition names
 * @param atype arrow type (0: unfilled, 1: filled)
 */
void Project::addMachine(QString n, QString v, QString a, QString d, int type, int nb, QString onamesm, int ni,
                         QString inames, int no, QString onames, QFont sf, QFont tf, int atype, bool draw_it)
{
  removeMachine();

  m_machine = new Machine{ this, n, v, a, d, type, nb, onamesm, ni, inames, no, onames, sf, tf, atype };
  m_machine->setDrawITrans(draw_it);

  connectMachine();

  m_mainWindow->updateIOView(m_machine);
}

Machine* Project::createMachine()
{
  removeMachine();
  m_machine = new Machine{ this };
  connectMachine();

  return m_machine;
}

void Project::removeMachine()
{
  if (m_machine == nullptr) {
    return;
  }
  m_machine->deleteLater();
  m_machine = nullptr;
}

/// Adds m_machine @a m to the project
void Project::addMachine(Machine* a_machine)
{
  if (a_machine == nullptr) {
    return;
  }

  if (m_machine != a_machine) {
    removeMachine();
    m_machine = a_machine;
    m_machine->setProject(this);
    connectMachine();
  }

  m_mainWindow->updateIOView(m_machine);
}

void Project::connectMachine()
{
  DrawArea* drawArea = m_mainWindow->getScrollView()->getDrawArea();
  connect(m_machine, &Machine::newCanvasSize, drawArea, &DrawArea::resizeContentsNotSmaller);
  connect(drawArea, &DrawArea::updateCanvasSize, m_machine, qOverload<int, int, double>(&Machine::updateCanvasSize));
  connect(m_machine, &Machine::repaint, m_mainWindow, &MainWindow::repaintViewport);
}

QString Project::copy() const
{
  QBuffer copyBuffer{};
  copyBuffer.open(QIODevice::WriteOnly);
  saveTo(&copyBuffer, true);
  copyBuffer.close();
  return QString{ copyBuffer.buffer() };
}

void Project::saveTo(QIODevice* a_device, bool a_onlySelected) const
{
  if (!a_device || !a_device->isOpen() || !a_device->isWritable()) {
    return;
  }

  QXmlStreamWriter xml{ a_device };
  xml.setAutoFormatting(true);
  xml.setAutoFormattingIndent(2);

  xml.writeStartDocument();
  xml.writeDTD(QStringLiteral("<!DOCTYPE qfsmproject SYSTEM \"qfsm.dtd\">"));

  xml.writeStartElement(QStringLiteral("qfsmproject"));
  xml.writeAttribute(QStringLiteral("author"), QStringLiteral("Qfsm"));
  xml.writeAttribute(QStringLiteral("version"), qfsm::AppInfo::getVersion());

  if (!m_machine) {
    xml.writeEndDocument();
    return;
  }

  xml.writeStartElement(QStringLiteral("machine"));
  xml.writeAttribute("name", m_machine->getName());
  xml.writeAttribute("version", m_machine->getVersion());
  xml.writeAttribute("author", m_machine->getAuthor());
  xml.writeAttribute("description", m_machine->getDescription());
  xml.writeAttribute("type", QString::number(m_machine->getType()));
  xml.writeAttribute("nummooreout", QString::number(m_machine->getNumMooreOutputs()));
  xml.writeAttribute("numbits", QString::number(m_machine->getNumEncodingBits()));
  xml.writeAttribute("numin", QString::number(m_machine->getNumInputs()));
  xml.writeAttribute("numout", QString::number(m_machine->getNumOutputs()));
  const GState* initialState = m_machine->getInitialState();
  if (initialState) {
    xml.writeAttribute("initialstate", QString::number(initialState->getEncoding()));
  }
  xml.writeAttribute("statefont", m_machine->getSFont().family());
  xml.writeAttribute("statefontsize", QString::number(m_machine->getSFont().pointSize()));
  xml.writeAttribute("statefontweight", QString::number(m_machine->getSFont().weight()));
  xml.writeAttribute("statefontitalic", QString::number(m_machine->getSFont().italic()));
  xml.writeAttribute("transfont", m_machine->getTFont().family());
  xml.writeAttribute("transfontsize", QString::number(m_machine->getTFont().pointSize()));
  xml.writeAttribute("transfontweight", QString::number(m_machine->getTFont().weight()));
  xml.writeAttribute("transfontitalic", QString::number(m_machine->getTFont().italic()));
  xml.writeAttribute("arrowtype", QString::number(m_machine->getArrowType()));
  xml.writeAttribute("draw_it", QString::number(m_machine->getDrawITrans()));

  xml.writeTextElement(QStringLiteral("outputnames_moore"), m_machine->getMooreOutputNames());
  xml.writeTextElement(QStringLiteral("inputnames"), m_machine->getMealyInputNames());
  xml.writeTextElement(QStringLiteral("outputnames"), m_machine->getMealyOutputNames());

  const GITransition* initialTransition = m_machine->getInitialTransition();
  if (initialTransition) {
    const QPointF position = initialTransition->position();
    const QPointF endPosition = initialTransition->endPosition();
    xml.writeStartElement(QStringLiteral("itransition"));
    xml.writeAttribute("xpos", QString::number(position.x()));
    xml.writeAttribute("ypos", QString::number(position.y()));
    xml.writeAttribute("endx", QString::number(endPosition.x()));
    xml.writeAttribute("endy", QString::number(endPosition.y()));
    xml.writeEndElement();
  }

  const QList<GState*>& statesList = m_machine->getSList();
  QList<GTransition*> transitionsList{};

  for (const GState* state : statesList) {
    if (!state || state->isDeleted() || (a_onlySelected && !state->isSelected())) {
      continue;
    }
    writeState(xml, state);

    transitionsList.append(state->tlist);
  }

  for (const GTransition* transition : transitionsList) {
    if (!transition || transition->isDeleted() || (a_onlySelected && !transition->isSelected())) {
      continue;
    }
    writeTransition(xml, transition, a_onlySelected);
  }

  const GState* phantomState = m_machine->getPhantomState();
  if (phantomState) {
    const QList<GTransition*>& phantomTransitions = phantomState->tlist;
    for (const GTransition* transition : phantomTransitions) {
      if (!transition || transition->isDeleted() || (a_onlySelected && !transition->isSelected())) {
        continue;
      }
      writeTransition(xml, transition, a_onlySelected, true);
    }
  }

  xml.writeEndDocument();
}

void Project::writeState(QXmlStreamWriter& a_xml, const GState* a_state)
{
  const QPointF position = a_state->position();

  a_xml.writeStartElement(QStringLiteral("state"));

  a_xml.writeAttribute("description", a_state->getDescription());
  a_xml.writeAttribute("code", QString::number(a_state->getEncoding()));
  a_xml.writeAttribute("moore_outputs", a_state->getMooreOutputsStr());
  a_xml.writeAttribute("xpos", QString::number(position.x()));
  a_xml.writeAttribute("ypos", QString::number(position.y()));
  a_xml.writeAttribute("radius", QString::number(a_state->getRadius()));
  a_xml.writeAttribute("pencolor", QString::number(a_state->getColor().rgb() & 0xffffff));
  a_xml.writeAttribute("linewidth", QString::number(a_state->getLineWidth()));
  a_xml.writeAttribute("finalstate", QString::number(a_state->isFinalState()));
  a_xml.writeAttribute("entry_actions", a_state->getEntryActions());
  a_xml.writeAttribute("exit_actions", a_state->getExitActions());

  a_xml.writeCharacters(a_state->getStateName());

  a_xml.writeEndElement();
}

void Project::writeTransition(QXmlStreamWriter& a_xml, const GTransition* a_transition, bool a_onlySelected,
                              bool a_isPhantom)
{
  a_xml.writeStartElement(QStringLiteral("transition"));

  const QPointF position = a_transition->position();
  const QPointF endPosition = a_transition->endPosition();
  const QPointF controlPoint1 = a_transition->controlPoint1();
  const QPointF controlPoint2 = a_transition->controlPoint2();

  a_xml.writeAttribute("type", QString::number(a_transition->getInfo()->getType()));
  a_xml.writeAttribute("xpos", QString::number(position.x(), 'f'));
  a_xml.writeAttribute("ypos", QString::number(position.y(), 'f'));
  a_xml.writeAttribute("endx", QString::number(endPosition.x(), 'f'));
  a_xml.writeAttribute("endy", QString::number(endPosition.y(), 'f'));
  a_xml.writeAttribute("c1x", QString::number(controlPoint1.x(), 'f'));
  a_xml.writeAttribute("c1y", QString::number(controlPoint1.y(), 'f'));
  a_xml.writeAttribute("c2x", QString::number(controlPoint2.x(), 'f'));
  a_xml.writeAttribute("c2y", QString::number(controlPoint2.y(), 'f'));
  a_xml.writeAttribute("straight", QString::number(a_transition->isStraight()));
  a_xml.writeAttribute("description", a_transition->getDescription());

  if (!a_isPhantom) {
    const GState* stateFrom = static_cast<const GState*>(a_transition->getStart());
    if (!a_onlySelected || stateFrom->isSelected()) {
      a_xml.writeTextElement(QStringLiteral("from"), QString::number(stateFrom->getEncoding()));
    }
  }

  const GState* stateTo = static_cast<const GState*>(a_transition->getEnd());
  if (!a_onlySelected || stateTo->isSelected()) {
    a_xml.writeTextElement(QStringLiteral("to"), QString::number(stateTo->getEncoding()));
  }

  a_xml.writeStartElement(QStringLiteral("inputs"));

  if (!a_isPhantom) {
    const IOInfo* inputInfo = a_transition->getInfo()->getInputInfo();
    a_xml.writeAttribute("invert", QString::number(inputInfo->isInverted()));
    a_xml.writeAttribute("any", QString::number(inputInfo->getAnyInput()));
    a_xml.writeAttribute("default", QString::number(inputInfo->isDefault()));
  }

  a_xml.writeCharacters(a_transition->getInfo()->getInputsStr());
  a_xml.writeEndElement();

  a_xml.writeTextElement(QStringLiteral("outputs"), a_transition->getInfo()->getOutputsStr());

  a_xml.writeEndElement();
}

/**
 * Creates a DOM document of this project and returns it.
 *
 * @param onlyselected If true, only selected objects are used, otherwise the
 * whole project is used
 * @param obj If != NULL, only @a obj is used from the project
 * @returns The DOM document
 */
// QDomDocument Project::getDomDocument(bool onlyselected /*=false*/, GObject* obj /*=NULL*/)
// {
//   Machine* m = m_machine;
//   m->correctCodes();
//   QString prolog =
//       "<?xml version=\"1.0\"?>\n"
//       "<!DOCTYPE qfsmproject SYSTEM \"qfsm.dtd\">\n\n"
//       "<qfsmproject>\n"
//       "</qfsmproject>\n";

//   QDomDocument domdoc;
//   QDomElement root, me, one, ine, onme, itranse;
//   QDomText ontext, intext, onmtext;
//   QString stmp;
//   int inits;
//   GState* s;
//   GITransition* initt;
//   GTransition* t;
//   QList<GState*> slist;
//   QList<GTransition*> tlist;
//   double xpos, ypos, endx, endy, c1x, c1y, c2x, c2y;

//   domdoc.setContent(prolog);

//   if (!m)
//     return domdoc;

//   root = domdoc.documentElement();

//   // Machine

//   root.setAttribute("author", "Qfsm");
//   root.setAttribute("version", qfsm::AppInfo::getVersion());
//   me = domdoc.createElement("machine");
//   me.setAttribute("name", m->getName());
//   me.setAttribute("version", m->getVersion());
//   me.setAttribute("author", m->getAuthor());
//   me.setAttribute("description", m->getDescription());
//   me.setAttribute("type", m->getType());
//   me.setAttribute("nummooreout", m->getNumMooreOutputs());
//   me.setAttribute("numbits", m->getNumEncodingBits());
//   me.setAttribute("numin", m->getNumInputs());
//   me.setAttribute("numout", m->getNumOutputs());
//   s = m->getInitialState();
//   if (s) {
//     inits = s->getEncoding();
//     me.setAttribute("initialstate", inits);
//   }
//   me.setAttribute("statefont", m->getSFont().family());
//   me.setAttribute("statefontsize", m->getSFont().pointSize());
//   me.setAttribute("statefontweight", m->getSFont().weight());
//   me.setAttribute("statefontitalic", m->getSFont().italic());
//   me.setAttribute("transfont", m->getTFont().family());
//   me.setAttribute("transfontsize", m->getTFont().pointSize());
//   me.setAttribute("transfontweight", m->getTFont().weight());
//   me.setAttribute("transfontitalic", m->getTFont().italic());
//   me.setAttribute("arrowtype", m->getArrowType());
//   me.setAttribute("draw_it", m->getDrawITrans());

//   root.appendChild(me);

//   // Input/Output names

//   onme = domdoc.createElement("outputnames_moore");
//   ine = domdoc.createElement("inputnames");
//   one = domdoc.createElement("outputnames");
//   intext = domdoc.createTextNode(m->getMealyInputNames());
//   ontext = domdoc.createTextNode(m->getMealyOutputNames());
//   onmtext = domdoc.createTextNode(m->getMooreOutputNames());
//   ine.appendChild(intext);
//   one.appendChild(ontext);
//   onme.appendChild(onmtext);
//   me.appendChild(onme);
//   me.appendChild(ine);
//   me.appendChild(one);

//   // Initial Transition

//   initt = m->getInitialTransition();
//   if (initt) {
//     initt->getPos(xpos, ypos);
//     initt->getEndPos(endx, endy);
//     itranse = domdoc.createElement("itransition");
//     itranse.setAttribute("xpos", xpos);
//     itranse.setAttribute("ypos", ypos);
//     itranse.setAttribute("endx", endx);
//     itranse.setAttribute("endy", endy);

//     me.appendChild(itranse);
//   }

//   // States

//   slist = m->getSList();
//   QListIterator<GState*> sit(slist);

//   for (; sit.hasNext();) {
//     s = sit.next();
//     if (!s->isDeleted() && (!onlyselected || s->isSelected() || (s == obj && s != NULL))) {
//       s->getPos(xpos, ypos);
//       QDomElement se = domdoc.createElement("state");
//       QDomText st = domdoc.createTextNode(s->getStateName());

//       se.setAttribute("description", s->getDescription());
//       se.setAttribute("code", s->getEncoding());
//       se.setAttribute("moore_outputs", s->getMooreOutputsStr());
//       se.setAttribute("xpos", xpos);
//       se.setAttribute("ypos", ypos);
//       se.setAttribute("radius", s->getRadius());
//       se.setAttribute("pencolor", s->getColor().rgb() & 0xffffff);
//       se.setAttribute("linewidth", s->getLineWidth());
//       se.setAttribute("finalstate", s->isFinalState());
//       se.setAttribute("entry_actions", s->getEntryActions());
//       se.setAttribute("exit_actions", s->getExitActions());

//       se.appendChild(st);

//       me.appendChild(se);
//     }
//   }

//   // Transitions

//   sit.toFront();
//   for (; sit.hasNext();) {
//     s = sit.next();
//     QListIterator<GTransition*> tit(s->tlist);
//     GState* send;

//     for (; tit.hasNext();) {
//       t = tit.next();
//       if (!t->isDeleted() && (!onlyselected || t->isSelected() || (t == obj && t != NULL))) {
//         t->getPos(xpos, ypos);
//         t->getEndPos(endx, endy);
//         t->getCPoint1(c1x, c1y);
//         t->getCPoint2(c2x, c2y);
//         QDomElement te = domdoc.createElement("transition");

//         te.setAttribute("type", t->getInfo()->getType());
//         te.setAttribute("xpos", xpos);
//         te.setAttribute("ypos", ypos);
//         te.setAttribute("endx", endx);
//         te.setAttribute("endy", endy);
//         te.setAttribute("c1x", c1x);
//         te.setAttribute("c1y", c1y);
//         te.setAttribute("c2x", c2x);
//         te.setAttribute("c2y", c2y);
//         te.setAttribute("straight", t->isStraight());
//         te.setAttribute("description", t->getDescription());

//         send = (GState*)t->getEnd();

//         if (!onlyselected || s->isSelected()) {
//           QDomElement from = domdoc.createElement("from");
//           QString sfrom = QString::number(s->getEncoding());
//           QDomText fromt = domdoc.createTextNode(sfrom);
//           from.appendChild(fromt);
//           te.appendChild(from);
//         }

//         if (send && (!onlyselected || send->isSelected())) {
//           QDomElement to = domdoc.createElement("to");
//           QString sto = QString::number(send->getEncoding());
//           // sto.sprintf("%d", send->getEncoding());
//           QDomText tot = domdoc.createTextNode(sto);
//           to.appendChild(tot);
//           te.appendChild(to);
//         }

//         QDomElement inpute, outpute;
//         QDomText inputt, outputt;

//         inpute = domdoc.createElement("inputs");
//         inpute.setAttribute("invert", t->getInfo()->getInputInfo()->isInverted());
//         inpute.setAttribute("any", t->getInfo()->getInputInfo()->getAnyInput());
//         inpute.setAttribute("default", t->getInfo()->getInputInfo()->isDefault());
//         outpute = domdoc.createElement("outputs");

//         inputt = domdoc.createTextNode(t->getInfo()->getInputsStr(NULL));
//         outputt = domdoc.createTextNode(t->getInfo()->getOutputsStr(NULL));

//         inpute.appendChild(inputt);
//         outpute.appendChild(outputt);

//         te.appendChild(inpute);
//         te.appendChild(outpute);

//         me.appendChild(te);
//       }
//     }
//   }

//   // Phantom State

//   s = m->getPhantomState();
//   QListIterator<GTransition*> tit(s->tlist);
//   GState* send;

//   for (; tit.hasNext();) {
//     t = tit.next();
//     if (!t->isDeleted() && (!onlyselected || t->isSelected() || (t == obj && t != NULL))) {
//       t->getPos(xpos, ypos);
//       t->getEndPos(endx, endy);
//       t->getCPoint1(c1x, c1y);
//       t->getCPoint2(c2x, c2y);
//       QDomElement te = domdoc.createElement("transition");

//       te.setAttribute("type", t->getInfo()->getType());
//       te.setAttribute("xpos", xpos);
//       te.setAttribute("ypos", ypos);
//       te.setAttribute("endx", endx);
//       te.setAttribute("endy", endy);
//       te.setAttribute("c1x", c1x);
//       te.setAttribute("c1y", c1y);
//       te.setAttribute("c2x", c2x);
//       te.setAttribute("c2y", c2y);
//       te.setAttribute("straight", t->isStraight());

//       send = (GState*)t->getEnd();
//       if (send && (!onlyselected || send->isSelected())) {
//         QDomElement to = domdoc.createElement("to");
//         QString sto = QString::number(send->getEncoding());
//         QDomText tot = domdoc.createTextNode(sto);
//         to.appendChild(tot);
//         te.appendChild(to);
//       }

//       QDomElement inpute, outpute;
//       QDomText inputt, outputt;

//       inpute = domdoc.createElement("inputs");
//       outpute = domdoc.createElement("outputs");

//       inputt = domdoc.createTextNode(t->getInfo()->getInputsStr(NULL));
//       outputt = domdoc.createTextNode(t->getInfo()->getOutputsStr(NULL));

//       inpute.appendChild(inputt);
//       outpute.appendChild(outputt);

//       te.appendChild(inpute);
//       te.appendChild(outpute);

//       me.appendChild(te);
//     }
//   }

//   return domdoc;
// }

} // namespace qfsm
