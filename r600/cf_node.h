#ifndef CF_NODE_H
#define CF_NODE_H

#include <r600/node.h>
#include <r600/defines.h>

#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <bitset>

extern const uint64_t valid_pixel_mode_bit;
extern const uint64_t end_of_program_bit;
extern const uint64_t barrier_bit;
extern const uint64_t whole_quad_mode_bit;
extern const uint64_t mark_bit;
extern const uint64_t rw_rel_bit;

using cf_flags = const std::bitset<16>;

class cf_node : public node {
public:
   using pointer = std::shared_ptr<cf_node>;

   cf_node(int bytecode_size, uint32_t opcode);

   static const uint16_t vpm = 0;
   static const uint16_t eop = 1;
   static const uint16_t qmb = 2;
   static const uint16_t barrier = 3;
   static const uint16_t wqm = 4;
   static const uint16_t alt_const = 5;
   static const uint16_t rw_rel = 6;
   static const uint16_t mark = 7;
   static const uint16_t sign = 8;

protected:

   static const char *m_index_mode_string;
   static uint32_t get_opcode(uint64_t bc);
   static uint32_t get_address(uint64_t bc);

   uint32_t opcode() const;
private:
   void print(std::ostream& os) const override;
   uint64_t create_bytecode_byte(int i) const override;


   virtual std::string op_from_opcode(uint32_t m_opcode) const;
   virtual void print_detail(std::ostream& os) const = 0;
   virtual void encode_parts(int i, uint64_t& bc) const = 0;

   uint32_t m_opcode;

   pointer parent;
   std::vector<pointer> children;
};

class cf_node_flags {
protected:
   cf_node_flags(uint64_t bc);
   cf_node_flags(const cf_flags& flags);
   void set_flag(int flag);
   void encode_flags(uint64_t& bc) const;
   void print_flags(std::ostream& os) const;
private:
   std::bitset<16> m_flags;
};

class cf_node_cf_word1: public cf_node_flags {
public:
   cf_node_cf_word1(uint64_t word1);
   cf_node_cf_word1(uint16_t pop_count,
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

class cf_node_alloc_export_word1: public cf_node_flags {
public:

   cf_node_alloc_export_word1(unsigned array_size,
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

class cf_node_with_address : public cf_node {
public:
   cf_node_with_address(unsigned bytecode_size, uint32_t opcode,
                        uint32_t addresss);
   void print_address(std::ostream& os) const;
   uint32_t address() const;

private:
   uint32_t m_addr;
};

class cf_alu_node: public cf_node_with_address,
      protected cf_node_flags {
public:
   cf_alu_node(uint64_t bc);
   cf_alu_node(uint64_t bc, uint64_t bc_ext);
   cf_alu_node(uint16_t opcode,
               const cf_flags& flags,
               uint32_t addr,
               uint16_t count = 0,
               const std::tuple<int, int,int>& kcache0 =
                     std::make_tuple(0,0,0),
               const std::tuple<int, int,int>& kcache1 =
                     std::make_tuple(0,0,0));

   cf_alu_node(uint16_t opcode,
               const cf_flags& flags,
               uint32_t addr,
               uint16_t count,
               const std::vector<uint16_t>& mode,
               const std::tuple<int,int,int>& kcache0,
               const std::tuple<int,int,int>& kcache1,
               const std::tuple<int,int,int>& kcache2,
               const std::tuple<int,int,int>& kcache3);
private:
   cf_alu_node(uint64_t bc, bool alu_ext);
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
   static constexpr uint64_t alt_const_bit = 1 << 25;
};

class cf_native_node : public cf_node_with_address {
public:
   cf_native_node(uint64_t bc);
   cf_native_node(uint16_t opcode,
                  const cf_flags &flags,
                  uint32_t address = 0,
                  uint16_t pop_count = 0,
                  uint16_t count = 0,
                  uint16_t jts = 0,
                  uint16_t cf_const = 0,
                  uint16_t cond = 0);
private:
   void print_detail(std::ostream& os) const override;
   void encode_parts(int i, uint64_t &bc) const override;

   uint16_t m_jumptable_se;
   cf_node_cf_word1 m_word1;

   static const char m_jts_names[6][3];
};

class cf_gws_node : public cf_node {
public:
   cf_gws_node(uint64_t bc);
   cf_gws_node(uint32_t opcode,
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
   cf_node_cf_word1 m_word1;
};

class cf_mem_node : public cf_node, protected cf_node_flags {
public:
   cf_mem_node(uint64_t bc);

   cf_mem_node(uint16_t opcode,
               uint16_t type,
               uint16_t rw_gpr,
               uint16_t index_gpr,
               uint16_t elem_size,
               uint16_t burst_count,
               const cf_flags &flags);

protected:
   void print_mem_detail(std::ostream& os) const;

   static const char *m_type_string[4];
   uint16_t m_type;
private:
   void encode_parts(int i, uint64_t& bc) const override final;
   virtual void encode_mem_parts(uint64_t& bc) const = 0;
   uint16_t m_rw_gpr;
   uint16_t m_index_gpr;
   uint16_t m_elem_size;
   uint16_t m_burst_count;
};

class cf_export_node : public cf_mem_node {
public:
   cf_export_node(uint64_t bc);
   cf_export_node(uint16_t opcode,
                  uint16_t type,
                  uint16_t rw_gpr,
                  uint16_t index_gpr,
                  uint16_t elem_size,
                  uint16_t array_size,
                  uint16_t comp_mask,
                  uint16_t burst_count,
                  const cf_flags &flags);
protected:
   void print_detail(std::ostream& os) const override;
   void encode_mem_parts(uint64_t& bc) const override final;

   uint16_t m_array_size;
   uint16_t m_comp_mask;
};

class cf_rat_node : public cf_export_node {
public:
   cf_rat_node(uint64_t bc);
private:
   static const char *m_type_string[4];
   void print_detail(std::ostream& os) const override;
   const char *rat_inst_string(int m_opcode) const;
   uint16_t m_rat_id;
   uint16_t m_rat_inst;
   uint16_t m_rat_index_mode;
};

class cf_export_mem_node: public cf_export_node {
public:
   cf_export_mem_node(uint64_t bc);

   cf_export_mem_node(uint16_t opcode,
                      uint16_t type,
                      uint16_t rw_gpr,
                      uint16_t index_gpr,
                      uint16_t elem_size,
                      uint16_t array_size,
                      uint16_t comp_mask,
                      uint16_t burst_count,
                      const cf_flags &flags);
private:
   void print_detail(std::ostream& os) const override;

   uint16_t m_array_base;
   uint16_t m_sel[4];
};

class cf_mem_stream_node: public cf_mem_node {
public:
   cf_mem_stream_node(uint64_t bc);
private:
   void print_detail(std::ostream& os) const override;

   uint16_t m_array_base;
};

#endif // CF_NODE_H