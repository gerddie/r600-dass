#ifndef DEFINES_H
#define DEFINES_H

enum EGWSOpCode {
        cf_sema_v = 0,
        cf_sema_p = 1,
        cf_gws_barrier = 2,
        cf_gws_init = 3,
};

/* ALU instructions [29:26], highest bit always set.
*/
enum ECFAluOpCode {
        cf_alu = 8,
        cf_alu_push_before = 9,
        cf_alu_pop_after = 10,
        cf_alu_pop2_after = 11,
        cf_alu_extended = 12,
        cf_alu_continue = 13,
        cf_alu_break = 14,
        cf_alu_else_after = 15,
};

enum ECFOpCode {
        cf_nop = 0,
        cf_tc = 1,
        cf_vc = 2,
        cf_gds = 3,
        cf_loop_start = 4,
        cf_loop_end = 5,
        cf_loop_start_dx10 = 6,
        cf_loop_start_no_al = 7,
        cf_loop_continue = 8,
        cf_loop_break = 9,
        cf_jump = 10,
        cf_push = 11,
        cf_else = 12,
        cf_pop = 13,
        /* 15 - 17 reserved */
        cf_call = 18,
        cf_call_fs = 19,
        cf_return = 20,
        cf_emit_vertex = 21,
        cf_emit_cut_vertex = 22,
        cf_cut_vertex = 23,
        cf_kill = 24,
        /* 25 reserved */
        cf_wait_ack = 26,
        cf_tc_ack = 27,
        cf_vc_ack = 28,
        cf_jump_table = 29,
        cf_global_wave_sync = 30,
        cf_halt = 31,
        /* gap 32-63*/
        cf_mem_stream0_buf0 = 64,
        cf_mem_stream0_buf1 = 65,
        cf_mem_stream0_buf2 = 66,
        cf_mem_stream0_buf3 = 67,

        cf_mem_stream1_buf0 = 68,
        cf_mem_stream1_buf1 = 69,
        cf_mem_stream1_buf2 = 70,
        cf_mem_stream1_buf3 = 71,

        cf_mem_stream2_buf0 = 72,
        cf_mem_stream2_buf1 = 73,
        cf_mem_stream2_buf2 = 74,
        cf_mem_stream2_buf3 = 75,

        cf_mem_stream3_buf0 = 76,
        cf_mem_stream3_buf1 = 77,
        cf_mem_stream3_buf2 = 78,
        cf_mem_stream3_buf3 = 79,

        cf_mem_write_scratch = 80,
        /* reserved 81 */
        cf_mem_ring = 82,
        cf_export = 83,
        cf_export_done = 84,
        cf_mem_export = 85,
        cf_mem_rat = 86,
        cf_mem_rat_cacheless = 87,

        cf_mem_ring1 = 88,
        cf_mem_ring2 = 89,
        cf_mem_ring3 = 90,
        cf_mem_export_combined = 91,
        cf_mem_rat_combined_cacheless = 92

};

#endif // DEFINES_H
