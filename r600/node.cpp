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

#include "node.h"
#include <cstdint>

namespace r600 {

node::node(unsigned bytecode_size):
        m_bytecode_size(bytecode_size)
{
}

node::node():node(0)
{
}

int node::bytecode_size() const
{
        return m_bytecode_size;
}

void node::append_bytecode(std::vector<uint64_t>& program) const
{
   for (int i = 0; i < m_bytecode_size; ++i)
      program.push_back(create_bytecode_byte(i));
}

uint64_t node::get_bytecode_byte(int i) const
{
   return create_bytecode_byte(i);
}

const char *node::component_names = "xyzw01?_";


} // ns r600
