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

#ifndef NODE_H
#define NODE_H

#include <cstdint>
#include <iosfwd>
#include <vector>

namespace r600 {

class node
{
public:
        node();
        virtual ~node();
        node(unsigned bytecode_size);
        int bytecode_size() const;

        friend std::ostream& operator << (std::ostream&os, const node& n);
        uint64_t get_bytecode_byte(int i) const;
        void append_bytecode(std::vector<uint64_t>& program) const;
protected:
        static const char *component_names;
private:
        unsigned m_bytecode_size;

        virtual void print(std::ostream& os) const = 0;
        virtual uint64_t create_bytecode_byte(int i) const = 0;
};

inline std::ostream& operator << (std::ostream&os, const node& n)
{
        n.print(os);
        return os;
}

} // ns r600

#endif // NODE_H
