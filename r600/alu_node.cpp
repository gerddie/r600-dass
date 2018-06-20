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

#include <r600/alu_node.h>

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cassert>

namespace r600 {
using std::runtime_error;
using std::vector;
using std::ostringstream;
using std::setw;

const uint64_t last_instr_bit = 1ul << 31;
const uint64_t up_exec_mask_bit = 1ul << 34;
const uint64_t up_pred_bit = 1ul << 35;
const uint64_t write_mask_bit = 1ul << 36;
const uint64_t clamp_bit = 1ul << 63;

AluNode *AluNode::decode(uint64_t bc, Value::LiteralFlags *literal_index)
{
   AluOpFlags flags;


   /* Decode common parts */
   auto index_mode = static_cast<EIndexMode>((bc >> 26) & 7);
   auto bank_swizzle = static_cast<EBankSwizzle>((bc >> 50) & 7);
   auto pred_sel = static_cast<EPredSelect>((bc >> 29) & 3);

   if (bc & clamp_bit)
      flags.set(1 << do_clamp);

   if (bc & last_instr_bit)
      flags.set(is_last_instr);

   uint16_t opcode = (bc >> 39) & 0x7ff;

   auto pdst = Value::create(bc, alu_op_dst, nullptr);
   GPRValue dst = dynamic_cast<GPRValue&>(*pdst);

   // op3?
   if (opcode & 0x700) {
      // opcode 3 uses the 5 upper bits of the opcode field
      opcode &= 0x7c0;

      if (opcode != op3_lds_idx_op) {
         flags.set(is_op3);
         auto src0 = Value::create(bc, alu_op3_src0, literal_index);
         auto src1 = Value::create(bc, alu_op3_src1, literal_index);
         auto src2 = Value::create(bc, alu_op3_src2, literal_index);
         return new AluNodeOp3(opcode, dst, src0, src1, src2, flags,
                               index_mode, bank_swizzle, pred_sel);
      } else {
         auto src0 = Value::create(bc, alu_lds_src0, literal_index);
         auto src1 = Value::create(bc, alu_lds_src1, literal_index);
         auto src2 = Value::create(bc, alu_lds_src2, literal_index);

         auto  lds_op = static_cast<ESDOp>((bc >> 53) & 0x3f);
         int dst_chan = (bc >> 61) & 0x3;
         int offset = ((bc >> 59) & 1) |
                      ((bc >> 58) & 4) |
                      ((bc >> 60) & 8) |
                      ((bc >> 43) & 2) |
                      ((bc >> 8) & 16) |
                      ((bc >> 20) & 32);

         return new AluNodeLDSIdxOP(opcode, lds_op,
                                    src0, src1, src2, flags,
                                    offset, dst_chan, index_mode,
                                    bank_swizzle);
      }

   } else {
      auto omod = static_cast<EOutputModify>((bc >> 37) & 3);

      if (bc & write_mask_bit)
         flags.set(do_write);

      if (bc & up_exec_mask_bit)
         flags.set(do_update_exec_mask);

      if (bc & up_pred_bit)
         flags.set(do_update_pred);

      auto src0 = Value::create(bc, alu_op2_src0, literal_index);
      auto src1 = Value::create(bc, alu_op2_src1, literal_index);
      return new AluNodeOp2(opcode, dst, src0, src1, flags,
                            index_mode, bank_swizzle, omod, pred_sel);
   }
}

AluNode::AluNode(uint16_t opcode, EIndexMode index_mode, EBankSwizzle bank_swizzle,
                 AluOpFlags flags, int dst_chan):
   m_opcode(static_cast<EAluOp>(opcode)),
   m_index_mode(index_mode),
   m_bank_swizzle(bank_swizzle),
   m_flags(flags),
   m_dst_chan(dst_chan)
{
   m_src.resize(3);
}

int AluNode::dst_chan() const
{
   return m_dst_chan;
}

bool AluNode::last_instr() const
{
   return m_flags.test(is_last_instr);
}

bool AluNode::test_flag(FlagsShifts f) const
{
   return m_flags.test(f);
}

bool AluNode::slot_supported(int flag) const
{
   auto op = alu_ops.find(m_opcode);
   if (op != alu_ops.end())
      return op->second.can_channel(1 << flag);
   throw runtime_error("Unknown op");
}

void AluNode::set_literal_info(uint64_t *literals)
{
   if (m_src[0])
      m_src[0]->set_literal_info(literals);

   if (m_src[1])
      m_src[1]->set_literal_info(literals);

   set_spec_literal_info(literals);
}

void AluNode::set_spec_literal_info(uint64_t *literals)
{
   DASS_UNUSED(literals);
}

void AluNode::allocate_literal(LiteralBuffer& lb) const
{
   if (m_src[0])
      m_src[0]->allocate_literal(lb);

   if (m_src[1])
      m_src[1]->allocate_literal(lb);

   allocate_spec_literal(lb);
}

void AluNode::allocate_spec_literal(LiteralBuffer& lb) const
{
   DASS_UNUSED(lb);
}

void AluNode::print(std::ostream& os) const
{
   print_flags(os);
   print_pred(os);

   bool uses_float = print_op(os);

   print_dst(os);

   Value::PrintFlags flags(m_index_mode, uses_float);

   if (nopsources() > 0) {
      m_src[0]->print(os, flags);

      for (int i = 1; i < nopsources(); ++i) {
         os << ", ";
         m_src[i]->print(os, flags);
      }
   }

   print_bank_swizzle(os);

}

void AluNode::print_bank_swizzle(std::ostream& os) const
{
   static const char* bank_swz[] = {
      "",
      " vec_021",
      " vec_120",
      " vec_102",
      " vec_201",
      " vec_210"
   };

   if (m_bank_swizzle < 6)
      os << bank_swz[m_bank_swizzle];
   else
      os << " vec_err ";
}

void AluNode::print_pred(std::ostream& os) const
{
   os << "  ";
}

void AluNode::print_omod(std::ostream& os) const
{
   (void)os;
}

bool AluNode::print_op(std::ostream& os) const
{
   bool retval = false;
   auto o = alu_ops.find(m_opcode);
   if (o != alu_ops.end()) {
      ostringstream s;
      s << o->second.name;
      print_omod(s);
      if (m_flags.test(do_clamp))
         s << " (C)";
      os << setw(32) << std::left  << s.str();
      retval = o->second.is_float;
   } else {
      os << setw(32) << std::left  << "E: Unknown opcode " << m_opcode;
   }
   return retval;
}

void AluNode::print_dst(std::ostream& os) const
{
   os << "__." << Value::component_names[m_dst_chan] <<", ";
}

void AluNode::print_flags(std::ostream& os) const
{
   os << (m_flags.test(do_update_exec_mask) ? "M" : " ");
   os << (m_flags.test(do_update_pred) ? "P" : " ");
}

uint64_t AluNode::bytecode() const
{
   uint64_t bc;

   bc = static_cast<uint64_t>(m_opcode) << 39;


   if (m_flags.test(do_clamp))
      bc |= clamp_bit;

   if (m_flags.test(is_last_instr))
      bc |= last_instr_bit;

   bc |= static_cast<uint64_t>(m_bank_swizzle) << 50;
   bc |= static_cast<uint64_t>(m_index_mode) << 26;

   encode(bc);
   return bc;
}
const Value& AluNode::src(unsigned idx) const
{
   assert(idx < m_src.size());
   return *m_src[idx];
}

Value& AluNode::src(unsigned idx)
{
   assert(idx < m_src.size());
   return *m_src[idx];
}

void AluNode::set_src(unsigned idx, PValue v)
{
   assert(idx < m_src.size());
   m_src[idx]= v;
}

int AluNode::nopsources() const
{
   auto k = alu_ops.find(m_opcode);
   if (k != alu_ops.end()) {
      return k->second.nsrc;
   } else {
      return -1;
   }
}

void AluNode::collect_values_with_literals(std::vector<PValue>& values) const
{
   for (auto v: m_src) {
      if (v &&
          ((v->type() == Value::literal) ||
           (v->type() == Value::lds_direct)))
         values.push_back(v);
   }
}

AluNodeWithDst::AluNodeWithDst(uint16_t opcode, const GPRValue& dst, EIndexMode index_mode,
                               EBankSwizzle bank_swizzle, EPredSelect pred_select,
                               AluOpFlags flags):
   AluNode(opcode, index_mode, bank_swizzle, flags, dst.chan()),
   m_dst(dst),
   m_pred_select(pred_select)
{
}

void AluNodeWithDst::print_pred(std::ostream& os) const
{
   switch (m_pred_select) {
   case pred_sel_zero: os << "0 ";
      break;
   case pred_sel_one: os << "1 ";
      break;
   default:
      os << "  ";
   }
}

void AluNodeWithDst::print_dst(std::ostream& os) const
{
   if (test_flag(do_write) || test_flag(is_op3))
      os << m_dst << ", ";
   else
      os << "__." << Value::component_names[m_dst.chan()] <<", ";
}

void AluNodeWithDst::encode_dst_and_pred(uint64_t& bc) const
{
   bc |= m_dst.encode_for(alu_op_dst);
   bc |= static_cast<uint64_t>(m_pred_select) << 29;
}

AluNodeOp2::AluNodeOp2(uint16_t opcode, const GPRValue& dst,
                       PValue src0, PValue src1, AluOpFlags flags,
                       EIndexMode index_mode, EBankSwizzle bank_swizzle,
                       EOutputModify output_modify, EPredSelect pred_select):
   AluNodeWithDst(opcode, dst, index_mode, bank_swizzle, pred_select, flags),
   m_output_modify(output_modify)
{

   set_src(0, src0);
   set_src(1, src1);
}

void AluNodeOp2::encode(uint64_t& bc) const
{
   encode_dst_and_pred(bc);

   auto nsrc = nopsources();

   if (nsrc > 0)
      bc |= src(0).encode_for(alu_op2_src0);
   if (nsrc > 1)
      bc |= src(1).encode_for(alu_op2_src1);


   if (test_flag(do_update_exec_mask))
      bc |= up_exec_mask_bit;

   if (test_flag(do_update_pred))
      bc |= up_pred_bit;

   if (test_flag(do_write))
      bc |= write_mask_bit;

   bc |= static_cast<uint64_t>(m_output_modify) << 37;
}

void AluNodeOp2::print_omod(std::ostream& os) const
{
   switch (m_output_modify) {
   case AluNode::omod_off: return;
   case AluNode::omod_mul_2: os << "*2"; break;
   case AluNode::omod_mul_4: os << "*4"; break;
   case AluNode::omod_div_2: os << "/2"; break;
   default:
      os << "(omod error)";
   }
}

AluNodeOp3::AluNodeOp3(uint16_t opcode, const GPRValue &dst,
                       PValue src0, PValue  src1, PValue  src2, AluOpFlags flags,
                       EIndexMode index_mode, EBankSwizzle bank_swizzle,
                       EPredSelect pred_select):
   AluNodeWithDst(opcode, dst, index_mode, bank_swizzle, pred_select, flags)
{
   set_src(0, src0);
   set_src(1, src1);
   set_src(2, src2);
}

void AluNodeOp3::encode(uint64_t& bc) const
{
   assert(nopsources() == 3);
   encode_dst_and_pred(bc);

   bc |= src(0).encode_for(alu_op3_src0);
   bc |= src(1).encode_for(alu_op3_src1);
   bc |= src(2).encode_for(alu_op3_src2);
}

void AluNodeOp3::set_spec_literal_info(uint64_t *literals)
{
   src(2).set_literal_info(literals);
}

AluNodeLDSIdxOP::AluNodeLDSIdxOP(uint16_t opcode, ESDOp lds_op,
                                 PValue src0, PValue src1,
                                 PValue src2, AluOpFlags flags,
                                 int offset, int dst_chan,
                                 EIndexMode index_mode,
                                 EBankSwizzle bank_swizzle):
   AluNode(opcode, index_mode, bank_swizzle, flags,
           dst_chan),
   m_lds_op(lds_op),
   m_offset(offset)
{
   set_src(0, src0);
   set_src(1, src1);
   set_src(2, src2);
}

void AluNodeLDSIdxOP::encode(uint64_t& bc) const
{
   /* needs to check actual numbers of ussed registers */
   bc |= src(0).encode_for(alu_op3_src0);
   bc |= src(1).encode_for(alu_op3_src1);
   bc |= src(2).encode_for(alu_op3_src2);

   bc |= static_cast<uint64_t>(m_lds_op) << 53;
   bc |= static_cast<uint64_t>(dst_chan()) << 61;
   bc |= static_cast<uint64_t>(m_offset & 1) << 59;
   bc |= static_cast<uint64_t>(m_offset & 2) << 43;
   bc |= static_cast<uint64_t>(m_offset & 4) << 58;
   bc |= static_cast<uint64_t>(m_offset & 8) << 60;
   bc |= static_cast<uint64_t>(m_offset & 0x10) << 8;
   bc |= static_cast<uint64_t>(m_offset & 0x20) << 20;
}

int AluNodeLDSIdxOP::nopsources() const
{
   auto o = lds_ops.find(m_lds_op);
   if (o != lds_ops.end()) {
      return o->second.nsrc;
   }
   return 0;
}

bool AluNodeLDSIdxOP::print_op(std::ostream& os) const
{
   auto o = lds_ops.find(m_lds_op);
   if (o != lds_ops.end()) {
      ostringstream s;
      s << 'L' << o->second.name;
      s << " OFS:" << m_offset;
      os << setw(32) << std::left  << s.str();
   } else {
      os << setw(32) << std::left  << "E: Unknown LDS opcode " << m_lds_op;
   }
   return true;
}

void AluNodeLDSIdxOP::set_spec_literal_info(uint64_t *literals)
{
   src(2).set_literal_info(literals);
}

AluGroup::AluGroup():
   m_ops(5)
{
}

size_t AluGroup::decode(const std::vector<uint64_t>& bc, size_t ofs, size_t end)
{
   PAluNode node;
   Value::LiteralFlags lflags;
   bool group_should_finish = false;
   assert(bc.size() >= end);

   do {
      if (group_should_finish)
         throw runtime_error("Alu group should have ended");
      node.reset(AluNode::decode(bc[ofs++], &lflags));
      int chan = node->dst_chan();
      if (!m_ops[chan]) {
         if (node->slot_supported(chan)) {
            m_ops[chan] = node;
            continue;
         }
      }
      /* Node could not be put into xyzw channel, try t */
      if (m_ops[4])
            throw runtime_error("Alu group scheduls a channel more than "
                                "once and trans is already occupied");
      if (node->slot_supported(AluOp::t))
         m_ops[4] = node;
      else {
         throw runtime_error("Alu group schedules an instruction into "
                             "trans that is not allowed there");
      }
      group_should_finish = true;
   } while (!node->last_instr() && ofs < end);

   uint64_t literals[2];

   for (int lp = 0; lp < 2; ++lp) {
      if (lflags.test(2*lp) || lflags.test(2*lp + 1)) {
         if (ofs >= end)
            throw runtime_error("Trying to decode literals past end of byte code");
         literals[lp] = bc[ofs++];
      }
   }

   for (auto op: m_ops) {
      if (op)
         op->set_literal_info(literals);
   }

   return ofs;
}

std::string AluGroup::as_string(int indent) const
{
   static const char slot_id[6]="xyzwt";
   ostringstream os;
   for (int i = 0; i < 5; ++i) {
      int ind = indent;
      if (m_ops[i]) {
         while (ind--)
            os << ' ';
         os << slot_id[i] << ": ";
         m_ops[i]->print(os);
         os << "\n";
      }
   }
   return os.str();
}

bool AluGroup::encode(std::vector<uint64_t>& bc) const
{
   vector<PValue> values;
   for (const auto& op: m_ops) {
      if (op)
         op->collect_values_with_literals(values);
   }

   bool has_lds_direct_address = false;
   uint64_t literals[2];
   int offset = 0;

   for (auto v :values) {
      if (v->type() == Value::lds_direct) {
         auto ldsdv = static_cast<const LDSDirectValue&>(*v);
         uint64_t l = ldsdv.address_bytecode();
         if (!has_lds_direct_address) {
            literals[0] = l;
         } else if (literals[0] != l) {
            return false;
         }
         offset = 2;
      }
   }

   for (auto v :values) {
      if (v->type() == Value::literal) {
         auto literal = static_cast<LiteralValue&>(*v);
         uint64_t l = literal.value();

         if (offset < 4) {
            literals[offset >> 1] =  l << (32 * (offset & 1));
            literal.set_chan(offset);
            ++offset;
         } else
            return false;
      }
   }

   std::vector<uint64_t> group;
   for (const auto& op: m_ops) {
      if (op) {
         group.push_back(op->bytecode());
      }
   }

   for (int i = 0; i < (offset >> 1); ++i) {
      group.push_back(literals[i]);
   }

   std::copy(group.begin(), group.end(),
             std::back_inserter<vector<uint64_t>>(bc));
   return true;
}

}
