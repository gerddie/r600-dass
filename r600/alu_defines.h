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
 * along with R600-disass; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef r600_alu_defines_h
#define r600_alu_defines_h

#include <map>
#include <bitset>

#define DASS_UNUSED(X) (void)X

namespace r600 {

/* ALU op2 instructions 17:7 top three bits alwayss zero. */
enum EAluOp {
   op2_add = 0,
   op2_mul = 1,
   op2_mul_ieee = 2,
   op2_max = 3,
   op2_min = 4,
   op2_max_dx10 = 5,
   op2_min_dx10 = 6,
   op2_sete = 8,
   op2_setgt = 9,
   op2_setge = 10,
   op2_setne = 11,
   op2_sete_dx10 = 12,
   op2_setgt_dx10 = 13,
   op2_setge_dx10 = 14,
   op2_setne_dx10 = 15,
   op2_fract = 16,
   op2_trunc = 17,
   op2_ceil = 18,
   op2_rndne = 19,
   op2_floor = 20,
   op2_ashr_int = 21,
   op2_lshr_int = 22,
   op2_lshl_int = 23,
   op2_mov = 25,
   op2_nop = 26,
   op2_mul_64 = 27,
   op2_flt64_to_flt32 = 28,
   OP2V_FLT32_TO_FLT64 = 29,
   op2_pred_setgt_uint = 30,
   op2_pred_setge_uint = 31,
   op2_pred_sete = 32,
   op2_pred_setgt = 33,
   op2_pred_setge = 34,
   op2_pred_setne = 35,
   op2_pred_set_inv = 36,
   op2_pred_set_pop = 37,
   op2_pred_set_clr = 38,
   op2_pred_set_restore = 39,
   op2_pred_sete_push = 40,
   op2_pred_setgt_push = 41,
   op2_pred_setge_push = 42,
   op2_pred_setne_push = 43,
   op2_kille = 44,
   op2_killgt = 45,
   op2_killge = 46,
   op2_killne = 47,
   op2_and_int = 48,
   op2_or_int = 49,
   op2_xor_int = 50,
   op2_not_int = 51,
   op2_add_int = 52,
   op2_sub_int = 53,
   op2_max_int = 54,
   op2_min_int = 55,
   op2_max_uint = 56,
   op2_min_uint = 57,
   op2_sete_int = 58,
   op2_setgt_int = 59,
   op2_setge_int = 60,
   op2_setne_int = 61,
   op2_setgt_uint = 62,
   op2_setge_uint = 63,
   op2_killgt_uint = 64,
   op2_killge_uint = 65,
   op2_prede_int = 66,
   op2_pred_setgt_int = 67,
   op2_pred_setge_int = 68,
   op2_pred_setne_int = 69,
   op2_kille_int = 70,
   op2_killgt_int = 71,
   op2_killge_int = 72,
   op2_killne_int = 73,
   op2_pred_sete_push_int = 74,
   op2_pred_setgt_push_int = 75,
   op2_pred_setge_push_int = 76,
   op2_pred_setne_push_int = 77,
   op2_pred_setlt_push_int = 78,
   op2_pred_setle_push_int = 79,
   op2_flt_to_int = 80,
   op2_bfrev_int = 81,
   op2_addc_uint = 82,
   op2_subb_uint = 83,
   op2_group_barrier = 84,
   op2_group_seq_begin = 85,
   op2_group_seq_end = 86,
   op2_set_mode = 87,
   op2_set_cf_idx0 = 88,
   op2_set_cf_idx1 = 89,
   op2_set_lds_size = 90,
   op2_exp_ieee = 129,
   op2_log_clamped = 130,
   op2_log_ieee = 131,
   op2_recip_clamped = 132,
   op2_recip_ff = 133,
   op2_recip_ieee = 134,
   op2_recipsqrt_clamped = 135,
   op2_recipsqrt_ff = 136,
   op2_recipsqrt_ieee = 137,
   op2_sqrt_ieee = 138,
   op2_sin = 141,
   op2_cos = 142,
   op2_mullo_int = 143,
   op2_mulhi_int = 144,
   op2_mullo_uint = 145,
   op2_mulhi_uint = 146,
   op2_recip_int = 147,
   op2_recip_uint = 148,
   op2_recip_64 = 149,
   op2_recip_clamped_64 = 150,
   op2_recipsqrt_64 = 151,
   op2_recipsqrt_clamped_64 = 152,
   op2_sqrt_64 = 153,
   op2_flt_to_uint = 154,
   op2_int_to_flt = 155,
   op2_uint_to_flt = 156,
   op2_bfm_int = 160,
   op2_flt32_to_flt16 = 162,
   op2_flt16_to_flt32 = 163,
   op2_ubyte0_flt = 164,
   op2_ubyte1_flt = 165,
   op2_ubyte2_flt = 166,
   op2_ubyte3_flt = 167,
   op2_bcnt_int = 170,
   op2_ffbh_uint = 171,
   op2_ffbl_int = 172,
   op2_ffbh_int = 173,
   op2_flt_to_uint4 = 174,
   op2_dot_ieee = 175,
   op2_flt_to_int_rpi = 176,
   op2_flt_to_int_floor = 177,
   op2_mulhi_uint24 = 178,
   op2_mbcnt_32hi_int = 179,
   op2_offset_to_flt = 180,
   op2_mul_uint24 = 181,
   op2_bcnt_accum_prev_int = 182,
   op2_mbcnt_32lo_accum_prev_int = 183,
   op2_sete_64 = 184,
   op2_setne_64 = 185,
   op2_setgt_64 = 186,
   op2_setge_64 = 187,
   op2_min_64 = 188,
   op2_max_64 = 189,
   op2_dot4 = 190,
   op2_dot4_ieee = 191,
   op2_cube = 192,
   op2_max4 = 193,
   op2_frexp_64 = 196,
   op2_ldexp_64 = 197,
   op2_fract_64 = 198,
   op2_pred_setgt_64 = 199,
   op2_pred_sete_64 = 198,
   op2_pred_setge_64 = 201,
   OP2V_MUL_64 = 202,
   op2_add_64 = 203,
   op2_mova_int = 204,
   OP2V_FLT64_TO_FLT32 = 205,
   op2_flt32_to_flt64 = 206,
   op2_sad_accum_prev_uint = 207,
   op2_dot = 208,
   op2_mul_prev = 209,
   op2_mul_ieee_prev = 210,
   op2_add_prev = 211,
   op2_muladd_prev = 212,
   op2_muladd_ieee_prev = 213,
   op2_interp_xy = 214,
   op2_interp_zw = 215,
   op2_interp_x = 216,
   op2_interp_z = 217,
   op2_store_flags = 218,
   op2_load_store_flags = 219,
   op2_lds_1a = 220,
   op2_lds_1a1d = 221,
   op2_lds_2a = 223,
   op2_interp_load_p0 = 224,
   op2_interp_load_p10 = 125,
   op2_interp_load_p20 = 126,
   // op 3 all left shift 6
   op3_bfe_uint = 4<< 6,
   op3_bfe_int = 5<< 6,
   op3_bfi_int = 6<< 6,
   op3_fma = 7<< 6,
   op3_cndne_64 = 9<< 6,
   op3_fma_64 = 10<< 6,
   op3_lerp_uint = 11<< 6,
   op3_bit_align_int = 12<< 6,
   op3_byte_align_int = 13<< 6,
   op3_sad_accum_uint = 14<< 6,
   op3_sad_accum_hi_uint = 15<< 6,
   op3_muladd_uint24 = 16<< 6,
   op3_lds_idx_op = 17<< 6,
   op3_muladd = 20<< 6,
   op3_muladd_m2 = 21<< 6,
   op3_muladd_m4 = 22<< 6,
   op3_muladd_d2 = 23<< 6,
   op3_muladd_ieee = 24<< 6,
   op3_cnde = 25<< 6,
   op3_cndgt = 26<< 6,
   op3_cndge = 27<< 6,
   op3_cnde_int = 28<< 6,
   op3_cndgt_int = 29<< 6,
   op3_cndge_int = 30<< 6,
   op3_mul_lit = 31<< 6
};

using AluOpFlags=std::bitset<16>;

struct AluOp {
   static constexpr int x = 1;
   static constexpr int y = 2;
   static constexpr int z = 4;
   static constexpr int w = 8;
   static constexpr int v = 15;
   static constexpr int t = 16;
   static constexpr int a = 31;

   AluOp(int ns, int f, int um, const char *n):
      nsrc(ns), is_float(f), unit_mask(um), name(n)
   {
   }

   bool can_channel(int flags) const {
      return flags & unit_mask;
   }

   int nsrc: 4;
   int is_float:1;
   int unit_mask: 5;
   const char *name;
};

extern const std::map<EAluOp, AluOp> alu_ops;

enum AluInlineConstants  {
   ALU_SRC_LDS_OQ_A = 219,
   ALU_SRC_LDS_OQ_B = 220,
   ALU_SRC_LDS_OQ_A_POP = 221,
   ALU_SRC_LDS_OQ_B_POP = 222,
   ALU_SRC_LDS_DIRECT_A = 223,
   ALU_SRC_LDS_DIRECT_B = 224,
   ALU_SRC_TIME_HI = 227,
   ALU_SRC_TIME_LO = 228,
   ALU_SRC_MASK_HI = 229,
   ALU_SRC_MASK_LO = 230,
   ALU_SRC_HW_WAVE_ID = 231,
   ALU_SRC_SIMD_ID = 232,
   ALU_SRC_SE_ID = 233,
   ALU_SRC_HW_THREADGRP_ID = 234,
   ALU_SRC_WAVE_ID_IN_GRP = 235,
   ALU_SRC_NUM_THREADGRP_WAVES = 236,
   ALU_SRC_HW_ALU_ODD = 237,
   ALU_SRC_LOOP_IDX = 238,
   ALU_SRC_PARAM_BASE_ADDR = 240,
   ALU_SRC_NEW_PRIM_MASK = 241,
   ALU_SRC_PRIM_MASK_HI = 242,
   ALU_SRC_PRIM_MASK_LO = 243,
   ALU_SRC_1_DBL_L = 244,
   ALU_SRC_1_DBL_M = 245,
   ALU_SRC_0_5_DBL_L = 246,
   ALU_SRC_0_5_DBL_M = 247,
   ALU_SRC_0 = 248,
   ALU_SRC_1 = 249,
   ALU_SRC_1_INT = 250,
   ALU_SRC_M_1_INT = 251,
   ALU_SRC_0_5 = 252,
   ALU_SRC_LITERAL = 253,
   ALU_SRC_PV = 254,
   ALU_SRC_PS = 255,
   ALU_SRC_UNKNOWN
};

struct AluInlineConstantDescr {
   bool use_chan;
   const char *descr;
};

extern const std::map<AluInlineConstants, AluInlineConstantDescr> alu_src_const;

enum ESDOp {
   DS_OP_ADD = 0,
   DS_OP_SUB = 1,
   DS_OP_RSUB = 2,
   DS_OP_INC = 3,
   DS_OP_DEC = 4,
   DS_OP_MIN_INT = 5,
   DS_OP_MAX_INT = 6,
   DS_OP_MIN_UINT = 7,
   DS_OP_MAX_UINT = 8,
   DS_OP_AND = 9,
   DS_OP_OR = 10,
   DS_OP_XOR = 11,
   DS_OP_MSKOR = 12,
   DS_OP_WRITE = 13,
   DS_OP_WRITE_REL = 14,
   DS_OP_WRITE2 = 15,
   DS_OP_CMP_STORE = 16,
   DS_OP_CMP_STORE_SPF = 17,
   DS_OP_BYTE_WRITE = 18,
   DS_OP_SHORT_WRITE = 19,
   DS_OP_ADD_RET = 32,
   DS_OP_SUB_RET = 33,
   DS_OP_RSUB_RET = 34,
   DS_OP_INC_RET = 35,
   DS_OP_DEC_RET = 36,
   DS_OP_MIN_INT_RET = 37,
   DS_OP_MAX_INT_RET = 38,
   DS_OP_MIN_UINT_RET = 39,
   DS_OP_MAX_UINT_RET = 40,
   DS_OP_AND_RET = 41,
   DS_OP_OR_RET = 42,
   DS_OP_XOR_RET = 43,
   DS_OP_MSKOR_RET = 44,
   DS_OP_XCHG_RET = 45,
   DS_OP_XCHG_REL_RET = 46,
   DS_OP_XCHG2_RET = 47,
   DS_OP_CMP_XCHG_RET = 48,
   DS_OP_CMP_XCHG_SPF_RET = 49,
   DS_OP_READ_RET = 50,
   DS_OP_READ_REL_RET = 51,
   DS_OP_READ2_RET = 52,
   DS_OP_READWRITE_RET = 53,
   DS_OP_BYTE_READ_RET = 54,
   DS_OP_UBYTE_READ_RET = 55,
   DS_OP_SHORT_READ_RET = 56,
   DS_OP_USHORT_READ_RET = 57,
   DS_OP_ATOMIC_ORDERED_ALLOC_RET = 63
};

struct LDSOp {
   int nsrc;
   const char *name;
};

extern const std::map<ESDOp, LDSOp> lds_ops;

}

#endif // ALU_DEFINES_H
