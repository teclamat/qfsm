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

#include "StatusBar.h"
// Added by qt3to4:
#include <QLabel>

/// Constructor
StatusBar::StatusBar(QWidget* parent, const char* name)
  : QStatusBar(parent)
{
  //  lmessage = new QLabel(this);

  zoom = new QLabel(this);
  zoom->setAlignment(Qt::AlignCenter);

  position = new QLabel(this);
  position->setAlignment(Qt::AlignRight);
  position->setContentsMargins(0, 0, 10, 0);

  selected = new QLabel(this);
  selected->setAlignment(Qt::AlignCenter);

  addWidget(zoom, 1);
  addWidget(selected, 1);
  addWidget(position, 1);
}

/// Destructor
StatusBar::~StatusBar()
{
}

/// Display @a sel as the number of selected objects
void StatusBar::setSelected(int sel)
{
  selected->setText(QString{ "Selected: %1" }.arg(sel > 0 ? QString::number(sel) : QStringLiteral("0")));
}

/// Sets the cursor position to @a x, @a y
void StatusBar::setPosition(double x, double y)
{
  QString s;
  if (x >= 0 && y >= 0)
    s = QString::asprintf("X: %6.1f  Y: %6.1f", x, y);
  position->setText(s);
}

/// Sets the current zoom factor (in percent)
void StatusBar::setZoom(int z)
{
  QString s{};
  if (z >= 0)
    s = QString::number(z) + "%";
  zoom->setText(QString{ "Zoom: %1" }.arg(s.isEmpty() ? QStringLiteral("Unknown") : s));
}
