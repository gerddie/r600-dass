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

   struct PrintFlags {
      PrintFlags():index_mode(0),
         literal_is_float(false)
      {
      }
      PrintFlags(int im, bool f):index_mode(im),
         literal_is_float(f)
      {
      }
      int index_mode;
      bool literal_is_float;
   };

   enum Type {
      gpr,
      kconst,
      literal,
      cinline,
      lds_direct,
      unknown
   };

   static const char *component_names;

   using LiteralFlags=std::bitset<4>;

   Value();

   virtual ~Value(){}

   static Pointer create(uint16_t sel, uint16_t chan,
                         bool abs, bool rel, bool neg,
                         LiteralFlags *literal_index);

   static Pointer create(uint64_t bc, ValueOpEncoding encoding,
                         LiteralFlags *literal_index);

   Type type() const;
   virtual uint64_t sel() const = 0;
   uint64_t chan() const {return m_chan;}
   bool rel() const {return m_rel;}
   bool neg() const {return m_neg;}
   bool abs() const {return m_abs;}
   void set_abs(bool flag);
   void set_neg(bool flag);

   uint64_t encode_for(ValueOpEncoding encoding) const;
   void set_chan(uint16_t chan);
   void print(std::ostream& os, const PrintFlags& flags) const;

   void print(std::ostream& os) const;

   virtual void set_literal_info(const uint64_t *literals);
   virtual void allocate_literal(LiteralBuffer& lb) const;


protected:

   Value(Type type, uint16_t chan, bool abs, bool rel, bool neg);

private:
   virtual void do_print(std::ostream& os) const = 0;
   virtual void do_print(std::ostream& os, const PrintFlags& flags) const;

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

   uint64_t sel() const override final;

private:
   void do_print(std::ostream& os) const override;
   void do_print(std::ostream& os, const PrintFlags& flags) const override;
   uint16_t m_sel;
};

inline std::ostream& operator << (std::ostream& os, const Value& v)
{
   v.print(os);
   return os;
}

class LiteralValue: public Value {
public:
   LiteralValue(uint16_t chan, bool abs, bool rel, bool neg);
   uint64_t sel() const override final;
   void set_literal_info(const uint64_t *literals) override final;
   uint32_t value() const;
private:
   void do_print(std::ostream& os) const override;
   void do_print(std::ostream& os, const PrintFlags& flags) const override;
   union {
      uint32_t i;
      float f;
   } m_value;
};

class SpecialValue: public Value {
protected:
   SpecialValue(Type type, int value, int chan, bool abs, bool neg);
   uint64_t sel() const override final;
private:
   void do_print(std::ostream& os) const override;
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
   void set_literal_info(const uint64_t *literals) override final;
   uint64_t address_bytecode() const;

private:
   void do_print(std::ostream& os) const override;
   AluInlineConstants m_value;
   int m_offset_a;
   int m_stride_a;
   bool m_thread_rel_a;
   int m_offset_b;
   int m_stride_b;
   bool m_thread_rel_b;
   bool m_direct_read_32;

};

class ConstValue: public Value {
public:
   ConstValue(uint16_t sel, uint16_t chan,
              bool abs, bool rel, bool neg);
   uint64_t sel() const override;
private:
   void do_print(std::ostream& os) const override;
   uint16_t m_index;
   uint16_t m_kcache_bank;
};

}

#endif // R600VALUE_H
