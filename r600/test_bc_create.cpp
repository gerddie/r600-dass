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
#include <r600/cf_node.h>
#include <gtest/gtest.h>

using namespace r600;

class BytecodeCFNativeTest: public BytecodeTest {
   void SetUp() {
      set_spacing({63, 62, 54, 53, 52, 48, 42, 40, 35, 32, 27, 24});
   }
};

class BytecodeCFAluTest: public BytecodeTest {
   void SetUp() {
      set_spacing({63, 62, 58, 57, 50, 42, 34, 32, 30, 26, 22});
   }
};

class BytecodeCFGlobalWaveSync: public BytecodeTest {
   void SetUp() {
      set_spacing({63, 62, 54, 53, 52, 48, 42, 40, 35, 32,
                   30, 28, 26, 25, 21, 16, 10});
   }
};

class BytecodeCFMemRat: public BytecodeTest {
   void SetUp() {
      set_spacing({63, 62, 54, 53, 52, 48, 44, 32,
                   30, 23, 22, 15, 13, 11, 10, 4});
   }
};

TEST_F(BytecodeCFNativeTest, BytecodeCreationNative)
{
   TEST_EQ(CFNativeNode(cf_nop, 0).get_bytecode_byte(0), 0);
   TEST_EQ(CFNativeNode(cf_nop, 1 << CFNode::eop).get_bytecode_byte(0),
         end_of_program_bit);

   TEST_EQ(CFNativeNode(cf_nop, 1 << CFNode::barrier).get_bytecode_byte(0),
         barrier_bit);

   TEST_EQ(CFNativeNode(cf_nop, 1 << CFNode::wqm).get_bytecode_byte(0),
         whole_quad_mode_bit);

   TEST_EQ(CFNativeNode(cf_nop, 1 << CFNode::vpm).get_bytecode_byte(0),
         valid_pixel_mode_bit);

   TEST_EQ(CFNativeNode(cf_pop, 0, 0, 1).get_bytecode_byte(0),
         0x0380000100000000ul);

   TEST_EQ(CFNativeNode(cf_pop, 0, 0, 7).get_bytecode_byte(0),
         0x0380000700000000ul);

   TEST_EQ(CFNativeNode(cf_pop, 1 << CFNode::vpm).get_bytecode_byte(0),
         0x0390000000000000ul);

   TEST_EQ(CFNativeNode(cf_push, 1 << CFNode::barrier).get_bytecode_byte(0),
         0x82C0000000000000ul);

   TEST_EQ(CFNativeNode(cf_tc, 1 << CFNode::wqm, 3, 0, 2).get_bytecode_byte(0),
         0x4040080000000003ul);

   TEST_EQ(CFNativeNode(cf_tc_ack, 0, 3, 0, 2).get_bytecode_byte(0),
         0x06C0080000000003ul);

   TEST_EQ(CFNativeNode(cf_vc, 1 << CFNode::eop, 10, 1, 5).get_bytecode_byte(0),
         0x00A014010000000Aul);

   TEST_EQ(CFNativeNode(cf_vc_ack, 1 << CFNode::wqm, 10, 1, 5).get_bytecode_byte(0),
         0x470014010000000Aul);

   TEST_EQ(CFNativeNode(cf_jump, 1 << CFNode::barrier, 20, 1).get_bytecode_byte(0),
         0x8280000100000014ul);

   TEST_EQ(CFNativeNode(cf_jump, 1 << CFNode::barrier, 0xFFFFFF, 0).get_bytecode_byte(0),
         0x8280000000FFFFFFul);

   TEST_EQ(CFNativeNode(cf_jump_table, 1 << CFNode::barrier, 256, 0, 0, 3).get_bytecode_byte(0),
         0x8740000003000100ul);

   TEST_EQ(CFNativeNode(cf_gds, 1 << CFNode::barrier, 256, 0, 3, 0).get_bytecode_byte(0),
         0x80C00C0000000100ul);
}

TEST_F(BytecodeCFNativeTest, BytecodeCFNativeRountrip)
{
   std::vector<uint64_t> bc = {
      0,
      end_of_program_bit,
      barrier_bit,
      whole_quad_mode_bit,
      valid_pixel_mode_bit,
      0x0380000100000000ul,
      0x0380000700000000ul,
      0x0390000000000000ul,
      0x82C0000000000000ul,
      0x4040080000000003ul,
      0x06C0080000000003ul,
      0x00A014010000000Aul,
      0x470014010000000Aul,
      0x8280000100000014ul,
      0x8280000000FFFFFFul,
      0x8740000003000100ul,
      0x80C00C0000000100ul
   };

   for (auto x: bc)
      TEST_EQ(CFNativeNode(x).get_bytecode_byte(0), x);
}

TEST_F(BytecodeCFAluTest, BytecodeCreationAlu)
{
   TEST_EQ(CFAluNode(cf_alu, 0, 2, 128).get_bytecode_byte(0),
         0x21FC000000000002ul);

   TEST_EQ(CFAluNode(cf_alu, 1 << CFNode::alt_const, 2, 128).get_bytecode_byte(0),
         0x23FC000000000002ul);

   TEST_EQ(CFAluNode(cf_alu, 1 << CFNode::wqm, 2, 128).get_bytecode_byte(0),
         0x61FC000000000002ul);

   TEST_EQ(CFAluNode(cf_alu, 1 << CFNode::barrier, 2, 128).get_bytecode_byte(0),
         0xA1FC000000000002ul);

   TEST_EQ(CFAluNode(cf_alu_else_after, 0, 0x3FFFFFu, 2).get_bytecode_byte(0),
         0x3C040000003FFFFFul);


   TEST_EQ(CFAluNode(cf_alu, 0, 0x3u, 2,{15, 0, 0}).get_bytecode_byte(0),
         0x2004000003C00003ul);
   TEST_EQ(CFAluNode(cf_alu, 0, 0x3u, 2,{0, 0, 0},{15, 0, 0}).get_bytecode_byte(0),
         0x200400003C000003ul);

   TEST_EQ(CFAluNode(cf_alu, 0, 0x3u, 2,{0, 3, 0}).get_bytecode_byte(0),
         0x20040000C0000003ul);
   TEST_EQ(CFAluNode(cf_alu, 0, 0x3u, 2,{0, 0, 0},{0, 3, 0}).get_bytecode_byte(0),
         0x2004000300000003ul);

   TEST_EQ(CFAluNode(cf_alu, 0, 0x3u, 2,{0, 0, 255}).get_bytecode_byte(0),
         0x200403FC00000003ul);
   TEST_EQ(CFAluNode(cf_alu, 0, 0x3u, 2,{0, 0, 0},{0, 0, 255}).get_bytecode_byte(0),
         0x2007FC0000000003ul);
}

TEST_F(BytecodeCFAluTest, BytecodeAluRoundtrip)
{
   std::vector<uint64_t> bc = {
      0x21FC000000000002ul,
      0x23FC000000000002ul,
      0x61FC000000000002ul,
      0xA1FC000000000002ul,
      0x3C040000003FFFFFul,
      0x2004000003C00003ul,
      0x200400003C000003ul,
      0x20040000C0000003ul,
      0x2004000300000003ul,
      0x200403FC00000003ul,
      0x2007FC0000000003ul
   };
   for (auto x: bc)
      TEST_EQ(CFAluNode(x).get_bytecode_byte(0), x);
}

TEST_F(BytecodeCFAluTest, BytecodeCreationAluExtended)
{
   CFAluNode ext0(cf_alu_extended, 0, 0x3u, 2, {1,0,0,0},
                    {0, 0, 0}, {0, 0, 255},
                    {0, 0, 0}, {0, 0, 0});

   TEST_EQ(ext0.get_bytecode_byte(1), 0x3007FC0000000003ul);
   TEST_EQ(ext0.get_bytecode_byte(0), 0x3000000000000010ul);

   CFAluNode ext1(cf_alu_extended, 0, 0x3u, 2, {0,1,0,0},
                    {0, 0, 0}, {0, 0, 0},
                    {0, 0, 255}, {0, 0, 0});

   TEST_EQ(ext1.get_bytecode_byte(1), 0x3004000000000003ul);
   TEST_EQ(ext1.get_bytecode_byte(0), 0x300003FC00000040ul);

   CFAluNode ext2(cf_alu_extended, 0, 0x3u, 2, {0,1,0,0},
                    {0, 0, 0}, {0, 0, 0},
                    {0, 0, 0}, {0, 0, 255});

   TEST_EQ(ext2.get_bytecode_byte(1), 0x3004000000000003ul);
   TEST_EQ(ext2.get_bytecode_byte(0), 0x3003FC0000000040ul);

   CFAluNode ext3(cf_alu_extended, 0, 0x3u, 2, {0,0,1,0},
                    {0, 0, 0}, {0, 0, 0},
                    {15, 0, 0}, {0, 0, 0});

   TEST_EQ(ext3.get_bytecode_byte(1), 0x3004000000000003ul);
   TEST_EQ(ext3.get_bytecode_byte(0), 0x3000000003C00100ul);

   CFAluNode ext4(cf_alu_extended, 0, 0x3u, 2, {0,0,0,1},
                    {0, 0, 0}, {0, 0, 0},
                    {0, 0, 0}, {15, 0, 0});

   TEST_EQ(ext4.get_bytecode_byte(1), 0x3004000000000003ul);
   TEST_EQ(ext4.get_bytecode_byte(0), 0x300000003C000400ul);

   CFAluNode ext5(cf_alu_extended, 0, 0x3u, 2, {2,0,0,0},
                    {0, 0, 0}, {0, 0, 0},
                    {0, 3, 0}, {0, 0, 0});

   TEST_EQ(ext5.get_bytecode_byte(1), 0x3004000000000003ul);
   TEST_EQ(ext5.get_bytecode_byte(0), 0x30000000C0000020ul);

   CFAluNode ext6(cf_alu_extended, 0, 0x3u, 2, {0,2,0,0},
                    {0, 0, 0}, {0, 0, 0},
                    {0, 0, 0}, {0, 3, 0});

   TEST_EQ(ext6.get_bytecode_byte(1), 0x3004000000000003ul);
   TEST_EQ(ext6.get_bytecode_byte(0), 0x3000000300000080ul);

   CFAluNode ext7(cf_alu_extended, 0, 0x3u, 2, {0,0,2,0},
                    {0, 0, 0}, {0, 3, 0},
                    {0, 0, 0}, {0, 0, 0});

   TEST_EQ(ext7.get_bytecode_byte(1), 0x3004000300000003ul);
   TEST_EQ(ext7.get_bytecode_byte(0), 0x3000000000000200ul);

   CFAluNode ext8(cf_alu_extended, 0, 0x3u, 2, {0,0,0,2},
                    {0, 0, 255}, {0, 3, 0},
                    {0, 0, 0}, {0, 0, 255});

   TEST_EQ(ext8.get_bytecode_byte(1), 0x300403FF00000003ul);
   TEST_EQ(ext8.get_bytecode_byte(0), 0x3003FC0000000800ul);
}

TEST_F(BytecodeCFGlobalWaveSync, gws)
{
   TEST_EQ(CFGwsNode(cf_global_wave_sync,
                       0 /* gws_opcode */,
                       0 /* flags */,
                       0 /* pop_count */,
                       0 /* cf_const */,
                       0 /* cond */,
                       0 /* count */,
                       0 /* value */,
                       0 /* resource */,
                       0 /* val_index_mode */,
                       0 /* rsrc_index_mode */).get_bytecode_byte(0),
           0x0780000000000000ul);

   TEST_EQ(CFGwsNode(cf_global_wave_sync,
                       3 /* gws_opcode */,
                       0 /* flags */,
                       0 /* pop_count */,
                       0 /* cf_const */,
                       0 /* cond */,
                       0 /* count */,
                       0 /* value */,
                       0 /* resource */,
                       0 /* val_index_mode */,
                       0 /* rsrc_index_mode */).get_bytecode_byte(0),
           0x07800000C0000000ul);

   TEST_EQ(CFGwsNode(cf_global_wave_sync,
                       0 /* gws_opcode */,
                       0 /* flags */,
                       0 /* pop_count */,
                       0 /* cf_const */,
                       0 /* cond */,
                       0 /* count */,
                       0 /* value */,
                       0 /* resource */,
                       0 /* val_index_mode */,
                       2 /* rsrc_index_mode */).get_bytecode_byte(0),
           0x0780000020000000ul);

   TEST_EQ(CFGwsNode(cf_global_wave_sync,
                       0 /* gws_opcode */,
                       0 /* flags */,
                       0 /* pop_count */,
                       0 /* cf_const */,
                       0 /* cond */,
                       0 /* count */,
                       0 /* value */,
                       0 /* resource */,
                       2 /* val_index_mode */,
                       0 /* rsrc_index_mode */).get_bytecode_byte(0),

           0x0780000008000000ul);

   TEST_EQ(CFGwsNode(cf_global_wave_sync,
                       0 /* gws_opcode */,
                       0 /* flags */,
                       0 /* pop_count */,
                       0 /* cf_const */,
                       0 /* cond */,
                       0 /* count */,
                       0 /* value */,
                       0x1F /* resource */,
                       0 /* val_index_mode */,
                       0 /* rsrc_index_mode */).get_bytecode_byte(0),
           0x07800000001F0000ul);

   TEST_EQ(CFGwsNode(cf_global_wave_sync,
                       0 /* gws_opcode */,
                       0 /* flags */,
                       0 /* pop_count */,
                       0 /* cf_const */,
                       0 /* cond */,
                       0 /* count */,
                       0x3FF /* value */,
                       0 /* resource */,
                       0 /* val_index_mode */,
                       0 /* rsrc_index_mode */).get_bytecode_byte(0),
           0x07800000000003FFul);

   TEST_EQ(CFGwsNode(cf_global_wave_sync,
                       0 /* gws_opcode */,
                       1 << CFNode::sign /* flags */,
                       0 /* pop_count */,
                       0 /* cf_const */,
                       0 /* cond */,
                       0 /* count */,
                       0 /* value */,
                       0 /* resource */,
                       0 /* val_index_mode */,
                       0 /* rsrc_index_mode */).get_bytecode_byte(0),
           0x0780000002000000ul);
}


TEST_F(BytecodeCFMemRat, memrat)
{
   TEST_EQ(CFRatNode(cf_mem_rat,
                       0 /* rat_inst */,
                       0 /* rat_id */,
                       0 /* rat_idx_mode */,
                       0 /* type */,
                       0 /* rw_gpr */,
                       0 /* index_gpr*/,
                       0 /* elm_size */,
                       0 /* array_size */,
                       0xf /* comp_mask */,
                       0 /* burst_count */,
                       0 /* flags */).get_bytecode_byte(0),
           0x1580F00000000000ul);

   TEST_EQ(CFRatNode(cf_mem_rat,
                       0 /* rat_inst */,
                       0 /* rat_id */,
                       0 /* rat_idx_mode */,
                       0 /* type */,
                       0 /* rw_gpr */,
                       0 /* index_gpr*/,
                       0 /* elm_size */,
                       0xfff /* array_size */,
                       0 /* comp_mask */,
                       0 /* burst_count */,
                       0 /* flags */).get_bytecode_byte(0),
           0x15800fff00000000ul);

   TEST_EQ(CFRatNode(cf_mem_rat,
                       0 /* rat_inst */,
                       0 /* rat_id */,
                       0 /* rat_idx_mode */,
                       0 /* type */,
                       0 /* rw_gpr */,
                       0 /* index_gpr*/,
                       0 /* elm_size */,
                       0 /* array_size */,
                       0 /* comp_mask */,
                       0xf /* burst_count */,
                       0 /* flags */).get_bytecode_byte(0),
           0x158f000000000000ul);

   TEST_EQ(CFRatNode(cf_mem_rat,
                       0 /* rat_inst */,
                       0xf /* rat_id */,
                       0 /* rat_idx_mode */,
                       0 /* type */,
                       0 /* rw_gpr */,
                       0 /* index_gpr*/,
                       0 /* elm_size */,
                       0 /* array_size */,
                       0 /* comp_mask */,
                       0 /* burst_count */,
                       0 /* flags */).get_bytecode_byte(0),
           0x158000000000000ful);

   TEST_EQ(CFRatNode(cf_mem_rat,
                       0x3F /* rat_inst */,
                       0 /* rat_id */,
                       0 /* rat_idx_mode */,
                       0 /* type */,
                       0 /* rw_gpr */,
                       0 /* index_gpr*/,
                       0 /* elm_size */,
                       0 /* array_size */,
                       0 /* comp_mask */,
                       0 /* burst_count */,
                       0 /* flags */).get_bytecode_byte(0),
           0x15800000000003f0ul);

   TEST_EQ(CFRatNode(cf_mem_rat,
                       0 /* rat_inst */,
                       0 /* rat_id */,
                       3 /* rat_idx_mode */,
                       0 /* type */,
                       0 /* rw_gpr */,
                       0 /* index_gpr*/,
                       0 /* elm_size */,
                       0 /* array_size */,
                       0 /* comp_mask */,
                       0 /* burst_count */,
                       0 /* flags */).get_bytecode_byte(0),
           0x1580000000001800ul);

   TEST_EQ(CFRatNode(cf_mem_rat,
                       0 /* rat_inst */,
                       0 /* rat_id */,
                       0 /* rat_idx_mode */,
                       3 /* type */,
                       0 /* rw_gpr */,
                       0 /* index_gpr*/,
                       0 /* elm_size */,
                       0 /* array_size */,
                       0 /* comp_mask */,
                       0 /* burst_count */,
                       0 /* flags */).get_bytecode_byte(0),
           0x1580000000006000ul);

   TEST_EQ(CFRatNode(cf_mem_rat,
                       0 /* rat_inst */,
                       0 /* rat_id */,
                       0 /* rat_idx_mode */,
                       0 /* type */,
                       0x7f/* rw_gpr */,
                       0 /* index_gpr*/,
                       0 /* elm_size */,
                       0 /* array_size */,
                       0 /* comp_mask */,
                       0 /* burst_count */,
                       0 /* flags */).get_bytecode_byte(0),
           0x15800000003f8000ul);

   TEST_EQ(CFRatNode(cf_mem_rat,
                       0 /* rat_inst */,
                       0 /* rat_id */,
                       0 /* rat_idx_mode */,
                       0 /* type */,
                       0/* rw_gpr */,
                       0x7f /* index_gpr*/,
                       0 /* elm_size */,
                       0 /* array_size */,
                       0 /* comp_mask */,
                       0 /* burst_count */,
                       0 /* flags */).get_bytecode_byte(0),
           0x158000003f800000ul);

   TEST_EQ(CFRatNode(cf_mem_rat,
                       0 /* rat_inst */,
                       0 /* rat_id */,
                       0 /* rat_idx_mode */,
                       0 /* type */,
                       0/* rw_gpr */,
                       0 /* index_gpr*/,
                       3 /* elm_size */,
                       0 /* array_size */,
                       0 /* comp_mask */,
                       0 /* burst_count */,
                       0 /* flags */).get_bytecode_byte(0),
           0x15800000c0000000ul);

   TEST_EQ(CFRatNode(cf_mem_rat,
                       0 /* rat_inst */,
                       0 /* rat_id */,
                       0 /* rat_idx_mode */,
                       0 /* type */,
                       0/* rw_gpr */,
                       0 /* index_gpr*/,
                       0 /* elm_size */,
                       0xfff /* array_size */,
                       0 /* comp_mask */,
                       0 /* burst_count */,
                       0 /* flags */).get_bytecode_byte(0),
           0x15800fff00000000ul);

   TEST_EQ(CFRatNode(cf_mem_rat,
                       0 /* rat_inst */,
                       0 /* rat_id */,
                       0 /* rat_idx_mode */,
                       0 /* type */,
                       0 /* rw_gpr */,
                       0 /* index_gpr*/,
                       0 /* elm_size */,
                       0 /* array_size */,
                       0xf /* comp_mask */,
                       0 /* burst_count */,
                       0 /* flags */).get_bytecode_byte(0),
           0x1580f00000000000ul);

   TEST_EQ(CFRatNode(cf_mem_rat,
                       0 /* rat_inst */,
                       0 /* rat_id */,
                       0 /* rat_idx_mode */,
                       0 /* type */,
                       0 /* rw_gpr */,
                       0 /* index_gpr*/,
                       0 /* elm_size */,
                       0 /* array_size */,
                       0 /* comp_mask */,
                       0xf /* burst_count */,
                       0 /* flags */).get_bytecode_byte(0),
           0x158f000000000000ul);

   TEST_EQ(CFRatNode(cf_mem_rat,
                       0 /* rat_inst */,
                       0 /* rat_id */,
                       0 /* rat_idx_mode */,
                       0 /* type */,
                       0 /* rw_gpr */,
                       0 /* index_gpr*/,
                       0 /* elm_size */,
                       0 /* array_size */,
                       0 /* comp_mask */,
                       0 /* burst_count */,
                       1 << CFNode::eop /* flags */).get_bytecode_byte(0),
           0x15a0000000000000ul);

   TEST_EQ(CFRatNode(cf_mem_rat,
                       0 /* rat_inst */,
                       0 /* rat_id */,
                       0 /* rat_idx_mode */,
                       0 /* type */,
                       0 /* rw_gpr */,
                       0 /* index_gpr*/,
                       0 /* elm_size */,
                       0 /* array_size */,
                       0 /* comp_mask */,
                       0 /* burst_count */,
                       1 << CFNode::mark/* flags */).get_bytecode_byte(0),
           0x5580000000000000ul);
}

TEST_F(BytecodeCFMemRat, memrat_rountrip)
{
   std::vector<uint64_t> bc = {
      0x1580F00000000000ul,
      0x15800fff00000000ul,
      0x158f000000000000ul,
      0x158000000000000ful,
      0x15800000000003f0ul,
      0x1580000000001800ul,
      0x1580000000006000ul,
      0x15800000003f8000ul,
      0x158000003f800000ul,
      0x15800000c0000000ul,
      0x15800fff00000000ul,
      0x1580f00000000000ul,
      0x158f000000000000ul,
      0x15a0000000000000ul,
      0x5580000000000000ul};

   for (auto x: bc)
      TEST_EQ(CFRatNode(x).get_bytecode_byte(0), x);
}

class BytecodeCFMemRing: public BytecodeTest {
   void SetUp() {
      set_spacing({63, 62, 54, 53, 52, 48, 44, 32,
                   30, 23, 22, 15, 13, 12});
   }
};

TEST_F(BytecodeCFMemRing, CFMemRingTest)
{
   TEST_EQ(CFMemRingNode(cf_mem_ring, /* opcpde */
                            0, /* type */
                            0, /* rw_gpr */
                            0, /* index_gpr */
                            0, /* elem_size */
                            0, /* array_size */
                            0, /* array_base */
                            0, /* comp_mask */
                            0, /* burst_count */
                            0  /* flags */).get_bytecode_byte(0),
           0x1480000000000000ul);

   TEST_EQ(CFMemRingNode(cf_mem_ring, /* opcpde */
                            3, /* type */
                            0, /* rw_gpr */
                            0, /* index_gpr */
                            0, /* elem_size */
                            0, /* array_size */
                            0, /* array_base */
                            0, /* comp_mask */
                            0, /* burst_count */
                            0  /* flags */).get_bytecode_byte(0),
           0x1480000000006000ul);

   TEST_EQ(CFMemRingNode(cf_mem_ring, /* opcpde */
                            0, /* type */
                            0x7f, /* rw_gpr */
                            0, /* index_gpr */
                            0, /* elem_size */
                            0, /* array_size */
                            0, /* array_base */
                            0, /* comp_mask */
                            0, /* burst_count */
                            0  /* flags */).get_bytecode_byte(0),
           0x14800000003f8000ul);

   TEST_EQ(CFMemRingNode(cf_mem_ring, /* opcpde */
                            0, /* type */
                            0, /* rw_gpr */
                            0x7f, /* index_gpr */
                            0, /* elem_size */
                            0, /* array_size */
                            0, /* array_base */
                            0, /* comp_mask */
                            0, /* burst_count */
                            0  /* flags */).get_bytecode_byte(0),
           0x148000003f800000ul);

   TEST_EQ(CFMemRingNode(cf_mem_ring, /* opcpde */
                            0, /* type */
                            0, /* rw_gpr */
                            0, /* index_gpr */
                            3, /* elem_size */
                            0, /* array_size */
                            0, /* array_base */
                            0, /* comp_mask */
                            0, /* burst_count */
                            0  /* flags */).get_bytecode_byte(0),
           0x14800000c0000000ul);

   TEST_EQ(CFMemRingNode(cf_mem_ring, /* opcpde */
                            0, /* type */
                            0, /* rw_gpr */
                            0, /* index_gpr */
                            0, /* elem_size */
                            0xfff, /* array_size */
                            0, /* array_base */
                            0, /* comp_mask */
                            0, /* burst_count */
                            0  /* flags */).get_bytecode_byte(0),
           0x14800fff00000000ul);

   TEST_EQ(CFMemRingNode(cf_mem_ring, /* opcpde */
                            0, /* type */
                            0, /* rw_gpr */
                            0, /* index_gpr */
                            0, /* elem_size */
                            0, /* array_size */
                            0x1fff, /* array_base */
                            0, /* comp_mask */
                            0, /* burst_count */
                            0  /* flags */).get_bytecode_byte(0),
           0x1480000000001ffful);

   TEST_EQ(CFMemRingNode(cf_mem_ring, /* opcpde */
                            0, /* type */
                            0, /* rw_gpr */
                            0, /* index_gpr */
                            0, /* elem_size */
                            0, /* array_size */
                            0, /* array_base */
                            0xf, /* comp_mask */
                            0, /* burst_count */
                            0  /* flags */).get_bytecode_byte(0),
           0x1480f00000000000ul);

   TEST_EQ(CFMemRingNode(cf_mem_ring, /* opcpde */
                            0, /* type */
                            0, /* rw_gpr */
                            0, /* index_gpr */
                            0, /* elem_size */
                            0, /* array_size */
                            0, /* array_base */
                            0, /* comp_mask */
                            0xf, /* burst_count */
                            0  /* flags */).get_bytecode_byte(0),
           0x148f000000000000ul);

   TEST_EQ(CFMemRingNode(cf_mem_ring, /* opcpde */
                            0, /* type */
                            0, /* rw_gpr */
                            0, /* index_gpr */
                            0, /* elem_size */
                            0, /* array_size */
                            0, /* array_base */
                            0, /* comp_mask */
                            0, /* burst_count */
                            1 << CFNode::vpm  /* flags */).get_bytecode_byte(0),
           0x1490000000000000ul);

   TEST_EQ(CFMemRingNode(cf_mem_ring, /* opcpde */
                            0, /* type */
                            0, /* rw_gpr */
                            0, /* index_gpr */
                            0, /* elem_size */
                            0, /* array_size */
                            0, /* array_base */
                            0, /* comp_mask */
                            0, /* burst_count */
                            1 << CFNode::eop  /* flags */).get_bytecode_byte(0),
           0x14a0000000000000ul);

   TEST_EQ(CFMemRingNode(cf_mem_ring, /* opcpde */
                            0, /* type */
                            0, /* rw_gpr */
                            0, /* index_gpr */
                            0, /* elem_size */
                            0, /* array_size */
                            0, /* array_base */
                            0, /* comp_mask */
                            0, /* burst_count */
                            1 << CFNode::barrier /* flags */).get_bytecode_byte(0),
           0x9480000000000000ul);

   TEST_EQ(CFMemRingNode(cf_mem_ring, /* opcpde */
                            0, /* type */
                            0, /* rw_gpr */
                            0, /* index_gpr */
                            0, /* elem_size */
                            0, /* array_size */
                            0, /* array_base */
                            0, /* comp_mask */
                            0, /* burst_count */
                            1 << CFNode::wqm /* flags */).get_bytecode_byte(0),
           0x5480000000000000ul);
}


TEST_F(BytecodeCFMemRing, CFMemRingRoundTripTest)
{
   std::vector<uint64_t> bc = {
      0x1480000000000000ul,
      0x1480000000006000ul,
      0x14800000003f8000ul,
      0x148000003f800000ul,
      0x14800000c0000000ul,
      0x14800fff00000000ul,
      0x1480000000001ffful,
      0x1480f00000000000ul,
      0x148f000000000000ul,
      0x1490000000000000ul,
      0x14a0000000000000ul,
      0x9480000000000000ul,
      0x5480000000000000ul};

   for (auto x: bc)
      TEST_EQ(CFMemRingNode(x).get_bytecode_byte(0), x);
}

class BytecodeCFMemExport: public BytecodeTest {
   void SetUp() {
      set_spacing({63, 62, 54, 53, 52, 48, 44, 41, 38, 35, 32,
                   30, 23, 22, 15, 13, 12});
   }
};

TEST_F(BytecodeCFMemExport, CFMemExportCreateTest)
{
   TEST_EQ(CFMemExportNode(cf_mem_export,
                              3, /* type */
                              0, /* rw_gpr */
                              0, /* index_gpr */
                              0, /* elem_size */
                              0, /* array_base */
                              0, /*burst_count */
                              {0,0,0,0}, /* sel */
                              0 /*flags */).get_bytecode_byte(0),
           0x1540000000006000ul);

   TEST_EQ(CFMemExportNode(cf_mem_export,
                              0, /* type */
                              0x7f, /* rw_gpr */
                              0, /* index_gpr */
                              0, /* elem_size */
                              0, /* array_base */
                              0, /*burst_count */
                              {0,0,0,0}, /* sel */
                              0 /*flags */).get_bytecode_byte(0),
           0x15400000003f8000ul);

   TEST_EQ(CFMemExportNode(cf_mem_export,
                              0, /* type */
                              0, /* rw_gpr */
                              0x7f, /* index_gpr */
                              0, /* elem_size */
                              0, /* array_base */
                              0, /*burst_count */
                              {0,0,0,0}, /* sel */
                              0 /*flags */).get_bytecode_byte(0),
           0x154000003f800000ul);

   TEST_EQ(CFMemExportNode(cf_mem_export,
                              0, /* type */
                              0, /* rw_gpr */
                              0, /* index_gpr */
                              3, /* elem_size */
                              0, /* array_base */
                              0, /*burst_count */
                              {0,0,0,0}, /* sel */
                              0 /*flags */).get_bytecode_byte(0),
           0x15400000c0000000ul);

   TEST_EQ(CFMemExportNode(cf_mem_export,
                              0, /* type */
                              0, /* rw_gpr */
                              0, /* index_gpr */
                              0, /* elem_size */
                              0x1fff, /* array_base */
                              0, /*burst_count */
                              {0,0,0,0}, /* sel */
                              0 /*flags */).get_bytecode_byte(0),
           0x1540000000001ffful);

   TEST_EQ(CFMemExportNode(cf_mem_export,
                              0, /* type */
                              0, /* rw_gpr */
                              0, /* index_gpr */
                              0, /* elem_size */
                              0, /* array_base */
                              0xf, /*burst_count */
                              {0,0,0,0}, /* sel */
                              0 /*flags */).get_bytecode_byte(0),
           0x154f000000000000ul);

   TEST_EQ(CFMemExportNode(cf_mem_export,
                              0, /* type */
                              0, /* rw_gpr */
                              0, /* index_gpr */
                              0, /* elem_size */
                              0, /* array_base */
                              0, /*burst_count */
                              {7,0,0,0}, /* sel */
                              0 /*flags */).get_bytecode_byte(0),
           0x1540000700000000ul);

   TEST_EQ(CFMemExportNode(cf_mem_export,
                              0, /* type */
                              0, /* rw_gpr */
                              0, /* index_gpr */
                              0, /* elem_size */
                              0, /* array_base */
                              0, /*burst_count */
                              {0,7,0,0}, /* sel */
                              0 /*flags */).get_bytecode_byte(0),
           0x1540003800000000ul);

   TEST_EQ(CFMemExportNode(cf_mem_export,
                              0, /* type */
                              0, /* rw_gpr */
                              0, /* index_gpr */
                              0, /* elem_size */
                              0, /* array_base */
                              0, /*burst_count */
                              {0,0,7,0}, /* sel */
                              0 /*flags */).get_bytecode_byte(0),
           0x154001c000000000ul);

   TEST_EQ(CFMemExportNode(cf_mem_export,
                              0, /* type */
                              0, /* rw_gpr */
                              0, /* index_gpr */
                              0, /* elem_size */
                              0, /* array_base */
                              0, /*burst_count */
                              {0,0,0,7}, /* sel */
                              0 /*flags */).get_bytecode_byte(0),
           0x15400e0000000000ul);
}

TEST_F(BytecodeCFMemExport, CFMemExportRoundtripTest)
{
   std::vector<uint64_t> bc = {
      0x1540000000006000ul,
      0x15400000003f8000ul,
      0x154000003f800000ul,
      0x15400000c0000000ul,
      0x1540000000001ffful,
      0x154f000000000000ul,
      0x1540000700000000ul,
      0x1540003800000000ul,
      0x154001c000000000ul,
      0x15400e0000000000ul};

   for (auto x: bc)
      TEST_EQ(CFMemExportNode(x).get_bytecode_byte(0), x);
}
