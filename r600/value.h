/* -*- mia-c++  -*-
 *
 * This file is part of R600-disass a tool to disassemble R600 byte code.
 * Copyright (c) Genoa 2018 Gert Wollny
 *
 * R600-disass  is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef R600VALUE_H
#define R600VALUE_H

#include <r600/defines.h>
#include <cstdint>
#include <memory>

namespace r600 {

class Value
{
public:
   enum Type {
      gpr,
      kconst,
      literal,
      cinline,
      unknown
   };

   Value();
   Value(Type type, bool abs, bool rel, bool neg);

   static Value *create(uint16_t sel, uint16_t chan,
                        bool abs, bool rel, bool neg,
                        int literal_index);

   Type get_type() const;

private:

   Type m_type;
   bool m_abs;
   bool m_rel;
   bool m_neg;
};

using PValue=std::unique_ptr<Value>;

class GPRValue : public Value
{
public:
        GPRValue(uint16_t sel, uint16_t chan,
                 bool abs, bool rel, bool neg);
private:
        uint16_t m_sel;
        uint16_t m_chan;

};

class LiteralValue: public Value
{
public:
        LiteralValue(uint32_t index, uint16_t chan,
                     bool abs, bool rel, bool neg);
private:
        uint32_t m_index;
        uint16_t m_chan;


};

class InlineConstValue: public Value
{
public:
        InlineConstValue(AluInlineConstants value,
                         bool abs, bool rel, bool neg);
private:
        AluInlineConstants m_value;
};

class ConstValue: public Value
{
public:
        ConstValue(uint16_t sel, uint16_t chan,
                   bool abs, bool rel, bool neg);
private:
        uint16_t m_index;
        uint16_t m_kcache_bank;
};

}

#endif // R600VALUE_H
