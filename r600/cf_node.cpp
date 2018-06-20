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

#include "cf_node.h"
#include <iostream>
#include <iomanip>
#include <cassert>

namespace r600 {

const uint64_t valid_pixel_mode_bit = 1ul << 52;
const uint64_t end_of_program_bit = 1ul << 53;
const uint64_t barrier_bit = 1ul << 63;
const uint64_t whole_quad_mode_bit = 1ul << 62;
const uint64_t mark_bit = 1ul << 62;
const uint64_t rw_rel_bit = 1ul << 22;
const uint64_t sign_bit = 1ul << 25;
const uint64_t alt_const_bit = 1ul << 57;

CFNode::CFNode(int bytecode_size, uint32_t opcode):
   node(bytecode_size),
   m_opcode(opcode),
   m_nesting_depth(0)
{
}

uint32_t CFNode::opcode() const
{
   return m_opcode;
}

bool CFNode::test_flag(int f) const
{
   return do_test_flag(f);
}

bool CFNode::do_test_flag(int f) const
{
   (void)f;
   return false;
}

void CFNode::set_nesting_depth(int nd)
{
   m_nesting_depth = nd;
}

int CFNode::get_nesting_depth() const
{
   return m_nesting_depth;
}

uint64_t CFNode::create_bytecode_byte(int i) const
{
   uint64_t result = static_cast<uint64_t>(m_opcode) << 54;
   encode_parts(i, result);
   return result;
}

void CFNode::print(std::ostream& os) const
{
   if (m_nesting_depth > 0)
      os << std::setw(4  * m_nesting_depth) << " ";
   os  << std::setw(22)
       << std::left << op_from_opcode(m_opcode);
   print_detail(os);
}

std::string CFNode::op_from_opcode(uint32_t opcode) const
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
   case cf_export: return "EXPORT";
   case cf_export_done: return "EXPORT_DONE";
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

CFNodeWithAddress::CFNodeWithAddress(unsigned bytecode_size,
                                     uint32_t opcode,
                                     uint32_t addresss):
   CFNode( bytecode_size, opcode),
   m_addr(addresss)

{
}

void CFNodeWithAddress::print_address(std::ostream& os) const
{
   os << " ADDR:" << m_addr;
}

uint32_t CFNodeWithAddress::address() const
{
   return m_addr;
}

uint32_t CFAluNode::get_alu_opcode(uint64_t bc)
{
   return (bc >> 54) & 0xF0;
}

uint32_t CFAluNode::get_alu_address(uint64_t bc)
{
   return bc  & 0x3FFFFF;
}

CFAluNode::CFAluNode(uint64_t bc, bool alu_ext):
   CFNodeWithAddress(alu_ext ? 2 : 1,
                     get_alu_opcode(bc),
                     get_alu_address(bc)),
   CFNodeFlags(bc),
   m_nkcache(alu_ext ? 4 : 2),
   m_count(((bc >> 50) & 0x7F) + 1)
{
   m_kcache_bank[0] = (bc >> 22) & 0xF;
   m_kcache_bank[1] = (bc >> 26) & 0xF;

   m_kcache_mode[0] = (bc >> 30) & 0x3;
   m_kcache_mode[1] = (bc >> 32) & 0x3;

   m_kcache_addr[0] = (bc >> 34) & 0xFF;
   m_kcache_addr[1] = (bc >> 42) & 0xFF;

   if (bc & alt_const_bit)
      set_flag(alt_const);
}

void CFAluNode::encode_parts(int i, uint64_t &bc) const
{
   assert( i == 0 || (((opcode() >> 4) == cf_alu_extended) && (i < 2)));

   if ((opcode() >> 4) == cf_alu_extended && i ==0) {
      for (int i = 0; i < 4; ++i) {
         bc |= static_cast<uint64_t>(m_kcache_bank_idx_mode[i])
               << (4 + 2*i);
      }
      bc |= static_cast<uint64_t>(m_kcache_bank[2]) << 22;
      bc |= static_cast<uint64_t>(m_kcache_bank[3]) << 26;
      bc |= static_cast<uint64_t>(m_kcache_mode[2]) << 30;
      bc |= static_cast<uint64_t>(m_kcache_mode[3]) << 32;
      bc |= static_cast<uint64_t>(m_kcache_addr[2]) << 34;
      bc |= static_cast<uint64_t>(m_kcache_addr[3]) << 42;
   } else {
      bc |= address();
      bc |= static_cast<uint64_t>(m_kcache_bank[0]) << 22;
      bc |= static_cast<uint64_t>(m_kcache_bank[1]) << 26;
      bc |= static_cast<uint64_t>(m_kcache_mode[0]) << 30;
      bc |= static_cast<uint64_t>(m_kcache_mode[1]) << 32;
      bc |= static_cast<uint64_t>(m_kcache_addr[0]) << 34;
      bc |= static_cast<uint64_t>(m_kcache_addr[1]) << 42;
      bc |= static_cast<uint64_t>(m_count - 1) << 50;
      encode_flags(bc);
   }
}

CFAluNode::CFAluNode(uint64_t bc):
   CFAluNode(bc, false)
{
}

CFAluNode::CFAluNode(uint64_t bc, uint64_t bc_ext):
   CFAluNode(bc, true)
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

CFAluNode::CFAluNode(uint16_t opcode,
                     const cf_flags& flags,
                     uint32_t addr,
                     uint16_t count,
                     const std::tuple<int, int,int>& kcache0,
                     const std::tuple<int, int,int>& kcache1
                     ):
   CFNodeWithAddress(1, opcode << 4, addr),
   CFNodeFlags(flags),
   m_count(count)
{
   assert(count > 0);
   m_kcache_bank[0] = std::get<0>(kcache0);
   m_kcache_mode[0] = std::get<1>(kcache0);
   m_kcache_addr[0] = std::get<2>(kcache0);

   m_kcache_bank[1] = std::get<0>(kcache1);
   m_kcache_mode[1] = std::get<1>(kcache1);
   m_kcache_addr[1] = std::get<2>(kcache1);
}

CFAluNode::CFAluNode(uint16_t opcode,
                     const cf_flags& flags,
                     uint32_t addr,
                     uint16_t count,
                     const std::vector<uint16_t>& mode,
                     const std::tuple<int,int,int>& kcache0,
                     const std::tuple<int,int,int>& kcache1,
                     const std::tuple<int,int,int>& kcache2,
                     const std::tuple<int,int,int>& kcache3):
   CFAluNode(opcode, flags, addr, count, kcache0, kcache1)
{
   assert(mode.size() == 4);
   std::copy(mode.begin(), mode.end(), &m_kcache_bank_idx_mode[0]);
   m_kcache_bank[2] = std::get<0>(kcache2);
   m_kcache_mode[2] = std::get<1>(kcache2);
   m_kcache_addr[2] = std::get<2>(kcache2);
   m_kcache_bank[3] = std::get<0>(kcache3);
   m_kcache_mode[3] = std::get<1>(kcache3);
   m_kcache_addr[3] = std::get<2>(kcache3);
}


std::string CFAluNode::op_from_opcode(uint32_t opcode) const
{
   switch (opcode >> 4) {
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

void CFAluNode::disassemble_clause(const std::vector<uint64_t>& bc)
{
   size_t ofs = address();
   size_t end = address() + m_count;

   while (ofs < end) {
      AluGroup g;
      ofs = g.decode(bc, ofs, end);
      m_clause_code.push_back(g);
   }
}

void CFAluNode::print_detail(std::ostream& os) const
{
   print_address(os);
   os << " COUNT:" << m_count;
   for (int i = 0; i < m_nkcache; ++i) {
      if (! (i & 1))
         os << "\n" << std::setw(4 * get_nesting_depth()) << " ";
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
         switch (m_kcache_bank_idx_mode[i]) {
         case 0: os << " none"; break;
         case 1: os << " IDX1"; break;
         case 2: os << " IDX2"; break;
         case 3: os << " INVALID"; break;
         }
      }
   }
   print_flags(os);
   os << "\n";
   for (const auto& g: m_clause_code)
      os << "\n" << g.as_string(4 * get_nesting_depth() +4);
}

CFNodeFlags::CFNodeFlags(uint64_t bc)
{
   if (bc & barrier_bit)
      m_flags.set(CFNode::barrier);

   if (bc & whole_quad_mode_bit)
      m_flags.set(CFNode::wqm);
}

bool CFNodeFlags::has_flag(int f) const
{
   return m_flags.test(f);
}

CFNodeFlags::CFNodeFlags(const cf_flags& flags):
   m_flags(flags)
{
}

void CFNodeFlags::set_flag(int flag)
{
   m_flags.set(flag);
}

void CFNodeFlags::encode_flags(uint64_t& bc) const
{
   if (m_flags.test(CFNode::vpm))
      bc |= valid_pixel_mode_bit;

   if (m_flags.test(CFNode::wqm))
      bc |= whole_quad_mode_bit;

   if (m_flags.test(CFNode::eop))
      bc |= end_of_program_bit;

   if (m_flags.test(CFNode::barrier))
      bc |= barrier_bit;

   if (m_flags.test(CFNode::sign))
      bc |= sign_bit;

   if (m_flags.test(CFNode::alt_const))
      bc |= alt_const_bit;

   if (m_flags.test(CFNode::mark))
      bc |= mark_bit;
}

void CFNodeFlags::print_flags(std::ostream& os) const
{
   if (m_flags.test(CFNode::vpm))
      os << " VPM";

   if (m_flags.test(CFNode::wqm))
      os << " WQM";

   if (m_flags.test(CFNode::eop))
      os << " EOP";

   if (m_flags.test(CFNode::barrier))
      os << " B";

   if (m_flags.test(CFNode::sign))
      os << " S";

   if (m_flags.test(CFNode::alt_const))
      os << " AC";
}

CFNodeCFWord1::CFNodeCFWord1(uint64_t word1):
   CFNodeFlags(word1),
   m_pop_count((word1 >> 32 )& 0x3),
   m_cf_const((word1 >> 34) & 0x1F),
   m_cond((word1 >> 40) & 0x3),
   m_count((word1 >> 42) & 0x3F)
{
   if (word1 & end_of_program_bit)
      set_flag(CFNode::eop);

   if (word1 & valid_pixel_mode_bit)
      set_flag(CFNode::vpm);
}

CFNodeCFWord1::CFNodeCFWord1(uint16_t pop_count,
                             uint16_t cf_const,
                             uint16_t cond,
                             uint16_t count,
                             const cf_flags& flags):
   CFNodeFlags(flags),
   m_pop_count(pop_count),
   m_cf_const(cf_const),
   m_cond(cond),
   m_count(count)
{
}

const char *CFNodeCFWord1::m_condition = "AFBN";

void CFNodeCFWord1::print(std::ostream& os) const
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
   print_flags(os);
}


uint64_t CFNodeCFWord1::encode() const
{
   uint64_t bc = 0;
   bc |= static_cast<uint64_t>(m_pop_count) << 32;
   bc |= static_cast<uint64_t>(m_cf_const) <<  34;
   bc |= static_cast<uint64_t>(m_cond) << 40;
   bc |= static_cast<uint64_t>(m_count) << 42;
   encode_flags(bc);
   return bc;
}

CFNativeNode::CFNativeNode(uint64_t bc):
   CFNodeWithAddress(1, get_opcode(bc), get_address(bc)),
   m_jumptable_se((bc >> 24) & 0x7),
   m_word1(bc)
{
}

CFNativeNode::CFNativeNode(uint16_t opcode,
                           const cf_flags& flags,
                           uint32_t address,
                           uint16_t pop_count,
                           uint16_t count,
                           uint16_t jts,
                           uint16_t cf_const,
                           uint16_t cond):
   CFNodeWithAddress(1, opcode, address),
   m_jumptable_se(jts),
   m_word1(pop_count, cf_const, cond, count, flags)
{
}

uint32_t CFNode::get_opcode(uint64_t bc)
{
   return (bc >> 54) & 0xFF;
}

bool CFNativeNode::do_test_flag(int f) const
{
   return m_word1.has_flag(f);
}

uint32_t CFNode::get_address(uint64_t bc)
{
   return bc  & 0xFFFFFF;
}

void CFNativeNode::encode_parts(int i, uint64_t& bc) const
{
   assert(i == 0);

   bc |= address();
   bc |= m_jumptable_se << 24;
   bc |= m_word1.encode();
}


const char CFNativeNode::m_jts_names[6][3] = {
   "CA", "CB", "CC", "CD", "I0", "I1"
};

void CFNativeNode::print_detail(std::ostream& os) const
{
   switch (opcode()) {
   case cf_jump_table:
      os << " JTS:" << m_jts_names[m_jumptable_se];
      /* fall through */
   case cf_call:
   case cf_call_fs:
   case cf_loop_start:
   case cf_loop_end:
   case cf_loop_break:
   case cf_loop_continue:
   case cf_loop_start_dx10:
   case cf_loop_start_no_al:
   case cf_jump:
   case cf_kill:
   case cf_gds:
   case cf_else:
   case cf_push:
   case cf_tc:
   case cf_vc:
      os << " ADDR:" << address();
      break;
   case cf_wait_ack:
      os << " WCNT:" << address();
      break;
   }

   m_word1.print(os);
}


CFGwsNode::CFGwsNode(uint64_t bc):
   CFNode(2, get_opcode(bc)),
   m_value(bc & 0x3FF),
   m_resource((bc >> 16) & 0x1F),
   m_val_index_mode((bc >> 26) & 0x3),
   m_rsrc_index_mode((bc >> 28) & 0x3),
   m_gws_opcode((bc >> 30) & 0x3),
   m_word1(bc)
{
}

CFNodeAllocExportWord1::
CFNodeAllocExportWord1(unsigned array_size,
                       unsigned comp_mask,
                       unsigned burst_count,
                       const cf_flags &flags):
   CFNodeFlags(flags),
   m_array_size(array_size),
   m_comp_mask(comp_mask),
   m_burst_count(burst_count)
{
}

void CFNodeAllocExportWord1::print(std::ostream& os) const
{
   os << "TODO: cf_node_alloc_export_word1\n";
}

uint64_t CFNodeAllocExportWord1::encode() const
{
   uint64_t bc = static_cast<uint64_t>(m_array_size) << 32;
   bc |= static_cast<uint64_t>(m_comp_mask) << 44;
   bc |= static_cast<uint64_t>(m_burst_count) << 48;
   encode_flags(bc);
   return bc;
}

const char *CFGwsNode::m_opcode_as_string[4] = {
   "SEMA_V", "SEMA_P", "BARRIER", "INIT"
};

const char *CFNode::m_index_mode_string = "N01_";


void CFGwsNode::print_detail(std::ostream& os) const
{
   os << m_opcode_as_string[m_gws_opcode] << " ";
   os << "V:" << m_value << " ";
   os << "SE:" << m_resource << " ";
   os << "VIDX:" << m_index_mode_string[m_val_index_mode] << " ";
   os << "RIDX:" << m_index_mode_string[m_rsrc_index_mode] << " ";
   m_word1.print(os);
}

CFGwsNode::CFGwsNode(uint32_t opcode,
                     short gws_opcode,
                     const cf_flags &flags,
                     uint16_t pop_count,
                     uint16_t cf_const,
                     uint16_t cond,
                     uint16_t count,
                     short value,
                     short resource,
                     short val_index_mode,
                     short rsrc_index_mode):
   CFNode(1, opcode),
   m_value(value),
   m_resource(resource),
   m_val_index_mode(val_index_mode),
   m_rsrc_index_mode(rsrc_index_mode),
   m_gws_opcode(gws_opcode),
   m_word1(pop_count, cf_const,cond, count, flags)
{
}

void CFGwsNode::encode_parts(int i, uint64_t &bc) const
{
   assert(i==0);
   bc |= m_word1.encode();
   bc |= m_value;
   bc |= m_resource << 16;
   bc |= m_val_index_mode << 26;
   bc |= m_rsrc_index_mode << 28;
   bc |= static_cast<uint64_t>(m_gws_opcode) << 30;
}

CFMemNode::CFMemNode(uint16_t opcode,
                     uint16_t type,
                     uint16_t rw_gpr,
                     uint16_t index_gpr,
                     uint16_t elem_size,
                     uint16_t burst_count,
                     const cf_flags& flags):
   CFNode(1, opcode),
   CFNodeFlags(flags),
   m_type(type),
   m_rw_gpr(rw_gpr),
   m_index_gpr(index_gpr),
   m_elem_size(elem_size),
   m_burst_count(burst_count)
{
}

CFMemNode::CFMemNode(uint64_t bc):
   CFNode(1, get_opcode(bc)),
   CFNodeFlags(bc),
   m_type((bc >> 13) & 0x3),
   m_rw_gpr((bc >> 15) & 0x7F),
   m_index_gpr((bc >> 23) & 0x7F),
   m_elem_size((bc >> 30) & 0x3),
   m_burst_count((bc >> 48) & 0xF)
{
   if (bc & valid_pixel_mode_bit)
      set_flag(CFNode::vpm);

   if (bc & mark_bit)
      set_flag(CFNode::mark);

   if (bc & rw_rel_bit)
      set_flag(CFNode::rw_rel);

   if (bc & end_of_program_bit)
      set_flag(CFNode::eop);
}

void CFMemNode::encode_parts(int i, uint64_t& bc) const
{
   assert(i == 0);

   bc |= m_type << 13;
   bc |= m_rw_gpr << 15;
   bc |= m_index_gpr << 23;
   bc |= static_cast<uint64_t>(m_elem_size) << 30;
   bc |= static_cast<uint64_t>(m_burst_count) << 48;

   encode_flags(bc);
   encode_mem_parts(bc);
}

void CFMemNode::print_detail(std::ostream& os) const
{
   os << " R" << m_rw_gpr;
   if (m_type & 1)
      os << "[R" << m_index_gpr << "]";

   print_mem_detail(os);
   print_elm_size(os);

   print_flags(os);
}

void CFMemNode::print_elm_size(std::ostream& os) const
{
   os << " ES:" << m_elem_size + 1;
   os << " BC:" << m_burst_count;
}

int CFMemNode::get_type() const
{
   return m_type;
}

int CFMemNode::get_burst_count() const
{
   return m_burst_count;
}

bool CFMemNode::is_type(types t) const
{
   return t == m_type;
}

CFMemCompNode::CFMemCompNode(uint64_t bc):
   CFMemNode(bc),
   m_array_size((bc >> 32) & 0xFFF),
   m_comp_mask((bc >> 44) & 0xf)
{
}

CFMemCompNode::CFMemCompNode(uint16_t opcode,
                             uint16_t type,
                             uint16_t rw_gpr,
                             uint16_t index_gpr,
                             uint16_t elem_size,
                             uint32_t array_size,
                             uint16_t comp_mask,
                             uint16_t burst_count,
                             const cf_flags &flags):
   CFMemNode(opcode, type, rw_gpr, index_gpr, elem_size,
             burst_count, flags),
   m_array_size(array_size),
   m_comp_mask(comp_mask)
{

}

void CFMemCompNode::print_mem_detail(std::ostream& os) const
{
   os << '.';
   for (int i = 0; i < 4; ++i) {
      if (m_comp_mask & 1 << i)
         os << component_names[i];
      else
         os << "_";
   }
   os << " ARR_SIZE:" << m_array_size;
   print_export_detail(os);
}

void CFMemCompNode::encode_mem_parts(uint64_t& bc) const
{
   bc |= static_cast<uint64_t>(m_array_size) << 32;
   bc |= static_cast<uint64_t>(m_comp_mask) << 44;
   encode_export_parts(bc);
}

CFRatNode::CFRatNode(uint64_t bc):
   CFMemCompNode(bc),
   m_rat_id(bc & 0xF),
   m_rat_inst((bc >> 4) & 0x3F),
   m_rat_index_mode((bc >> 11) & 0x3)
{
}

CFRatNode::CFRatNode(uint16_t opcode,
                     uint16_t rat_inst,
                     uint16_t rat_id,
                     uint16_t rat_index_mode,
                     uint16_t type,
                     uint16_t rw_gpr,
                     uint16_t index_gpr,
                     uint16_t elem_size,
                     uint32_t array_size,
                     uint16_t comp_mask,
                     uint16_t burst_count,
                     const cf_flags &flags):
   CFMemCompNode(opcode, type, rw_gpr, index_gpr, elem_size,
                 array_size, comp_mask, burst_count, flags),
   m_rat_id(rat_id),
   m_rat_inst(rat_inst),
   m_rat_index_mode(rat_index_mode)
{
}

const char *CFRatNode::m_type_string[4] = {
   "WRITE", "WRITE_IND", "WRITE_ACK", "WRITE_IND_ACK"
};

void CFRatNode::encode_export_parts(uint64_t &bc) const
{
   bc |= m_rat_id;
   bc |= m_rat_inst << 4;
   bc |= m_rat_index_mode << 11;
}


void CFRatNode::print_export_detail(std::ostream& os) const
{
   os << std::setw(23) << " " << rat_inst_string(m_rat_inst) << " ";
   os << "ID:" << m_rat_id << " ";
   os << "IDXM:" << m_index_mode_string[m_rat_index_mode] << " ";
   os << m_type_string[get_type()];
}

CFMemRingNode::CFMemRingNode(uint64_t bc):
   CFMemCompNode(bc),
   m_array_base(bc & 0x1FFF)
{
}

CFMemRingNode::CFMemRingNode(uint16_t opcode,
                             uint16_t type,
                             uint16_t rw_gpr,
                             uint16_t index_gpr,
                             uint16_t elem_size,
                             uint16_t array_size,
                             uint16_t array_base,
                             uint16_t comp_mask,
                             uint16_t burst_count,
                             const cf_flags &flags):
   CFMemCompNode(opcode, type, rw_gpr, index_gpr, elem_size,
                 array_size, comp_mask, burst_count, flags),
   m_array_base(array_base)
{
}

void CFMemRingNode::print_export_detail(std::ostream& os) const
{
   os << " ARR_BASE:" << m_array_base;
}

void CFMemRingNode::encode_export_parts(uint64_t &bc) const
{
   bc |= m_array_base;
}


CFMemExportNode::CFMemExportNode(uint64_t bc):
   CFMemNode(bc),
   m_array_base(bc & 0x1fff),
   m_sel(4)
{
   for (int i = 0; i < 4; ++i)
      m_sel[i] = (bc >> (32 + 3 * i)) & 0x7;
}

CFMemExportNode::CFMemExportNode(uint16_t opcode,
                                 uint16_t type,
                                 uint16_t rw_gpr,
                                 uint16_t index_gpr,
                                 uint16_t elem_size,
                                 uint16_t array_base,
                                 uint16_t burst_count,
                                 const std::vector<unsigned>& sel,
                                 const cf_flags &flags):
   CFMemNode(opcode, type, rw_gpr, index_gpr, elem_size,
             burst_count, flags),
   m_array_base(array_base),
   m_sel(sel)
{

}

void CFMemExportNode::print_mem_detail(std::ostream& os) const
{
   os << ".";
   for (int i = 0; i < 4; ++i) {
      os << component_names[m_sel[i]];
   }
   os << " ARR_BASE:" << m_array_base;
}

void CFMemExportNode::encode_mem_parts(uint64_t &bc) const
{
   bc |= m_array_base;
   for (int i = 0; i < 4; ++i)
      bc |= static_cast<uint64_t>(m_sel[i]) << (32 + 3 * i);
}

CFExportNode::CFExportNode(uint64_t bc):
   CFMemNode(bc),
   m_array_base(bc & 0x1fff),
   m_sel(4)
{
   for (int i = 0; i < 4; ++i)
      m_sel[i] = (bc >> (32 + 3 * i)) & 0x7;
}

CFExportNode::CFExportNode(uint16_t opcode,
                           uint16_t type,
                           uint16_t rw_gpr,
                           uint16_t index_gpr,
                           uint16_t array_base,
                           uint16_t burst_count,
                           const std::vector<unsigned>& sel,
                           const cf_flags &flags):
   CFMemNode(opcode, type, rw_gpr, index_gpr, 0,
             burst_count, flags),
   m_array_base(array_base),
   m_sel(sel)
{
}

const char *CFExportNode::m_type_string[4] = {
   "PIXEL", "POS", "PARAM", "undefined"
};

void CFExportNode::print_mem_detail(std::ostream& os) const
{
   os << ".";
   for (int i = 0; i < 4; ++i) {
      os << component_names[m_sel[i]];
   }

   os << " " << m_type_string[get_type()];


   int base = m_array_base;
   if (is_type(export_param))
      base += 60;

   os << base;

   int bc = get_burst_count();
   if (bc)
      os << "-" << base + bc;
}

void CFExportNode::encode_mem_parts(uint64_t &bc) const
{
   bc |= m_array_base;
   for (int i = 0; i < 4; ++i)
      bc |= static_cast<uint64_t>(m_sel[i]) << (32 + 3 * i);
}

void CFExportNode::print_elm_size(std::ostream& os) const
{
   (void)os;
}

const char *CFRatNode::rat_inst_string(int opcode) const
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

} // ns r600
