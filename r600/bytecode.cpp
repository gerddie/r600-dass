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

#include <r600/bytecode.h>

#include <r600/defines.h>

bytecode::bytecode(uint64_t bc)
{
        if (bc & (1 << 29))
                read_alu
}


bytecode::bytecode(uint64_t bc0, uint64_t bc1):
        bytecode(bc0)
{

}

bool bytecode::require_two_quadwords(uint64_t bc)
{
        return ((bc >> 26) & 0xF) == cf_alu_extended;
}
