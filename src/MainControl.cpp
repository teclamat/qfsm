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

#include "MainControl.h"

#include "Const.h"
#include "MainWindow.h"

namespace qfsm {

MainControl::MainControl(const QString& a_language)
  : QObject{}
  , m_language{ a_language.isEmpty() ? tr("English") : a_language }
{
}

void MainControl::newWindow()
{
  MainWindow* window = new MainWindow{ this };
  window->setLanguage(m_language);
  window->show();
}

/// Creates a new main window and opens the file named @a fileName in it
void MainControl::newWindow(const char* fileName)
{
  MainWindow* w = new MainWindow(this);
  w->setLanguage(m_language);
  w->show();
  w->fileOpenRecent(fileName);
}

void MainControl::quitWindow(MainWindow* a_window)
{
  //  w->hide();
  //  w->close();
  // if (w->aboutToClose)
  //   delete w;
  a_window->deleteLater();
}

void MainControl::addMRUEntry(const QString& a_fileName)
{
  m_recentList.removeAll(a_fileName);
  m_recentList.prepend(a_fileName);

  if (m_recentList.count() > MAX_MRUENTRIES) {
    m_recentList.erase(m_recentList.begin() + MAX_MRUENTRIES, m_recentList.end());
  }
}

void MainControl::removeMRUEntry(const QString& a_fileName)
{
  m_recentList.removeAll(a_fileName);
}

}
