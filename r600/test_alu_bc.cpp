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

#include <r600/bc_test.h>
#include <r600/alu_node.h>
#include <gtest/gtest.h>
#include <vector>

using namespace r600;
using std::vector;

class BytecodeAluOpTest: public BytecodeTest {
protected:
   void CreateRegisters();
   Value::LiteralFlags li;
   PValue src;
   GPRValue dst;
   AluOpFlags empty_flags;
};

void BytecodeAluOpTest::CreateRegisters()
{
   src = Value::create(0, 0, 0, 0, 0, &li);
   dst = GPRValue(0,0,0,0,0);
}

class BytecodeAluOp2ATest: public BytecodeAluOpTest {
   void SetUp();
};

class BytecodeAluOp3ATest: public BytecodeAluOpTest {
   void SetUp();
};

class BytecodeAluLDSIdxOpTest: public BytecodeAluOpTest {
   void SetUp();
};

void BytecodeAluOp2ATest::SetUp()
{
   CreateRegisters();
   set_spacing({63, 61, 60, 53, 50, 40, 38, 37, 36, 35, 34, 33, 32,
                31, 29, 26, 25, 23, 22, 13, 12, 10, 9});
}

void BytecodeAluOp3ATest::SetUp()
{
   CreateRegisters();
   set_spacing({63, 61, 60, 53, 50, 45, 44, 42, 41, 32,
                31, 29, 26, 25, 23, 22, 13, 12, 10, 9});
}

void  BytecodeAluLDSIdxOpTest::SetUp()
{
   CreateRegisters();
   set_spacing({63, 62, 60, 59, 53, 50, 45, 44, 42, 41, 32,
                31,  26, 25, 23, 22, 13, 12, 10, 9});
}

TEST_F(BytecodeAluOp2ATest, BitCreateDecodeBytecodeRountrip)
{
   Value::LiteralFlags literal_flags = 0;
   for (int i = 0; i < 63; ++i) {
      uint64_t bc = 1ul << i;
      if (i == 48)
         bc |= 1ul << 47;
      if (i == 46)
         bc |= 1ul << 39;
      auto alu_node = AluNode::decode(bc, &literal_flags);
      TEST_EQ(alu_node->bytecode(), bc);
   }
}

TEST_F(BytecodeAluOp2ATest, TestValueType)
{
   auto v = Value::create(0, 0, 0, 0, 0, &li);
   EXPECT_EQ(v->type(), Value::gpr);

   v = Value::create(127, 0, 0, 0, 0, &li);
   EXPECT_EQ(v->type(), Value::gpr);

   v = Value::create(128, 0, 0, 0, 0, &li);
   EXPECT_EQ(v->type(), Value::kconst);

   v = Value::create(160, 0, 0, 0, 0, &li);
   EXPECT_EQ(v->type(), Value::kconst);

   v = Value::create(191, 0, 0, 0, 0, &li);
   EXPECT_EQ(v->type(), Value::kconst);

   v = Value::create(256, 0, 0, 0, 0, &li);
   EXPECT_EQ(v->type(), Value::kconst);

   v = Value::create(319, 0, 0, 0, 0, &li);
   EXPECT_EQ(v->type(), Value::kconst);

   v = Value::create(219, 0, 0, 0, 0, &li);
   EXPECT_EQ(v->type(), Value::cinline);

   v = Value::create(255, 0, 0, 0, 0, &li);
   EXPECT_EQ(v->type(), Value::cinline);

   v = Value::create(253, 0, 0, 0, 0, &li);
   EXPECT_EQ(v->type(), Value::literal);
}

TEST_F(BytecodeAluOp2ATest, TestValueSrc0Sel)
{
   for (int s = 0; s < 9; ++s) {
      auto v =Value::create(1 << s, 0, 0, 0, 0, &li);
      TEST_EQ(v->encode_for(alu_op2_src0), 1ul << s);
      TEST_EQ(v->encode_for(alu_op2_src1), 1ul << (s+13));
      TEST_EQ(v->encode_for(alu_op3_src2), 1ul << (s+32));
      if (s < 7)
         TEST_EQ(v->encode_for(alu_op_dst), 1ul << (s+53));
   }
}

TEST_F(BytecodeAluOp2ATest, TestValueSrc0Chan)
{

   for (int c = 0; c < 2; ++c) {
      auto v =Value::create(0, 1 << c, 0, 0, 0, &li);

      TEST_EQ(v->encode_for(alu_op2_src0), 1ul << (c+10));
      TEST_EQ(v->encode_for(alu_op2_src1), 1ul << (c+23));
      TEST_EQ(v->encode_for(alu_op3_src2), 1ul << (c+42));
      TEST_EQ(v->encode_for(alu_op_dst), 1ul << (c+61));
   }
}

TEST_F(BytecodeAluOp2ATest, TestValueSrc0Abs)
{
    auto vabs =Value::create(0, 0, 1, 0, 0, &li);
    TEST_EQ(vabs->encode_for(alu_op2_src0), src0_abs_bit);
    TEST_EQ(vabs->encode_for(alu_op2_src1), src1_abs_bit);
}

TEST_F(BytecodeAluOp2ATest, TestValueSrc0Neg)
{
   auto vneg =Value::create(0, 0, 0, 0, 1, &li);
   TEST_EQ(vneg->encode_for(alu_op2_src0), src0_neg_bit);
   TEST_EQ(vneg->encode_for(alu_op2_src1), src1_neg_bit);
   TEST_EQ(vneg->encode_for(alu_op3_src2), src2_neg_bit);
}

TEST_F(BytecodeAluOp2ATest, TestValueSrc0Rel)
{
   auto vrel =Value::create(0, 0, 0, 1, 0, &li);
   TEST_EQ(vrel->encode_for(alu_op2_src0), src0_rel_bit);
   TEST_EQ(vrel->encode_for(alu_op2_src1), src1_rel_bit);
   TEST_EQ(vrel->encode_for(alu_op3_src2), src2_rel_bit);
   TEST_EQ(vrel->encode_for(alu_op_dst), dst_rel_bit);

}

TEST_F(BytecodeAluOp2ATest, TestValuePS)
{
   auto vop2_alu_src_ps =Value::create(255, 0, 0, 0, 1, &li);
   TEST_EQ(vop2_alu_src_ps->encode_for(alu_op2_src0), 0x00000000000010fful);
   TEST_EQ(vop2_alu_src_ps->encode_for(alu_op2_src1), 0x00000000021fe000ul);
   TEST_EQ(vop2_alu_src_ps->encode_for(alu_op3_src2), 0x000010ff00000000ul);

   auto vop2_alu_src_kcache3 =Value::create(319, 3, 1, 1, 1, &li);
   TEST_EQ(vop2_alu_src_kcache3 ->encode_for(alu_op2_src0), 0x0000000100001f3ful);
   TEST_EQ(vop2_alu_src_kcache3 ->encode_for(alu_op2_src1), 0x0000000203e7e000ul);

   auto alu_src2_kcache3 =Value::create(319, 3, 0, 1, 1, &li);
   TEST_EQ(alu_src2_kcache3->encode_for(alu_op3_src2), 0x00001f3f00000000ul);

   auto dst = Value::create(127, 3, 0, 1, 0, &li);
   TEST_EQ(dst->encode_for(alu_op_dst), 0x7fe0000000000000ul);
}


TEST_F(BytecodeAluOp2ATest, TestOp2IndexModeBits)
{
   std::set<AluNode::EIndexMode> im = {
      AluNode::idx_ar_x, AluNode::idx_loop,
      AluNode::idx_global, AluNode::idx_global_ar_x
   };

   for(auto i: im) {
      AluNodeOp2 n(0, dst, src, src, empty_flags, i,
                   AluNode::alu_vec_012, AluNode::omod_off,
                   AluNode::pred_sel_off);
      TEST_EQ(n.bytecode(), static_cast<uint64_t>(i) << 26);
   }
}

TEST_F(BytecodeAluOp2ATest, TestOp2PredSelBits)
{
   std::set<AluNode::EPredSelect> ps = {
      AluNode::pred_sel_off, AluNode::pred_sel_zero,
      AluNode::pred_sel_one
   };

   for(auto i: ps) {
      AluNodeOp2 n(0, dst, src, src, empty_flags,
                   AluNode::idx_ar_x, AluNode::alu_vec_012,
                   AluNode::omod_off, i);
      TEST_EQ(n.bytecode(), static_cast<uint64_t>(i) << 29);
   }
}


TEST_F(BytecodeAluOp2ATest, TestOp2BankSwizzleBits)
{
   std::set<AluNode::EBankSwizzle> bs = {
      AluNode::alu_vec_012,
      AluNode::sq_alu_scl_201,
      AluNode::alu_vec_021,
      AluNode::sq_alu_scl_122,
      AluNode::alu_vec_120,
      AluNode::sq_alu_scl_212,
      AluNode::alu_vec_102,
      AluNode::sq_alu_scl_221,
      AluNode::alu_vec_201,
      AluNode::alu_vec_210
   };

   for(auto i: bs) {
      AluNodeOp2 n(0, dst, src, src, empty_flags, AluNode::idx_ar_x, i,
                   AluNode::omod_off, AluNode::pred_sel_off);
      TEST_EQ(n.bytecode(), static_cast<uint64_t>(i) << 50);
   }
}

TEST_F(BytecodeAluOp2ATest, TestOp2FlagsLastInstrBits)
{

   AluOpFlags flags;
   flags.set(AluNode::is_last_instr);
   AluNodeOp2 n(0, dst, src, src, flags, AluNode::idx_ar_x, AluNode::alu_vec_012,
                AluNode::omod_off, AluNode::pred_sel_off);

   TEST_EQ(n.bytecode(), 1ul << 31);
}

TEST_F(BytecodeAluOp2ATest, TestOp2FlagsClampBits)
{
   AluOpFlags flags;
   flags.set(AluNode::do_clamp);
   AluNodeOp2 n(0, dst, src, src, flags, AluNode::idx_ar_x, AluNode::alu_vec_012,
                AluNode::omod_off, AluNode::pred_sel_off);

   TEST_EQ(n.bytecode(), 1ul << 63);
}

TEST_F(BytecodeAluOp2ATest, TestOp2FlagsWriteBits)
{
   AluOpFlags flags;
   flags.set(AluNode::do_write);
   AluNodeOp2 n(0, dst, src, src, flags, AluNode::idx_ar_x, AluNode::alu_vec_012,
                AluNode::omod_off, AluNode::pred_sel_off);

   TEST_EQ(n.bytecode(), 1ul << 36);
}

TEST_F(BytecodeAluOp2ATest, TestOp2FlagsUpdateExecMaskBits)
{
   AluOpFlags flags;
   flags.set(AluNode::do_update_exec_mask);
   AluNodeOp2 n(0, dst, src, src, flags, AluNode::idx_ar_x, AluNode::alu_vec_012,
                AluNode::omod_off, AluNode::pred_sel_off);

   TEST_EQ(n.bytecode(), 1ul << 34);
}

TEST_F(BytecodeAluOp2ATest, TestOp2FlagsUpdatePredBits)
{
   AluOpFlags flags;
   flags.set(AluNode::do_update_pred);
   AluNodeOp2 n(0, dst, src, src, flags, AluNode::idx_ar_x, AluNode::alu_vec_012,
                AluNode::omod_off, AluNode::pred_sel_off);

   TEST_EQ(n.bytecode(), 1ul << 35);
}

TEST_F(BytecodeAluOp3ATest, TestOp3FlagsLastInstrBits)
{
   AluOpFlags flags;
   flags.set(AluNode::is_last_instr);
   AluNodeOp3 n(op3_bfe_uint, dst, src, src, src, flags, AluNode::idx_ar_x, AluNode::alu_vec_012,
                AluNode::pred_sel_off);

   TEST_EQ(n.bytecode(), (1ul << 31) | (1ul << 47));
}

TEST_F(BytecodeAluOp3ATest, TestOp3FlagsClampBits)
{
   AluOpFlags flags;
   flags.set(AluNode::do_clamp);
   AluNodeOp3 n(op3_bfe_uint, dst, src, src, src, flags, AluNode::idx_ar_x,
                AluNode::alu_vec_012, AluNode::pred_sel_off);

   TEST_EQ(n.bytecode(), (1ul << 63) | (1ul << 47));
}

TEST_F(BytecodeAluOp3ATest, TestOp3PredSelBits)
{
   std::set<AluNode::EPredSelect> ps = {
      AluNode::pred_sel_off, AluNode::pred_sel_zero,
      AluNode::pred_sel_one
   };

   for(auto i: ps) {
      AluNodeOp3 n(op3_bfe_uint, dst, src, src, src, empty_flags, AluNode::idx_ar_x,
                   AluNode::alu_vec_012, i);
      TEST_EQ(n.bytecode(), (static_cast<uint64_t>(i) << 29) | (1ul << 47));
   }
}


TEST_F(BytecodeAluOp3ATest, TestOp3BankSwizzleBits)
{
   std::set<AluNode::EBankSwizzle> bs = {
      AluNode::alu_vec_012,
      AluNode::sq_alu_scl_201,
      AluNode::alu_vec_021,
      AluNode::sq_alu_scl_122,
      AluNode::alu_vec_120,
      AluNode::sq_alu_scl_212,
      AluNode::alu_vec_102,
      AluNode::sq_alu_scl_221,
      AluNode::alu_vec_201,
      AluNode::alu_vec_210
   };

   for(auto i: bs) {
      AluNodeOp3 n(op3_bfe_uint, dst, src, src, src, empty_flags, AluNode::idx_ar_x, i,
                   AluNode::pred_sel_off);
      TEST_EQ(n.bytecode(), (static_cast<uint64_t>(i) << 50)
              | (1ul << 47));
   }
}

TEST_F(BytecodeAluLDSIdxOpTest, TestOpcodes)
{
   vector<std::pair<ESDOp, uint64_t>> test_pairs = {
      {DS_OP_ADD,      0x0002200000000000ul},
      {DS_OP_SUB,      0x0022200000000000ul},
      {DS_OP_RSUB,     0x0042200000000000ul},
      {DS_OP_DEC,      0x0082200000000000ul},
      {DS_OP_MAX_UINT, 0x0102200000000000ul},
      {DS_OP_CMP_STORE,0x0202200000000000ul},
      {DS_OP_ADD_RET,  0x0402200000000000ul}
   };

   for (auto p : test_pairs) {
      AluNodeLDSIdxOP n(op3_lds_idx_op, p.first,
                        src, src, src, empty_flags, 0, 0,
                        AluNode::idx_ar_x, AluNode::alu_vec_012);
      TEST_EQ(n.bytecode(), p.second);
   }
}

TEST_F(BytecodeAluLDSIdxOpTest, TestOffset)
{
   uint64_t expect[6] = {
      0x0802200000000000ul,
      0x0002300000000000ul,
      0x1002200000000000ul,
      0x8002200000000000ul,
      0x0002200000001000ul,
      0x0002200002000000ul
   };

   for (int ofs = 0; ofs < 6; ++ofs) {
      AluNodeLDSIdxOP n(op3_lds_idx_op, DS_OP_ADD,
                        src, src, src, empty_flags, 1 << ofs);
      TEST_EQ(n.bytecode(), expect[ofs]);
   }
}

TEST_F(BytecodeAluLDSIdxOpTest, TestOffsetRoundtrip)
{
   uint64_t expect[6] = {
      0x0402200000000000ul,
      0x0002300000000000ul,
      0x0802200000000000ul,
      0x8002200000000000ul,
      0x0002200000001000ul,
      0x0002200002000000ul
   };

   for (int ofs = 0; ofs < 6; ++ofs) {
      auto n = AluNode::decode(expect[ofs], nullptr);
      TEST_EQ(n->bytecode(), expect[ofs]);
   }
}

class TestValuePrintout: public testing::Test {

protected:
   void run (const Value& v, const std::string& expect);
};

void TestValuePrintout::run(const Value& v, const std::string& expect)
{
   std::ostringstream result;
   result << v;
   EXPECT_EQ(result.str(), expect);
}

TEST_F(TestValuePrintout, GPRValue0z)
{
   run(GPRValue(0, 2, false, false,false), "R0.z");
}

TEST_F(TestValuePrintout, GPRValue2yNeg)
{
   run(GPRValue(2, 1, false, false, true), "-R2.y");
}

TEST_F(TestValuePrintout, GPRValue2yAbs)
{
   run(GPRValue(2, 1, true, false, false), "|R2.y|");
}

TEST_F(TestValuePrintout, GPRValue10xAbsNeg)
{
   run(GPRValue(10, 1, true, false, true), "-|R10.y|");
}

TEST_F(TestValuePrintout, GPRValue11xRel)
{
   run(GPRValue(10, 1, false, true, false), "R[10+AR].y");
}


TEST_F(TestValuePrintout, GPRValue123x)
{
   run(GPRValue(123, 0, false, false,false), "R123.x");
}

TEST_F(TestValuePrintout, GPRValueT0w)
{
   run(GPRValue(124, 3, false, false,false), "T0.w");
}


TEST_F(TestValuePrintout, GPRValueT0Array)
{
   run(GPRValue(124, 3, false, true, false),
       "T0[E:indirect access to clause-local temporary].w");
}


TEST_F(TestValuePrintout, ConstValueKC0_12_indirect_y)
{
   run(ConstValue(12, 1, false, true, false), "KC0[12+AR].y");
}

TEST_F(TestValuePrintout, LiteralValue0x00004000)
{
   uint64_t literals = 0x3f800000;
   LiteralValue l(0, false, true, false);
   l.set_literal_info(&literals);
   run(l, "[0x3f800000 1].x");
}

TEST_F(TestValuePrintout, InlineConstLDS_OQ)
{
   run(InlineConstValue(ALU_SRC_LDS_OQ_A, 0, false, false),
       "LDS_OQ_A");
   run(InlineConstValue(ALU_SRC_LDS_OQ_B, 0, false, false),
       "LDS_OQ_B");

   run(InlineConstValue(ALU_SRC_LDS_OQ_A_POP, 0, false, false),
       "LDS_OQ_A_POP");
   run(InlineConstValue(ALU_SRC_LDS_OQ_B_POP, 0, false, false),
       "LDS_OQ_B_POP");
}

TEST_F(TestValuePrintout, InlineConstPVPS)
{
   run(InlineConstValue(ALU_SRC_PV, 1, false, false), "PV.y");
   run(InlineConstValue(ALU_SRC_PV, 2, true, false), "|PV.z|");
   run(InlineConstValue(ALU_SRC_PV, 3, false, true), "-PV.w");
   run(InlineConstValue(ALU_SRC_PS, 0, false, false), "PS");
}

TEST_F(TestValuePrintout, InlineConstIgnoreComp)
{
   run(InlineConstValue(ALU_SRC_HW_ALU_ODD, 1, false, false),
       "HW_ALU_ODD.y (W: Channel ignored)");
}


TEST_F(TestValuePrintout, InlineConstUnknown)
{
   run(InlineConstValue(ALU_SRC_UNKNOWN, 1, false, false),
       "E: unknown inline constant 256");
}

class ALUByteCodeDissass: public testing::Test {
protected:
   void run (uint64_t bc, const char *expect) const;
};

void ALUByteCodeDissass::run (uint64_t bc, const char *expect) const
{
   auto n = AluNode::decode(bc, nullptr);
   std::ostringstream result;
   result << *n;
   EXPECT_EQ(result.str(), expect);
}

TEST_F(ALUByteCodeDissass, Op2)
{
   run(0x2f800710010fa47cul,
       "    SETGE_DX10                      T0.y, T0.y, T1.z");
}

TEST_F(ALUByteCodeDissass, Op2IntLiteral)
{
   Value::LiteralFlags li;
   uint64_t literals = 10 | (0xc0000000ul << 32);

   auto src0 = Value::create(ALU_SRC_LITERAL, 0, false, false, false, &li);
   src0->set_literal_info(&literals);

   auto src2 = Value::create(ALU_SRC_LITERAL, 1, false, false, false, &li);
   src2->set_literal_info(&literals);

   auto src1 = Value::create(10, 3, false, false, false, nullptr);
   GPRValue dst(10, 2, false, false, false);
   AluOpFlags flags;
   flags.set(AluNode::do_write);
   AluNodeOp2 op(op2_add_int, dst, src0, src1, flags);
   std::ostringstream s;

   s << op;
   EXPECT_EQ(s.str(),
             "    ADD_INT                         R10.z, [0xa 10i], R10.w");

   AluNodeOp2 op2(op2_add, dst, src2, src1, flags);
   std::ostringstream s2;
   s2 << op2;
   EXPECT_EQ(s2.str(),
             "    ADD                             R10.z, [0xc0000000 -2f], R10.w");
}


TEST_F(ALUByteCodeDissass, Op2PredSet)
{
   run(0x0560229c801f00feul,
       "MP  PRED_SETNE_INT                  R43.x, PV.x, 0");
}

TEST_F(ALUByteCodeDissass, Op2_omod)
{
   run(0x01a00030020020f9,
       "    ADD*2                           R13.x, 1.0, -R1.x");
   run(0x01a00050020020f9,
       "    ADD*4                           R13.x, 1.0, -R1.x");
   run(0x01a00070020020f9,
       "    ADD/2                           R13.x, 1.0, -R1.x");
}

TEST_F(ALUByteCodeDissass, Op3)
{
   run(0x05a200fe8004e429ul,
       "    MULADD_UINT24                   R45.x, R41.y, R39.x, PV.x");
}

TEST_F(ALUByteCodeDissass, Op3WithBankSwizzle)
{
   run(0x01f2080c0100000c,
       "    MULADD_UINT24                   R15.x, R12.x, R0.z, R12.z vec_201");
}

TEST_F(ALUByteCodeDissass, Op3LDS_Read)
{
   run(0x064220f8801f0011,
       "    LDS_READ_RET OFS:0              __.x, R17.x");
}

TEST_F(ALUByteCodeDissass, Op3LDS_Write_Rel)
{
   /* Mesa dissass prints OFS:1 here, why? */
   run(0x09c224248004802c,
       "    LDS_WRITE_REL OFS:1             __.x, R44.x, R36.x, R36.y");
}

class ALUGroupDissass: public testing::Test {
protected:
   void run (const vector<uint64_t>& bc, size_t expect_ofs,
             size_t clause_length, const char *expect) const;
};

void ALUGroupDissass::run (const vector<uint64_t>&  bc, size_t expect_ofs,
                           size_t clause_length, const char *expect) const
{
   AluGroup g;

   size_t ofs = g.decode(bc, 0, clause_length);
   EXPECT_EQ(ofs, expect_ofs);
   EXPECT_EQ(g.as_string(), expect);
}

TEST_F(ALUGroupDissass, Op3LDS_Write_Rel)
{
   vector<uint64_t> bc{0x09c224248004802c};

   run(bc, 1, 1,
       "x:     LDS_WRITE_REL OFS:1             __.x, R44.x, R36.x, R36.y\n");
}


TEST_F(ALUGroupDissass, Dot4_ieee)
{
   vector<uint64_t> bc{0x01e05f900024200cul,
                       0x21e05f8000a4240cul,
                       0x41e05f800124280cul,
                       0x61e05f8081a42c0cul
                      };
   run(bc, 4, 4,
       "x:     DOT4_IEEE                       R15.x, R12.x, KC3[1].x\n"
       "y:     DOT4_IEEE                       __.y, R12.y, KC3[1].y\n"
       "z:     DOT4_IEEE                       __.z, R12.z, KC3[1].z\n"
       "w:     DOT4_IEEE                       __.w, R12.w, KC3[1].w\n");
}

TEST_F(ALUGroupDissass, Various_5_slot)
{
   vector<uint64_t> bc{0x0180011000200001ul,
                       0x2180011000200401ul,
                       0x41a00c90000000feul,
                       0x61a00c90000004feul,
                       0x4180011080200801ul};
   run(bc, 5, 5,
       "x:     MUL_IEEE                        R12.x, R1.x, KC2[0].x\n"
       "y:     MUL_IEEE                        R12.y, R1.y, KC2[0].x\n"
       "z:     MOV                             R13.z, PV.x\n"
       "w:     MOV                             R13.w, PV.y\n"
       "t:     MUL_IEEE                        R12.z, R1.z, KC2[0].x\n");

}

TEST_F(ALUGroupDissass, Various_AluGroupEarlyFinish_xy)
{
   vector<uint64_t> bc{
      0x01400890000000feul,
      0x21400890800004feul,
      0x00804d10800000feul
   };

   run(bc, 2, 3,
       "x:     TRUNC                           R10.x, PV.x\n"
       "y:     TRUNC                           R10.y, PV.y\n");
}

TEST_F(ALUGroupDissass, AluGroupEarlyFinish_x)
{
   vector<uint64_t> bc{
      0x064220f8801f00feul,
      0x00c00c90800000ddul
   };

   run(bc, 1, 2,
       "x:     LDS_READ_RET OFS:0              __.x, PV.x\n");
}
