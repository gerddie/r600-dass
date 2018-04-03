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
#include <r600/cf_node.h>
#include <r600/disassembler.h>
#include <gtest/gtest.h>
#include <vector>

using namespace r600;
using std::vector;

using ProgramDisassTest = testing::Test;

TEST_F(ProgramDisassTest, Various_5_slot)
{
   vector<uint64_t> bc;
   cf_alu_node(cf_alu, 0, 2, 2).append_bytecode(bc);
   cf_native_node(cf_nop, 1 << cf_node::eop).append_bytecode(bc);
   bc.push_back(0x0180011000200001ul);
   bc.push_back(0x2180011000200401ul);
   bc.push_back(0x4180011080200801ul);

   disassembler diss(bc);

   const char *expect =
       "ALU                    ADDR:2 COUNT:3\n"
       "    KC0: 0@0x0 nop    KC1: 0@0x0 nop\n\n"
       "    x:     MUL_IEEE                        R12.x, R1.x, KC2[0].x\n"
       "    y:     MUL_IEEE                        R12.y, R1.y, KC2[0].x\n"
       "    z:     MUL_IEEE                        R12.z, R1.z, KC2[0].x\n\n"
       "NOP                    EOP\n"
         ;

   ASSERT_EQ(diss.as_string(), expect);
}




