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