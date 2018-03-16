#include <r600/disassembler.h>

#include <gtest/gtest.h>

using std::vector;

class TestDisassember : public testing::Test {
protected:
   void run(const std::vector<uint64_t>& bc, const std::string& expect) const;

};

void TestDisassember::run(const std::vector<uint64_t>& bc,
                          const std::string& expect) const
{
   disassembler diss(bc);
   ASSERT_EQ(diss.as_string(), expect);
}

TEST_F(TestDisassember, NopEOP)
{
   vector<uint64_t> bc = {
      end_of_program_bit
   };

   run(bc, "NOP                    EOP\n");
}

TEST_F(TestDisassember, NopNopEOP)
{
   vector<uint64_t> bc = {
      0,
      end_of_program_bit
   };

   run(bc, "NOP                    \n"
           "NOP                    EOP\n"
       );
}

TEST_F(TestDisassember, AluNopEOP)
{
   vector<uint64_t> bc = {
      2ul | 1ul << 50 | 1ul << 61,
      end_of_program_bit
   };

   run(bc, "ALU                    ADDR:2 COUNT:2\n"
           "    KC0: 0@0x0 nop    KC1: 0@0x0 nop\n"
           "NOP                    EOP\n"
       );
}

TEST_F(TestDisassember, LoopEOP)
{
   cf_native_node loop_begin(cf_loop_start_dx10, 0, 1);
   cf_native_node loop_end(cf_loop_end, 0, 0);

   vector<uint64_t> bc;
   loop_begin.append_bytecode(bc);
   loop_end.append_bytecode(bc);
   bc.push_back(end_of_program_bit);

   run(bc, "LOOP_START_DX10        ADDR:1\n"
           "LOOP_END               ADDR:0\n"
           "NOP                    EOP\n"
       );
}

TEST_F(TestDisassember, JUMPElseNopEOP)
{
   vector<uint64_t> bc;
   cf_alu_node(cf_alu_push_before, 0, 6).append_bytecode(bc);
   cf_native_node(cf_jump, cf_node::barrier, 4).append_bytecode(bc);
   cf_alu_node(cf_alu, 0, 7).append_bytecode(bc);
   cf_native_node(cf_else, 0, 5, 1).append_bytecode(bc);
   cf_alu_node(cf_alu_pop_after, 0, 8, 0).append_bytecode(bc);
   cf_native_node(cf_nop, cf_node::eop).append_bytecode(bc);

   run(bc, "ALU_PUSH_BEFORE        ADDR:6 COUNT:1\n"
           "    KC0: 0@0x0 nop    KC1: 0@0x0 nop\n"
           "JUMP                   B ADDR:4\n"
           "ALU                    ADDR:7 COUNT:1\n"
           "    KC0: 0@0x0 nop    KC1: 0@0x0 nop\n"
           "ELSE                   ADDR:5 POP:1\n"
           "ALU_POP_AFTER          ADDR:8 COUNT:1\n"
           "    KC0: 0@0x0 nop    KC1: 0@0x0 nop\n"
           "NOP                    EOP\n"
       );
}

TEST_F(TestDisassember, LoopBreakEOP)
{
   vector<uint64_t> bc;
   cf_native_node(cf_loop_start_dx10, 0, 2).append_bytecode(bc);
   cf_native_node(cf_loop_break, 0, 2).append_bytecode(bc);
   cf_native_node(cf_loop_end, 0, 0).append_bytecode(bc);
   cf_native_node(cf_nop, cf_node::eop).append_bytecode(bc);

   run(bc, "LOOP_START_DX10        ADDR:2\n"
           "LOOP_BREAK             ADDR:2\n"
           "LOOP_END               ADDR:0\n"
           "NOP                    EOP\n"
       );
}

TEST_F(TestDisassember, LoopContinueEOP)
{
   vector<uint64_t> bc;
   cf_native_node(cf_loop_start_dx10, 0, 2).append_bytecode(bc);
   cf_native_node(cf_loop_continue, 0, 2).append_bytecode(bc);
   cf_native_node(cf_loop_end, 0, 0).append_bytecode(bc);
   cf_native_node(cf_nop, cf_node::eop).append_bytecode(bc);

   run(bc, "LOOP_START_DX10        ADDR:2\n"
           "LOOP_CONTINUE          ADDR:2\n"
           "LOOP_END               ADDR:0\n"
           "NOP                    EOP\n"
       );
}

TEST_F(TestDisassember, WriteScratchEop)
{
   vector<uint64_t> bc;
   cf_export_node(cf_mem_write_scratch, 0, 5, 0, 3,
                  2, 0xf, 0, cf_node::eop).append_bytecode(bc);

   run(bc, "MEM_WRITE_SCRATCH       ES:4 BC:0 R5.xyzw ARR_SIZE:2 EOP\n");
}

using BasicTest=testing::Test;


::testing::AssertionResult SameBitmap(const char* dummy,
                                      const char* m_expr,
                                      const char* n_expr,
                                      const uint8_t *spacing,
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

#define EXPECT_BITS_EQ(F, X, Y) \
   EXPECT_PRED_FORMAT3(SameBitmap, F, X, Y);


class BytecodeCFTest: public testing::Test {
protected:
   void do_check(const uint8_t spacing[], uint64_t data, uint64_t expect) const {
      EXPECT_BITS_EQ(spacing, data, expect);
   }
};

class BytecodeCFNativeTest: public BytecodeCFTest {
   static const uint8_t spaces[];
protected:
   void check(uint64_t data, uint64_t expect) const {
      do_check(spaces, data, expect);
   }
};

const uint8_t BytecodeCFNativeTest::spaces[] = {
     63, 62, 54, 53, 52, 48, 42, 40, 35, 32, 27, 24, 0
};

class BytecodeCFAluTest: public BytecodeCFTest {
   static const uint8_t spaces[];
protected:
   void check(uint64_t data, uint64_t expect) const {
      do_check(spaces, data, expect);
   }
};

const uint8_t BytecodeCFAluTest::spaces[] = {
   63, 62, 58, 57, 40, 42, 34, 32, 30, 26, 22, 0
};

TEST_F(BytecodeCFNativeTest, BytecodeCreationNative)
{
   check(cf_native_node(cf_nop, 0).get_bytecode_byte(0), 0);
   check(cf_native_node(cf_nop, cf_node::eop).get_bytecode_byte(0),
         end_of_program_bit);

   check(cf_native_node(cf_nop, cf_node::barrier).get_bytecode_byte(0),
         barrier_bit);

   check(cf_native_node(cf_nop, cf_node::wqm).get_bytecode_byte(0),
         whole_quad_mode_bit);

   check(cf_native_node(cf_nop, cf_node::vpm).get_bytecode_byte(0),
         valid_pixel_mode_bit);

   check(cf_native_node(cf_pop, 0, 0, 1).get_bytecode_byte(0),
         0x0380000100000000ul);

   check(cf_native_node(cf_pop, 0, 0, 7).get_bytecode_byte(0),
         0x0380000700000000ul);

   check(cf_native_node(cf_pop, cf_node::vpm).get_bytecode_byte(0),
         0x0390000000000000ul);

   check(cf_native_node(cf_push, cf_node::barrier).get_bytecode_byte(0),
         0x82C0000000000000ul);

   check(cf_native_node(cf_tc, cf_node::wqm, 3, 0, 2).get_bytecode_byte(0),
         0x4040080000000003ul);

   check(cf_native_node(cf_tc_ack, 0, 3, 0, 2).get_bytecode_byte(0),
         0x06C0080000000003ul);

   check(cf_native_node(cf_vc, cf_node::eop, 10, 1, 5).get_bytecode_byte(0),
         0x00A014010000000Aul);

   check(cf_native_node(cf_vc_ack, cf_node::wqm, 10, 1, 5).get_bytecode_byte(0),
         0x470014010000000Aul);

   check(cf_native_node(cf_jump, cf_node::barrier, 20, 1).get_bytecode_byte(0),
         0x8280000100000014ul);

   check(cf_native_node(cf_jump_table, cf_node::barrier, 256, 0, 0, 3).get_bytecode_byte(0),
         0x8740000003000100ul);

   check(cf_native_node(cf_gds, cf_node::barrier, 256, 0, 3, 0).get_bytecode_byte(0),
         0x80C00C0000000100ul);
}

TEST_F(BytecodeCFAluTest, BytecodeCreationAlu)
{
   check(cf_alu_node(cf_alu, 0, 2, 127).get_bytecode_byte(0),
         0x21FC000000000002ul);

   check(cf_alu_node(cf_alu_else_after, 0, 0x3FFFFFu, 1).get_bytecode_byte(0),
         0x3C040000003FFFFFul);
}