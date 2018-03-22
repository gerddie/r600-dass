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

AluNode::AluNode(uint16_t opcode, PValue src0, PValue src1,
                 EIndexMode index_mode, EBankSwizzle bank_swizzle,
                 AluOpFlags flags, int dst_chan):
   m_opcode(static_cast<EAluOp>(opcode)),
   m_src0(src0),
   m_src1(src1),
   m_index_mode(index_mode),
   m_bank_swizzle(bank_swizzle),
   m_flags(flags),
   m_dst_chan(dst_chan)
{
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
const Value& AluNode::src0() const
{
   assert(m_src0);
   return *m_src0;
}

const Value& AluNode::src1() const
{
   assert(m_src1);
   return *m_src1;
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

AluNodeWithDst::AluNodeWithDst(uint16_t opcode, const GPRValue& dst,
                               PValue src0, PValue src1, EIndexMode index_mode,
                               EBankSwizzle bank_swizzle, EPredSelect pred_select,
                               AluOpFlags flags):
   AluNode(opcode, src0, src1, index_mode, bank_swizzle,
           flags, dst.get_chan()),
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
   AluNodeWithDst(opcode, dst, src0, src1, index_mode,
                  bank_swizzle, pred_select, flags),
   m_output_modify(output_modify)
{
}

void AluNodeOp2::encode(uint64_t& bc) const
{
   encode_dst_and_pred(bc);

   auto nsrc = nopsources();

   if (nsrc > 0)
      bc |= src0().encode_for(alu_op2_src0);
   if (nsrc > 1)
      bc |= src1().encode_for(alu_op2_src1);


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
   AluNodeWithDst(opcode, dst, src0, src1, index_mode,
                  bank_swizzle, pred_select, flags),
   m_src2(src2)
{
}

void AluNodeOp3::encode(uint64_t& bc) const
{
   assert(nopsources() == 3);
   encode_dst_and_pred(bc);

   bc |= src0().encode_for(alu_op3_src0);
   bc |= src1().encode_for(alu_op3_src1);
   bc |= m_src2->encode_for(alu_op3_src2);
}

AluNodeLDSIdxOP::AluNodeLDSIdxOP(uint16_t opcode, ELSDIndexOp lds_op,
                                 PValue src0, PValue src1,
                                 PValue src2, AluOpFlags flags,
                                 int offset, int dst_chan,
                                 EIndexMode index_mode,
                                 EBankSwizzle bank_swizzle):
   AluNode(opcode, src0, src1, index_mode, bank_swizzle, flags,
           dst_chan),
   m_lds_op(lds_op),
   m_offset(offset),
   m_src2(src2)
{
}

void AluNodeLDSIdxOP::encode(uint64_t& bc) const
{
   /* needs to check actual numbers of ussed registers */
   bc |= src0().encode_for(alu_op3_src0);
   bc |= src1().encode_for(alu_op3_src1);
   bc |= m_src2->encode_for(alu_op3_src2);

   bc |= static_cast<uint64_t>(m_lds_op) << 53;
   bc |= static_cast<uint64_t>(get_dst_chan()) << 61;
   bc |= static_cast<uint64_t>(m_offset & 1) << 58;
   bc |= static_cast<uint64_t>(m_offset & 2) << 43;
   bc |= static_cast<uint64_t>(m_offset & 4) << 57;
   bc |= static_cast<uint64_t>(m_offset & 8) << 60;
   bc |= static_cast<uint64_t>(m_offset & 0x10) << 8;
   bc |= static_cast<uint64_t>(m_offset & 0x20) << 20;
   }

AluGroup::AluGroup():
   m_ops(5),
   m_nlinterals(0)
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
      node.reset(AluNode::decode(*bc, &lflags));
      int chan = node->get_dst_chan();
      if (m_ops[chan]) {
         if (m_ops[4])
            throw runtime_error("Alu group wants to use same slot more than "
                                "once and trans is already occupied");
         m_ops[4] = node;
         group_should_finish = true;
      } else {
         m_ops[chan] = node;
      }
      ++bc;
   } while (!node->last_instr());

   for (int lp = 0; lp < 2; ++lp)
      if (lflags.test(2*lp) || lflags.test(2*lp + 1)) {
         m_nlinterals++;
         m_literals[2*lp] = *bc & 0xffffffff;
         m_literals[2*lp+1] = (*bc >> 32 ) & 0xffffffff;
         ++bc;
      }
   return bc;
}

void AluGroup::encode(std::vector<uint64_t>& bc) const
{
   for (const auto& op: m_ops) {
      if (op)
         bc.push_back(op->get_bytecode());
   }
   for (int i = 0; i < m_nlinterals; ++i) {
      uint64_t l =(static_cast<uint64_t>(m_literals[2*i+1]) << 32) |
            m_literals[2*i+1];
      bc.push_back(l);
   }
}

const std::map<EAluOp, AluOp> alu_ops = {
   {OP2_ADD                 ,AluOp(2, AluOp::a,"ADD")},
   {OP2_MUL                 ,AluOp(2, AluOp::a,"MUL")},
   {OP2_MUL_IEEE            ,AluOp(2, AluOp::a,"MUL_IEEE")},
   {OP2_MAX                 ,AluOp(2, AluOp::a,"MAX")},
   {OP2_MIN                 ,AluOp(2, AluOp::a,"MIN")},
   {OP2_MAX_DX10            ,AluOp(2, AluOp::a,"MAX_DX10")},
   {OP2_MIN_DX10            ,AluOp(2, AluOp::a,"MIN_DX10")},
   {OP2_SETE                ,AluOp(2, AluOp::a,"SETE")},
   {OP2_SETGT               ,AluOp(2, AluOp::a,"SETGT")},
   {OP2_SETGE               ,AluOp(2, AluOp::a,"SETGE")},
   {OP2_SETNE               ,AluOp(2, AluOp::a,"SETNE")},
   {OP2_SETE_DX10           ,AluOp(2, AluOp::a,"SETE_DX10")},
   {OP2_SETGT_DX10          ,AluOp(2, AluOp::a,"SETGT_DX10")},
   {OP2_SETGE_DX10          ,AluOp(2, AluOp::a,"SETGE_DX10")},
   {OP2_SETNE_DX10          ,AluOp(2, AluOp::a,"SETNE_DX10")},
   {OP2_FRACT               ,AluOp(1, AluOp::a,"FRACT")},
   {OP2_TRUNC               ,AluOp(1, AluOp::a,"TRUNC")},
   {OP2_CEIL                ,AluOp(1, AluOp::a,"CEIL")},
   {OP2_RNDNE               ,AluOp(1, AluOp::a,"RNDNE")},
   {OP2_FLOOR               ,AluOp(1, AluOp::a,"FLOOR")},
   {OP2_ASHR_INT            ,AluOp(2, AluOp::a,"ASHR_INT")},
   {OP2_LSHR_INT            ,AluOp(2, AluOp::a,"LSHR_INT")},
   {OP2_LSHL_INT            ,AluOp(2, AluOp::a,"LSHL_INT")},
   {OP2_MOV                 ,AluOp(1, AluOp::a,"MOV")},
   {OP2_NOP                 ,AluOp(0, AluOp::a,"NOP")},
   {OP2_MUL_64              ,AluOp(2, AluOp::a,"MUL_64")},
   {OP2_FLT64_TO_FLT32      ,AluOp(1, AluOp::a,"FLT64_TO_FLT32")},
   {OP2V_FLT32_TO_FLT64     ,AluOp(1, AluOp::a,"FLT32_TO_FLT64")},
   {OP2_PRED_SETGT_UINT     ,AluOp(2, AluOp::a,"PRED_SETGT_UINT")},
   {OP2_PRED_SETGE_UINT     ,AluOp(2, AluOp::a,"PRED_SETGE_UINT")},
   {OP2_PRED_SETE           ,AluOp(2, AluOp::a,"PRED_SETE")},
   {OP2_PRED_SETGT          ,AluOp(2, AluOp::a,"PRED_SETGT")},
   {OP2_PRED_SETGE          ,AluOp(2, AluOp::a,"PRED_SETGE")},
   {OP2_PRED_SETNE          ,AluOp(2, AluOp::a,"PRED_SETNE")},
   {OP2_PRED_SET_INV        ,AluOp(1, AluOp::a,"PRED_SET_INV")},
   {OP2_PRED_SET_POP        ,AluOp(2, AluOp::a,"PRED_SET_POP")},
   {OP2_PRED_SET_CLR        ,AluOp(0, AluOp::a,"PRED_SET_CLR")},
   {OP2_PRED_SET_RESTORE    ,AluOp(1, AluOp::a,"PRED_SET_RESTORE")},
   {OP2_PRED_SETE_PUSH      ,AluOp(2, AluOp::a,"PRED_SETE_PUSH")},
   {OP2_PRED_SETGT_PUSH     ,AluOp(2, AluOp::a,"PRED_SETGT_PUSH")},
   {OP2_PRED_SETGE_PUSH     ,AluOp(2, AluOp::a,"PRED_SETGE_PUSH")},
   {OP2_PRED_SETNE_PUSH     ,AluOp(2, AluOp::a,"PRED_SETNE_PUSH")},
   {OP2_KILLE               ,AluOp(2, AluOp::a,"KILLE")},
   {OP2_KILLGT              ,AluOp(2, AluOp::a,"KILLGT")},
   {OP2_KILLGE              ,AluOp(2, AluOp::a,"KILLGE")},
   {OP2_KILLNE              ,AluOp(2, AluOp::a,"KILLNE")},
   {OP2_AND_INT             ,AluOp(2, AluOp::a,"AND_INT")},
   {OP2_OR_INT              ,AluOp(2, AluOp::a,"OR_INT")},
   {OP2_XOR_INT             ,AluOp(2, AluOp::a,"XOR_INT")},
   {OP2_NOT_INT             ,AluOp(1, AluOp::a,"NOT_INT")},
   {OP2_ADD_INT             ,AluOp(2, AluOp::a,"ADD_INT")},
   {OP2_SUB_INT             ,AluOp(2, AluOp::a,"SUB_INT")},
   {OP2_MAX_INT             ,AluOp(2, AluOp::a,"MAX_INT")},
   {OP2_MIN_INT             ,AluOp(2, AluOp::a,"MIN_INT")},
   {OP2_MAX_UINT            ,AluOp(2, AluOp::a,"MAX_UINT")},
   {OP2_MIN_UINT            ,AluOp(2, AluOp::a,"MIN_UINT")},
   {OP2_SETE_INT            ,AluOp(2, AluOp::a,"SETE_INT")},
   {OP2_SETGT_INT           ,AluOp(2, AluOp::a,"SETGT_INT")},
   {OP2_SETGE_INT           ,AluOp(2, AluOp::a,"SETGE_INT")},
   {OP2_SETNE_INT           ,AluOp(2, AluOp::a,"SETNE_INT")},
   {OP2_SETGT_UINT          ,AluOp(2, AluOp::a,"SETGT_UINT")},
   {OP2_SETGE_UINT          ,AluOp(2, AluOp::a,"SETGE_UINT")},
   {OP2_KILLGT_UINT         ,AluOp(2, AluOp::a,"KILLGT_UINT")},
   {OP2_KILLGE_UINT         ,AluOp(2, AluOp::a,"KILLGE_UINT")},
   {OP2_PREDE_INT           ,AluOp(2, AluOp::a,"PREDE_INT")},
   {OP2_PRED_SETGT_INT      ,AluOp(2, AluOp::a,"PRED_SETGT_INT")},
   {OP2_PRED_SETGE_INT      ,AluOp(2, AluOp::a,"PRED_SETGE_INT")},
   {OP2_PRED_SETNE_INT      ,AluOp(2, AluOp::a,"PRED_SETNE_INT")},
   {OP2_KILLE_INT           ,AluOp(2, AluOp::a,"KILLE_INT")},
   {OP2_KILLGT_INT          ,AluOp(2, AluOp::a,"KILLGT_INT")},
   {OP2_KILLGE_INT          ,AluOp(2, AluOp::a,"KILLGE_INT")},
   {OP2_KILLNE_INT          ,AluOp(2, AluOp::a,"KILLNE_INT")},
   {OP2_PRED_SETE_PUSH_INT  ,AluOp(2, AluOp::a,"PRED_SETE_PUSH_INT")},
   {OP2_PRED_SETGT_PUSH_INT ,AluOp(2, AluOp::a,"PRED_SETGT_PUSH_INT")},
   {OP2_PRED_SETGE_PUSH_INT ,AluOp(2, AluOp::a,"PRED_SETGE_PUSH_INT")},
   {OP2_PRED_SETNE_PUSH_INT ,AluOp(2, AluOp::a,"PRED_SETNE_PUSH_INT")},
   {OP2_PRED_SETLT_PUSH_INT ,AluOp(2, AluOp::a,"PRED_SETLT_PUSH_INT")},
   {OP2_PRED_SETLE_PUSH_INT ,AluOp(2, AluOp::a,"PRED_SETLE_PUSH_INT")},
   {OP2_FLT_TO_INT          ,AluOp(1, AluOp::a,"FLT_TO_INT")},
   {OP2_BFREV_INT           ,AluOp(1, AluOp::a,"BFREV_INT")},
   {OP2_ADDC_UINT           ,AluOp(2, AluOp::a,"ADDC_UINT")},
   {OP2_SUBB_UINT           ,AluOp(2, AluOp::a,"SUBB_UINT")},
   {OP2_GROUP_BARRIER       ,AluOp(0, AluOp::a,"GROUP_BARRIER")},
   {OP2_GROUP_SEQ_BEGIN     ,AluOp(0, AluOp::a,"GROUP_SEQ_BEGIN")},
   {OP2_GROUP_SEQ_END       ,AluOp(0, AluOp::a,"GROUP_SEQ_END")},
   {OP2_SET_MODE            ,AluOp(2, AluOp::a,"SET_MODE")},
   {OP2_SET_CF_IDX0         ,AluOp(1, AluOp::a,"SET_CF_IDX0")}, /* Reads from AR register? */
   {OP2_SET_CF_IDX1         ,AluOp(1, AluOp::a,"SET_CF_IDX1")}, /* Reads from AR register? */
   {OP2_SET_LDS_SIZE        ,AluOp(2, AluOp::a,"SET_LDS_SIZE")},
   {OP2_EXP_IEEE            ,AluOp(1, AluOp::t,"EXP_IEEE")},
   {OP2_LOG_CLAMPED         ,AluOp(1, AluOp::t,"LOG_CLAMPED")},
   {OP2_LOG_IEEE            ,AluOp(1, AluOp::t,"LOG_IEEE")},
   {OP2_RECIP_CLAMPED       ,AluOp(1, AluOp::t,"RECIP_CLAMPED")},
   {OP2_RECIP_FF            ,AluOp(1, AluOp::t,"RECIP_FF")},
   {OP2_RECIP_IEEE          ,AluOp(1, AluOp::t,"RECIP_IEEE")},
   {OP2_RECIPSQRT_CLAMPED   ,AluOp(1, AluOp::t,"RECIPSQRT_CLAMPED")},
   {OP2_RECIPSQRT_FF        ,AluOp(1, AluOp::t,"RECIPSQRT_FF")},
   {OP2_RECIPSQRT_IEEE      ,AluOp(1, AluOp::t,"RECIPSQRT_IEEE")},
   {OP2_SQRT_IEEE           ,AluOp(1, AluOp::t,"SQRT_IEEE")},
   {OP2_SIN                 ,AluOp(1, AluOp::t,"SIN")},
   {OP2_COS                 ,AluOp(1, AluOp::t,"COS")},
   {OP2_MULLO_INT           ,AluOp(2, AluOp::t,"MULLO_INT")},
   {OP2_MULHI_INT           ,AluOp(2, AluOp::t,"MULHI_INT")},
   {OP2_MULLO_UINT          ,AluOp(2, AluOp::t,"MULLO_UINT")},
   {OP2_MULHI_UINT          ,AluOp(2, AluOp::t,"MULHI_UINT")},
   {OP2_RECIP_INT           ,AluOp(1, AluOp::t,"RECIP_INT")},
   {OP2_RECIP_UINT          ,AluOp(1, AluOp::t,"RECIP_UINT")},
   {OP2_RECIP_64            ,AluOp(1, AluOp::t,"RECIP_64")},
   {OP2_RECIP_CLAMPED_64    ,AluOp(1, AluOp::t,"RECIP_CLAMPED_64")},
   {OP2_RECIPSQRT_64        ,AluOp(1, AluOp::t,"RECIPSQRT_64")},
   {OP2_RECIPSQRT_CLAMPED_64,AluOp(1, AluOp::t,"RECIPSQRT_CLAMPED_64")},
   {OP2_SQRT_64             ,AluOp(1, AluOp::t,"SQRT_64")},
   {OP2_FLT_TO_UINT         ,AluOp(1, AluOp::t,"FLT_TO_UINT")},
   {OP2_INT_TO_FLT          ,AluOp(1, AluOp::t,"INT_TO_FLT")},
   {OP2_UINT_TO_FLT         ,AluOp(1, AluOp::t,"UINT_TO_FLT")},
   {OP2_BFM_INT             ,AluOp(2, AluOp::v,"BFM_INT")},
   {OP2_FLT32_TO_FLT16      ,AluOp(1, AluOp::v,"FLT32_TO_FLT16")},
   {OP2_FLT16_TO_FLT32      ,AluOp(1, AluOp::v,"FLT16_TO_FLT32")},
   {OP2_UBYTE0_FLT          ,AluOp(1, AluOp::v,"UBYTE0_FLT")},
   {OP2_UBYTE1_FLT          ,AluOp(1, AluOp::v,"UBYTE1_FLT")},
   {OP2_UBYTE2_FLT          ,AluOp(1, AluOp::v,"UBYTE2_FLT")},
   {OP2_UBYTE3_FLT          ,AluOp(1, AluOp::v,"UBYTE3_FLT")},
   {OP2_BCNT_INT            ,AluOp(1, AluOp::v,"BCNT_INT")},
   {OP2_FFBH_UINT           ,AluOp(1, AluOp::v,"FFBH_UINT")},
   {OP2_FFBL_INT            ,AluOp(1, AluOp::v,"FFBL_INT")},
   {OP2_FFBH_INT            ,AluOp(1, AluOp::v,"FFBH_INT")},
   {OP2_FLT_TO_UINT4        ,AluOp(1, AluOp::v,"FLT_TO_UINT4")},
   {OP2_DOT_IEEE            ,AluOp(2, AluOp::v,"DOT_IEEE")},
   {OP2_FLT_TO_INT_RPI      ,AluOp(1, AluOp::v,"FLT_TO_INT_RPI")},
   {OP2_FLT_TO_INT_FLOOR    ,AluOp(1, AluOp::v,"FLT_TO_INT_FLOOR")},
   {OP2_MULHI_UINT24        ,AluOp(2, AluOp::v,"MULHI_UINT24")},
   {OP2_MBCNT_32HI_INT      ,AluOp(1, AluOp::v,"MBCNT_32HI_INT")},
   {OP2_OFFSET_TO_FLT       ,AluOp(1, AluOp::v,"OFFSET_TO_FLT")},
   {OP2_MUL_UINT24          ,AluOp(2, AluOp::v,"MUL_UINT24")},
   {OP2_BCNT_ACCUM_PREV_INT ,AluOp(1, AluOp::v,"BCNT_ACCUM_PREV_INT")},
   {OP2_MBCNT_32LO_ACCUM_PREV_INT ,AluOp(1, AluOp::v,"MBCNT_32LO_ACCUM_PREV_INT")},
   {OP2_SETE_64             ,AluOp(2, AluOp::v,"SETE_64")},
   {OP2_SETNE_64            ,AluOp(2, AluOp::v,"SETNE_64")},
   {OP2_SETGT_64            ,AluOp(2, AluOp::v,"SETGT_64")},
   {OP2_SETGE_64            ,AluOp(2, AluOp::v,"SETGE_64")},
   {OP2_MIN_64              ,AluOp(2, AluOp::v,"MIN_64")},
   {OP2_MAX_64              ,AluOp(2, AluOp::v,"MAX_64")},
   {OP2_DOT4                ,AluOp(2, AluOp::v,"DOT4")},
   {OP2_DOT4_IEEE           ,AluOp(2, AluOp::v,"DOT4_IEEE")},
   {OP2_CUBE                ,AluOp(2, AluOp::v,"CUBE")},
   {OP2_MAX4                ,AluOp(1, AluOp::v,"MAX4")},
   {OP2_FREXP_64            ,AluOp(1, AluOp::v,"FREXP_64")},
   {OP2_LDEXP_64            ,AluOp(1, AluOp::v,"LDEXP_64")},
   {OP2_FRACT_64            ,AluOp(1, AluOp::v,"FRACT_64")},
   {OP2_PRED_SETGT_64       ,AluOp(2, AluOp::v,"PRED_SETGT_64")},
   {OP2_PRED_SETE_64        ,AluOp(2, AluOp::v,"PRED_SETE_64")},
   {OP2_PRED_SETGE_64       ,AluOp(2, AluOp::v,"PRED_SETGE_64")},
   {OP2V_MUL_64             ,AluOp(2, AluOp::v,"MUL_64")},
   {OP2_ADD_64              ,AluOp(2, AluOp::v,"ADD_64")},
   {OP2_MOVA_INT            ,AluOp(1, AluOp::v,"MOVA_INT")},
   {OP2V_FLT64_TO_FLT32     ,AluOp(1, AluOp::v,"FLT64_TO_FLT32")},
   {OP2_FLT32_TO_FLT64      ,AluOp(1, AluOp::v,"FLT32_TO_FLT64")},
   {OP2_SAD_ACCUM_PREV_UINT ,AluOp(2, AluOp::v,"SAD_ACCUM_PREV_UINT")},
   {OP2_DOT                 ,AluOp(2, AluOp::v,"DOT")},
   {OP2_MUL_PREV            ,AluOp(2, AluOp::v,"MUL_PREV")},
   {OP2_MUL_IEEE_PREV       ,AluOp(2, AluOp::v,"MUL_IEEE_PREV")},
   {OP2_ADD_PREV            ,AluOp(2, AluOp::v,"ADD_PREV")},
   {OP2_MULADD_PREV         ,AluOp(2, AluOp::v,"MULADD_PREV")},
   {OP2_MULADD_IEEE_PREV    ,AluOp(2, AluOp::v,"MULADD_IEEE_PREV")},
   {OP2_INTERP_XY           ,AluOp(1, AluOp::v,"INTERP_XY")},
   {OP2_INTERP_ZW           ,AluOp(1, AluOp::v,"INTERP_ZW")},
   {OP2_INTERP_X            ,AluOp(1, AluOp::v,"INTERP_X")},
   {OP2_INTERP_Z            ,AluOp(1, AluOp::v,"INTERP_Z")},
   {OP2_STORE_FLAGS         ,AluOp(0, AluOp::v,"STORE_FLAGS")},
   {OP2_LOAD_STORE_FLAGS    ,AluOp(1, AluOp::v,"LOAD_STORE_FLAGS")},
   {OP2_LDS_1A              ,AluOp(0, AluOp::v,"LDS_1A")},
   {OP2_LDS_1A1D            ,AluOp(0, AluOp::v,"LDS_1A1D")},
   {OP2_LDS_2A              ,AluOp(0, AluOp::v,"LDS_2A")},
   {OP2_INTERP_LOAD_P0      ,AluOp(1, AluOp::v,"INTERP_LOAD_P0")},
   {OP2_INTERP_LOAD_P10     ,AluOp(1, AluOp::v,"INTERP_LOAD_P10")},
   {OP2_INTERP_LOAD_P20     ,AluOp(1, AluOp::v,"INTERP_LOAD_P20")},
   {OP3_INST_BFE_UINT       ,AluOp(3, AluOp::v,"INST_BFE_UINT")},
   {OP3_INST_BFE_INT        ,AluOp(3, AluOp::v,"INST_BFE_INT")},
   {OP3_INST_BFI_INT        ,AluOp(3, AluOp::v,"INST_BFI_INT")},
   {OP3_INST_FMA            ,AluOp(3, AluOp::v,"INST_FMA")},
   {OP3_INST_CNDNE_64       ,AluOp(3, AluOp::v,"INST_CNDNE_64")},
   {OP3_INST_FMA_64         ,AluOp(3, AluOp::v,"INST_FMA_64")},
   {OP3_INST_LERP_UINT      ,AluOp(3, AluOp::v,"INST_LERP_UINT")},
   {OP3_INST_BIT_ALIGN_INT  ,AluOp(3, AluOp::v,"INST_BIT_ALIGN_INT")},
   {OP3_INST_BYTE_ALIGN_INT ,AluOp(3, AluOp::v,"INST_BYTE_ALIGN_INT")},
   {OP3_INST_SAD_ACCUM_UINT ,AluOp(3, AluOp::v,"INST_SAD_ACCUM_UINT")},
   {OP3_INST_SAD_ACCUM_HI_UINT ,AluOp(3, AluOp::v,"INST_SAD_ACCUM_HI_UINT")},
   {OP3_INST_MULADD_UINT24  ,AluOp(3, AluOp::v,"INST_MULADD_UINT24")},
   {OP3_INST_LDS_IDX_OP     ,AluOp(3, AluOp::x,"INST_LDS_IDX_OP")},
   {OP3_INST_MULADD         ,AluOp(3, AluOp::a,"INST_MULADD")},
   {OP3_INST_MULADD_M2      ,AluOp(3, AluOp::a,"INST_MULADD_M2")},
   {OP3_INST_MULADD_M4      ,AluOp(3, AluOp::a,"INST_MULADD_M4")},
   {OP3_INST_MULADD_D2      ,AluOp(3, AluOp::a,"INST_MULADD_D2")},
   {OP3_INST_MULADD_IEEE    ,AluOp(3, AluOp::a,"INST_MULADD_IEEE")},
   {OP3_INST_CNDE           ,AluOp(3, AluOp::a,"INST_CNDE")},
   {OP3_INST_CNDGT          ,AluOp(3, AluOp::a,"INST_CNDGT")},
   {OP3_INST_CNDGE          ,AluOp(3, AluOp::a,"INST_CNDGE")},
   {OP3_INST_CNDE_INT       ,AluOp(3, AluOp::a,"INST_CNDE_INT")},
   {OP3_INST_CNDGT_INT      ,AluOp(3, AluOp::a,"INST_CNDGT_INT")},
   {OP3_INST_CNDGE_INT      ,AluOp(3, AluOp::a,"INST_CNDGE_INT")},
   {OP3_INST_MUL_LIT        ,AluOp(3, AluOp::t,"INST_MUL_LIT")}
};

}
