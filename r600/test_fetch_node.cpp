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
#include <r600/fetch_node.h>
#include <gtest/gtest.h>
#include <vector>

using namespace r600;
using std::vector;
using std::ostringstream;

template <typename Node>
class FetchNodeDisass: public testing::Test {
public:
   void run(uint64_t bc0, uint64_t bc1, const char *expect) const;
};

template <typename Node>
void FetchNodeDisass<Node>::run(uint64_t bc0, uint64_t bc1, const char *expect) const
{
   auto n = FetchNode::decode(bc0, bc1);
   ostringstream os;
   os << *n;
   EXPECT_EQ(os.str(), expect);
}

using VertexFetchNodeDisass=FetchNodeDisass<VertexFetchNode>;

TEST_F(VertexFetchNodeDisass, test_gpr_fetch)
{
   run(0x188d10017c000000ul, 0x00080000ul,
       "Fetch VTX R1.xyzw, R0.x BUFID:0 FMT:(32_32_32_32 int noswap) MFC:31");
}

TEST_F(VertexFetchNodeDisass, test_gpr_fetch_with_offset)
{
   run(0x08cd10027c000000ul, 0x00080010ul,
       "Fetch VTX R2.xyzw, R0.x+16 BUFID:0 FMT:(32_32_32_32F norm noswap) MFC:31");
}

TEST_F(VertexFetchNodeDisass, test_gpr_dst_limits)
{
   run(0x08cd107b7c000000ul, 0x00080010ul,
       "Fetch VTX R123.xyzw, R0.x+16 BUFID:0 FMT:(32_32_32_32F norm noswap) MFC:31");

   run(0x08c0a67b7c000000ul, 0x00080010ul,
       "Fetch VTX R123.wzyx, R0.x+16 BUFID:0 FMT:(32_32_32_32F norm noswap) MFC:31");

   run(0x08cd107b7c7b0000ul, 0x00080010ul,
       "Fetch VTX R123.xyzw, R123.x+16 BUFID:0 FMT:(32_32_32_32F norm noswap) MFC:31");

   run(0x08cd107b7f7b0000ul, 0x00080010ul,
       "Fetch VTX R123.xyzw, R123.w+16 BUFID:0 FMT:(32_32_32_32F norm noswap) MFC:31");

   run(0x08cd107b7f7b0000ul, 0x0008fffful,
       "Fetch VTX R123.xyzw, R123.w+65535 BUFID:0 FMT:(32_32_32_32F norm noswap) MFC:31");

   run(0x08cd107b7f7b0000ul, 0x0009fffful,
       "Fetch VTX R123.xyzw, R123.w+65535 BUFID:0 FMT:(32_32_32_32F norm 8in16) MFC:31");

   run(0x08cd107b7f7b0000ul, 0x000afffful,
       "Fetch VTX R123.xyzw, R123.w+65535 BUFID:0 FMT:(32_32_32_32F norm 8in32) MFC:31");

   run(0x08cd107bff7b0000ul, 0x000afffful,
       "Fetch VTX R123.xyzw, R123.w+65535 BUFID:0 FMT:(32_32_32_32F norm 8in32) MFC:63");

   run(0x08cd107bffFb0000ul, 0x000a0000ul,
       "Fetch VTX R123.xyzw, R[123+LoopIDX].w BUFID:0 FMT:(32_32_32_32F norm 8in32) MFC:63");
}


using TexFetchNodeDisass=FetchNodeDisass<TexFetchNode>;

TEST_F(TexFetchNodeDisass, test_LD_fetch)
{
   run( 0xf00d100300041203ul, 0x68800000,
        "LD             R3.xyzw, R4.xyzw, RID:18, SID:0 CT:uuuu");
}

TEST_F(TexFetchNodeDisass, test_vfetch_fetch)
{
   run( 0x68cd100040000140ul, 0x00080000,
        "Fetch VTX R0.xyzw, R0.x BUFID:1 FMT:(32_32_32_32F scaled noswap) MFC:16 Flags: signed");
}
