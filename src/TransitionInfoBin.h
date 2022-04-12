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

#ifndef TRANSITIONINFOBIN_H
#define TRANSITIONINFOBIN_H

#include "IOInfoBin.h"
#include "TransitionInfo.h"

/**
 * @class TransitionInfoBin
 * @brief Stores information about a binary transition condition.
 */
class TransitionInfoBin : public TransitionInfo {
 public:
  TransitionInfoBin();
  TransitionInfoBin(const TransitionInfoBin&);
  TransitionInfoBin(IOInfoBin bin, IOInfoBin bout);
  ~TransitionInfoBin();

  TransitionInfoBin& operator=(const TransitionInfoBin&);

  TransitionInfo* clone() { return new TransitionInfoBin(*this); };

  /// Returns array with input bits
  /*char**/ IOInfoBin getInputs() const { return *(IOInfoBin*)inputs; };
  /// Returns array with output bits
  /*char**/ IOInfoBin getOutputs() const { return *(IOInfoBin*)outputs; };
  void setInputs(QString, int numin = -1);
  void setOutputs(QString, int numout = -1);
  void setInputsSize(int bits);
  void setOutputsSize(int bits);

  QString getCompleteInputsStr(Machine* m = nullptr, Options* opt = nullptr) const;
  QString getInputsStr(Machine* m = nullptr, Options* opt = nullptr) const;
  QString getOutputsStr(Machine* m = nullptr, Options* opt = nullptr) const;
  QString getInputsStrBin(Machine* m = nullptr, Options* opt = nullptr) const;
  QString getOutputsStrBin(Machine* m = nullptr, Options* opt = nullptr) const;
  QString getInputsStrHex(/*int*/) const;
  QString getOutputsStrHex(/*int*/) const;
  QString getInputsStrASCII() const;
  QString getOutputsStrASCII() const;
  QString getSeparator(Options* opt) const;

  bool matches(IOInfo*);

 private:
};

#endif
