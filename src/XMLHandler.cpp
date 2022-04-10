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

#include "AppInfo.h"
#include "Convert.h"
#include "Error.h"
#include "FileIO.h"
#include "IOInfoASCII.h"
#include "IOInfoBin.h"
#include "IOInfoText.h"
#include "Machine.h"
#include "Project.h"
#include "Selection.h"
#include "TransitionInfo.h"
#include "TransitionInfoASCII.h"
#include "TransitionInfoBin.h"
#include "TransitionInfoText.h"
#include "UndoBuffer.h"
#include "XMLHandler.h"

/// Constructor
XMLHandler::XMLHandler(Project* newProject, Selection* sel /*=nullptr*/, bool keepquiet /*=true*/,
                       bool createnewmachine /*=true*/)
  : QObject((QObject*)newProject)
  , QXmlStreamReader{}
{
  project = newProject;
  machine = nullptr;
  quiet = keepquiet;
  create_new_machine = createnewmachine;
  selection = sel;

  state = nullptr;
  itransition = nullptr;
  transition = nullptr;
  oldinitialstate = nullptr;
  oldinitialtrans = nullptr;
  oldnummooreout = 0;
  oldnumin = 0;
  oldnumout = 0;
  version = 1;
  saveinitialstate = 0;
  inamescont = false;
  onamescont = false;
  monamescont = false;
  snamecont = false;
  hascode = true;
  ttype = 0;
  invert = false;
  any = false;
  def = false;
  fromcont = false;
  tocont = false;
  tincont = false;
  toutcont = false;
  hasfrom = false;
  hasto = false;
  addstate = false;
  hasinitialstate = true;
  newinitialstate = nullptr;
  newinitialtrans = nullptr;
  nummooreout = 0;
  numin = 0;
  numout = 0;
  state_code_size = 1;
  //  undostatelist.setAutoDelete(false);
  //  undotranslist.setAutoDelete(false);
}

bool XMLHandler::parse()
{
  while (!atEnd()) {
    readNext();
    switch (tokenType()) {
      case TokenType::StartDocument:
        startDocument();
        break;
      case TokenType::StartElement:
        startElement(qualifiedName().toString(), attributes());
        break;
      case TokenType::EndElement:
        endElement(qualifiedName().toString());
        break;
      case TokenType::Characters:
        characters(text().toString());
        break;
    }
  }

  return !hasError();
}

/// Starts a new document
bool XMLHandler::startDocument()
{
  version = qfsm::AppInfo::getVersionDouble();
  itransition = nullptr;
  saveinitialstate = -1;
  inamescont = false;
  onamescont = false;
  monamescont = false;
  snamecont = false;
  hascode = false;
  fromcont = false;
  tocont = false;
  tincont = false;
  toutcont = false;
  hasfrom = false;
  hasto = false;
  rstatelist.clear();
  codemap.clear();

  return true;
}

/// Called when a start element was parsed
bool XMLHandler::startElement(const QString& qName, const QXmlStreamAttributes& atts)
{
  QString aname, avalue;

  if (qName == "qfsmproject") {
    // validate author and version
    version = atts.value("version").toDouble();
    // int len = atts.length();
    // for (int i = 0; i < len; i++) {
    //   aname = atts.qName(i);
    //   if (aname == "version") {
    //     version = atts.value(i).toDouble();
    //   }
    // }
  } else if (qName == "machine") {
    machine = project->machine;
    if (create_new_machine || !machine)
      machine = new Machine(project);
    if (machine->getInitialState())
      hasinitialstate = true;
    else
      hasinitialstate = false;
    if (!create_new_machine) {
      oldnummooreout = machine->getNumMooreOutputs();
      oldnumin = machine->getNumInputs();
      oldnumout = machine->getNumOutputs();
      oldinitialstate = machine->getInitialState();
      oldinitialtrans = machine->getInitialTransition();
      undostatelist.clear();
      undotranslist.clear();
    }
    saveinitialstate = -1;
    inamescont = false;
    onamescont = false;
    monamescont = false;
    snamecont = false;
    itransition = nullptr;
    rstatelist.clear();
    codemap.clear();

    int len = atts.length();

    // for (int i = 0; i < len; i++) {
    // aname = atts.qName(i);

    if (create_new_machine) {
      // if (aname == "name")
      machine->setName(atts.value("name").toString());
      // else if (aname == "version")
      machine->setVersion(atts.value("version").toString());
      // else if (aname == "author")
      machine->setAuthor(atts.value("author").toString());
      // else if (aname == "description")
      machine->setDescription(atts.value("description").toString());
      // else if (aname == "type") {
      // int mtype = atts.value("type").toInt();
      machine->setType(atts.value("type").toInt());
      // } else if (aname == "statefont") {
      QFont ftmp = machine->getSFont();
      ftmp.setStyleHint(QFont::Helvetica);
      // ftmp.setStyleStrategy(QFont::PreferBitmap);
      ftmp.setFamily(atts.value("statefont").toString());
      ftmp.setPointSize(atts.value("statefontsize").toInt());
      ftmp.setWeight((QFont::Weight)atts.value("statefontweight").toInt());
      ftmp.setItalic(atts.value("statefontitalic").toInt());
      machine->setSFont(ftmp);
      // } else if (aname == "statefontsize") {
      //   QFont ftmp = machine->getSFont();
      //   machine->setSFont(ftmp);
      // } else if (aname == "statefontweight") {
      //   QFont ftmp = machine->getSFont();
      //   ftmp.setWeight(atts.value(i).toInt());
      //   machine->setSFont(ftmp);
      // } else if (aname == "statefontitalic") {
      //   QFont ftmp = machine->getSFont();
      //   ftmp.setItalic(atts.value(i).toInt());
      //   machine->setSFont(ftmp);
      // } else if (aname == "transfont") {
      ftmp = machine->getTFont();
      ftmp.setStyleHint(QFont::Helvetica);
      // ftmp.setStyleStrategy(QFont::PreferBitmap);
      ftmp.setFamily(atts.value("transfont").toString());
      ftmp.setPointSize(atts.value("transfontsize").toInt());
      ftmp.setWeight((QFont::Weight)atts.value("transfontweight").toInt());
      ftmp.setItalic(atts.value("transfontitalic").toInt());
      machine->setTFont(ftmp);
      // } else if (aname == "transfontsize") {
      //   QFont ftmp = machine->getTFont();
      //   ftmp.setPointSize(atts.value(i).toInt());
      //   machine->setTFont(ftmp);
      // } else if (aname == "transfontweight") {
      //   QFont ftmp = machine->getTFont();
      //   ftmp.setWeight(atts.value(i).toInt());
      //   machine->setTFont(ftmp);
      // } else if (aname == "transfontitalic") {
      //   QFont ftmp = machine->getTFont();
      //   ftmp.setItalic(atts.value(i).toInt());
      //   machine->setTFont(ftmp);
      // } else if (aname == "arrowtype")
      machine->setArrowType(atts.value("arrowtype").toInt());
      // else if (aname == "draw_it")
      machine->setDrawITrans((bool)atts.value("draw_it").toInt());
    } else // !create_new_machine
    {
      // if (aname == "type") {
      const int mtype = atts.value("type").toInt();
      if (mtype != machine->getType()) {
        // if (!quiet)
        ::Error::warningOk(tr("The types of the two machines are not compatible."));
        return false;
      }
      // }
    }

    // if ((aname == "numbits" && version <= 0.41) || aname == "nummooreout") {
    nummooreout = atts.value((version <= 0.41) ? "numbits" : "nummooreout").toInt();
    // } else if (aname == "numin") {
    numin = atts.value("numin").toInt();
    // } else if (aname == "numout") {
    numout = atts.value("numout").toInt();
    // } else if (aname == "initialstate")
    saveinitialstate = atts.value("initialstate").toInt();

    // } // end for

    if (!quiet && nummooreout > machine->getNumMooreOutputs()) {
      if (::Error::warningOkCancel(tr("The number of moore outputs exceeds the limit "
                                      "of this machine.\nDo you want to increase the number of "
                                      "moore outputs of the "
                                      "machine?")) == QMessageBox::Ok) {
        machine->setNumMooreOutputs(nummooreout);
      }
    } else if (create_new_machine)
      machine->setNumMooreOutputs(nummooreout);

    if (!quiet && numin > machine->getNumInputs()) {
      if (::Error::warningOkCancel(tr("The number of mealy inputs exceeds the limit "
                                      "of this machine.\nDo you want to increase the number of "
                                      "mealy inputs of the "
                                      "machine?")) == QMessageBox::Ok) {
        machine->setNumInputs(numin);
      }
    } else if (create_new_machine)
      machine->setNumInputs(numin);

    if (!quiet && numout > machine->getNumOutputs()) {
      if (::Error::warningOkCancel(tr("The number of mealy outputs exceeds the limit "
                                      "of this machine.\nDo you want to increase the number of "
                                      "mealy outputs of the "
                                      "machine?")) == QMessageBox::Ok) {
        machine->setNumOutputs(numout);
      }
    } else if (create_new_machine)
      machine->setNumOutputs(numout);

  } else if (qName == "outputnames_moore") {
    monames = "";
    monamescont = true;
  } else if (qName == "inputnames") {
    inames = "";
    inamescont = true;
  } else if (qName == "outputnames") {
    onames = "";
    onamescont = true;
  } else if (qName == "state") {
    if (machine) {
      QString aname;
      state = new GState(machine);

      hascode = false;

      // for (int i = 0; i < atts.length(); i++) {
      // aname = atts.qName(i);
      if (atts.hasAttribute("code")) {
        // int code;
        int code = atts.value("code").toInt();

        if (machine->getState(code)) {
          int newcode = machine->getNewCode();
          codemap.insert(code, newcode);
          code = newcode;
          addstate = true;
        } else
          addstate = true;
        state->setEncoding(code);
        hascode = true;
        if (version <= 0.41) {
          IOInfoBin* iotmp = new IOInfoBin(IO_MooreOut, code, machine->getNumMooreOutputs());
          state->setMooreOutputs(iotmp);
        }
      }
      if (atts.hasAttribute("moore_outputs")) {
        Convert conv;
        IOInfo* iotmp;
        if (machine->getType() == Binary || version <= 0.41) {
          iotmp = new IOInfoBin(IO_MooreOut);
          iotmp->setBin(atts.value("moore_outputs").toString(), machine->getNumMooreOutputs());
        } else if (machine->getType() == Ascii) {
          iotmp = new IOInfoASCII(IO_MooreOut, atts.value("moore_outputs").toString());
        } else {
          iotmp = new IOInfoText(IO_MooreOut, atts.value("moore_outputs").toString());
        }
        // iotmp = conv.binStrToX10(machine->getNumMooreOutputs(),
        // atts.value(i));
        state->setMooreOutputs(iotmp);
      }
      // else if (aname == "description")
      state->setDescription(atts.value("description").toString());
      // else if (aname == "xpos")
      state->setXPos(atts.value("xpos").toDouble());
      // else if (aname == "ypos")
      state->setYPos(atts.value("ypos").toDouble());
      // else if (aname == "radius")
      state->setRadius(atts.value("radius").toInt());
      // else if (aname == "finalstate" || aname == "endstate")
      if (atts.hasAttribute("finalstate")) {
        state->setFinalState(atts.value("finalstate").toInt());
      } else if (atts.hasAttribute("endstate")) {
        state->setFinalState(atts.value("endstate").toInt());
      }
      // state->setFinalState(atts.value(i).toInt());
      // else if (aname == "brushcolor") {
      QBrush b(QColor(atts.value("brushcolor").toInt()));
      state->setBrush(b);
      // } else if (aname == "pencolor")
      state->setColor(QColor(QRgb(atts.value("pencolor").toInt())));
      // else if (aname == "linewidth")
      state->setLineWidth(atts.value("linewidth").toInt());
      // else if (aname == "entry_actions")
      state->setEntryActions(atts.value("entry_actions").toString());
      // else if (aname == "exit_actions")
      state->setExitActions(atts.value("exit_actions").toString());
      // }

      sname = "";
      snamecont = true;
    }
  } else if (qName == "transition") {
    iinfo = "";
    oinfo = "";
    tincont = false;
    toutcont = false;
    fromcont = false;
    tocont = false;
    hasfrom = false;
    hasto = false;
    if (machine) {
      QString aname;
      transition = new GTransition();
      ttype = 1;

      // for (int i = 0; i < atts.length(); i++) {
      // aname = atts.qName(i);
      // if (aname == "type")
      ttype = atts.value("type").toInt();
      // else if (aname == "description")
      transition->setDescription(atts.value("description").toString());
      // else if (aname == "xpos")
      transition->setXPos(atts.value("xpos").toDouble());
      // else if (aname == "ypos")
      transition->setYPos(atts.value("ypos").toDouble());
      // else if (aname == "endx")
      transition->setEndPosX(atts.value("endx").toDouble());
      // else if (aname == "endy")
      transition->setEndPosY(atts.value("endy").toDouble());
      // else if (aname == "c1x")
      transition->setCPoint1X(atts.value("c1x").toDouble());
      // else if (aname == "c1y")
      transition->setCPoint1Y(atts.value("c1y").toDouble());
      // else if (aname == "c2x")
      transition->setCPoint2X(atts.value("c2x").toDouble());
      // else if (aname == "c2y")
      transition->setCPoint2Y(atts.value("c2y").toDouble());
      // else if (aname == "straight")
      transition->setStraight(atts.value("straight").toDouble());
      // }
    }
  } else if (qName == "from") {
    hasfrom = true;
    from = "";
    fromcont = true;
  } else if (qName == "to") {
    hasto = true;
    to = "";
    tocont = true;
  } else if (qName == "inputs") {
    iinfo = "";
    invert = false;
    any = false;
    def = false;
    // int len = atts.length();
    // for (int i = 0; i < len; i++) {
    // aname = atts.qName(i);
    // if (aname == "invert")
    invert = (bool)atts.value("invert").toInt();
    // else if (aname == "any")
    any = (bool)atts.value("any").toInt();
    // else if (aname == "default")
    def = (bool)atts.value("default").toInt();
    // }
    tincont = true;
  } else if (qName == "outputs") {
    oinfo = "";
    toutcont = true;
  } else if (qName == "itransition") {
    if (machine) {
      QString aname;
      itransition = new GITransition();
      itransition->setStart(machine->getPhantomState());

      // for (int i = 0; i < atts.length(); i++) {
      // aname = atts.qName(i);
      // if (aname == "xpos")
      itransition->setXPos(atts.value("xpos").toDouble());
      // if (aname == "ypos")
      itransition->setYPos(atts.value("ypos").toDouble());
      // if (aname == "endx")
      itransition->setEndPosX(atts.value("endx").toDouble());
      // if (aname == "endy")
      itransition->setEndPosY(atts.value("endy").toDouble());
      // }
    }
  }
  return true;
}

/// Called when a closing tag was parsed
bool XMLHandler::endElement(const QString& qName)
{
  if (qName == "machine") {
    QMap<int, int>::Iterator mit;
    mit = codemap.find(saveinitialstate);
    if (mit != codemap.end())
      saveinitialstate = mit.value();

    if (!hasinitialstate && saveinitialstate >= 0) {
      GState* is;
      GITransition* it;
      it = machine->getInitialTransition();

      if (it) {
        is = machine->getState(saveinitialstate);
        if (is) {
          machine->setInitialState(is);
          it->setEnd(is);
        }
        if (itransition) {
          delete it;
          itransition->setEnd(is);
          machine->setInitialTransition(itransition);
          machine->attachInitialTransition();
          itransition = nullptr;
        }
      }
    }

    if (itransition) {
      delete itransition;
      itransition = nullptr;
    }

    if (create_new_machine)
      project->addMachine(machine);

    if (!create_new_machine) {
      newinitialstate = machine->getInitialState();
      newinitialtrans = machine->getInitialTransition();

      project->getUndoBuffer()->paste(&undostatelist, &undotranslist, oldinitialstate, newinitialstate, oldinitialtrans,
                                      newinitialtrans, oldnummooreout, oldnumin, oldnumout);
    }
  } else if (qName == "inputnames") {
    if (machine)
      machine->setMealyInputNames(machine->getNumInputs(), inames);
    inames = "";
    inamescont = false;
  } else if (qName == "outputnames") {
    if (machine)
      machine->setMealyOutputNames(machine->getNumOutputs(), onames);
    onames = "";
    onamescont = false;
  } else if (qName == "outputnames_moore") {
    if (machine)
      machine->setMooreOutputNames(machine->getNumMooreOutputs(), monames);
    monames = "";
    monamescont = false;
  } else if (qName == "state") {
    state->setStateName(sname);
    if (machine) {
      if (addstate) {
        machine->addState(state, false);
        undostatelist.append(state);
        if (selection)
          selection->select(state, false);
        if (!hascode)
          state->setEncoding(machine->getNewCode());
      }
    }
    sname = "";
    snamecont = false;
  } else if (qName == "transition") {
    if (machine) {
      TransitionInfo* info;
      GState* sfrom;
      Convert conv;

      if (ttype == Binary) {
        IOInfoBin bin(IO_MealyIn), bout(IO_MealyOut);

        bin = conv.binStrToX10(machine->getNumInputs(), iinfo, IO_MealyIn);
        bout = conv.binStrToX10(machine->getNumOutputs(), oinfo, IO_MealyOut);
        bin.setInvert(invert);
        bin.setAnyInput(any);
        bin.setDefault(def);

        info = new TransitionInfoBin(bin, bout);
      } else if (ttype == Ascii) {
        ttype = Ascii;
        IOInfoASCII ain(IO_MealyIn, iinfo), aout(IO_MealyOut, oinfo);
        ain.setInvert(invert);
        ain.setAnyInput(any);
        ain.setDefault(def);

        info = new TransitionInfoASCII(ain, aout);
      } else {
        ttype = Text;
        IOInfoText tin(IO_MealyIn, iinfo), tout(IO_MealyOut, oinfo);
        tin.setAnyInput(any);
        tin.setDefault(def);

        info = new TransitionInfoText(tin, tout);
      }

      info->setType(ttype);
      transition->setInfo(info);

      QMap<int, int>::Iterator mit;

      if (hasto && !rstatelist.contains(to.toInt())) {
        int ito;
        ito = to.toInt();
        mit = codemap.find(ito);
        if (mit != codemap.end())
          ito = mit.value();
        transition->setEnd(machine->getState(ito));
      } else
        transition->setEnd(nullptr);
      if (hasfrom && !rstatelist.contains(from.toInt())) {
        int ifrom = from.toInt();
        mit = codemap.find(ifrom);
        if (mit != codemap.end())
          ifrom = mit.value();
        sfrom = machine->getState(ifrom);
        if (!sfrom)
          sfrom = machine->getPhantomState();
      } else
        sfrom = machine->getPhantomState();
      transition->setStart(sfrom);
      if (transition->isStraight())
        transition->straighten();
      sfrom->addTransition(project, transition, false);
      undotranslist.append(transition);
      if (selection)
        selection->select(transition, false);
    }
    iinfo = "";
    oinfo = "";
    fromcont = tocont = tincont = toutcont = false;
  } else if (qName == "from")
    fromcont = false;
  else if (qName == "to")
    tocont = false;
  else if (qName == "inputs")
    tincont = false;
  else if (qName == "outputs")
    toutcont = false;

  return true;
}

/// Deprecated
bool XMLHandler::characters(const QString& ch)
{
  if (inamescont)
    inames += ch;
  else if (onamescont)
    onames += ch;
  else if (monamescont)
    monames += ch;
  else if (snamecont)
    sname += ch;
  else if (tincont)
    iinfo += ch;
  else if (toutcont)
    oinfo += ch;
  else if (fromcont)
    from += ch;
  else if (tocont)
    to += ch;

  return true;
}
