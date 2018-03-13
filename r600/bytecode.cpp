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
