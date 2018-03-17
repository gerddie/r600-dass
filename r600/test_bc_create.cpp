


#include <r600/cf_node.h>
#include <gtest/gtest.h>


/* Below tests strife to check whether the bits are properly arranged in the
 * byte code. Doing this is a pre-requisit for testing the disassembler part
 * properly.
 */
::testing::AssertionResult SameBitmap(const uint8_t *spacing,
                                      const char* m_expr,
                                      const char* n_expr,
                                      uint64_t m,
                                      uint64_t n) {
  if (m == n)
    return ::testing::AssertionSuccess();

  std::ostringstream msg;
  msg << "Expected:" << m_expr << " == " << n_expr << "\n got\n"
      << " -" << std::setbase(16) << std::setw(16) << std::setfill('0') << m << "\n"
      << " +" << std::setw(16) << n << "\n"
      << " delta: w1: ";
  uint64_t delta = m ^ n;
  int tabs = 0;
  for (int i = 63; i >= 0; --i) {
     msg << ((delta & (1ul << i)) ? '1' : '0');
     if (i == spacing[tabs]) {
        msg << " ";
        ++tabs;
     }
     if (i == 32)
         msg << "\n        w0: ";
  }

  return ::testing::AssertionFailure() << msg.str();
}

class BytecodeCFTest: public testing::Test {
protected:
   void do_check(const uint8_t spacing[],
                 const char *s_data, const char *s_expect,
                 uint64_t data, uint64_t expect) const {
      GTEST_ASSERT_(SameBitmap(spacing, s_data, s_expect, data, expect),
                    GTEST_NONFATAL_FAILURE_);
   }
};

class BytecodeCFNativeTest: public BytecodeCFTest {
   static const uint8_t spaces[];
protected:
   void check(const char *s_data, const char *s_expect, uint64_t data, uint64_t expect) const {
      do_check(spaces, s_data, s_expect, data, expect);
   }
};

const uint8_t BytecodeCFNativeTest::spaces[] = {
     63, 62, 54, 53, 52, 48, 42, 40, 35, 32, 27, 24, 0
};

class BytecodeCFAluTest: public BytecodeCFTest {
   static const uint8_t spaces[];
protected:
   void check(const char *s_data, const char *s_expect,
              uint64_t data, uint64_t expect) const {
      do_check(spaces, s_data, s_expect, data, expect);
   }
};

const uint8_t BytecodeCFAluTest::spaces[] = {
   63, 62, 58, 57, 50, 42, 34, 32, 30, 26, 22, 0
};

class BytecodeCFGlobalWaveSync: public BytecodeCFTest {
   static const uint8_t spaces[];
protected:
   void check(const char *s_data, const char *s_expect,
              uint64_t data, uint64_t expect) const {
      do_check(spaces, s_data, s_expect, data, expect);
   }
};

const uint8_t BytecodeCFGlobalWaveSync::spaces[] = {
   63, 62, 54, 53, 52, 48, 42, 40, 35, 32,
   30, 28, 26, 25, 21, 16, 10, 0
};

class BytecodeCFMemRat: public BytecodeCFTest {
   static const uint8_t spaces[];
protected:
   void check(const char *s_data, const char *s_expect,
              uint64_t data, uint64_t expect) const {
      do_check(spaces, s_data, s_expect, data, expect);
   }
};

const uint8_t BytecodeCFMemRat::spaces[] = {
   63, 62, 54, 53, 52, 48, 44, 32,
   30, 23, 22, 15, 13, 11, 10, 4, 0
};


#define TEST_EQ(X, Y) check(#X, #Y, X, Y)

TEST_F(BytecodeCFNativeTest, BytecodeCreationNative)
{
   TEST_EQ(cf_native_node(cf_nop, 0).get_bytecode_byte(0), 0);
   TEST_EQ(cf_native_node(cf_nop, 1 << cf_node::eop).get_bytecode_byte(0),
         end_of_program_bit);

   TEST_EQ(cf_native_node(cf_nop, 1 << cf_node::barrier).get_bytecode_byte(0),
         barrier_bit);

   TEST_EQ(cf_native_node(cf_nop, 1 << cf_node::wqm).get_bytecode_byte(0),
         whole_quad_mode_bit);

   TEST_EQ(cf_native_node(cf_nop, 1 << cf_node::vpm).get_bytecode_byte(0),
         valid_pixel_mode_bit);

   TEST_EQ(cf_native_node(cf_pop, 0, 0, 1).get_bytecode_byte(0),
         0x0380000100000000ul);

   TEST_EQ(cf_native_node(cf_pop, 0, 0, 7).get_bytecode_byte(0),
         0x0380000700000000ul);

   TEST_EQ(cf_native_node(cf_pop, 1 << cf_node::vpm).get_bytecode_byte(0),
         0x0390000000000000ul);

   TEST_EQ(cf_native_node(cf_push, 1 << cf_node::barrier).get_bytecode_byte(0),
         0x82C0000000000000ul);

   TEST_EQ(cf_native_node(cf_tc, 1 << cf_node::wqm, 3, 0, 2).get_bytecode_byte(0),
         0x4040080000000003ul);

   TEST_EQ(cf_native_node(cf_tc_ack, 0, 3, 0, 2).get_bytecode_byte(0),
         0x06C0080000000003ul);

   TEST_EQ(cf_native_node(cf_vc, 1 << cf_node::eop, 10, 1, 5).get_bytecode_byte(0),
         0x00A014010000000Aul);

   TEST_EQ(cf_native_node(cf_vc_ack, 1 << cf_node::wqm, 10, 1, 5).get_bytecode_byte(0),
         0x470014010000000Aul);

   TEST_EQ(cf_native_node(cf_jump, 1 << cf_node::barrier, 20, 1).get_bytecode_byte(0),
         0x8280000100000014ul);

   TEST_EQ(cf_native_node(cf_jump, 1 << cf_node::barrier, 0xFFFFFF, 0).get_bytecode_byte(0),
         0x8280000000FFFFFFul);

   TEST_EQ(cf_native_node(cf_jump_table, 1 << cf_node::barrier, 256, 0, 0, 3).get_bytecode_byte(0),
         0x8740000003000100ul);

   TEST_EQ(cf_native_node(cf_gds, 1 << cf_node::barrier, 256, 0, 3, 0).get_bytecode_byte(0),
         0x80C00C0000000100ul);
}

TEST_F(BytecodeCFAluTest, BytecodeCreationAlu)
{
   TEST_EQ(cf_alu_node(cf_alu, 0, 2, 127).get_bytecode_byte(0),
         0x21FC000000000002ul);

   TEST_EQ(cf_alu_node(cf_alu, 1 << cf_node::alt_const, 2, 127).get_bytecode_byte(0),
         0x23FC000000000002ul);

   TEST_EQ(cf_alu_node(cf_alu, 1 << cf_node::wqm, 2, 127).get_bytecode_byte(0),
         0x61FC000000000002ul);

   TEST_EQ(cf_alu_node(cf_alu, 1 << cf_node::barrier, 2, 127).get_bytecode_byte(0),
         0xA1FC000000000002ul);

   TEST_EQ(cf_alu_node(cf_alu_else_after, 0, 0x3FFFFFu, 1).get_bytecode_byte(0),
         0x3C040000003FFFFFul);


   TEST_EQ(cf_alu_node(cf_alu, 0, 0x3u, 1,{15, 0, 0}).get_bytecode_byte(0),
         0x2004000003C00003ul);
   TEST_EQ(cf_alu_node(cf_alu, 0, 0x3u, 1,{0, 0, 0},{15, 0, 0}).get_bytecode_byte(0),
         0x200400003C000003ul);

   TEST_EQ(cf_alu_node(cf_alu, 0, 0x3u, 1,{0, 3, 0}).get_bytecode_byte(0),
         0x20040000C0000003ul);
   TEST_EQ(cf_alu_node(cf_alu, 0, 0x3u, 1,{0, 0, 0},{0, 3, 0}).get_bytecode_byte(0),
         0x2004000300000003ul);

   TEST_EQ(cf_alu_node(cf_alu, 0, 0x3u, 1,{0, 0, 255}).get_bytecode_byte(0),
         0x200403FC00000003ul);
   TEST_EQ(cf_alu_node(cf_alu, 0, 0x3u, 1,{0, 0, 0},{0, 0, 255}).get_bytecode_byte(0),
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
      TEST_EQ(cf_alu_node(x).get_bytecode_byte(0), x);
}

TEST_F(BytecodeCFAluTest, BytecodeCreationAluExtended)
{
   cf_alu_node ext0(cf_alu_extended, 0, 0x3u, 1, {1,0,0,0},
                    {0, 0, 0}, {0, 0, 255},
                    {0, 0, 0}, {0, 0, 0});

   TEST_EQ(ext0.get_bytecode_byte(1), 0x3007FC0000000003ul);
   TEST_EQ(ext0.get_bytecode_byte(0), 0x3000000000000010ul);

   cf_alu_node ext1(cf_alu_extended, 0, 0x3u, 1, {0,1,0,0},
                    {0, 0, 0}, {0, 0, 0},
                    {0, 0, 255}, {0, 0, 0});

   TEST_EQ(ext1.get_bytecode_byte(1), 0x3004000000000003ul);
   TEST_EQ(ext1.get_bytecode_byte(0), 0x300003FC00000040ul);

   cf_alu_node ext2(cf_alu_extended, 0, 0x3u, 1, {0,1,0,0},
                    {0, 0, 0}, {0, 0, 0},
                    {0, 0, 0}, {0, 0, 255});

   TEST_EQ(ext2.get_bytecode_byte(1), 0x3004000000000003ul);
   TEST_EQ(ext2.get_bytecode_byte(0), 0x3003FC0000000040ul);

   cf_alu_node ext3(cf_alu_extended, 0, 0x3u, 1, {0,0,1,0},
                    {0, 0, 0}, {0, 0, 0},
                    {15, 0, 0}, {0, 0, 0});

   TEST_EQ(ext3.get_bytecode_byte(1), 0x3004000000000003ul);
   TEST_EQ(ext3.get_bytecode_byte(0), 0x3000000003C00100ul);

   cf_alu_node ext4(cf_alu_extended, 0, 0x3u, 1, {0,0,0,1},
                    {0, 0, 0}, {0, 0, 0},
                    {0, 0, 0}, {15, 0, 0});

   TEST_EQ(ext4.get_bytecode_byte(1), 0x3004000000000003ul);
   TEST_EQ(ext4.get_bytecode_byte(0), 0x300000003C000400ul);

   cf_alu_node ext5(cf_alu_extended, 0, 0x3u, 1, {2,0,0,0},
                    {0, 0, 0}, {0, 0, 0},
                    {0, 3, 0}, {0, 0, 0});

   TEST_EQ(ext5.get_bytecode_byte(1), 0x3004000000000003ul);
   TEST_EQ(ext5.get_bytecode_byte(0), 0x30000000C0000020ul);

   cf_alu_node ext6(cf_alu_extended, 0, 0x3u, 1, {0,2,0,0},
                    {0, 0, 0}, {0, 0, 0},
                    {0, 0, 0}, {0, 3, 0});

   TEST_EQ(ext6.get_bytecode_byte(1), 0x3004000000000003ul);
   TEST_EQ(ext6.get_bytecode_byte(0), 0x3000000300000080ul);

   cf_alu_node ext7(cf_alu_extended, 0, 0x3u, 1, {0,0,2,0},
                    {0, 0, 0}, {0, 3, 0},
                    {0, 0, 0}, {0, 0, 0});

   TEST_EQ(ext7.get_bytecode_byte(1), 0x3004000300000003ul);
   TEST_EQ(ext7.get_bytecode_byte(0), 0x3000000000000200ul);

   cf_alu_node ext8(cf_alu_extended, 0, 0x3u, 1, {0,0,0,2},
                    {0, 0, 255}, {0, 3, 0},
                    {0, 0, 0}, {0, 0, 255});

   TEST_EQ(ext8.get_bytecode_byte(1), 0x300403FF00000003ul);
   TEST_EQ(ext8.get_bytecode_byte(0), 0x3003FC0000000800ul);
}

TEST_F(BytecodeCFGlobalWaveSync, gws)
{
   TEST_EQ(cf_gws_node(cf_global_wave_sync,
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

   TEST_EQ(cf_gws_node(cf_global_wave_sync,
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

   TEST_EQ(cf_gws_node(cf_global_wave_sync,
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

   TEST_EQ(cf_gws_node(cf_global_wave_sync,
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

   TEST_EQ(cf_gws_node(cf_global_wave_sync,
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

   TEST_EQ(cf_gws_node(cf_global_wave_sync,
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

   TEST_EQ(cf_gws_node(cf_global_wave_sync,
                       0 /* gws_opcode */,
                       1 << cf_node::sign /* flags */,
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
   TEST_EQ(cf_rat_node(cf_mem_rat,
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

   TEST_EQ(cf_rat_node(cf_mem_rat,
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

   TEST_EQ(cf_rat_node(cf_mem_rat,
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

   TEST_EQ(cf_rat_node(cf_mem_rat,
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

   TEST_EQ(cf_rat_node(cf_mem_rat,
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

   TEST_EQ(cf_rat_node(cf_mem_rat,
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

   TEST_EQ(cf_rat_node(cf_mem_rat,
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

   TEST_EQ(cf_rat_node(cf_mem_rat,
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

   TEST_EQ(cf_rat_node(cf_mem_rat,
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

   TEST_EQ(cf_rat_node(cf_mem_rat,
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

   TEST_EQ(cf_rat_node(cf_mem_rat,
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

   TEST_EQ(cf_rat_node(cf_mem_rat,
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

   TEST_EQ(cf_rat_node(cf_mem_rat,
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

   TEST_EQ(cf_rat_node(cf_mem_rat,
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
                       1 << cf_node::eop /* flags */).get_bytecode_byte(0),
           0x15a0000000000000ul);

   TEST_EQ(cf_rat_node(cf_mem_rat,
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
                       1 << cf_node::mark/* flags */).get_bytecode_byte(0),
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
      TEST_EQ(cf_rat_node(x).get_bytecode_byte(0), x);
}


