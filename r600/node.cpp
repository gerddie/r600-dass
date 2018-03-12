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

const char *node::component_names = "xyzw01?_";