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

