#include "cf_node.h"
#include <iostream>
#include <iomanip>

const uint64_t valid_pixel_mode_bit = 1ul << 20;
const uint64_t end_of_program_bit = 1ul << 21;
const uint64_t barrier_bit = 1ul << 63;
const uint64_t whole_quad_mode_bit = 1ul << 62;
const uint64_t mark_bit = 1ul << 62;

cf_node::cf_node(int bytecode_size, int opcode, bool barrier):
   node(bytecode_size),
   m_opcode(opcode),
   m_barrier(barrier)
{
}

uint32_t cf_node::opcode() const
{
   return m_opcode;
}

void cf_node::print(std::ostream& os) const
{
   os << std::setw(23) << std::left << op_from_opcode(m_opcode);
   if (m_barrier)
      os << "B";
   print_detail(os);
}

std::string cf_node::op_from_opcode(uint32_t opcode) const
{
   switch (opcode) {
   case cf_nop: return "NOP";
   case cf_tc: return "TC";
   case cf_vc: return "VC";
   case cf_gds: return "GDS";
   case cf_loop_start: return "LOOP_START";
   case cf_loop_end: return "LOOP_END";
   case cf_loop_start_dx10: return "LOOP_START_DX10";
   case cf_loop_start_no_al: return "LOOP_START_NO_AL";
   case cf_loop_continue: return "LOOP_CONTINUE";
   case cf_loop_break: return "LOOP_BREAK";
   case cf_jump: return "JUMP";
   case cf_push: return "PUSH";
   case cf_else: return "ELSE";
   case cf_pop: return "POP";
      /* 15 - 17 reserved */
   case cf_call: return "CALL";
   case cf_call_fs: return "CALL_FS";
   case cf_return: return "RETURN";
   case cf_emit_vertex: return "EMIT_VERTEX";
   case cf_emit_cut_vertex: return "EMIT_CUT_VERTEX";
   case cf_cut_vertex: return "CUT_VERTEX";
   case cf_kill: return "KILL";
      /* 25 reserved */
   case cf_wait_ack: return "WAIT_ACK";
   case cf_tc_ack: return "TC_ACK";
   case cf_vc_ack: return "VC_ACK";
   case cf_jump_table: return "JUMP_TABLE";
   case cf_global_wave_sync: return "GLOBAL_WAVE_SYNC";
   case cf_halt: return "HALT";
      /* gap 32-63*/
   case cf_mem_stream0_buf0: return "MEM_STREAM0_BUF0";
   case cf_mem_stream0_buf1: return "MEM_STREAM0_BUF1";
   case cf_mem_stream0_buf2: return "MEM_STREAM0_BUF2";
   case cf_mem_stream0_buf3: return "MEM_STREAM0_BUF3";

   case cf_mem_stream1_buf0: return "MEM_STREAM1_BUF0";
   case cf_mem_stream1_buf1: return "MEM_STREAM1_BUF1";
   case cf_mem_stream1_buf2: return "MEM_STREAM1_BUF2";
   case cf_mem_stream1_buf3: return "MEM_STREAM1_BUF3";

   case cf_mem_stream2_buf0: return "MEM_STREAM2_BUF0";
   case cf_mem_stream2_buf1: return "MEM_STREAM2_BUF1";
   case cf_mem_stream2_buf2: return "MEM_STREAM2_BUF2";
   case cf_mem_stream2_buf3: return "MEM_STREAM2_BUF3";

   case cf_mem_stream3_buf0: return "MEM_STREAM3_BUF0";
   case cf_mem_stream3_buf1: return "MEM_STREAM3_BUF1";
   case cf_mem_stream3_buf2: return "MEM_STREAM3_BUF2";
   case cf_mem_stream3_buf3: return "MEM_STREAM3_BUF3";

   case cf_mem_write_scratch: return "MEM_WRITE_SCRATCH";
      /* reserved 81 */
   case cf_mem_ring: return "MEM_RING";
      /* reserved 83 */
      /* reserved 84 */
   case cf_mem_export: return "MEM_EXPORT";
   case cf_mem_rat: return "MEM_RAT";
   case cf_mem_rat_cacheless: return "MEM_RAT_CACHELESS";

   case cf_mem_ring1: return "MEM_RING1";
   case cf_mem_ring2: return "MEM_RING2";
   case cf_mem_ring3: return "MEM_RING3";
   case cf_mem_export_combined: return "MEM_EXPORT_COMB";
   case cf_mem_rat_combined_cacheless: return "MEM_RAT_COMB_CACHELESS";

   default: return "UNKNOWN";
   }
}

cf_node_with_address::cf_node_with_address(unsigned bytecode_size,
                                           uint32_t opcode,
                                           bool barrier,
                                           uint32_t addresss):
   cf_node(bytecode_size, opcode, barrier),
   m_addr(addresss)

{
}

void cf_node_with_address::print_address(std::ostream& os) const
{
   os << "ADDR:" << m_addr;
}

uint32_t cf_alu_node::get_alu_opcode(uint64_t bc)
{
   return (bc >> 58) & 0xF;
}

uint32_t cf_alu_node::get_alu_address(uint64_t bc)
{
   return bc  & 0x3FFFFF;
}

cf_alu_node::cf_alu_node(uint64_t bc, bool alu_ext):
   cf_node_with_address(alu_ext ? 4 : 2,
                        get_alu_opcode(bc),
                        bc & barrier_bit,
                        get_alu_address(bc)),
   m_nkcache(alu_ext ? 4 : 2),
   m_count((bc >> 50) & 0x7F),
   m_alt_const(bc & (1 << 25)),
   m_whole_quad_mode(bc & whole_quad_mode_bit)
{
   m_kcache_bank[0] = (bc >> 22) & 0xF;
   m_kcache_bank[1] = (bc >> 26) & 0xF;

   m_kcache_mode[0] = (bc >> 30) & 0x3;
   m_kcache_mode[1] = (bc >> 32) & 0x3;

   m_kcache_addr[0] = (bc >> 34) & 0xFF;
   m_kcache_addr[1] = (bc >> 42) & 0xFF;

}

cf_alu_node::cf_alu_node(uint64_t bc):
   cf_alu_node(bc, false)
{
}

cf_alu_node::cf_alu_node(uint64_t bc, uint64_t bc_ext):
   cf_alu_node(bc, true)
{
   for (int i = 0; i < 4; ++i) {
      m_kcache_bank_idx_mode[i] = (bc_ext >> (4 + 2*i)) & 3;
   }
   m_kcache_bank[2] = (bc_ext >> 22) & 0xF;
   m_kcache_bank[3] = (bc_ext >> 26) & 0xF;
   m_kcache_mode[2] = (bc_ext >> 30) & 0x3;
   m_kcache_mode[3] = (bc_ext >> 32) & 0x3;
   m_kcache_addr[2] = (bc_ext >> 34) & 0xFF;
   m_kcache_addr[3] = (bc_ext >> 42) & 0xFF;
}

std::string cf_alu_node::op_from_opcode(uint32_t opcode) const
{
   switch (opcode) {
   case  8: return "ALU";
   case  9: return "ALU_PUSH_BEFORE";
   case 10: return "ALU_POP_AFTER";
   case 11: return "ALU_POP2_AFTER";
   case 12: return "ALU_EXTENDED";
   case 13: return "ALU_CONTINUE";
   case 14: return "ALU_BREAK";
   case 15: return "ALU_ELSE_AFTER";
   default:
      return "ALU_UNKNOWN";
   }
}

void cf_alu_node::print_detail(std::ostream& os) const
{
   print_address(os);
   os << " COUNT:" << m_count + 1;
   for (int i = 0; i < m_nkcache; ++i) {
      if (! (i & 1))
         os << "\n";
      os << "    KC" << i << ": " << m_kcache_bank[i]
            << "@0x" << std::setbase(16) << m_kcache_addr[i]
               << std::setbase(10);

      switch (m_kcache_mode[i]) {
      case 0: os << " nop"; break;
      case 1: os << " L1"; break;
      case 2: os << " L2"; break;
      case 3: os << " LLI"; break;
      }
      if (m_nkcache == 4) {
         switch (m_kcache_bank_idx_mode[0]) {
         case 0: os << " none"; break;
         case 1: os << " IDX1"; break;
         case 2: os << " IDX2"; break;
         case 3: os << " INVALID"; break;
         }
      }
   }
}

cf_node_cf_word1::cf_node_cf_word1(uint64_t word1):
   m_pop_count((word1 >> 32 )& 0x3),
   m_cf_const((word1 >> 34) & 0x1F),
   m_cond((word1 >> 40) & 0x3),
   m_count((word1 >> 42) & 0x3F),
   m_valid_pixel_mode(word1 & valid_pixel_mode_bit),
   m_end_of_program((word1 & end_of_program_bit) != 0),
   m_whole_quad_mode(word1 & whole_quad_mode_bit)
{
}

cf_native_node::cf_native_node(uint64_t bc):
   cf_node_with_address(2, get_opcode(bc),
                        bc & barrier_bit,
                        get_address(bc)),
   m_jumptable_se((bc >> 24) & 0x7),
   m_word1(bc)
{
}

uint32_t cf_node::get_opcode(uint64_t bc)
{
   return (bc >> 22) & 0xFF;
}

uint32_t cf_node::get_address(uint64_t bc)
{
   return bc  & 0xFFFFFF;
}

const char cf_native_node::m_jts_names[6][3] = {
   "CA", "CB", "CC", "CD", "I0", "I1"
};

void cf_native_node::print_detail(std::ostream& os) const
{
   if (opcode() == cf_jump_table)
      os << "JTS:" << m_jts_names[m_jumptable_se] << " ";
   m_word1.print(os);
}

const char *cf_node_cf_word1::m_condition = "AFBN";

void cf_node_cf_word1::print(std::ostream& os) const
{

   if (m_pop_count)
      os << " POP:" << m_pop_count;

   /* Figure out when it is actually used
    *
   if (m_condition[m_cond] > 1) {
      if (m_cf_const)
         os << " COND_CONST:" <<  m_cf_const;

      << " COND:" << m_condition[m_cond]
         << " CNT: " << m_count;
  */
   if (m_valid_pixel_mode)
      os << "VPM";

   if (m_whole_quad_mode)
      os << "WQM";

   if (m_end_of_program)
      os << "EOP";
}

cf_gws_node::cf_gws_node(uint64_t bc):
   cf_node(2, get_opcode(bc),
           bc & barrier_bit),
   m_value(bc & 0x3FF),
   m_resource((bc >> 16) & 0x1F),
   m_val_index_mode((bc >> 26) & 0x3),
   m_rsrc_index_mode((bc >> 28) & 0x3),
   m_gws_opcode((bc >> 30) & 0x3),
   m_word1(bc)
{
}

const char *cf_gws_node::m_opcode_as_string[4] = {
   "SEMA_V", "SEMA_P", "BARRIER", "INIT"
};

const char *cf_node::m_index_mode_string = "N01_";


void cf_gws_node::print_detail(std::ostream& os) const
{
   os << m_opcode_as_string[m_gws_opcode] << " ";
   os << "V:" << m_value << " ";
   os << "SE:" << m_resource << " ";
   os << "VIDX:" << m_index_mode_string[m_val_index_mode] << " ";
   os << "RIDX:" << m_index_mode_string[m_rsrc_index_mode] << " ";
   m_word1.print(os);
}

cf_mem_node::cf_mem_node(uint64_t bc):
   cf_node(2, get_opcode(bc), bc & barrier_bit),
   m_type((bc >> 13) & 0x3),
   m_rw_gpr((bc >> 15) & 0x7F),
   m_rw_rel(bc & ( 1 << 22)),
   m_index_gpr((bc >> 23) & 0x7F),
   m_elem_size((bc >> 30) & 0x3),
   m_burst_count((bc >> 48) & 0xF),
   m_valid_pixel_mode(bc & valid_pixel_mode_bit),
   m_mark(bc & mark_bit)
{
}

const char *cf_mem_node::m_type_string[4] = {
   "PIXEL", "POS", "PARAM", "undefined"
};


void cf_mem_node::print_mem_detail(std::ostream& os) const
{
   os << " ES:" << m_elem_size + 1 << " ";
   os << "BC:"  << m_burst_count << " ";

   if (m_rw_rel)
      os << "Loop-Rel";

   if (m_valid_pixel_mode)
      os << "VPM ";

   if (m_mark)
      os << "Req-ACK ";

   os << "R" << m_rw_gpr;
   if (m_type & 1)
      os << "[R" << m_index_gpr << "]";
}

cf_export_node::cf_export_node(uint64_t bc):
   cf_mem_node(bc),
   m_array_size((bc >> 32) & 0xFFF),
   m_comp_mask((bc >> 44) & 0xF),
   m_end_of_program(bc & end_of_program_bit)
{
}

cf_rat_node::cf_rat_node(uint64_t bc):
   cf_export_node(bc),
   m_rat_id(bc & 0xF),
   m_rat_inst((bc >> 4) & 0x3F),
   m_rat_index_mode((bc >> 11) & 0x3)
{
}

const char *cf_rat_node::m_type_string[4] = {
   "WRITE", "WRITE_IND", "WRITE_ACK", "WRITE_IND_ACK"
};


void cf_rat_node::print_detail(std::ostream& os) const
{
   os << std::setw(23) << rat_inst_string(m_rat_inst) << " ";
   os << "ID:" << m_rat_id << " ";
   os << "IDXM:" << m_index_mode_string[m_rat_index_mode] << " ";
   os << m_type_string[m_type] << " ";
   print_mem_detail(os);

   for (int i = 0; i < 4; ++i) {
      if (m_comp_mask & 1 << i)
         os << component_names[i];
      else
         os << "_";
   }

   if (m_end_of_program)
      os << "  EOP";
}

cf_export_mem_node::cf_export_mem_node(uint64_t bc):
   cf_export_node(bc),
   m_array_base(bc & 0x1FFF)
{
   for (int i = 0; i < 4; ++i)
      m_sel[i] = (bc >> (3*i + 32)) & 0x7;
}

void cf_export_mem_node::print_detail(std::ostream& os) const
{
   print_mem_detail(os);
   for (int i = 0; i < 4; ++i)
      os << component_names[m_sel[i]];

   if (m_end_of_program)
      os << "  EOP";
}

cf_mem_stream_node::cf_mem_stream_node(uint64_t bc):
   cf_mem_node(bc),
   m_array_base(bc & 0x1FFF)
{
}

void cf_mem_stream_node::print_detail(std::ostream& os) const
{
   print_mem_detail(os);
}

const char *cf_rat_node::rat_inst_string(int opcode) const
{
   switch (opcode) {
   case 0: return "NOP";
   case 1: return "STORE_TYPED";
   case 2: return "STORE_RAW";
   case 3: return "STORE_RAW_FDNORM";
   case 4: return "INT_CMPXCHG";
   case 5: return "FLT_CMPXCHG";
   case 6: return "FLT_CMPXCHG_DENORM";
   case 7: return "INT_ADD";
   case 8: return "INT_SUB";
   case 9: return "INT_RSUB";
   case 10: return "INT_MIN";
   case 11: return "UINT_MIN";
   case 12: return "INT_MAX";
   case 13: return "UINT_MAX";
   case 14: return "INT_AND";
   case 15: return "INT_OR";
   case 16: return "INT_XOR";
   case 17: return "INT_MSKOR";
   case 18: return "UINT_INC";
   case 19: return "UINT_DEC";
   case 32: return "NOP_RTN";
   case 34: return "XCHG_RTN_DWORD";
   case 35: return "XCHG_FDNORM_RTN_FLT";
   case 36: return "INT_CMPXCHG_RTN";
   case 37: return "FLT_CMPXCHG_RTN";
   case 38: return "FLT_CMPXCHG_FDNORM_RTN";
   case 39: return "INT_ADD_RTN";
   case 40: return "INT_SUB_RTN";
   case 41: return "INT_RSUB_RTN";
   case 42: return "INT_MIN_RTN";
   case 43: return "UINT_MIN_RTN";
   case 44: return "INT_MAX_RTN";
   case 45: return "UINT_MAX_RTN";
   case 46: return "INT_AND_RTN";
   case 47: return "INT_OR_RTN";
   case 48: return "INT_XOR_RTN";
   case 49: return "INT_MSKOR_RTN";
   case 50: return "UINT_INC_RTN";
   case 51: return "UINT_DEC_RTN";
   default: return "UNDEF";
   }
}