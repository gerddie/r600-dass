#include "cf_node.h"
#include <iostream>
#include <iomanip>

cf_node::cf_node(unsigned bytecode_size, uint32_t *bytecode, bool alu_node):
        node(bytecode_size),
        barrier(bytecode[1] & (1 << 31))
{
        if (alu_node)
                opcode = bytecode[1] >> 26 & 0xF;
        else
                opcode = bytecode[1] >> 22 & 0xFF;
}

void cf_node::print(std::ostream& os) const
{
        os << std::setw(23) << op_from_opcode(opcode);
        os << (barrier ? "B" : "_");
        print_detail(os);
}

cf_node_with_address::cf_node_with_address(unsigned bytecode_size, uint32_t *bytecode,
                               bool alu_node):
        cf_node(bytecode_size, bytecode, alu_node)
{
        addr = bytecode[0] & (alu_node ? 0x3FFFFF : 0xFFFFFF);
}

cf_alu_node::cf_alu_node(uint32_t *bytecode, bool alu_ext):
        cf_node_with_address(alu_ext ? 4 : 2, bytecode, true),
        nkcache(alu_ext ? 4 : 2),
        count((bytecode[1] >> 10) & 0x7F),
        alt_const(bytecode[1] & (1 << 25)),
        whole_quad_mode(bytecode[1] & (1 << 30))
{
        kcache_bank[0] = (bytecode[0] >> 22) & 0xF;
        kcache_mode[0] = (bytecode[0] >> 30) & 0x3;
        kcache_addr[0] = (bytecode[1] >>  2) & 0xFF;

        kcache_bank[1] = (bytecode[0] >> 26) & 0xF;
        kcache_mode[1] = bytecode[1] & 0x3;
        kcache_addr[1] = (bytecode[1] >> 10) & 0xFF;

        if (alu_ext) {
                for (int i = 0; i < 4; ++i) {
                        kcache_bank_idx_mode[i] = (bytecode[2] >> (4 + 2*i)) & 3;
                }
                kcache_bank[2] = (bytecode[2] >> 22) & 0xF;
                kcache_bank[3] = (bytecode[2] >> 26) & 0xF;
                kcache_mode[2] = (bytecode[2] >> 30) & 0x3;
                kcache_mode[3] = (bytecode[3] & 0x3);
                kcache_addr[2] = (bytecode[3] >> 2) & 0xFF;
                kcache_addr[3] = (bytecode[3] >> 10) & 0xFF;
        }
}

std::string cf_alu_node::op_from_opcode(uint32_t opcode) const
{
        switch (opcode) {
        case  8: return "ALU";
        case  9: return "ALU_PUSH_BEFORE";
        case 10: return "ALU_POP_AFTER";
        case 11: return "ALU_POP2_AFTER";
        case 12: return "ALU_EXTENDED";
        case 13: return "ALU_CONTINUE";
        case 14: return "ALU_BREAK";
        case 15: return "ALU_ELSE_AFTER";
        default:
                return "UNDEFINED";
        }
}

void cf_alu_node::print_detail(std::ostream& os) const
{
        for (int i = 0; i < nkcache; ++i) {
                os << "\n    KC" << i << ": " << kcache_bank[i]
                   << "@0x" << std::setbase(16) << kcache_addr[i]
                   << std::setbase(10);

                switch (kcache_mode[i]) {
                case 0: os << " NOP"; break;
                case 1: os << " L1"; break;
                case 2: os << " L2"; break;
                case 3: os << " LLI"; break;
                }
                if (nkcache == 4) {
                        switch (kcache_bank_idx_mode[0]) {
                        case 0: os << " NONE"; break;
                        case 1: os << " IDX1"; break;
                        case 2: os << " IDX2"; break;
                        case 3: os << " INVALID"; break;
                        }
                }
                os << "\n";
        }
}

cf_node_cf_word1::cf_node_cf_word1(uint32_t word1):
        pop_count(word1 & 0x3),
        cf_const((word1 >> 2) & 0x1F),
        cond((word1 >> 8) & 0x3),
        count((word1 >> 10) & 0x3F),
        valid_pixel_mode(word1 & (1 << 20)),
        end_of_program(word1 & (1 << 21)),
        whole_quad_mode(word1 & (1 << 30))
{
}

cf_native_node::cf_native_node(uint32_t *bytecode):
        cf_node_with_address(2, bytecode, false),
        jumptable_se((bytecode[0] >> 24) & 0x7),
        word1(bytecode[1])
{
}

std::string cf_native_node::cf_native_node::op_from_opcode(uint32_t opcode) const
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
        /* reserved 83 */
        /* reserved 84 */
        case cf_mem_export: return "MEM_EXPORT";
        case cf_mem_rat: return "MEM_RAT";
        case cf_mem_rat_cacheless: return "MEM_RAT_CACHELESS";

        case cf_mem_ring1: return "MEM_RING1";
        case cf_mem_ring2: return "MEM_RING2";
        case cf_mem_ring3: return "MEM_RING3";
        case cf_mem_export_combined: return "MEM_EXPORT_COMB";
        case cf_mem_rat_combined_cacheless: return "MEM_RAT_COMB_CACHELESS";

        default: return "UNSUPPORTED";
        }
}

const char cf_native_node::jts_names[6][3] = {
        "CA", "CB", "CC", "CD", "I0", "I1"
};

void cf_native_node::print_detail(std::ostream& os) const
{
        os << "JTS:" << jts_names[jumptable_se] << " ";
        word1.print(os);
}

const char *cf_node_cf_word1::condition = "AFBN";

void cf_node_cf_word1::print(std::ostream& os) const
{
        os << " POP:" << pop_count
           << " CONST:" <<  cf_const
           << " COND:" << condition[cond]
           << " CNT: " << count;

        if (valid_pixel_mode)
                os << "VPM";

        if (whole_quad_mode)
                os << "WQM";

        if (end_of_program)
                os << "EOP";
}

cf_gws_node::cf_gws_node(uint32_t *bytecode):
        cf_node(2, bytecode, false),
        value(bytecode[0] & 0x3FF),
        resource((bytecode[0] >> 16) & 0x1F),
        val_index_mode((bytecode[0] >> 26) & 0x3),
        rsrc_index_mode((bytecode[0] >> 28) & 0x3),
        gws_opcode((bytecode[0] >> 30) & 0x3),
        word1(bytecode[1])
{
}


cf_mem_node::cf_mem_node(uint32_t *bytecode):
        cf_node(2, bytecode, false),
        type((bytecode[0] >> 13) & 0x3),
        rw_gpr((bytecode[0] >> 15) & 0x7F),
        rw_rel(bytecode[0] & ( 1 << 22)),
        index_gpr((bytecode[0] >> 23) & 0x7F),
        elem_size((bytecode[0] >> 30) & 0x3),
        burst_count((bytecode[1] >> 16) & 0xF),
        valid_pixel_mode(bytecode[1] & (1 << 20)),
        mark(bytecode[1] & (1 << 30))
{
}

cf_export_node::cf_export_node(uint32_t *bytecode):
        cf_mem_node(bytecode),
        array_size(bytecode[1] & 0xFFF),
        comp_mask((bytecode[1] >> 12) & 0xF),
        end_of_program(bytecode[1] & (1 << 21))
{
}

cf_rat_node::cf_rat_node(uint32_t *bytecode):
        cf_export_node(bytecode),
        rat_id(bytecode[0] & 0xF),
        rat_inst((bytecode[0] >> 4) & 0x3F),
        rat_index_mode((bytecode[0] >> 11) & 0x3)
{
}

cf_export_mem_node::cf_export_mem_node(uint32_t *bytecode):
        cf_export_node(bytecode),
        array_base(bytecode[0] & 0x1FFF)
{
}

cf_mem_ring_node::cf_mem_ring_node(uint32_t *bytecode):
        cf_mem_node(bytecode),
        array_base(bytecode[0] & 0x1FFF),
        sel_x(bytecode[1] & 0x7),
        sel_y((bytecode[1] >> 3) & 0x7),
        sel_z((bytecode[1] >> 6) & 0x7),
        sel_w((bytecode[1] >> 9) & 0x7)
{
}

