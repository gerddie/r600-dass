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

#ifndef r600_alu_node_h
#define r600_alu_node_h

#include <r600/node.h>
#include <r600/value.h>
#include <bitset>

namespace r600 {

using AluOpFlags=std::bitset<16>;

struct AluOP {
   constexpr int x = 1;
   constexpr int y = 2;
   constexpr int z = 4;
   constexpr int w = 8;
   constexpr int v = 15;
   constexpr int t = 16;
   constexpr int a = 31;
   int opcode: 10;
   int nsrc: 2;
   int unit_mask: 5;
};

const AluOP alu_ops[];

class AluNode {
public:
   enum EIndexMode {
      idx_ar_x,
      idx_loop,
      idx_global,
      idx_global_ar_x,
      idx_unknown
   };

   enum EPredSelect {
      pred_sel_off = 0,
      pred_sel_reserved,
      pred_sel_zero,
      pred_sel_one
   };

   enum EOutputModify {
      omod_off,
      omod_mul_2,
      omod_mul_4,
      omod_div_2,
      omod_unknown
   };

   enum EBankSwizzle {
      alu_vec_012 = 0,
      sq_alu_scl_201 = 0,
      alu_vec_021 = 1,
      sq_alu_scl_122 = 1,
      alu_vec_120 = 2,
      sq_alu_scl_212 = 2,
      alu_vec_102 = 3,
      sq_alu_scl_221 = 3,
      alu_vec_201 = 4,
      alu_vec_210 = 5,
      alu_vec_unknown = 6
   };

   enum FlagsShifts {
      is_last_instr,
      do_clamp,
      do_write,
      do_update_exec_mask,
      do_update_pred
   };

   static AluNode *decode(uint64_t bc, Value::LiteralFlags& literal_index);

   AluNode(uint16_t opcode, const GPRValue& dst,
           PValue src0, PValue src1, EIndexMode index_mode,
           EBankSwizzle bank_swizzle, EPredSelect pred_select,
           AluOpFlags flags);

   int get_dst_chan() const;
   bool last_instr() const;

   uint64_t get_bytecode() const;
protected:
   bool test_flag(FlagsShifts f) const;
   const Value& src0() const;
   const Value& src1() const;
   int nopsources() const;


private:
   virtual void encode(uint64_t& bc) const = 0;
   uint64_t shared_flags() const;

   uint16_t m_opcode;
   PValue m_src0;
   PValue m_src1;
   GPRValue m_dst;
   EIndexMode m_index_mode;
   EBankSwizzle m_bank_swizzle;
   EPredSelect m_pred_select;
   AluOpFlags m_flags;
};

using PAluNode = std::shared_ptr<AluNode>;

class AluNodeOp2: public AluNode {
public:
   AluNodeOp2(uint16_t opcode, const GPRValue& dst,
              PValue src0, PValue src1, AluOpFlags flags,
              EIndexMode index_mode = idx_ar_x,
              EBankSwizzle bank_swizzle = alu_vec_012,
              EOutputModify output_modify = omod_off,
              EPredSelect pred_select = pred_sel_off);
private:
   void encode(uint64_t& bc) const override;

   EOutputModify m_output_modify;
};

class AluNodeOp3: public AluNode {
public:
   AluNodeOp3(uint16_t opcode, const GPRValue& dst,
              PValue src0, PValue src1, PValue src2,
              AluOpFlags flags,
              EIndexMode index_mode = idx_ar_x,
              EBankSwizzle bank_swizzle = alu_vec_012,
              EPredSelect pred_select = pred_sel_off);
private:
   void encode(uint64_t& bc) const override;
   PValue m_src2;
};

class AluGroup {
public:
   AluGroup();

   std::vector<uint64_t>::const_iterator
   decode(std::vector<uint64_t>::const_iterator bc);
   void encode(std::vector<uint64_t>& bc) const;
private:
   std::vector<PAluNode> m_ops;
   int m_nlinterals;
   uint32_t m_literals[4];
};

}

#endif
