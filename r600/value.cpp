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

#include "r600/value.h"

#include <iostream>

namespace r600 {

using std::unique_ptr;
using std::make_unique;

Value::Value()
{
}

Value::Value(Type type, uint16_t chan, bool abs, bool rel, bool neg):
   m_type(type),
   m_chan(chan),
   m_abs(abs),
   m_rel(rel),
   m_neg(neg)
{
}

Value::Type Value::get_type() const
{
   return m_type;
}

Value *Value::create(uint16_t sel, uint16_t chan, bool abs,
                     bool rel, bool neg, LiteralFlags &literal_index)
{
   if (sel < 128)
      return new GPRValue(sel, chan, abs, rel, neg);

   if ((sel < 192) || (sel >=256 && sel < 320))
      return new ConstValue(sel, chan, abs, rel, neg);

   if (sel == ALU_SRC_LITERAL) {
      literal_index.set(chan);
      return new LiteralValue(chan, abs, rel, neg);
   }

   if (sel < 255)
      return new InlineConstValue(sel, abs, rel, neg);

   return nullptr;
}

GPRValue::GPRValue(uint16_t sel, uint16_t chan, bool abs, bool rel, bool neg):
   Value(Value::gpr, chan, abs, rel, neg),
   m_sel(sel)
{
}

uint64_t GPRValue::get_sel() const
{
   return m_sel;
}

LiteralValue::LiteralValue(uint16_t chan,
                           bool abs, bool rel, bool neg):
   Value(Value::literal, chan, abs, rel, neg)
{
}

uint64_t LiteralValue::get_sel() const
{
   return ALU_SRC_LITERAL;
}

InlineConstValue::InlineConstValue(int value,
                                   bool abs, bool rel, bool neg):
   Value(Value::cinline, 0, abs, rel, neg),
   m_value(static_cast<AluInlineConstants>(value))
{
}

uint64_t InlineConstValue::get_sel() const
{
   return m_value;
}

ConstValue::ConstValue(uint16_t sel, uint16_t chan,
                       bool abs, bool rel, bool neg):
   Value(Value::kconst, chan, abs, rel, neg),
   m_index(sel & 0x1f),
   m_kcache_bank(((sel >> 5) & 1) |  ((sel >> 7) & 2))
{
}

uint64_t ConstValue::get_sel() const
{
   const int bank_base[4] = {128, 160, 256, 288};
   return m_index + bank_base[m_kcache_bank];
}

}