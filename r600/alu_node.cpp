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
#include <iomanip>
#include <cassert>

namespace r600 {
using std::runtime_error;

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

      if (opcode != OP3_INST_LDS_IDX_OP) {

         auto src0 = Value::create(bc, alu_op3_src0, literal_index);
         auto src1 = Value::create(bc, alu_op3_src1, literal_index);
         auto src2 = Value::create(bc, alu_op3_src2, literal_index);
         return new AluNodeOp3(opcode, dst, src0, src1, src2, flags,
                               index_mode, bank_swizzle, pred_sel);
      } else {
         auto src0 = Value::create(bc, alu_lds_src0, literal_index);
         auto src1 = Value::create(bc, alu_lds_src1, literal_index);
         auto src2 = Value::create(bc, alu_lds_src2, literal_index);

         auto  lds_op = static_cast<ELSDIndexOp>((bc >> 53) & 0x3f);
         int dst_chan = (bc >> 61) & 0x3;
         int offset = ((bc >> 58) & 1) |
                      ((bc >> 57) & 4) |
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

int AluNode::get_dst_chan() const
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

void AluNode::set_literal_info(uint32_t *literals)
{
   if (m_src[0])
      m_src[0]->set_literal_info(literals);

   if (m_src[1])
      m_src[1]->set_literal_info(literals);

   set_spec_literal_info(literals);
}

void AluNode::set_spec_literal_info(uint32_t *literals)
{
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
}

uint64_t AluNode::get_bytecode() const
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

AluNodeWithDst::AluNodeWithDst(uint16_t opcode, const GPRValue& dst, EIndexMode index_mode,
                               EBankSwizzle bank_swizzle, EPredSelect pred_select,
                               AluOpFlags flags):
   AluNode(opcode, index_mode, bank_swizzle, flags, dst.get_chan()),
   m_dst(dst),
   m_pred_select(pred_select)
{
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

void AluNodeOp3::set_spec_literal_info(uint32_t *literals)
{
   src(2).set_literal_info(literals);
}

void AluNodeOp3::allocate_spec_literal(LiteralBuffer& lb) const
{
   src(2).allocate_literal(lb);
}

AluNodeLDSIdxOP::AluNodeLDSIdxOP(uint16_t opcode, ELSDIndexOp lds_op,
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
   bc |= static_cast<uint64_t>(get_dst_chan()) << 61;
   bc |= static_cast<uint64_t>(m_offset & 1) << 58;
   bc |= static_cast<uint64_t>(m_offset & 2) << 43;
   bc |= static_cast<uint64_t>(m_offset & 4) << 57;
   bc |= static_cast<uint64_t>(m_offset & 8) << 60;
   bc |= static_cast<uint64_t>(m_offset & 0x10) << 8;
   bc |= static_cast<uint64_t>(m_offset & 0x20) << 20;
}

void AluNodeLDSIdxOP::set_spec_literal_info(uint32_t *literals)
{
   src(2).set_literal_info(literals);
}

void AluNodeLDSIdxOP::allocate_spec_literal(LiteralBuffer& lb) const
{
   src(2).allocate_literal(lb);
}


AluGroup::AluGroup():
   m_ops(5)
{
}

std::vector<uint64_t>::const_iterator
AluGroup::decode(std::vector<uint64_t>::const_iterator bc)
{
   PAluNode node;
   Value::LiteralFlags lflags;
   bool group_should_finish = false;

   do {
      if (group_should_finish)
         throw runtime_error("Alu group should have ended");
      node.reset(AluNode::decode(*bc++, &lflags));
      int chan = node->get_dst_chan();
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
   } while (!node->last_instr());

   uint32_t literals[4];

   for (int lp = 0; lp < 2; ++lp) {
      if (lflags.test(2*lp) || lflags.test(2*lp + 1)) {
         literals[2*lp] = *bc & 0xffffffff;
         literals[2*lp+1] = (*bc >> 32 ) & 0xffffffff;
         ++bc;
      }
   }

   for (auto op: m_ops) {
      if (op)
         op->set_literal_info(literals);
   }

   return bc;
}

void AluGroup::encode(std::vector<uint64_t>& bc) const
{
   std::vector<uint64_t> group;
   LiteralBuffer literal_bufffer;

   for (const auto& op: m_ops) {
      if (op) {
         group.push_back(op->get_bytecode());
         op->allocate_literal(literal_bufffer);
      }
   }
   // Add code to store literals
}

}
