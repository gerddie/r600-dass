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
#include <r600/alu_defines.h>
#include <bitset>

#include <map>

namespace r600 {

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

   static AluNode *decode(uint64_t bc, Value::LiteralFlags *literal_index);

   AluNode(uint16_t opcode, EIndexMode index_mode, EBankSwizzle bank_swizzle,
           AluOpFlags flags, int dst_chan);

   int dst_chan() const;
   bool last_instr() const;

   bool slot_supported(int flag) const;
   uint64_t bytecode() const;

   void set_literal_info(uint64_t *literals);
   void allocate_literal(LiteralBuffer& lb) const;

   void collect_values_with_literals(std::vector<PValue>& values) const;

   void print(std::ostream& os) const;

protected:
   bool test_flag(FlagsShifts f) const;
   const Value& src(unsigned idx) const;
   Value& src(unsigned idx);
   void set_src(unsigned idx, PValue v);
   int nopsources() const;

private:
   void print_flags(std::ostream& os) const;
   virtual void print_pred(std::ostream& os) const;
   virtual void print_dst(std::ostream& os) const;
   virtual void print_op(std::ostream& os) const;

   virtual void allocate_spec_literal(LiteralBuffer& lb) const;
   virtual void set_spec_literal_info(uint64_t *literals);
   virtual void encode(uint64_t& bc) const = 0;
   uint64_t shared_flags() const;

   EAluOp m_opcode;
   std::vector<PValue> m_src;
   EIndexMode m_index_mode;
   EBankSwizzle m_bank_swizzle;
   AluOpFlags m_flags;
   int m_dst_chan;
};

inline std::ostream& operator << (std::ostream& os, const AluNode& n)
{
   n.print(os);
   return os;
}

using PAluNode = std::shared_ptr<AluNode>;

class AluNodeWithDst: public AluNode {
protected:
   AluNodeWithDst(uint16_t opcode, const GPRValue& dst,
                  EIndexMode index_mode,
                  EBankSwizzle bank_swizzle, EPredSelect pred_select,
                  AluOpFlags flags);
   void encode_dst_and_pred(uint64_t& bc) const;
private:
   void print_pred(std::ostream& os) const override;
   void print_dst(std::ostream& os) const override;

   GPRValue m_dst;
   EPredSelect m_pred_select;
};

class AluNodeOp2: public AluNodeWithDst {
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

class AluNodeOp3: public AluNodeWithDst {
public:
   AluNodeOp3(uint16_t opcode, const GPRValue& dst,
              PValue src0, PValue src1, PValue src2,
              AluOpFlags flags,
              EIndexMode index_mode = idx_ar_x,
              EBankSwizzle bank_swizzle = alu_vec_012,
              EPredSelect pred_select = pred_sel_off);
private:
   void set_spec_literal_info(uint64_t *literals) override final;
   void encode(uint64_t& bc) const override;
};

class AluNodeLDSIdxOP: public AluNode {
public:
   AluNodeLDSIdxOP(uint16_t opcode, ELSDIndexOp lds_op,
                   PValue src0, PValue src1,
                   PValue src2, AluOpFlags flags,
                   int offset = 0, int dst_chan = 0,
                   EIndexMode index_mode = idx_ar_x,
                   EBankSwizzle bank_swizzle = alu_vec_012);
private:
   void print_op(std::ostream& os) const override final;
   void set_spec_literal_info(uint64_t *literals) override final;
   void encode(uint64_t& bc) const override;
   ELSDIndexOp m_lds_op;
   int m_offset;
};

class AluGroup {
public:
   AluGroup();

   std::vector<uint64_t>::const_iterator
   decode(std::vector<uint64_t>::const_iterator bc);
   bool encode(std::vector<uint64_t>& bc) const;
private:
   std::vector<PAluNode> m_ops;
};

}

#endif
