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

#include "disassembler.h"
#include "defines.h"

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cassert>

namespace r600 {

using std::make_shared;
using std::vector;
using std::invalid_argument;
using std::ostringstream;

disassembler::disassembler(const vector<uint64_t>& bc)
{
   bool eop = false;
   auto i = bc.begin();

   cf_node::pointer cf_instr;

   while (i != bc.end() && !eop) {

      auto node_type = get_cf_node_type(*i);
      switch (node_type) {
      case nt_cf_alu:
         if (require_two_quadwords(*i)) {
            cf_instr = cf_node::pointer(new cf_alu_node(i[0],i[1]));
            ++i;
         } else {
            cf_instr = cf_node::pointer(new cf_alu_node(*i));
         }
         break;
      case nt_cf_native:
         cf_instr = cf_node::pointer(new cf_native_node(*i));
         break;
      case nt_cf_mem_scratch:
      case nt_cf_mem_stream:
      case nt_cf_mem_ring:
         cf_instr = cf_node::pointer(new cf_mem_ring_node(*i));
         break;
      case nt_cf_mem_rat:
         cf_instr = cf_node::pointer(new cf_rat_node(*i));
         break;
      case nt_cf_mem_export:
         cf_instr = cf_node::pointer(new cf_mem_export_node(*i));
         break;
      case nt_cf_export:
         cf_instr = cf_node::pointer(new cf_export_node(*i));
         break;
      default:
         std::cerr << std::setbase(16) << *i << ": unknown node type " <<
                      node_type << " encountered\n";
      }
      program.push_back(cf_instr);
      ++i;
   }
}

bool disassembler::require_two_quadwords(uint64_t bc)
{
   return ((bc >> 26) & 0xF) == cf_alu_extended;
}

disassembler::ECFNodeType
disassembler::get_cf_node_type(uint64_t bc)
{
   if (bc & 1ul << 61)
      return nt_cf_alu;

   int opcode = (bc >> 54) & 0xFF;

   if (opcode < 32)
      return nt_cf_native;

   if (opcode >= cf_mem_stream0_buf0 &&
       opcode <= cf_mem_stream3_buf3)
      return nt_cf_mem_stream;

   switch (opcode) {
   case cf_mem_write_scratch:
      return nt_cf_mem_scratch;
   case cf_mem_ring:
   case cf_mem_ring1:
   case cf_mem_ring2:
   case cf_mem_ring3:
      return nt_cf_mem_ring;
   case cf_mem_export:
   case cf_mem_export_combined:
      return nt_cf_mem_export;
   case cf_export:
   case cf_export_done:
      return nt_cf_export;
   case cf_mem_rat:
   case cf_mem_rat_cacheless:
   case cf_mem_rat_combined_cacheless:
      return nt_cf_mem_rat;
   default:
      return nt_cf_unknown;
   }
}

std::string disassembler::as_string() const
{
   ostringstream os;
   for (auto i: program) {
      os << *i << "\n";
   }
   return os.str();
}

} // ns r600
