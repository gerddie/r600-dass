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

#ifndef CF_NODE_H
#define CF_NODE_H

#include <r600/alu_node.h>
#include <r600/defines.h>

#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <bitset>

namespace r600 {

extern const uint64_t valid_pixel_mode_bit;
extern const uint64_t end_of_program_bit;
extern const uint64_t barrier_bit;
extern const uint64_t whole_quad_mode_bit;
extern const uint64_t mark_bit;
extern const uint64_t rw_rel_bit;

using cf_flags = const std::bitset<16>;

class CFNode : public node {
public:
   using pointer = std::shared_ptr<CFNode>;

   CFNode(int bytecode_size, uint32_t opcode);

   static const uint16_t vpm = 0;
   static const uint16_t eop = 1;
   static const uint16_t qmb = 2;
   static const uint16_t barrier = 3;
   static const uint16_t wqm = 4;
   static const uint16_t alt_const = 5;
   static const uint16_t rw_rel = 6;
   static const uint16_t mark = 7;
   static const uint16_t sign = 8;

   bool test_flag(int f) const;

   uint32_t opcode() const;

   void set_nesting_depth(int nd);

protected:
   int get_nesting_depth() const;

   static const char *m_index_mode_string;
   static uint32_t get_opcode(uint64_t bc);
   static uint32_t get_address(uint64_t bc);

private:
   void print(std::ostream& os) const override;
   uint64_t create_bytecode_byte(int i) const override;

   virtual bool do_test_flag(int f) const;

   virtual std::string op_from_opcode(uint32_t m_opcode) const;
   virtual void print_detail(std::ostream& os) const = 0;
   virtual void encode_parts(int i, uint64_t& bc) const = 0;

   uint32_t m_opcode;
   pointer m_parent;
   std::vector<pointer> m_children;
   int m_nesting_depth;
};

class CFNodeFlags {
public:
   bool has_flag(int f) const;
protected:
   CFNodeFlags(uint64_t bc);
   CFNodeFlags(const cf_flags& flags);
   void set_flag(int flag);
   void encode_flags(uint64_t& bc) const;
   void print_flags(std::ostream& os) const;

private:
   std::bitset<16> m_flags;
};

class CFNodeCFWord1: public CFNodeFlags {
public:
   CFNodeCFWord1(uint64_t word1);
   CFNodeCFWord1(uint16_t pop_count,
                 uint16_t cf_const,
                 uint16_t cond,
                 uint16_t count,
                 const cf_flags& flags);

   void print(std::ostream& os) const;
   uint64_t encode() const;

private:
   static const char *m_condition;
   uint16_t m_pop_count;
   uint16_t m_cf_const;
   uint16_t m_cond;
   uint16_t m_count;
};

class CFNodeAllocExportWord1: public CFNodeFlags {
public:

   CFNodeAllocExportWord1(unsigned array_size,
                          unsigned comp_mask,
                          unsigned burst_count,
                          const cf_flags& flags);
   void print(std::ostream& os) const;
   uint64_t encode() const;
private:
   unsigned m_array_size;
   unsigned m_comp_mask;
   unsigned m_burst_count;
};

class CFNodeWithAddress : public CFNode {
public:
   CFNodeWithAddress(unsigned bytecode_size,
                     uint32_t opcode, uint32_t addresss);
   void print_address(std::ostream& os) const;
   uint32_t address() const;

private:
   uint32_t m_addr;
};

class CFAluNode: public CFNodeWithAddress,
      protected CFNodeFlags {
public:
   CFAluNode(uint64_t bc);
   CFAluNode(uint64_t bc, uint64_t bc_ext);
   CFAluNode(uint16_t opcode,
             const cf_flags& flags,
             uint32_t addr,
             uint16_t count = 0,
             const std::tuple<int, int,int>& kcache0 =
         std::make_tuple(0,0,0),
             const std::tuple<int, int,int>& kcache1 =
         std::make_tuple(0,0,0));

   CFAluNode(uint16_t opcode,
             const cf_flags& flags,
             uint32_t addr,
             uint16_t count,
             const std::vector<uint16_t>& mode,
             const std::tuple<int,int,int>& kcache0,
             const std::tuple<int,int,int>& kcache1,
             const std::tuple<int,int,int>& kcache2,
             const std::tuple<int,int,int>& kcache3);

   void disassemble_clause(const std::vector<uint64_t>& bc);

private:
   CFAluNode(uint64_t bc, bool alu_ext);
   static uint32_t get_alu_opcode(uint64_t bc);
   static uint32_t get_alu_address(uint64_t bc);

   std::string op_from_opcode(uint32_t m_opcode) const override final;
   void print_detail(std::ostream& os) const override;
   void encode_parts(int i, uint64_t& bc) const override;

   uint16_t m_nkcache;
   uint16_t m_kcache_bank_idx_mode[4];
   uint16_t m_kcache_bank[4];
   uint16_t m_kcache_mode[4];
   uint16_t m_kcache_addr[4];
   uint16_t m_count;
   std::vector<AluGroup> m_clause_code;

   static constexpr uint64_t alt_const_bit = 1ul << 57;
};

class CFNativeNode : public CFNodeWithAddress {
public:
   CFNativeNode(uint64_t bc);
   CFNativeNode(uint16_t opcode,
                const cf_flags &flags,
                uint32_t address = 0,
                uint16_t pop_count = 0,
                uint16_t count = 0,
                uint16_t jts = 0,
                uint16_t cf_const = 0,
                uint16_t cond = 0);
private:
   bool do_test_flag(int f) const override;

   void print_detail(std::ostream& os) const override;
   void encode_parts(int i, uint64_t &bc) const override;

   uint16_t m_jumptable_se;
   CFNodeCFWord1 m_word1;

   static const char m_jts_names[6][3];
};

class CFGwsNode : public CFNode {
public:
   CFGwsNode(uint64_t bc);
   CFGwsNode(uint32_t opcode,
             short gws_opcode,
             const cf_flags &flags,
             uint16_t pop_count,
             uint16_t cf_const,
             uint16_t cond,
             uint16_t count,
             short value,
             short resources,
             short val_index_mode,
             short res_index_mode);

private:
   void print_detail(std::ostream& os) const override;
   void encode_parts(int i, uint64_t &bc) const override;
   static const char *m_opcode_as_string[4];
   uint16_t m_value;
   uint16_t m_resource;
   uint16_t m_val_index_mode;
   uint16_t m_rsrc_index_mode;
   uint16_t m_gws_opcode;
   CFNodeCFWord1 m_word1;
};

class CFMemNode : public CFNode, protected CFNodeFlags {

public:
   CFMemNode(uint64_t bc);

   CFMemNode(uint16_t opcode,
             uint16_t type,
             uint16_t rw_gpr,
             uint16_t index_gpr,
             uint16_t elem_size,
             uint16_t burst_count,
             const cf_flags &flags);

protected:
   enum types {
      export_pixel,
      export_pos,
      export_param,
      export_unknown
   };

   int get_type() const;
   int get_burst_count() const;
   bool is_type(types t) const;

private:
   void print_detail(std::ostream& os) const override final;
   void encode_parts(int i, uint64_t& bc) const override final;
   virtual void print_mem_detail(std::ostream& os) const = 0;
   virtual void encode_mem_parts(uint64_t& bc) const = 0;
   virtual void print_elm_size(std::ostream& os) const;

   uint16_t m_type;
   uint16_t m_rw_gpr;
   uint16_t m_index_gpr;
   uint16_t m_elem_size;
   uint16_t m_burst_count;
};

class CFMemCompNode : public CFMemNode {
public:
   CFMemCompNode(uint64_t bc);

   CFMemCompNode(uint16_t opcode,
                 uint16_t type,
                 uint16_t rw_gpr,
                 uint16_t index_gpr,
                 uint16_t elem_size,
                 uint32_t array_size,
                 uint16_t comp_mask,
                 uint16_t burst_count,
                 const cf_flags &flags);
private:
   void print_mem_detail(std::ostream& os) const override final;
   void encode_mem_parts(uint64_t& bc) const override final;
   virtual void print_export_detail(std::ostream& os) const = 0;
   virtual void encode_export_parts(uint64_t& bc) const = 0;
   uint32_t m_array_size;
   uint16_t m_comp_mask;
};

class CFRatNode : public CFMemCompNode {
public:
   CFRatNode(uint64_t bc);
   CFRatNode(uint16_t opcode,
             uint16_t rat_inst,
             uint16_t rat_id,
             uint16_t rat_index_mode,
             uint16_t type,
             uint16_t rw_gpr,
             uint16_t index_gpr,
             uint16_t elem_size,
             uint32_t array_size,
             uint16_t comp_mask,
             uint16_t burst_count,
             const cf_flags &flags);


private:
   static const char *m_type_string[4];
   void print_export_detail(std::ostream& os) const override;
   void encode_export_parts(uint64_t& bc) const override;

   const char *rat_inst_string(int m_opcode) const;
   uint16_t m_rat_id;
   uint16_t m_rat_inst;
   uint16_t m_rat_index_mode;
};

/* Scratch, stream, and ring buffers */
class CFMemRingNode: public CFMemCompNode {
public:
   CFMemRingNode(uint64_t bc);

   CFMemRingNode(uint16_t opcode,
                 uint16_t type,
                 uint16_t rw_gpr,
                 uint16_t index_gpr,
                 uint16_t elem_size,
                 uint16_t array_size,
                 uint16_t array_base,
                 uint16_t comp_mask,
                 uint16_t burst_count,
                 const cf_flags &flags);
private:
   void print_export_detail(std::ostream& os) const override final;
   void encode_export_parts(uint64_t& bc) const override final;
   uint16_t m_array_base;
};

class CFMemExportNode: public CFMemNode {
public:
   CFMemExportNode(uint64_t bc);
   CFMemExportNode(uint16_t opcode,
                   uint16_t type,
                   uint16_t rw_gpr,
                   uint16_t index_gpr,
                   uint16_t elem_size,
                   uint16_t array_base,
                   uint16_t burst_count,
                   const std::vector<unsigned>& sel,
                   const cf_flags &flags);

private:
   void print_mem_detail(std::ostream& os) const override;
   void encode_mem_parts(uint64_t& bc) const override;
   uint16_t m_array_base;
   std::vector<unsigned> m_sel;
};

class CFExportNode: public CFMemNode {
public:
   CFExportNode(uint64_t bc);
   CFExportNode(uint16_t opcode,
                uint16_t type,
                uint16_t rw_gpr,
                uint16_t index_gpr,
                uint16_t array_base,
                uint16_t burst_count,
                const std::vector<unsigned>& sel,
                const cf_flags &flags);
private:
   static const char *m_type_string[4];
   void print_mem_detail(std::ostream& os) const override;
   void encode_mem_parts(uint64_t& bc) const override;
   void print_elm_size(std::ostream& os) const override;
   uint16_t m_array_base;
   std::vector<unsigned> m_sel;
};

}

#endif // CF_NODE_H
