#include "node.h"

#include <cstdint>

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