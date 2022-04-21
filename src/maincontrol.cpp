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

#include "maincontrol.hpp"

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

void MainControl::newWindow(const QString& a_fileName)
{
  MainWindow* window = new MainWindow{ this };
  window->setLanguage(m_language);
  window->show();
  window->fileOpen(a_fileName);
}

void MainControl::quitWindow(MainWindow* a_window)
{
  a_window->deleteLater();
}

} // namespace qfsm
