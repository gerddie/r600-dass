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


namespace r600 {
using std::unique_ptr;

const uint64_t src0_rel_bit = 1ul << 9;
const uint64_t src1_rel_bit = 1ul << 22;
const uint64_t src2_rel_bit = 1ul << 41;
const uint64_t src0_neg_bit = 1ul << 12;
const uint64_t src1_neg_bit = 1ul << 25;
const uint64_t src2_neg_bit = 1ul << 44;
const uint64_t last_instr_bit = 1ul << 31;
const uint64_t src0_abs_bit = 1ul << 32;
const uint64_t src1_abs_bit = 1ul << 33;
const uint64_t up_exec_mask_bit = 1ul << 34;
const uint64_t up_pred_bit = 1ul << 35;
const uint64_t write_mask_bit = 1ul << 36;
const uint64_t dst_rel_bit = 1ul << 60;
const uint64_t clamp_bit = 1ul << 63;

AluNode *AluNode::decode(uint64_t bc, int literal_index)
{
   AluOpFlags flags;
   unique_ptr<Value> src2;
   // decode word 0:
   uint16_t src0_sel = bc & 0x1ff;
   uint16_t src1_sel = (bc >> 13) & 0x1ff;
   bool src0_rel = bc & src0_rel_bit;
   bool src1_rel = bc & src1_rel_bit;
   bool src0_neg = bc & src0_neg_bit;
   bool src1_neg = bc & src1_neg_bit;

   uint16_t src0_chan = (bc >> 10) & 3;
   uint16_t src1_chan = (bc >> 23) & 3;

   uint16_t index_mode = (bc >> 26) & 3;

   uint16_t pred_sel = (bc >> 29) & 3;
   if (bc & last_instr_bit)
      flags.set(is_last_instr);

   bool src0_abs = 0;
   bool src1_abs = 0;

   uint16_t bank_swizzle = (bc >> 18) & 3;
   uint16_t dst_sel = (bc >> 21) & 0x3f;
   bool dst_rel = bc & dst_rel_bit;
   bool clamp = bc & clamp_bit;
   uint16_t dst_chan = (bc >> 61) & 3;
   uint16_t opcode = 0;
   uint16_t omod = 0;

   bool is_op2 = (bc & (7ul << 47)) == 0;
   if (is_op2) {
      /* op2 */
      opcode = (bc >> 39) & 0x7ff;
      omod = (bc >> 37) & 3;

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
      src2.reset(Value::create(src2_sel, src2_chan, 0,
                               src2_rel, src2_neg, literal_index));

      opcode = (bc >> 45) & 0x1f;
   }

   unique_ptr<Value> src0(Value::create(src0_sel, src0_chan, src0_abs,
                                        src0_rel, src0_neg, literal_index));

   unique_ptr<Value> src1(Value::create(src1_sel, src1_chan, src1_abs,
                                        src1_rel, src1_neg, literal_index));

   unique_ptr<Value> dst(new GPRValue(dst_sel, dst_chan, 0, dst_rel, 0));

   if (is_op2) {
      return new AluNodeOp2(opcode, std::move(src0), std::move(src1), std::move(dst), index_mode, bank_swizzle,
                            omod, flags);
   } else {
      return new AluNodeOp3(opcode, src0, src1, src2, dst, index_mode,
                            bank_swizzle, omod, flags);
   }
}


}