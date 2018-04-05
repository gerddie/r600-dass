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

class FetchNodeDisass: public testing::Test {
public:
   void run(uint64_t bc0, uint64_t bc1, const char *expect) const;
};

void FetchNodeDisass::run(uint64_t bc0, uint64_t bc1, const char *expect) const
{
   VertexFetchNode n(bc0, bc1);
   ostringstream os;
   os << n;
   EXPECT_EQ(os.str(), expect);
}


TEST_F(FetchNodeDisass, test_gpr_fetch)
{
   run(0x188d10017c000000ul, 0x00080000ul,
       "Fetch VTX R1.xyzw, R0.x BUFID:0 FMT:(34 int noswap) MFC:31 Flags: ______");
}

TEST_F(FetchNodeDisass, test_gpr_fetch_with_offset)
{
   run(0x08cd10027c000000ul, 0x00080010ul,
       "Fetch VTX R2.xyzw, R0.x+16 BUFID:0 FMT:(35 norm noswap) MFC:31 Flags: ______");
}






