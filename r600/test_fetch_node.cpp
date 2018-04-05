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
#include <r600/cffetchnode.h>
#include <gtest/gtest.h>
#include <vector>

using namespace r600;
using std::vector;
using std::ostringstream;

using FetchNodeDisass=testing::Test;

TEST_F(FetchNodeDisass, test_gpr_fetch)
{
   const uint64_t bc0 = 0x188d10017c000000ul;
   const uint64_t bc1 = 0x00080000ul;

   VertexFetchNode n(bc0, bc1);

   ostringstream os;
   os << n;

   EXPECT_EQ(os.str(),
             "Fetch VTX R1.xyzw, R0.x BUFID:0 FMT:(34 int noswap) MFC:31 Flags: ______") ;

     //00080000 VFETCH              R1.xyzw, R0.x,   RID:0  VERTEX MFC:31 UCF:0 FMT(DTA:34 NUM:1 COMP:0 MODE:0)
}

TEST_F(FetchNodeDisass, test_gpr_fetch_with_offset)
{
   const uint64_t bc0 = 0x08cd10027c000000ul;
   const uint64_t bc1 = 0x00080010ul;

   VertexFetchNode n(bc0, bc1);

   ostringstream os;
   os << n;

   EXPECT_EQ(os.str(),
             "Fetch VTX R2.xyzw, R0.x+16 BUFID:0 FMT:(35 norm noswap) MFC:31 Flags: ______") ;

}




