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
   vector<PValue> vgpr;
   vector<PValue> vlit;
   vector<PValue> vinline;
   vector<PValue> vkconst;
   vector<GPRValue> vdst;
};


void BytecodeAluOp2ATest::SetUp()
{

   Value::LiteralFlags literal_index;
   set_spacing({63, 61, 60, 53, 50, 45, 44, 42, 41, 32,
                31, 29, 26, 25, 23, 22, 13, 12, 10, 9});

   vgpr.push_back(Value::create(120, 0, 0, 0, 0, literal_index));
   vgpr.push_back(Value::create(12,  1, 1, 0, 0, literal_index));
   vgpr.push_back(Value::create(2,   2, 0, 1, 0, literal_index));
   vgpr.push_back(Value::create(23,  3, 0, 0, 1, literal_index));

   //vlit.push_back(Value::create());


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

TEST_F(BytecodeAluOp2ATest, TestAllGpr)
{
   Value::LiteralFlags literal_index;
   PValue v0gpr(Value::create(120, 3, 0, 0, 0, literal_index));
   PValue v1gpr(Value::create(120, 3, 0, 0, 0, literal_index));


}

