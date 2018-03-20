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

#include <r600/bc_test.h>
#include <r600/alu_node.h>
#include <gtest/gtest.h>
#include <vector>

using namespace r600;
using std::vector;

class BytecodeAluOp2ATest: public BytecodeTest {
   void SetUp();
protected:
   Value::LiteralFlags li;
};


void BytecodeAluOp2ATest::SetUp()
{

   Value::LiteralFlags literal_index;
   set_spacing({63, 61, 60, 53, 50, 45, 44, 42, 41, 32,
                31, 29, 26, 25, 23, 22, 13, 12, 10, 9});

}

TEST_F(BytecodeAluOp2ATest, BitCreateDecodeBytecodeRountrip)
{
   Value::LiteralFlags literal_flags = 0;
   for (int i = 0; i < 63; ++i) {
      uint64_t bc = 1ul << i;
      auto alu_node = AluNode::decode(bc, literal_flags);
      TEST_EQ(alu_node->get_bytecode(), bc);
   }
}

TEST_F(BytecodeAluOp2ATest, TestValueType)
{
   auto v = Value::create(0, 0, 0, 0, 0, li);
   EXPECT_EQ(v->get_type(), Value::gpr);

   v = Value::create(127, 0, 0, 0, 0, li);
   EXPECT_EQ(v->get_type(), Value::gpr);

   v = Value::create(128, 0, 0, 0, 0, li);
   EXPECT_EQ(v->get_type(), Value::kconst);

   v = Value::create(160, 0, 0, 0, 0, li);
   EXPECT_EQ(v->get_type(), Value::kconst);

   v = Value::create(191, 0, 0, 0, 0, li);
   EXPECT_EQ(v->get_type(), Value::kconst);

   v = Value::create(256, 0, 0, 0, 0, li);
   EXPECT_EQ(v->get_type(), Value::kconst);

   v = Value::create(319, 0, 0, 0, 0, li);
   EXPECT_EQ(v->get_type(), Value::kconst);

   v = Value::create(219, 0, 0, 0, 0, li);
   EXPECT_EQ(v->get_type(), Value::cinline);

   v = Value::create(255, 0, 0, 0, 0, li);
   EXPECT_EQ(v->get_type(), Value::cinline);

   v = Value::create(253, 0, 0, 0, 0, li);
   EXPECT_EQ(v->get_type(), Value::literal);
}

TEST_F(BytecodeAluOp2ATest, TestValueSrc0Sel)
{
   for (int s = 0; s < 9; ++s) {
      auto v =Value::create(1 << s, 0, 0, 0, 0, li);
      TEST_EQ(v->encode_for(alu_op2_src0), 1ul << s);
      TEST_EQ(v->encode_for(alu_op2_src1), 1ul << (s+13));
      TEST_EQ(v->encode_for(alu_op3_src2), 1ul << (s+32));
      if (s < 7)
         TEST_EQ(v->encode_for(alu_op_dst), 1ul << (s+53));
   }
}

TEST_F(BytecodeAluOp2ATest, TestValueSrc0Chan)
{

   for (int c = 0; c < 2; ++c) {
      auto v =Value::create(0, 1 << c, 0, 0, 0, li);

      TEST_EQ(v->encode_for(alu_op2_src0), 1ul << (c+10));
      TEST_EQ(v->encode_for(alu_op2_src1), 1ul << (c+23));
      TEST_EQ(v->encode_for(alu_op3_src2), 1ul << (c+42));
      TEST_EQ(v->encode_for(alu_op_dst), 1ul << (c+61));
   }
}

TEST_F(BytecodeAluOp2ATest, TestValueSrc0Abs)
{
    auto vabs =Value::create(0, 0, 1, 0, 0, li);
    TEST_EQ(vabs->encode_for(alu_op2_src0), src0_abs_bit);
    TEST_EQ(vabs->encode_for(alu_op2_src1), src1_abs_bit);
}

TEST_F(BytecodeAluOp2ATest, TestValueSrc0Neg)
{
   auto vneg =Value::create(0, 0, 0, 0, 1, li);
   TEST_EQ(vneg->encode_for(alu_op2_src0), src0_neg_bit);
   TEST_EQ(vneg->encode_for(alu_op2_src1), src1_neg_bit);
   TEST_EQ(vneg->encode_for(alu_op3_src2), src2_neg_bit);
}

TEST_F(BytecodeAluOp2ATest, TestValueSrc0Rel)
{
   auto vrel =Value::create(0, 0, 0, 1, 0, li);
   TEST_EQ(vrel->encode_for(alu_op2_src0), src0_rel_bit);
   TEST_EQ(vrel->encode_for(alu_op2_src1), src1_rel_bit);
   TEST_EQ(vrel->encode_for(alu_op3_src2), src2_rel_bit);
   TEST_EQ(vrel->encode_for(alu_op_dst), dst_rel_bit);

}

TEST_F(BytecodeAluOp2ATest, TestValuePS)
{
   auto vop2_alu_src_ps =Value::create(255, 0, 0, 0, 1, li);
   TEST_EQ(vop2_alu_src_ps->encode_for(alu_op2_src0), 0x00000000000010fful);
   TEST_EQ(vop2_alu_src_ps->encode_for(alu_op2_src1), 0x00000000021fe000ul);
   TEST_EQ(vop2_alu_src_ps->encode_for(alu_op3_src2), 0x000010ff00000000ul);

   auto vop2_alu_src_kcache3 =Value::create(319, 3, 1, 1, 1, li);
   TEST_EQ(vop2_alu_src_kcache3 ->encode_for(alu_op2_src0), 0x0000000100001f3ful);
   TEST_EQ(vop2_alu_src_kcache3 ->encode_for(alu_op2_src1), 0x0000000203e7e000ul);

   auto alu_src2_kcache3 =Value::create(319, 3, 0, 1, 1, li);
   TEST_EQ(alu_src2_kcache3->encode_for(alu_op3_src2), 0x00001f3f00000000ul);

   auto dst = Value::create(127, 3, 0, 1, 0, li);
   TEST_EQ(dst->encode_for(alu_op_dst), 0x7fe0000000000000ul);

}