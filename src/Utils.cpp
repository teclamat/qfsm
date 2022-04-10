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

#include <QRegularExpression>

#include "Utils.h"

/// Transforms a string @a s to upper case, removes leading and trailing white
/// spaces and replaces other white spaces to underscores
QString Utils::toUpperNoWS(QString s) {
  s = s.trimmed();
  s = s.replace(QRegularExpression("\\s"), "_");
  s = s.toUpper();
  return s;
}

/// Removes leading and trailing white spaces and replaces other white spaces to
/// underscores of string @a s
QString Utils::noWS(QString s) {
  s = s.trimmed();
  s = s.replace(QRegularExpression("\\s"), "_");
  return s;
}

/**
 * Validates the a string of binary digits.
 * @returns true if string valid, otherwise false.
 */
bool Utils::binStringValid(QString s) {
  QChar c;
  char cl;
  int count = 0;

  do {
    c = s[count++];
    cl = c.toLatin1();
    if (cl != '0' && cl != '1' && c != QChar::Null)
      return false;
  } while (c != QChar::Null);

  return true;
}
