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

#include <iostream>
#include <QRegularExpression>

#include "ExportSCXML.h"
#include "Machine.h"
#include "TransitionInfo.h"
//#include "AppInfo.h"
#include "IOInfo.h"

// using namespace std;

ExportSCXML::ExportSCXML(Options *opt) : Export(opt) {}

/// Writes all the relevant data into the tdf file.
void ExportSCXML::doExport() { writeMain(); }

QString ExportSCXML::fileFilter() { return "SCXML (*.xml)"; }
QString ExportSCXML::defaultExtension() { return "xml"; }

/// Writes the reset state and the transitions to the output stream
void ExportSCXML::writeMain() {
  using namespace std;

  GState *initial = NULL;

  initial = machine->getInitialState();

  *out << "<scxml xmlns=\"http://www.w3.org/2005/07/scxml\" version=\"1.0\" "
          "initialstate=\""
       << initial->getStateName().toStdString() << "\">" << endl;
  *out << endl;

  writeHeader("<!--", "-->");
  writeTransitions();

  *out << "</scxml>" << endl;
}

/// Writes the transitions to the output stream
void ExportSCXML::writeTransitions() {
  using namespace std;

  GState *s;
  GTransition *t;
  QString tinfoi, tinfoo, sn1, sn2;
  State *stmp;
  TransitionInfo *tinfo;
  QStringList tinfoo_list;
  QString tinfoi_processed, tinfoo_processed;
  QStringList::iterator sit;

  QMutableListIterator<GState *> is(machine->getSList());

  for (; is.hasNext();) {
    s = is.next();
    if (s->isDeleted())
      continue;

    sn1 = s->getStateName();
    sn1.replace(QRegularExpression(" "), "_");

    if (s->isFinalState())
      *out << "  <final id=\"" << sn1.toStdString() << "\">" << endl;
    else
      *out << "  <state id=\"" << sn1.toStdString() << "\">" << endl;

    QMutableListIterator<GTransition *> it(s->tlist);

    for (; it.hasNext();) {
      t = it.next();
      tinfo = t->getInfo();

      if (!t->isDeleted() && t->getEnd()) {
        tinfoi = tinfo->getInputsStr();
        tinfoo = tinfo->getOutputsStr();
        tinfoo_list = tinfoo.split(',');

        tinfoi_processed = tinfoi.trimmed();
        tinfoi_processed.replace(QRegularExpression(" "), "_");
        stmp = t->getEnd();

        if (!tinfoi.isEmpty() && stmp) {
          sn2 = stmp->getStateName();
          sn2.replace(QRegularExpression(" "), "_");

          *out << "    <transition event=\"" << tinfoi_processed.toStdString()
               << "\" target=\"" << sn2.toStdString() << "\" >" << endl;
          /*
          if (!tinfoo.isEmpty())
            *out << "      <log expr=\"\'" << tinfoo.latin1() << "\'\" />" <<
          endl;
            */
          if (!tinfoo.isEmpty()) {
            sit = tinfoo_list.begin();
            while (sit != tinfoo_list.end()) {
              tinfoo_processed = (*sit).trimmed();
              tinfoo_processed.replace(QRegularExpression(" "), "_");
              *out << "      <send event=\"" << tinfoo_processed.toStdString()
                   << "\" />" << endl;
              ++sit;
            }
          }
          *out << "    </transition>" << endl;
        }
      }
    }
    if (s->isFinalState())
      *out << "  </final>" << endl;
    else
      *out << "  </state>" << endl;
  }
}
