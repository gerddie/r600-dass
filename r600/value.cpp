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

const char *Value::component_names = "xyzw01?_";

Value::Value():
   m_type(gpr),
   m_chan(0),
   m_abs(false),
   m_rel(false),
   m_neg(false)
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

Value::Type Value::type() const
{
   return m_type;
}

void Value::set_abs(bool flag)
{
   m_abs = flag;
}

void Value::set_neg(bool flag)
{
   m_neg = flag;
}

void Value::set_chan(uint64_t chan)
{
   m_chan = chan;
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

void Value::print(std::ostream& os) const
{
   if (m_neg)
      os << "-";

   if (m_abs)
      os << "|";

   do_print(os);

   if (m_abs)
      os << "|";
}

void Value::print(std::ostream& os, const PrintFlags& flags) const
{
   if (m_neg)
      os << "-";

   if (m_abs)
      os << "|";

   do_print(os, flags);

   if (m_abs)
      os << "|";
}

void Value::do_print(std::ostream& os, const PrintFlags& flags) const
{
   (void)flags;
   do_print(os);
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
   return bc | sel() | (chan() << 10);
}

uint64_t Value::encode_for_alu_op3_src1() const
{
   uint64_t bc = 0;
   if (m_rel)
      bc |= src1_rel_bit;
   if (m_neg)
      bc |= src1_neg_bit;
   return bc | (sel() << 13) | (chan() << 23);
}

uint64_t Value::encode_for_alu_op3_src2() const
{
   uint64_t bc = 0;
   if (m_rel)
      bc |= src2_rel_bit;
   if (m_neg)
      bc |= src2_neg_bit;
   return bc | (sel() << 32) | (chan() << 42);
}

uint64_t Value::encode_for_alu_op_dst() const
{
   assert(m_type == gpr);
   uint64_t bc = (sel() << 53) | (chan() << 61);
   if (m_rel)
      bc |= dst_rel_bit;
   return bc;
}

PValue Value::create(uint16_t sel, uint16_t chan, bool abs,
                     bool rel, bool neg, LiteralFlags *literal_index)
{
   if (sel < 128)
      return PValue(new GPRValue(sel, chan, abs, rel, neg));

   if ((sel < 192) || (sel >=256 && sel < 320))
      return PValue(new ConstValue(sel, chan, abs, rel, neg));

   if (sel == ALU_SRC_LITERAL) {
      assert(literal_index);
      literal_index->set(chan);
      return PValue(new LiteralValue(chan, abs, rel, neg));
   }

   if (sel == ALU_SRC_LDS_DIRECT_A || sel == ALU_SRC_LDS_DIRECT_B) {
      assert(literal_index);
      literal_index->set(0);
      literal_index->set(1);
      return PValue(new LDSDirectValue(sel, chan, abs, neg));
   }

   if (sel > 218 && sel < 256) {
      if (rel)
         std::cerr << "rel bit on inline constant ignored";
      return PValue(new InlineConstValue(sel, chan, abs, neg));
   }

   assert("unknown src_sel value");
   return Pointer();
}

PValue Value::create(uint64_t bc, ValueOpEncoding encoding, LiteralFlags *li)
{
   switch (encoding) {
   case alu_op2_src0: return decode_from_alu_op2_src0(bc, li);
   case alu_op2_src1: return decode_from_alu_op2_src1(bc, li);
   case alu_op3_src0: return decode_from_alu_op3_src0(bc, li);
   case alu_op3_src1: return decode_from_alu_op3_src1(bc, li);
   case alu_op3_src2: return decode_from_alu_op3_src2(bc, li);
   case alu_lds_src0: return decode_from_alu_lds_src0(bc, li);
   case alu_lds_src1: return decode_from_alu_lds_src1(bc, li);
   case alu_lds_src2: return decode_from_alu_lds_src2(bc, li);

   case alu_op_dst: return decode_from_alu_op_dst(bc);
   default:
      return Pointer();
   }
}

void Value::set_literal_info(const uint64_t *literals)
{
   (void)literals;
}

void Value::allocate_literal(LiteralBuffer& lb) const
{

}

PValue Value::decode_from_alu_op2_src0(uint64_t bc, LiteralFlags *li)
{
   PValue result = decode_from_alu_op3_src0(bc, li);
   if (bc & src0_abs_bit)
      result->set_abs(true);
   return result;
}

PValue Value::decode_from_alu_op2_src1(uint64_t bc, LiteralFlags *li)
{
   PValue result = decode_from_alu_op3_src1(bc, li);
   if (bc & src1_abs_bit)
      result->set_abs(true);
   return result;
}

PValue Value::decode_from_alu_op3_src0(uint64_t bc, LiteralFlags *li)
{
   auto value = decode_from_alu_lds_src0(bc, li);
   if (bc & src0_neg_bit)
      value->set_neg(true);
   return value;
}

PValue Value::decode_from_alu_op3_src1(uint64_t bc, LiteralFlags *li)
{
   auto value = decode_from_alu_lds_src1(bc, li);
   if (bc & src1_neg_bit)
      value->set_neg(true);
   return value;
}

PValue Value::decode_from_alu_op3_src2(uint64_t bc, LiteralFlags *li)
{
   auto value = decode_from_alu_lds_src2(bc, li);
   if (bc & src2_neg_bit)
      value->set_neg(true);
   return value;
}

PValue Value::decode_from_alu_lds_src0(uint64_t bc, LiteralFlags *li)
{
   int sel = bc & 0x1ff;
   int chan = (bc >> 10) & 3;
   bool rel = bc & src0_rel_bit;
   return create(sel, chan, false, rel, false, li);
}

PValue Value::decode_from_alu_lds_src1(uint64_t bc, LiteralFlags *li)
{
   int sel = (bc>> 13) & 0x1ff;
   int chan = (bc >> 23) & 3;
   bool rel = bc & src1_rel_bit;
   return create(sel, chan, false, rel, false, li);
}

PValue Value::decode_from_alu_lds_src2(uint64_t bc, LiteralFlags *li)
{
   uint16_t sel = (bc >> 32) & 0x1ff;
   uint16_t chan = (bc >> 42) & 3;
   bool rel = bc & src2_rel_bit;
   return create(sel, chan, false, rel, false, li);
}


PValue Value::decode_from_alu_op_dst(uint64_t bc)
{
   uint16_t sel = (bc >> 53) & 0x7f;
   bool rel = bc & dst_rel_bit;
   uint16_t chan = (bc >> 61) & 3;
   return create(sel, chan, false, rel, false, nullptr);
}

GPRValue::GPRValue(uint16_t sel, uint16_t chan, bool abs, bool rel, bool neg):
   Value(Value::gpr, chan, abs, rel, neg),
   m_sel(sel)
{
}

uint64_t GPRValue::sel() const
{
   return m_sel;
}

void GPRValue::do_print(std::ostream& os) const
{
   if (m_sel < 124) {
      os << 'R';
      if (rel())
         os << '[';
      os << m_sel;
      if (rel())
         os << "+AR]";
   } else {
      os << 'T' << m_sel - 124;
      if (rel()) {
         os << "[E:indirect access to clause-local temporary]";
      }
   }
   os << '.' << component_names[chan()];
}

void GPRValue::do_print(std::ostream& os, const PrintFlags& flags) const
{
   if (m_sel < 124) {
      os << 'R';
      if (rel())
         os << '[';

      os << m_sel;
      if (rel()) {
         switch (flags.index_mode) {
         case 0: os << "+AR"; break;
         case 4: os << "+LoopIDX"; break;
         case 5: os << "g"; break;
         case 6: os << "g+AR"; break;
         default: os << "(ERRIDX)";
         }
      }
   } else {
      os << 'T' << m_sel - 124;
      if (rel()) {
         os << "[E:indirect access to clause-local temporary]";
      }
   }
   os << '.' << component_names[chan()];
}

LiteralValue::LiteralValue(uint16_t chan,
                           bool abs, bool rel, bool neg):
   Value(Value::literal, chan, abs, rel, neg),
   m_value(0)
{
}

uint64_t LiteralValue::sel() const
{
   return ALU_SRC_LITERAL;
}

uint32_t LiteralValue::value() const
{
   return m_value;
}

void LiteralValue::do_print(std::ostream& os) const
{
   os << "[0x" << std::setbase(16) << m_value << " "
      << *reinterpret_cast<const float*>(&m_value) << "].";
   os << component_names[chan()];
}

void LiteralValue::do_print(std::ostream& os, const PrintFlags& flags) const
{
   os << "[0x" << std::setbase(16) << m_value << " "
      << std::setbase(10);

   if (flags.literal_is_float)
      os << *reinterpret_cast<const float*>(&m_value) << "f";
   else
      os << m_value << "i";

   os<< "]";
}

void LiteralValue::set_literal_info(const uint64_t *literals)
{
   m_value = (literals[chan()>>1] >> (32 * chan())) & 0xffffffff;
}

SpecialValue::SpecialValue(Type type, int value, int chan, bool abs, bool neg):
   Value(type, chan, abs, 0, neg),
   m_value(static_cast<AluInlineConstants>(value))
{
}

uint64_t SpecialValue::sel() const
{
   return m_value;
}

void SpecialValue::do_print(std::ostream& os) const
{
   auto sv_info = alu_src_const.find(m_value);
   if (sv_info != alu_src_const.end()) {
      os << sv_info->second.descr;
      if (sv_info->second.use_chan)
         os << '.' << component_names[chan()];
      else if (chan() > 0)
         os << "." << component_names[chan()]
            << " (W: Channel ignored)";
   } else {
      os << "E: unknown inline constant " << m_value;
   }
}

InlineConstValue::InlineConstValue(int value, int chan, bool abs, bool neg):
   SpecialValue(Value::cinline, value, chan, abs, neg)
{
}

LDSDirectValue::LDSDirectValue(int value, int chan, bool abs, bool neg):
   SpecialValue(Value::lds_direct, value, chan, abs, neg),
   m_value(static_cast<AluInlineConstants>(value)),
   m_offset_a(0),
   m_stride_a(0),
   m_thread_rel_a(0),
   m_offset_b(0),
   m_stride_b(0),
   m_thread_rel_b(0),
   m_direct_read_32(false)
{
}

uint64_t LDSDirectValue::address_bytecode() const
{
   uint64_t bc = m_offset_a;
   bc |= static_cast<uint64_t>(m_stride_a) << 13;

   bc |= static_cast<uint64_t>(m_offset_a) << 32;
   bc |= static_cast<uint64_t>(m_stride_b) << 45;

   if (m_thread_rel_a)
      bc |= 1 << 22;

   if (m_thread_rel_a)
      bc |= 1ul << 54;

   if (m_direct_read_32)
      bc |= 1ul << 63;

   return bc;
}

void LDSDirectValue::do_print(std::ostream& os) const
{
   os << "[LDS Direct value].";
   os << component_names[chan()];
}


void LDSDirectValue::set_literal_info(const uint64_t *literals)
{
   const uint64_t l = *literals;
   m_offset_a = l & 0x1FFF;
   m_stride_a = (l >> 13) & 0x7f;
   m_thread_rel_a = l & (1 << 22);
   m_offset_b = (l >> 32) & 0x1FFF;
   m_stride_b = (l >> 45) & 0x1FFF;
   m_thread_rel_b = l  & (1ul << 54);
   m_direct_read_32 = literals[1] & (1ul << 63);
}

ConstValue::ConstValue(uint16_t sel, uint16_t chan,
                       bool abs, bool rel, bool neg):
   Value(Value::kconst, chan, abs, rel, neg),
   m_index(sel & 0x1f),
   m_kcache_bank(((sel >> 5) & 1) |  ((sel >> 7) & 2))
{
}

uint64_t ConstValue::sel() const
{
   const int bank_base[4] = {128, 160, 256, 288};
   return m_index + bank_base[m_kcache_bank];
}

void ConstValue::do_print(std::ostream& os) const
{
   os << "KC" << m_kcache_bank << "[" << m_index;
   if (rel())
      os << "+AR";
   os << "]." << component_names[chan()];
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