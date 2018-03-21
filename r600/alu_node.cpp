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
#include <cassert>

namespace r600 {
using std::runtime_error;

const uint64_t last_instr_bit = 1ul << 31;
const uint64_t up_exec_mask_bit = 1ul << 34;
const uint64_t up_pred_bit = 1ul << 35;
const uint64_t write_mask_bit = 1ul << 36;
const uint64_t clamp_bit = 1ul << 63;

AluNode *AluNode::decode(uint64_t bc, Value::LiteralFlags &literal_index)
{
   AluOpFlags flags;
   std::shared_ptr<Value> src2;
   // decode word 0:
   uint16_t src0_sel = bc & 0x1ff;
   uint16_t src1_sel = (bc >> 13) & 0x1ff;
   bool src0_rel = bc & src0_rel_bit;
   bool src1_rel = bc & src1_rel_bit;
   bool src0_neg = bc & src0_neg_bit;
   bool src1_neg = bc & src1_neg_bit;

   uint16_t src0_chan = (bc >> 10) & 3;
   uint16_t src1_chan = (bc >> 23) & 3;

   auto index_mode = static_cast<EIndexMode>((bc >> 26) & 7);

   EPredSelect pred_sel = static_cast<EPredSelect>((bc >> 29) & 3);
   if (bc & last_instr_bit)
      flags.set(is_last_instr);

   bool src0_abs = 0;
   bool src1_abs = 0;

   auto bank_swizzle = static_cast<EBankSwizzle>((bc >> 50) & 7);
   uint16_t dst_sel = (bc >> 53) & 0x7f;
   bool dst_rel = bc & dst_rel_bit;

   if (bc & clamp_bit)
      flags.set(1 << do_clamp);

   uint16_t dst_chan = (bc >> 61) & 3;
   uint16_t opcode = 0;
   EOutputModify omod = omod_off;

   bool is_op2 = (bc & (7ul << 47)) == 0;
   if (is_op2) {
      /* op2 */
      opcode = (bc >> 39) & 0x7ff;
      omod = static_cast<EOutputModify>((bc >> 37) & 3);

      src0_abs = bc & src0_abs_bit;
      src1_abs = bc & src1_abs_bit;

      if (bc & write_mask_bit)
         flags.set(do_write);

      if (bc & up_exec_mask_bit)
         flags.set(do_update_exec_mask);

      if (bc & up_pred_bit)
         flags.set(do_update_pred);


   } else {
      uint16_t src2_sel = (bc >> 32) & 0x1ff;
      uint16_t src2_chan = (bc >> 42) & 3;
      bool src2_rel = bc & src2_rel_bit;
      bool src2_neg = bc & src2_neg_bit;
      src2 = Value::create(src2_sel, src2_chan, 0,
                           src2_rel, src2_neg, literal_index);

      opcode = (bc >> 45) & 0x1f;
   }

   PValue src0 = Value::create(src0_sel, src0_chan, src0_abs,
                               src0_rel, src0_neg, literal_index);

   PValue src1 = Value::create(src1_sel, src1_chan, src1_abs,
                               src1_rel, src1_neg, literal_index);

   GPRValue dst(dst_sel, dst_chan, 0, dst_rel, 0);

   if (is_op2) {
      return new AluNodeOp2(opcode, dst, src0, src1, flags,
                            index_mode, bank_swizzle, omod, pred_sel);
   } else {
      return new AluNodeOp3(opcode << 6, dst, src0, src1, src2, flags,
                            index_mode, bank_swizzle, pred_sel);
   }
}

AluNode::AluNode(uint16_t opcode, const GPRValue& dst,
                 PValue src0, PValue src1,
                 EIndexMode index_mode, EBankSwizzle bank_swizzle,
                 EPredSelect pred_select, AluOpFlags flags):
   m_opcode(opcode),
   m_src0(src0),
   m_src1(src1),
   m_dst(dst),
   m_index_mode(index_mode),
   m_bank_swizzle(bank_swizzle),
   m_pred_select(pred_select),
   m_flags(flags)
{
}

int AluNode::get_dst_chan() const
{
   return m_dst.get_chan();
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
   bc |= m_dst.encode_for(alu_op_dst);

   if (m_flags.test(do_clamp))
      bc |= clamp_bit;

   if (m_flags.test(is_last_instr))
      bc |= last_instr_bit;

   bc |= static_cast<uint64_t>(m_bank_swizzle) << 50;
   bc |= static_cast<uint64_t>(m_index_mode) << 26;
   bc |= static_cast<uint64_t>(m_pred_select) << 29;

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
   assert(0 && "not yet implemented");
}

AluNodeOp2::AluNodeOp2(uint16_t opcode, const GPRValue& dst,
                       PValue src0, PValue src1, AluOpFlags flags,
                       EIndexMode index_mode, EBankSwizzle bank_swizzle,
                       EOutputModify output_modify, EPredSelect pred_select):
   AluNode(opcode, dst, src0, src1, index_mode, bank_swizzle, pred_select, flags),
   m_output_modify(output_modify)
{
}

void AluNodeOp2::encode(uint64_t& bc) const
{
   bc |= src0().encode_for(alu_op2_src0);
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
   AluNode(opcode, dst, src0, src1, index_mode, bank_swizzle,
           pred_select, flags),
   m_src2(src2)
{

}

void AluNodeOp3::encode(uint64_t& bc) const
{
   bc |= src0().encode_for(alu_op3_src0);
   bc |= src1().encode_for(alu_op3_src1);
   bc |= m_src2->encode_for(alu_op3_src2);
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
      node.reset(AluNode::decode(*bc, lflags));
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

const AluOP alu_ops[] = {
   {OP2_ADD,                 2, AluOP::a},
   {OP2_MUL,                 2, AluOP::a},
   {OP2_MUL_IEEE,            2, AluOP::a},
   {OP2_MAX,                 2, AluOP::a},
   {OP2_MIN,                 2, AluOP::a},
   {OP2_MAX_DX10,            2, AluOP::a},
   {OP2_MIN_DX10,            2, AluOP::a},
   {OP2_SETE,                2, AluOP::a},
   {OP2_SETGT,               2, AluOP::a},
   {OP2_SETGE,               2, AluOP::a},
   {OP2_SETNE,               2, AluOP::a},
   {OP2_SETE_DX10,           2, AluOP::a},
   {OP2_SETGT_DX10,          2, AluOP::a},
   {OP2_SETGE_DX10,          2, AluOP::a},
   {OP2_SETNE_DX10,          2, AluOP::a},
   {OP2_FRACT,               1, AluOP::a},
   {OP2_TRUNC,               1, AluOP::a},
   {OP2_CEIL,                1, AluOP::a},
   {OP2_RNDNE,               1, AluOP::a},
   {OP2_FLOOR,               1, AluOP::a},
   {OP2_ASHR_INT,            2, AluOP::a},
   {OP2_LSHR_INT,            2, AluOP::a},
   {OP2_LSHL_INT,            2, AluOP::a},
   {OP2_MOV,                 1, AluOP::a},
   {OP2_NOP,                 0, AluOP::a},
   {OP2_MUL_64,              2, AluOP::a},
   {OP2_FLT64_TO_FLT32,      1, AluOP::a},
   {OP2V_FLT32_TO_FLT64,     1, AluOP::a},
   {OP2_PRED_SETGT_UINT,     2, AluOP::a},
   {OP2_PRED_SETGE_UINT,     2, AluOP::a},
   {OP2_PRED_SETE,           2, AluOP::a},
   {OP2_PRED_SETGT,          2, AluOP::a},
   {OP2_PRED_SETGE,          2, AluOP::a},
   {OP2_PRED_SETNE,          2, AluOP::a},
   {OP2_PRED_SET_INV,        1, AluOP::a},
   {OP2_PRED_SET_POP,        2, AluOP::a},
   {OP2_PRED_SET_CLR,        0, AluOP::a},
   {OP2_PRED_SET_RESTORE,    1, AluOP::a},
   {OP2_PRED_SETE_PUSH,      2, AluOP::a},
   {OP2_PRED_SETGT_PUSH,     2, AluOP::a},
   {OP2_PRED_SETGE_PUSH,     2, AluOP::a},
   {OP2_PRED_SETNE_PUSH,     2, AluOP::a},
   {OP2_KILLE,               2, AluOP::a},
   {OP2_KILLGT,              2, AluOP::a},
   {OP2_KILLGE,              2, AluOP::a},
   {OP2_KILLNE,              2, AluOP::a},
   {OP2_AND_INT,             2, AluOP::a},
   {OP2_OR_INT,              2, AluOP::a},
   {OP2_XOR_INT,             2, AluOP::a},
   {OP2_NOT_INT,             1, AluOP::a},
   {OP2_ADD_INT,             2, AluOP::a},
   {OP2_SUB_INT,             2, AluOP::a},
   {OP2_MAX_INT,             2, AluOP::a},
   {OP2_MIN_INT,             2, AluOP::a},
   {OP2_MAX_UINT,            2, AluOP::a},
   {OP2_MIN_UINT,            2, AluOP::a},
   {OP2_SETE_INT,            2, AluOP::a},
   {OP2_SETGT_INT,           2, AluOP::a},
   {OP2_SETGE_INT,           2, AluOP::a},
   {OP2_SETNE_INT,           2, AluOP::a},
   {OP2_SETGT_UINT,          2, AluOP::a},
   {OP2_SETGE_UINT,          2, AluOP::a},
   {OP2_KILLGT_UINT,         2, AluOP::a},
   {OP2_KILLGE_UINT,         2, AluOP::a},
   {OP2_PREDE_INT,           2, AluOP::a},
   {OP2_PRED_SETGT_INT,      2, AluOP::a},
   {OP2_PRED_SETGE_INT,      2, AluOP::a},
   {OP2_PRED_SETNE_INT,      2, AluOP::a},
   {OP2_KILLE_INT,           2, AluOP::a},
   {OP2_KILLGT_INT,          2, AluOP::a},
   {OP2_KILLGE_INT,          2, AluOP::a},
   {OP2_KILLNE_INT,          2, AluOP::a},
   {OP2_PRED_SETE_PUSH_INT,  2, AluOP::a},
   {OP2_PRED_SETGT_PUSH_INT, 2, AluOP::a},
   {OP2_PRED_SETGE_PUSH_INT, 2, AluOP::a},
   {OP2_PRED_SETNE_PUSH_INT, 2, AluOP::a},
   {OP2_PRED_SETLT_PUSH_INT, 2, AluOP::a},
   {OP2_PRED_SETLE_PUSH_INT, 2, AluOP::a},
   {OP2_FLT_TO_INT,          1, AluOP::a},
   {OP2_BFREV_INT,           1, AluOP::a},
   {OP2_ADDC_UINT,           2, AluOP::a},
   {OP2_SUBB_UINT,           2, AluOP::a},
   {OP2_GROUP_BARRIER,       0, AluOP::a},
   {OP2_GROUP_SEQ_BEGIN,     0, AluOP::a},
   {OP2_GROUP_SEQ_END,       0, AluOP::a},
   {OP2_SET_MODE,            2, AluOP::a},
   {OP2_SET_CF_IDX0,         1, AluOP::a}, /* Reads from AR register? */
   {OP2_SET_CF_IDX1,         1, AluOP::a}, /* Reads from AR register? */
   {OP2_SET_LDS_SIZE,        2, AluOP::a},
   {OP2_EXP_IEEE,            1, AluOP::t},
   {OP2_LOG_CLAMPED,         1, AluOP::t},
   {OP2_LOG_IEEE,            1, AluOP::t},
   {OP2_RECIP_CLAMPED,       1, AluOP::t},
   {OP2_RECIP_FF,            1, AluOP::t},
   {OP2_RECIP_IEEE,          1, AluOP::t},
   {OP2_RECIPSQRT_CLAMPED,   1, AluOP::t},
   {OP2_RECIPSQRT_FF,        1, AluOP::t},
   {OP2_RECIPSQRT_IEEE,      1, AluOP::t},
   {OP2_SQRT_IEEE,           1, AluOP::t},
   {OP2_SIN,                 1, AluOP::t},
   {OP2_COS,                 1, AluOP::t},
   {OP2_MULLO_INT,           2, AluOP::t},
   {OP2_MULHI_INT,           2, AluOP::t},
   {OP2_MULLO_UINT,          2, AluOP::t},
   {OP2_MULHI_UINT,          2, AluOP::t},
   {OP2_RECIP_INT,           1, AluOP::t},
   {OP2_RECIP_UINT,          1, AluOP::t},
   {OP2_RECIP_64,            1, AluOP::t},
   {OP2_RECIP_CLAMPED_64,    1, AluOP::t},
   {OP2_RECIPSQRT_64,        1, AluOP::t},
   {OP2_RECIPSQRT_CLAMPED_64,1, AluOP::t},
   {OP2_SQRT_64,             1, AluOP::t},
   {OP2_FLT_TO_UINT,         1, AluOP::t},
   {OP2_INT_TO_FLT,          1, AluOP::t},
   {OP2_UINT_TO_FLT,         1, AluOP::t},
   {OP2_BFM_INT,             2, AluOP::v},
   {OP2_FLT32_TO_FLT16,      1, AluOP::v},
   {OP2_FLT16_TO_FLT32,      1, AluOP::v},
   {OP2_UBYTE0_FLT,          1, AluOP::v},
   {OP2_UBYTE1_FLT,          1, AluOP::v},
   {OP2_UBYTE2_FLT,          1, AluOP::v},
   {OP2_UBYTE3_FLT,          1, AluOP::v},
   {OP2_BCNT_INT,            1, AluOP::v},
   {OP2_FFBH_UINT,           1, AluOP::v},
   {OP2_FFBL_INT,            1, AluOP::v},
   {OP2_FFBH_INT,            1, AluOP::v},
   {OP2_FLT_TO_UINT4,        1, AluOP::v},
   {OP2_DOT_IEEE,            2, AluOP::v},
   {OP2_FLT_TO_INT_RPI,      1, AluOP::v},
   {OP2_FLT_TO_INT_FLOOR,    1, AluOP::v},
   {OP2_MULHI_UINT24,        2, AluOP::v},
   {OP2_MBCNT_32HI_INT,      1, AluOP::v},
   {OP2_OFFSET_TO_FLT,       1, AluOP::v},
   {OP2_MUL_UINT24,          2, AluOP::v},
   {OP2_BCNT_ACCUM_PREV_INT, 1, AluOP::v},
   {OP2_MBCNT_32LO_ACCUM_PREV_INT, 1, AluOP::v},
   {OP2_SETE_64,             2, AluOP::v},
   {OP2_SETNE_64,            2, AluOP::v},
   {OP2_SETGT_64,            2, AluOP::v},
   {OP2_SETGE_64,            2, AluOP::v},
   {OP2_MIN_64,              2, AluOP::v},
   {OP2_MAX_64,              2, AluOP::v},
   {OP2_DOT4,                2, AluOP::v},
   {OP2_DOT4_IEEE,           2, AluOP::v},
   {OP2_CUBE,                2, AluOP::v},
   {OP2_MAX4,                1, AluOP::v},
   {OP2_FREXP_64,            1, AluOP::v},
   {OP2_LDEXP_64,            1, AluOP::v},
   {OP2_FRACT_64,            1, AluOP::v},
   {OP2_PRED_SETGT_64,       2, AluOP::v},
   {OP2_PRED_SETE_64,        2, AluOP::v},
   {OP2_PRED_SETGE_64,       2, AluOP::v},
   {OP2V_MUL_64,             2, AluOP::v},
   {OP2_ADD_64,              2, AluOP::v},
   {OP2_MOVA_INT,            1, AluOP::v},
   {OP2V_FLT64_TO_FLT32,     1, AluOP::v},
   {OP2_FLT32_TO_FLT64,      1, AluOP::v},
   {OP2_SAD_ACCUM_PREV_UINT, 2, AluOP::v},
   {OP2_DOT,                 2, AluOP::v},
   {OP2_MUL_PREV,            2, AluOP::v},
   {OP2_MUL_IEEE_PREV,       2, AluOP::v},
   {OP2_ADD_PREV,            2, AluOP::v},
   {OP2_MULADD_PREV,         2, AluOP::v},
   {OP2_MULADD_IEEE_PREV,    2, AluOP::v},
   {OP2_INTERP_XY,           1, AluOP::v},
   {OP2_INTERP_ZW,           1, AluOP::v},
   {OP2_INTERP_X,            1, AluOP::v},
   {OP2_INTERP_Z,            1, AluOP::v},
   {OP2_STORE_FLAGS,         0, AluOP::v},
   {OP2_LOAD_STORE_FLAGS,    1, AluOP::v},
   {OP2_LDS_1A,              0, AluOP::v},
   {OP2_LDS_1A1D,            0, AluOP::v},
   {OP2_LDS_2A,              0, AluOP::v},
   {OP2_INTERP_LOAD_P0,      1, AluOP::v},
   {OP2_INTERP_LOAD_P10,     1, AluOP::v},
   {OP2_INTERP_LOAD_P20,     1, AluOP::v},
   {OP3_INST_BFE_UINT,       3, AluOP::v},
   {OP3_INST_BFE_INT,        3, AluOP::v},
   {OP3_INST_BFI_INT,        3, AluOP::v},
   {OP3_INST_FMA,            3, AluOP::v},
   {OP3_INST_CNDNE_64,       3, AluOP::v},
   {OP3_INST_FMA_64,         3, AluOP::v},
   {OP3_INST_LERP_UINT,      3, AluOP::v},
   {OP3_INST_BIT_ALIGN_INT,  3, AluOP::v},
   {OP3_INST_BYTE_ALIGN_INT, 3, AluOP::v},
   {OP3_INST_SAD_ACCUM_UINT, 3, AluOP::v},
   {OP3_INST_SAD_ACCUM_HI_UINT, 3, AluOP::v},
   {OP3_INST_MULADD_UINT24,  3, AluOP::v},
   {OP3_INST_LDS_IDX_OP,     3, AluOP::v},
   {OP3_INST_MULADD,         3, AluOP::a},
   {OP3_INST_MULADD_M2,      3, AluOP::a},
   {OP3_INST_MULADD_M4,      3, AluOP::a},
   {OP3_INST_MULADD_D2,      3, AluOP::a},
   {OP3_INST_MULADD_IEEE,    3, AluOP::a},
   {OP3_INST_CNDE,           3, AluOP::a},
   {OP3_INST_CNDGT,          3, AluOP::a},
   {OP3_INST_CNDGE,          3, AluOP::a},
   {OP3_INST_CNDE_INT,       3, AluOP::a},
   {OP3_INST_CNDGT_INT,      3, AluOP::a},
   {OP3_INST_CNDGE_INT,      3, AluOP::a},
   {OP3_INST_MUL_LIT,        3, AluOP::t},
};

}