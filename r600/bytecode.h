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

#ifndef BYTECODE_H
#define BYTECODE_H

#include <cstdint>
#include <set>
#include <bitset>

namespace r600 {

class bytecode
{
public:
   enum ENodeType {
      nt_cf,
      nt_alu,
      nt_vtx_fetch,
      nt_tex_fetch
   };

   bytecode(uint64_t bc);
   bytecode(uint64_t bc0, uint64_t bc1);

   enum ECFFlags {
      cff_barrier,
      cff_end_of_program,
      cff_valid_pixel_mode,
      cff_whole_quad_mode,
      cff_alt_constant,
      cff_rw_rel,
      cff_mark,
      cff_count
   };

   static bool require_two_quadwords(uint64_t bc);
private:

   ENodeType m_type;
   int16_t m_opcode;

   std::bitset<cff_count> m_flags;

   /* native */
   uint8_t m_pop_count;
   uint8_t m_cf_const;
   uint8_t m_cond;
   uint8_t m_count;

   /* alu */
   uint16_t m_nkcache;
   uint16_t m_kcache_bank_idx_mode[4];
   uint16_t m_kcache_bank[4];
   uint16_t m_kcache_mode[4];
   uint16_t m_kcache_addr[4];

   /* native */
   uint8_t m_jumptable_se;

   /* gws */
   uint16_t m_value;
   uint8_t m_resource;
   uint8_t m_val_index_mode;
   uint8_t m_rsrc_index_mode;
   uint8_t m_gws_opcode;

   /* mem */
   uint8_t m_type;
   int16_t m_mem_op;
   uint16_t m_rw_gpr;

   uint16_t m_index_gpr;
   uint8_t m_elem_size;
   uint8_t m_burst_count;

   uint16_t m_array_size;
   uint16_t m_array_base;
   uint8_t m_comp_mask;

   uint8_t m_rat_id;
   uint8_t m_rat_inst;
   uint8_t m_rat_index_mode;

   uint8_t m_sel[4];

};

}

#endif // BYTECODE_H
