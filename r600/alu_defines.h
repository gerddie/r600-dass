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

namespace r600 {

/* ALU op2 instructions 17:7 top three bits alwayss zero. */
enum EAluOp {
   OP2_ADD = 0,
   OP2_MUL = 1,
   OP2_MUL_IEEE = 2,
   OP2_MAX = 3,
   OP2_MIN = 4,
   OP2_MAX_DX10 = 5,
   OP2_MIN_DX10 = 6,
   OP2_SETE = 8,
   OP2_SETGT = 9,
   OP2_SETGE = 10,
   OP2_SETNE = 11,
   OP2_SETE_DX10 = 12,
   OP2_SETGT_DX10 = 13,
   OP2_SETGE_DX10 = 14,
   OP2_SETNE_DX10 = 15,
   OP2_FRACT = 16,
   OP2_TRUNC = 17,
   OP2_CEIL = 18,
   OP2_RNDNE = 19,
   OP2_FLOOR = 20,
   OP2_ASHR_INT = 21,
   OP2_LSHR_INT = 22,
   OP2_LSHL_INT = 23,
   OP2_MOV = 25,
   OP2_NOP = 26,
   OP2_MUL_64 = 27,
   OP2_FLT64_TO_FLT32 = 28,
   OP2V_FLT32_TO_FLT64 = 29,
   OP2_PRED_SETGT_UINT = 30,
   OP2_PRED_SETGE_UINT = 31,
   OP2_PRED_SETE = 32,
   OP2_PRED_SETGT = 33,
   OP2_PRED_SETGE = 34,
   OP2_PRED_SETNE = 35,
   OP2_PRED_SET_INV = 36,
   OP2_PRED_SET_POP = 37,
   OP2_PRED_SET_CLR = 38,
   OP2_PRED_SET_RESTORE = 39,
   OP2_PRED_SETE_PUSH = 40,
   OP2_PRED_SETGT_PUSH = 41,
   OP2_PRED_SETGE_PUSH = 42,
   OP2_PRED_SETNE_PUSH = 43,
   OP2_KILLE = 44,
   OP2_KILLGT = 45,
   OP2_KILLGE = 46,
   OP2_KILLNE = 47,
   OP2_AND_INT = 48,
   OP2_OR_INT = 49,
   OP2_XOR_INT = 50,
   OP2_NOT_INT = 51,
   OP2_ADD_INT = 52,
   OP2_SUB_INT = 53,
   OP2_MAX_INT = 54,
   OP2_MIN_INT = 55,
   OP2_MAX_UINT = 56,
   OP2_MIN_UINT = 57,
   OP2_SETE_INT = 58,
   OP2_SETGT_INT = 59,
   OP2_SETGE_INT = 60,
   OP2_SETNE_INT = 61,
   OP2_SETGT_UINT = 62,
   OP2_SETGE_UINT = 63,
   OP2_KILLGT_UINT = 64,
   OP2_KILLGE_UINT = 65,
   OP2_PREDE_INT = 66,
   OP2_PRED_SETGT_INT = 67,
   OP2_PRED_SETGE_INT = 68,
   OP2_PRED_SETNE_INT = 69,
   OP2_KILLE_INT = 70,
   OP2_KILLGT_INT = 71,
   OP2_KILLGE_INT = 72,
   OP2_KILLNE_INT = 73,
   OP2_PRED_SETE_PUSH_INT = 74,
   OP2_PRED_SETGT_PUSH_INT = 75,
   OP2_PRED_SETGE_PUSH_INT = 76,
   OP2_PRED_SETNE_PUSH_INT = 77,
   OP2_PRED_SETLT_PUSH_INT = 78,
   OP2_PRED_SETLE_PUSH_INT = 79,
   OP2_FLT_TO_INT = 80,
   OP2_BFREV_INT = 81,
   OP2_ADDC_UINT = 82,
   OP2_SUBB_UINT = 83,
   OP2_GROUP_BARRIER = 84,
   OP2_GROUP_SEQ_BEGIN = 85,
   OP2_GROUP_SEQ_END = 86,
   OP2_SET_MODE = 87,
   OP2_SET_CF_IDX0 = 88,
   OP2_SET_CF_IDX1 = 89,
   OP2_SET_LDS_SIZE = 90,
   OP2_EXP_IEEE = 129,
   OP2_LOG_CLAMPED = 130,
   OP2_LOG_IEEE = 131,
   OP2_RECIP_CLAMPED = 132,
   OP2_RECIP_FF = 133,
   OP2_RECIP_IEEE = 134,
   OP2_RECIPSQRT_CLAMPED = 135,
   OP2_RECIPSQRT_FF = 136,
   OP2_RECIPSQRT_IEEE = 137,
   OP2_SQRT_IEEE = 138,
   OP2_SIN = 141,
   OP2_COS = 142,
   OP2_MULLO_INT = 143,
   OP2_MULHI_INT = 144,
   OP2_MULLO_UINT = 145,
   OP2_MULHI_UINT = 146,
   OP2_RECIP_INT = 147,
   OP2_RECIP_UINT = 148,
   OP2_RECIP_64 = 149,
   OP2_RECIP_CLAMPED_64 = 150,
   OP2_RECIPSQRT_64 = 151,
   OP2_RECIPSQRT_CLAMPED_64 = 152,
   OP2_SQRT_64 = 153,
   OP2_FLT_TO_UINT = 154,
   OP2_INT_TO_FLT = 155,
   OP2_UINT_TO_FLT = 156,
   OP2_BFM_INT = 160,
   OP2_FLT32_TO_FLT16 = 162,
   OP2_FLT16_TO_FLT32 = 163,
   OP2_UBYTE0_FLT = 164,
   OP2_UBYTE1_FLT = 165,
   OP2_UBYTE2_FLT = 166,
   OP2_UBYTE3_FLT = 167,
   OP2_BCNT_INT = 170,
   OP2_FFBH_UINT = 171,
   OP2_FFBL_INT = 172,
   OP2_FFBH_INT = 173,
   OP2_FLT_TO_UINT4 = 174,
   OP2_DOT_IEEE = 175,
   OP2_FLT_TO_INT_RPI = 176,
   OP2_FLT_TO_INT_FLOOR = 177,
   OP2_MULHI_UINT24 = 178,
   OP2_MBCNT_32HI_INT = 179,
   OP2_OFFSET_TO_FLT = 180,
   OP2_MUL_UINT24 = 181,
   OP2_BCNT_ACCUM_PREV_INT = 182,
   OP2_MBCNT_32LO_ACCUM_PREV_INT = 183,
   OP2_SETE_64 = 184,
   OP2_SETNE_64 = 185,
   OP2_SETGT_64 = 186,
   OP2_SETGE_64 = 187,
   OP2_MIN_64 = 188,
   OP2_MAX_64 = 189,
   OP2_DOT4 = 190,
   OP2_DOT4_IEEE = 191,
   OP2_CUBE = 192,
   OP2_MAX4 = 193,
   OP2_FREXP_64 = 196,
   OP2_LDEXP_64 = 197,
   OP2_FRACT_64 = 198,
   OP2_PRED_SETGT_64 = 199,
   OP2_PRED_SETE_64 = 198,
   OP2_PRED_SETGE_64 = 201,
   OP2V_MUL_64 = 202,
   OP2_ADD_64 = 203,
   OP2_MOVA_INT = 204,
   OP2V_FLT64_TO_FLT32 = 205,
   OP2_FLT32_TO_FLT64 = 206,
   OP2_SAD_ACCUM_PREV_UINT = 207,
   OP2_DOT = 208,
   OP2_MUL_PREV = 209,
   OP2_MUL_IEEE_PREV = 210,
   OP2_ADD_PREV = 211,
   OP2_MULADD_PREV = 212,
   OP2_MULADD_IEEE_PREV = 213,
   OP2_INTERP_XY = 214,
   OP2_INTERP_ZW = 215,
   OP2_INTERP_X = 216,
   OP2_INTERP_Z = 217,
   OP2_STORE_FLAGS = 218,
   OP2_LOAD_STORE_FLAGS = 219,
   OP2_LDS_1A = 220,
   OP2_LDS_1A1D = 221,
   OP2_LDS_2A = 223,
   OP2_INTERP_LOAD_P0 = 224,
   OP2_INTERP_LOAD_P10 = 125,
   OP2_INTERP_LOAD_P20 = 126,
   // op 3 all left shift 6
   OP3_INST_BFE_UINT = 4<< 6,
   OP3_INST_BFE_INT = 5<< 6,
   OP3_INST_BFI_INT = 6<< 6,
   OP3_INST_FMA = 7<< 6,
   OP3_INST_CNDNE_64 = 9<< 6,
   OP3_INST_FMA_64 = 10<< 6,
   OP3_INST_LERP_UINT = 11<< 6,
   OP3_INST_BIT_ALIGN_INT = 12<< 6,
   OP3_INST_BYTE_ALIGN_INT = 13<< 6,
   OP3_INST_SAD_ACCUM_UINT = 14<< 6,
   OP3_INST_SAD_ACCUM_HI_UINT = 15<< 6,
   OP3_INST_MULADD_UINT24 = 16<< 6,
   OP3_INST_LDS_IDX_OP = 17<< 6,
   OP3_INST_MULADD = 20<< 6,
   OP3_INST_MULADD_M2 = 21<< 6,
   OP3_INST_MULADD_M4 = 22<< 6,
   OP3_INST_MULADD_D2 = 23<< 6,
   OP3_INST_MULADD_IEEE = 24<< 6,
   OP3_INST_CNDE = 25<< 6,
   OP3_INST_CNDGT = 26<< 6,
   OP3_INST_CNDGE = 27<< 6,
   OP3_INST_CNDE_INT = 28<< 6,
   OP3_INST_CNDGT_INT = 29<< 6,
   OP3_INST_CNDGE_INT = 30<< 6,
   OP3_INST_MUL_LIT = 31<< 6
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

   AluOp(int ns, int um, const char *n):
      nsrc(ns), unit_mask(um), name(n)
   {
   }

   bool can_channel(int flags) const {
      return flags & unit_mask;
   }

   int nsrc: 4;
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
   ALU_SRC_PS = 255
};

struct AluInlineConstantDescr {
   bool use_chan;
   const char *descr;
};

extern const std::map<AluInlineConstants, AluInlineConstantDescr> alu_src_const;

enum ELSDIndexOp {
   LDS_OP_ADD = 0,
   LDS_OP_SUB = 1,
   LDS_OP_RSUB = 2,
   LDS_OP_INC = 3,
   LDS_OP_DEC = 4,
   LDS_OP_MIN_INT = 5,
   LDS_OP_MAX_INT = 6,
   LDS_OP_MIN_UINT = 7,
   LDS_OP_MAX_UINT = 8,
   LDS_OP_AND = 9,
   LDS_OP_OR = 10,
   LDS_OP_XOR = 11,
   LDS_OP_MSKOR = 12,
   LDS_OP_WRITE = 13,
   LDS_OP_WRITE_REL = 14,
   LDS_OP_WRITE2 = 15,
   LDS_OP_CMP_STORE = 16,
   LDS_OP_CMP_STORE_SPF = 17,
   LDS_OP_BYTE_WRITE = 18,
   LDS_OP_SHORT_WRITE = 19,
   LDS_OP_ADD_RET = 32,
   LDS_OP_SUB_RET = 33,
   LDS_OP_RSUB_RET = 34,
   LDS_OP_INC_RET = 35,
   LDS_OP_DEC_RET = 36,
   LDS_OP_MIN_INT_RET = 37,
   LDS_OP_MAX_INT_RET = 38,
   LDS_OP_MIN_UINT_RET = 39,
   LDS_OP_MAX_UINT_RET = 40,
   LDS_OP_AND_RET = 41,
   LDS_OP_OR_RET = 42,
   LDS_OP_XOR_RET = 43,
   LDS_OP_MSKOR_RET = 44,
   LDS_OP_XCHG_RET = 45,
   LDS_OP_XCHG_REL_RET = 46,
   LDS_OP_XCHG2_RET = 47,
   LDS_OP_CMP_XCHG_RET = 48,
   LDS_OP_CMP_XCHG_SPF_RET = 49,
   LDS_OP_READ_RET = 50,
   LDS_OP_READ_REL_RET = 51,
   LDS_OP_READ2_RET = 52,
   LDS_OP_READWRITE_RET = 53,
   LDS_OP_BYTE_READ_RET = 54,
   LDS_OP_UBYTE_READ_RET = 55,
   LDS_OP_SHORT_READ_RET = 56,
   LDS_OP_USHORT_READ_RET = 57,
   LDS_OP_ATOMIC_ORDERED_ALLOC_RET = 63
};

struct LDSOp {
   int nsrc;
   const char *name;
};

extern const std::map<ELSDIndexOp, LDSOp> lds_ops;

}

#endif // ALU_DEFINES_H
