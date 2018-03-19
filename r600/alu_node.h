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

class AluNode {
public:
        enum EIndexMode {
                idx_ar_x,
                idx_loop,
                idx_global,
                idx_global_ar_x
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
        };

        enum EBankSwizzle {
                alu_vec_012 = 0,
                sq_alu_scl_201 = 0,
                alu_vec_021 = 1,
                sq_alu_scl_122 = 1,
                alu_vec_120 = 2,
                sq_alu_scl_212 = 2,
                alu_vec_102 = 3,
                aq_alu_scl_221 = 3,
                alu_vec_201 = 4,
                alu_vec_210 = 5
        };

        enum FlagsShifts {
           is_last_instr,
           do_clamp,
           do_write,
           do_update_exec_mask,
           do_update_pred
        };

        static AluNode *decode(uint64_t bc, Value::LiteralFlags& literal_index);

        AluNode(uint16_t opcode,
                PValue src0, PValue src1,
                const GPRValue& dst, EIndexMode index_mode,
                EBankSwizzle bank_swizzle, EPredSelect pred_select,
                AluOpFlags flags);

        int get_dst_chan() const;
        bool last_instr() const;

        uint64_t get_bytecode() const;
protected:
        bool get_src0_abs() const;
        bool get_src1_abs() const;
        bool test_flag(FlagsShifts f) const;

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
        AluNodeOp2(uint16_t opcode,
                   PValue src0, PValue src1, const GPRValue& dst,
                   EIndexMode index_mode, EBankSwizzle bank_swizzle,
                   EOutputModify output_modify, EPredSelect pred_select,
                   AluOpFlags flags);
private:
        void encode(uint64_t& bc) const override;

        EOutputModify m_output_modify;
};

class AluNodeOp3: public AluNode {
public:
        AluNodeOp3(uint16_t opcode,
                   PValue src0, PValue src1, PValue src2,
                   const GPRValue& dst, EIndexMode index_mode,
                   EBankSwizzle bank_swizzle, EPredSelect pred_select,
                   AluOpFlags flags);
private:
        void encode(uint64_t& bc) const override;
        PValue m_src2;
};

class AluGroup {
public:
   AluGroup();

   std::vector<uint64_t>::const_iterator
   decode(std::vector<uint64_t>::const_iterator bc);

private:
   std::vector<PAluNode> m_ops;
   uint32_t m_literals[4];
};

}

#endif
