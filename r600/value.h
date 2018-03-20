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

#include <r600/alu_defines.h>
#include <cstdint>
#include <memory>
#include <bitset>

namespace r600 {

class Value
{
public:
   using Pointer=std::shared_ptr<Value>;

   enum Type {
      gpr,
      kconst,
      literal,
      cinline,
      unknown
   };

   using LiteralFlags=std::bitset<4>;

   Value();
   Value(Type type, uint16_t chan, bool abs, bool rel, bool neg);

   static Pointer create(uint16_t sel, uint16_t chan,
                         bool abs, bool rel, bool neg,
                         LiteralFlags& literal_index);

   Type get_type() const;
   virtual uint64_t get_sel() const = 0;
   uint64_t get_chan() const {return m_chan;}
   bool get_rel() const {return m_rel;}
   bool get_neg() const {return m_neg;}
   bool get_abs() const {return m_abs;}

private:

   Type m_type;
   uint16_t m_chan;
   bool m_abs;
   bool m_rel;
   bool m_neg;
};

using PValue=Value::Pointer;

class GPRValue : public Value
{
public:
        GPRValue(uint16_t sel, uint16_t chan,
                 bool abs, bool rel, bool neg);

        uint64_t get_sel() const override;
private:
        uint16_t m_sel;
};

class LiteralValue: public Value
{
public:
        LiteralValue(uint16_t chan, bool abs, bool rel, bool neg);
        uint64_t get_sel() const override;
};

class InlineConstValue: public Value
{
public:
        InlineConstValue(int value,
                         bool abs, bool rel, bool neg);
        uint64_t get_sel() const override;
private:
        AluInlineConstants m_value;
};

class ConstValue: public Value
{
public:
        ConstValue(uint16_t sel, uint16_t chan,
                   bool abs, bool rel, bool neg);
        uint64_t get_sel() const override;
private:
        uint16_t m_index;
        uint16_t m_kcache_bank;
};

}

#endif // R600VALUE_H
