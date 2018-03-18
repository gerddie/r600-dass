

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

   run(bc, "NOP                   \n"
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
   cf_native_node(cf_jump, 1 << cf_node::barrier, 4).append_bytecode(bc);
   cf_alu_node(cf_alu, 0, 7).append_bytecode(bc);
   cf_native_node(cf_else, 0, 5, 1).append_bytecode(bc);
   cf_alu_node(cf_alu_pop_after, 0, 8, 0).append_bytecode(bc);
   cf_native_node(cf_nop, 1 << cf_node::eop).append_bytecode(bc);

   run(bc, "ALU_PUSH_BEFORE        ADDR:6 COUNT:1\n"
           "    KC0: 0@0x0 nop    KC1: 0@0x0 nop\n"
           "JUMP                   ADDR:4 B\n"
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
   cf_native_node(cf_nop, 1 << cf_node::eop).append_bytecode(bc);

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
   cf_native_node(cf_nop, 1 << cf_node::eop).append_bytecode(bc);

   run(bc, "LOOP_START_DX10        ADDR:2\n"
           "LOOP_CONTINUE          ADDR:2\n"
           "LOOP_END               ADDR:0\n"
           "NOP                    EOP\n"
       );
}

TEST_F(TestDisassember, WriteScratchEop)
{
   vector<uint64_t> bc;

   cf_mem_ring_node(cf_mem_write_scratch,
                    0,
                    5,
                    0,
                    3,
                    3,
                    4,
                    0xf,
                    2,
                    1 << cf_node::eop).append_bytecode(bc);
   run(bc, "MEM_WRITE_SCRATCH      R5.xyzw ARR_SIZE:3 ARR_BASE:4 ES:4 BC:2 EOP\n");
}

TEST_F(TestDisassember, AllCFOpsEOP)
{
   vector<uint64_t> bc = {
      static_cast<uint64_t>(cf_nop) << 54,
      static_cast<uint64_t>(cf_tc) << 54,
      static_cast<uint64_t>(cf_vc) << 54,
      static_cast<uint64_t>(cf_gds) << 54,
      static_cast<uint64_t>(cf_loop_start) << 54,
      static_cast<uint64_t>(cf_loop_end) << 54,
      static_cast<uint64_t>(cf_loop_start_dx10) << 54,
      static_cast<uint64_t>(cf_loop_start_no_al) << 54,
      static_cast<uint64_t>(cf_loop_continue ) << 54,
      static_cast<uint64_t>(cf_loop_break ) << 54,
      static_cast<uint64_t>(cf_jump ) << 54,
      static_cast<uint64_t>(cf_push ) << 54,
      static_cast<uint64_t>(cf_else ) << 54,
      static_cast<uint64_t>(cf_pop ) << 54,
      static_cast<uint64_t>(cf_call ) << 54,
      static_cast<uint64_t>(cf_call_fs ) << 54,
      static_cast<uint64_t>(cf_return ) << 54,
      static_cast<uint64_t>(cf_emit_vertex ) << 54,
      static_cast<uint64_t>(cf_emit_cut_vertex ) << 54,
      static_cast<uint64_t>(cf_cut_vertex ) << 54,
      static_cast<uint64_t>(cf_kill ) << 54,
      static_cast<uint64_t>(cf_wait_ack ) << 54,
      static_cast<uint64_t>(cf_tc_ack ) << 54,
      static_cast<uint64_t>(cf_vc_ack ) << 54,
      static_cast<uint64_t>(cf_jump_table ) << 54,
      static_cast<uint64_t>(cf_global_wave_sync ) << 54,
      static_cast<uint64_t>(cf_halt ) << 54,
      static_cast<uint64_t>(cf_mem_stream0_buf0 ) << 54,
      static_cast<uint64_t>(cf_mem_stream0_buf1 ) << 54,
      static_cast<uint64_t>(cf_mem_stream0_buf2 ) << 54,
      static_cast<uint64_t>(cf_mem_stream0_buf3 ) << 54,
      static_cast<uint64_t>(cf_mem_stream1_buf0 ) << 54,
      static_cast<uint64_t>(cf_mem_stream1_buf1 ) << 54,
      static_cast<uint64_t>(cf_mem_stream1_buf2 ) << 54,
      static_cast<uint64_t>(cf_mem_stream1_buf3 ) << 54,
      static_cast<uint64_t>(cf_mem_stream2_buf0 ) << 54,
      static_cast<uint64_t>(cf_mem_stream2_buf1 ) << 54,
      static_cast<uint64_t>(cf_mem_stream2_buf2 ) << 54,
      static_cast<uint64_t>(cf_mem_stream2_buf3 ) << 54,
      static_cast<uint64_t>(cf_mem_stream3_buf0 ) << 54,
      static_cast<uint64_t>(cf_mem_stream3_buf1 ) << 54,
      static_cast<uint64_t>(cf_mem_stream3_buf2 ) << 54,
      static_cast<uint64_t>(cf_mem_stream3_buf3 ) << 54,
      static_cast<uint64_t>(cf_mem_write_scratch ) << 54,
      static_cast<uint64_t>(cf_mem_ring ) << 54,
      static_cast<uint64_t>(cf_export ) << 54,
      static_cast<uint64_t>(cf_export_done ) << 54,
      static_cast<uint64_t>(cf_mem_export ) << 54,
      static_cast<uint64_t>(cf_mem_rat ) << 54,
      static_cast<uint64_t>(cf_mem_rat_cacheless ) << 54,
      static_cast<uint64_t>(cf_mem_ring1) << 54,
      static_cast<uint64_t>(cf_mem_ring2) << 54,
      static_cast<uint64_t>(cf_mem_ring3) << 54,
      static_cast<uint64_t>(cf_mem_export_combined ) << 54,
      static_cast<uint64_t>(cf_mem_rat_combined_cacheless ) << 54
   };

   run(bc,
       "NOP                   \n"
       "TC                     ADDR:0\n"
       "VC                     ADDR:0\n"
       "GDS                    ADDR:0\n"
       "LOOP_START             ADDR:0\n"
       "LOOP_END               ADDR:0\n"
       "LOOP_START_DX10        ADDR:0\n"
       "LOOP_START_NO_AL       ADDR:0\n"
       "LOOP_CONTINUE          ADDR:0\n"
       "LOOP_BREAK             ADDR:0\n"
       "JUMP                   ADDR:0\n"
       "PUSH                   ADDR:0\n"
       "ELSE                   ADDR:0\n"
       "POP                   \n"
          /* 15 - 17 reserved */
       "CALL                   ADDR:0\n"
       "CALL_FS                ADDR:0\n"
       "RETURN                \n"
       "EMIT_VERTEX           \n"
       "EMIT_CUT_VERTEX       \n"
       "CUT_VERTEX            \n"
       "KILL                   ADDR:0\n"
          /* 25 reserved */
       "WAIT_ACK               WCNT:0\n"
       "TC_ACK                \n"
       "VC_ACK                \n"
       "JUMP_TABLE             JTS:CA ADDR:0\n"
       "GLOBAL_WAVE_SYNC      \n"
       "HALT                  \n"
          /* gap 32-63*/
       "MEM_STREAM0_BUF0       R0.____ ARR_SIZE:0 ARR_BASE:0 ES:1 BC:0\n"
       "MEM_STREAM0_BUF1       R0.____ ARR_SIZE:0 ARR_BASE:0 ES:1 BC:0\n"
       "MEM_STREAM0_BUF2       R0.____ ARR_SIZE:0 ARR_BASE:0 ES:1 BC:0\n"
       "MEM_STREAM0_BUF3       R0.____ ARR_SIZE:0 ARR_BASE:0 ES:1 BC:0\n"

       "MEM_STREAM1_BUF0       R0.____ ARR_SIZE:0 ARR_BASE:0 ES:1 BC:0\n"
       "MEM_STREAM1_BUF1       R0.____ ARR_SIZE:0 ARR_BASE:0 ES:1 BC:0\n"
       "MEM_STREAM1_BUF2       R0.____ ARR_SIZE:0 ARR_BASE:0 ES:1 BC:0\n"
       "MEM_STREAM1_BUF3       R0.____ ARR_SIZE:0 ARR_BASE:0 ES:1 BC:0\n"

       "MEM_STREAM2_BUF0       R0.____ ARR_SIZE:0 ARR_BASE:0 ES:1 BC:0\n"
       "MEM_STREAM2_BUF1       R0.____ ARR_SIZE:0 ARR_BASE:0 ES:1 BC:0\n"
       "MEM_STREAM2_BUF2       R0.____ ARR_SIZE:0 ARR_BASE:0 ES:1 BC:0\n"
       "MEM_STREAM2_BUF3       R0.____ ARR_SIZE:0 ARR_BASE:0 ES:1 BC:0\n"

       "MEM_STREAM3_BUF0       R0.____ ARR_SIZE:0 ARR_BASE:0 ES:1 BC:0\n"
       "MEM_STREAM3_BUF1       R0.____ ARR_SIZE:0 ARR_BASE:0 ES:1 BC:0\n"
       "MEM_STREAM3_BUF2       R0.____ ARR_SIZE:0 ARR_BASE:0 ES:1 BC:0\n"
       "MEM_STREAM3_BUF3       R0.____ ARR_SIZE:0 ARR_BASE:0 ES:1 BC:0\n"

       "MEM_WRITE_SCRATCH      R0.____ ARR_SIZE:0 ARR_BASE:0 ES:1 BC:0\n"
          /* reserved 81 */
       "MEM_RING               R0.____ ARR_SIZE:0 ARR_BASE:0 ES:1 BC:0\n"
       "EXPORT                 R0.xxxx PIXEL0\n"
       "EXPORT_DONE            R0.xxxx PIXEL0\n"
       "MEM_EXPORT             R0.xxxx ARR_BASE:0 ES:1 BC:0\n"
       "MEM_RAT                R0.____ ARR_SIZE:0                       NOP ID:0 IDXM:N WRITE ES:1 BC:0\n"
       "MEM_RAT_CACHELESS      R0.____ ARR_SIZE:0                       NOP ID:0 IDXM:N WRITE ES:1 BC:0\n"

       "MEM_RING1              R0.____ ARR_SIZE:0 ARR_BASE:0 ES:1 BC:0\n"
       "MEM_RING2              R0.____ ARR_SIZE:0 ARR_BASE:0 ES:1 BC:0\n"
       "MEM_RING3              R0.____ ARR_SIZE:0 ARR_BASE:0 ES:1 BC:0\n"
       "MEM_EXPORT_COMB        R0.xxxx ARR_BASE:0 ES:1 BC:0\n"
       "MEM_RAT_COMB_CACHELESS R0.____ ARR_SIZE:0                       NOP ID:0 IDXM:N WRITE ES:1 BC:0\n"
       );
}


