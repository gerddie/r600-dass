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

#ifndef R600VALUE_H
#define R600VALUE_H

#include <r600/alu_defines.h>
#include <cstdint>
#include <memory>
#include <vector>
#include <bitset>

namespace r600 {

enum ValueOpEncoding {
   alu_op2_src0,
   alu_op2_src1,
   alu_op3_src0,
   alu_op3_src1,
   alu_op3_src2,
   alu_lds_src0,
   alu_lds_src1,
   alu_lds_src2,
   alu_op_dst,
   alu_unknown
};

extern const uint64_t src0_rel_bit;
extern const uint64_t src1_rel_bit;
extern const uint64_t src2_rel_bit;
extern const uint64_t src0_neg_bit;
extern const uint64_t src1_neg_bit;
extern const uint64_t src2_neg_bit;
extern const uint64_t src0_abs_bit;
extern const uint64_t src1_abs_bit;
extern const uint64_t dst_rel_bit;

class LiteralBuffer {

};

class Value {
public:
   using Pointer=std::shared_ptr<Value>;

   enum Type {
      gpr,
      kconst,
      literal,
      cinline,
      lds_direct,
      unknown
   };

   using LiteralFlags=std::bitset<4>;

   Value();

   static Pointer create(uint16_t sel, uint16_t chan,
                         bool abs, bool rel, bool neg,
                         LiteralFlags *literal_index);

   static Pointer create(uint64_t bc, ValueOpEncoding encoding,
                         LiteralFlags *literal_index);

   Type get_type() const;
   virtual uint64_t get_sel() const = 0;
   uint64_t get_chan() const {return m_chan;}
   bool get_rel() const {return m_rel;}
   bool get_neg() const {return m_neg;}
   bool get_abs() const {return m_abs;}
   void set_abs(bool flag);
   void set_neg(bool flag);

   uint64_t encode_for(ValueOpEncoding encoding) const;

   virtual void set_literal_info(const uint32_t *literals);
   virtual void allocate_literal(LiteralBuffer& lb) const;
protected:

   Value(Type type, uint16_t chan, bool abs, bool rel, bool neg);

private:

   uint64_t encode_for_alu_op2_src0() const;
   uint64_t encode_for_alu_op2_src1() const;
   uint64_t encode_for_alu_op3_src0() const;
   uint64_t encode_for_alu_op3_src1() const;
   uint64_t encode_for_alu_op3_src2() const;
   uint64_t encode_for_alu_op_dst() const;

   static Pointer decode_from_alu_op2_src0(uint64_t bc, LiteralFlags *li);
   static Pointer decode_from_alu_op2_src1(uint64_t bc, LiteralFlags *li);
   static Pointer decode_from_alu_op3_src0(uint64_t bc, LiteralFlags *li);
   static Pointer decode_from_alu_op3_src1(uint64_t bc, LiteralFlags *li);
   static Pointer decode_from_alu_op3_src2(uint64_t bc, LiteralFlags *li);
   static Pointer decode_from_alu_op_dst(uint64_t bc);

   static Pointer decode_from_alu_lds_src0(uint64_t bc, LiteralFlags *li);
   static Pointer decode_from_alu_lds_src1(uint64_t bc, LiteralFlags *li);
   static Pointer decode_from_alu_lds_src2(uint64_t bc, LiteralFlags *li);


   Type m_type;
   uint16_t m_chan;
   bool m_abs;
   bool m_rel;
   bool m_neg;
};

using PValue=Value::Pointer;

class GPRValue : public Value {
public:
   GPRValue() = default;
   GPRValue(GPRValue&& orig) = default;
   GPRValue(const GPRValue& orig) = default;

   GPRValue(uint16_t sel, uint16_t chan,
            bool abs, bool rel, bool neg);

   GPRValue& operator = (const GPRValue& orig) = default;
   GPRValue& operator = (GPRValue&& orig) = default;

   uint64_t get_sel() const override final;
private:
   uint16_t m_sel;
};

class LiteralValue: public Value {
public:
   LiteralValue(uint16_t chan, bool abs, bool rel, bool neg);
   uint64_t get_sel() const override final;
   void set_literal_info(const uint32_t *literals) override final;
private:
   uint32_t m_value;
};

class SpecialValue: public Value {
protected:
   SpecialValue(Type type, int value, int chan, bool abs, bool neg);
   uint64_t get_sel() const override final;
private:
   AluInlineConstants m_value;
};

class InlineConstValue: public SpecialValue {
public:
   InlineConstValue(int value, int chan, bool abs, bool neg);
private:
   AluInlineConstants m_value;
};

class LDSDirectValue: public SpecialValue {
public:
   LDSDirectValue(int value, int chan, bool abs, bool neg);
   void set_literal_info(const uint32_t *literals) override final;
private:

   AluInlineConstants m_value;
   struct DirectAccess {
      DirectAccess();
      DirectAccess(uint32_t l);
      int offset;
      int stride;
      bool thread_rel;
   };

   std::vector<DirectAccess> m_addr;
   bool m_direct_read_32;

};

class ConstValue: public Value {
public:
   ConstValue(uint16_t sel, uint16_t chan,
              bool abs, bool rel, bool neg);
   uint64_t get_sel() const override;
private:
   uint16_t m_index;
   uint16_t m_kcache_bank;
};

}

#endif // R600VALUE_H
