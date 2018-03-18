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

#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <r600/cf_node.h>

#include <vector>
#include <memory>
#include <cstdint>
#include <string>

class disassembler
{
public:
   disassembler(const std::vector<uint64_t> &bc);

   std::string as_string() const;
private:
   enum ECFNodeType {
      nt_cf_native,
      nt_cf_alu,
      nt_cf_export,
      nt_cf_mem_export,
      nt_cf_mem_rat,
      nt_cf_mem_ring,
      nt_cf_mem_scratch,
      nt_cf_mem_stream,
      nt_cf_unknown
   };

   bool require_two_quadwords(uint64_t bc);
   ECFNodeType get_cf_node_type(uint64_t bc);


   std::vector<cf_node::pointer> program;
};

#endif // DISASSEMBLER_H
