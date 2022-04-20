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

#include <QMessageBox>
#include <QRegularExpression>
#include <fstream>
#include <iostream>

#include "ExportRagel.h"
#include "Machine.h"
#include "TransitionInfo.h"
#include "Error.h"
#include "IOInfo.h"
#include "Options.h"
#include "UndoBuffer.h"

// using namespace std;

ExportRagel::ExportRagel(Options* opt)
  : Export(opt)
{
  create_action = false;
  lang_action = 0;
}

bool ExportRagel::validateMachine(Machine* m)
{
  QMutableListIterator<GState*> is(m->getSList());
  bool result = true;
  QString msg =
      tr("The state name \'%1\' is invalid as it is has a special "
         "meaning for the Ragel state machine compiler.");
  QString msg2 = tr("The name of state \'%1\' begins with an invalid character.");

  for (; is.hasNext();) {
    // verify if there is no state called 'start' or 'final'
    //   and if every state name begins with a letter
    QString n = is.next()->getStateName();
    if (n == "start") {
      result = false;
      if (Error::warningOkCancel(msg.arg("start")) == QMessageBox::Cancel)
        return false;
    } else if (n == "final") {
      result = false;
      if (Error::warningOkCancel(msg.arg("final")) == QMessageBox::Cancel)
        return false;
    }
    QString firstletter;
    firstletter = n[0];
    QRegularExpression regexp("[a-zA-Z_]");
    if (!regexp.match(firstletter).hasMatch()) {
      result = false;
      if (Error::warningOkCancel(msg2.arg(n)) == QMessageBox::Cancel)
        return false;
    }
  }
  return result;
}

/// Writes all the relevant data into the tdf file.
void ExportRagel::doExport()
{
  create_action = options->getRagelCreateAction();
  lang_action = options->getRagelLangAction();
  if (options->getRagelDefaultTransitions()) {
    machine->addDefaultTransitionsToEveryState();
    machine->updateDefaultTransitions();
  }

  writeHeader("#");

  *out << std::endl << "%%{" << std::endl;

  writeName();
  writeMain();

  *out << std::endl << "}%%" << std::endl;

  // remove the added default transitions (using the undo buffer)
  if (options->getRagelDefaultTransitions()) {
    UndoBuffer* u = machine->getProject()->undoBuffer();
    for (int i = 0; i < machine->getNumStates(); i++)
      u->undo();
    machine->updateDefaultTransitions();
  }
}

QString ExportRagel::fileFilter()
{
  return "Ragel (*.rl)";
}
QString ExportRagel::defaultExtension()
{
  return "rl";
}

/// Writes the SUBDESIGN line to the output stream
void ExportRagel::writeName()
{
  const std::string machineName = machine->getName().replace(QRegularExpression("\\s"), "_").toStdString();

  *out << "machine " << machineName << ";" << std::endl << std::endl;
}

/// Writes the definition of the subdesign (from BEGIN to END) to the output
/// stream
void ExportRagel::writeMain()
{
  *out << "main :=" << std::endl;

  writeTransitions();

  *out << ";" << std::endl;
}

/// Writes the CASE part to the output stream
void ExportRagel::writeTransitions()
{
  GState* s;
  State* stmp;
  GTransition* t;
  QString tinfoi, tinfoo, sn;
  TransitionInfo* tinfo;
  QString iosingle;
  IOInfo* tioinfo;
  //  Convert conv;
  bool first;
  bool sfirst = true;
  bool tfirst;

  //*out << "\tCASE fsm IS" << endl;

  QMutableListIterator<GState*> is(machine->getSList());

  for (; is.hasNext();) {
    s = is.next();
    if (s->isDeleted())
      continue;

    sn = s->getStateName();
    sn.replace(QRegularExpression(" "), "_");
    // if (s->countTransitions()>0)
    {
      if (!sfirst)
        *out << "," << std::endl;
      if (machine->getInitialState() == s)
        *out << "start:" << std::endl;
      *out << sn.toStdString() << ": (" << std::endl;
    }
    if (s->countTransitions() == 0) {
      *out << "  \"\" @not_accept -> final"; // << sn;
    }

    QMutableListIterator<GTransition*> it(s->tlist);
    tfirst = true;

    for (; it.hasNext();) {
      t = it.next();
      tinfo = t->getInfo();
      tioinfo = tinfo->getInputInfo();

      if (!t->isDeleted() && t->getEnd()) {
        QStringList strlist;
        strlist = tioinfo->getRagelConditions();

        QStringList::iterator ioit;

        if (!tfirst)
          *out << " | " << std::endl;

        *out << "  "; // indent
        if (strlist.count() > 1)
          *out << "( ";

        first = true;
        for (ioit = strlist.begin(); ioit != strlist.end(); ++ioit) {
          iosingle = *ioit;

          if (!first) {
            *out << tioinfo->isInverted() ? " & " : " | ";
          }

          *out << iosingle.toStdString();

          first = false;
        }

        if (strlist.count() > 1)
          *out << " )";

        // next state
        stmp = t->getEnd();
        if (stmp) {
          sn = stmp->getStateName();
          sn.replace(QRegularExpression(" "), "_");
        }

        *out << stmp->isFinalState() ? " @accept " : " @not_accept ";
        *out << " -> " << sn.toStdString();

        tfirst = false;
      }
    }

    // if (s->countTransitions()>0)
    *out << ")";

    sfirst = false;
  }
}

int ExportRagel::writeActionFile(const char* filename, const char* ragelfile)
{
  std::ofstream aout{ filename };

  if (!aout)
    return -1;

  const std::string machineName = machine->getName().replace(QRegularExpression("\\s"), "_").toStdString();

  if (lang_action == 1) // Java
  {
    aout << "public class FSM_" << machineName << std::endl;
    aout << "{" << std::endl;
    aout << std::endl;
  }

  aout << "%%{" << std::endl << std::endl;
  aout << "machine " << machineName << ";" << std::endl << std::endl;
  aout << "action accept { res = 1; }" << std::endl;
  aout << "action not_accept { res = 0; }" << std::endl << std::endl;
  aout << "include \"" << ragelfile << "\";" << std::endl << std::endl;
  aout << "}%%" << std::endl << std::endl << std::endl;

  if (lang_action == 0) // C/C++
  {
    aout << "#include <stdio.h>" << std::endl;
    aout << "#include <string.h>" << std::endl;
    aout << std::endl;
  }
  aout << "%% write data;" << std::endl;
  aout << std::endl;

  switch (lang_action) {
    case 0: // C/C++
      aout << "int parse(char* string)" << std::endl;
      break;
    case 1: // Java
      aout << "public int parse(String string)" << std::endl;
      break;
    case 2:
      aout << "def parse( data )" << std::endl;
      break;
  }

  switch (lang_action) {
    case 0:
    case 1:
      aout << "{" << std::endl;
      aout << "  int cs;" << std::endl;
      aout << "  int res=0;" << std::endl;
      break;
    case 2:
      aout << "  cs=0;" << std::endl;
      aout << "  res=0;" << std::endl;
      break;
  }

  switch (lang_action) {
    case 0:
      aout << "  char *p, *pe;" << std::endl;
      aout << std::endl;
      aout << "  p = string;" << std::endl;
      aout << "  pe = p + strlen(string);" << std::endl;
      break;
    case 1:
      aout << "  int p, pe;" << std::endl;
      aout << std::endl;
      aout << "  p = 0;" << std::endl;
      aout << "  pe = string.length();" << std::endl;
      aout << std::endl;
      aout << "  char data[] = new char[pe];" << std::endl;
      aout << "  for(int i=0; i<pe; i++)" << std::endl;
      aout << "    data[i] = string.charAt(i);" << std::endl;
      break;
    case 2:
      aout << "  p = 0;" << std::endl;
      aout << "  pe = data.length();" << std::endl;
  }

  aout << std::endl;
  aout << "  %% write init;" << std::endl;
  aout << "  %% write exec;" << std::endl;
  aout << std::endl;
  aout << "  return res;" << std::endl;

  switch (lang_action) {
    case 0: // C/C++
    case 1:
      aout << "}" << std::endl << std::endl;
      break;
    case 2:
      aout << "end" << std::endl << std::endl;
      break;
  }

  if (lang_action == 1)  // Java
    aout << "}" << std::endl; // end of class definition

  return 0;
}
