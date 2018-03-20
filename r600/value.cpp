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
#include <iomanip>
#include <cassert>

namespace r600 {

using std::unique_ptr;
using std::make_shared;

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

uint64_t Value::encode_for(ValueOpEncoding encoding) const
{
   switch (encoding) {
   case alu_op2_src0: return encode_for_alu_op2_src0();
   case alu_op2_src1: return encode_for_alu_op2_src1();
   case alu_op3_src0: return encode_for_alu_op3_src0();
   case alu_op3_src1: return encode_for_alu_op3_src1();
   case alu_op3_src2: return encode_for_alu_op3_src2();
   case alu_op_dst: return encode_for_alu_op_dst();
   default:
      assert(0 && "unknown ALU register target");
   }
   return 0;
}

uint64_t Value::encode_for_alu_op2_src0() const
{
   uint64_t bc = m_abs ? src0_abs_bit : 0;
   return bc | encode_for_alu_op3_src0();
}

uint64_t Value::encode_for_alu_op2_src1() const
{
   uint64_t bc = m_abs ? src1_abs_bit : 0;
   return bc | encode_for_alu_op3_src1();
}

uint64_t Value::encode_for_alu_op3_src0() const
{
   uint64_t bc = 0;
   if (m_rel)
      bc |= src0_rel_bit;
   if (m_neg)
      bc |= src0_neg_bit;
   return bc | get_sel() | (get_chan() << 10);
}

uint64_t Value::encode_for_alu_op3_src1() const
{
   uint64_t bc = 0;
   if (m_rel)
      bc |= src1_rel_bit;
   if (m_neg)
      bc |= src1_neg_bit;
   return bc | (get_sel() << 13) | (get_chan() << 23);
}

uint64_t Value::encode_for_alu_op3_src2() const
{
   uint64_t bc = 0;
   if (m_rel)
      bc |= src2_rel_bit;
   if (m_neg)
      bc |= src2_neg_bit;
   return bc | (get_sel() << 32) | (get_chan() << 42);
}

uint64_t Value::encode_for_alu_op_dst() const
{
   assert(m_type == gpr);
   uint64_t bc = (get_sel() << 53) | (get_chan() << 61);
   if (m_rel)
      bc |= dst_rel_bit;
   return bc;
}

PValue Value::create(uint16_t sel, uint16_t chan, bool abs,
                     bool rel, bool neg, LiteralFlags &literal_index)
{
   if (sel < 128)
      return PValue(new GPRValue(sel, chan, abs, rel, neg));

   if ((sel < 192) || (sel >=256 && sel < 320))
      return PValue(new ConstValue(sel, chan, abs, rel, neg));

   if (sel == ALU_SRC_LITERAL) {
      literal_index.set(chan);
      return PValue(new LiteralValue(chan, abs, rel, neg));
   }

   if (sel > 218 && sel < 256) {
      if (rel)
         std::cerr << "rel bit on inline constant ignored";
      return PValue(new InlineConstValue(sel, chan, abs, neg));
   }

   assert("unknown src_sel value");
   return Pointer();
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

InlineConstValue::InlineConstValue(int value, int chan, bool abs, bool neg):
   Value(Value::cinline, chan, abs, 0, neg),
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

const uint64_t src0_rel_bit = 1ul << 9;
const uint64_t src1_rel_bit = 1ul << 22;
const uint64_t src2_rel_bit = 1ul << 41;
const uint64_t src0_neg_bit = 1ul << 12;
const uint64_t src1_neg_bit = 1ul << 25;
const uint64_t src2_neg_bit = 1ul << 44;
const uint64_t src0_abs_bit = 1ul << 32;
const uint64_t src1_abs_bit = 1ul << 33;
const uint64_t dst_rel_bit = 1ul << 60;

}