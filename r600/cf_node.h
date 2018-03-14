#ifndef CF_NODE_H
#define CF_NODE_H

#include <r600/node.h>
#include <r600/defines.h>

#include <memory>
#include <string>
#include <vector>

extern const uint64_t valid_pixel_mode_bit;
extern const uint64_t end_of_program_bit;
extern const uint64_t barrier_bit;
extern const uint64_t whole_quad_mode_bit;
extern const uint64_t mark_bit;

class cf_node : public node {
public:
   using pointer=std::shared_ptr<cf_node>;

   cf_node(int bytecode_size, int opcode, bool barrier);
protected:

   static const char *m_index_mode_string;
   static uint32_t get_opcode(uint64_t bc);
   static uint32_t get_address(uint64_t bc);

   uint32_t opcode() const;
private:
   void print(std::ostream& os) const override;
   void do_append_bytecode(std::vector<uint64_t>& program) const override;

   virtual std::string op_from_opcode(uint32_t m_opcode) const;
   virtual void print_detail(std::ostream& os) const = 0;
   virtual void encode_parts(std::vector<uint64_t>& program) const = 0;

   uint32_t m_opcode;
   bool m_barrier;


   pointer parent;
   std::vector<pointer> children;
};

class cf_node_cf_word1 {
public:
   cf_node_cf_word1(uint64_t word1);
   void print(std::ostream& os) const;
private:
   static const char *m_condition;
   uint16_t m_pop_count;
   uint16_t m_cf_const;
   uint16_t m_cond;
   uint16_t m_count;
   bool m_valid_pixel_mode;
   bool m_end_of_program;
   bool m_whole_quad_mode;
};

class cf_node_with_address : public cf_node {
public:
   cf_node_with_address(unsigned bytecode_size,
                        uint32_t opcode,
                        bool barrier,
                        uint32_t addresss);
   void print_address(std::ostream& os) const;

private:
   uint32_t m_addr;
};

class cf_alu_node : public cf_node_with_address {
public:
   cf_alu_node(uint64_t bc);
   cf_alu_node(uint64_t bc, uint64_t bc_ext);
private:
   cf_alu_node(uint64_t bc, bool alu_ext);
   static uint32_t get_alu_opcode(uint64_t bc);
   static uint32_t get_alu_address(uint64_t bc);

   std::string op_from_opcode(uint32_t m_opcode) const override final;
   void print_detail(std::ostream& os) const override;
   void encode_parts(std::vector<uint64_t>& program) const override;

   uint16_t m_nkcache;
   uint16_t m_kcache_bank_idx_mode[4];
   uint16_t m_kcache_bank[4];
   uint16_t m_kcache_mode[4];
   uint16_t m_kcache_addr[4];
   uint16_t m_count;
   bool m_alt_const;
   bool m_whole_quad_mode;

   static constexpr uint64_t alt_const_bit = 1 << 25;
};

class cf_native_node : public cf_node_with_address {
public:
   cf_native_node(uint64_t bc);
private:
   void print_detail(std::ostream& os) const override;
   void encode_parts(std::vector<uint64_t>& program) const override;

   uint16_t m_jumptable_se;
   cf_node_cf_word1 m_word1;

   static const char m_jts_names[6][3];
};

class cf_gws_node : public cf_node {
public:
   cf_gws_node(uint64_t bc);
private:
   void print_detail(std::ostream& os) const override;

   static const char *m_opcode_as_string[4];
   uint16_t m_value;
   uint16_t m_resource;
   uint16_t m_val_index_mode;
   uint16_t m_rsrc_index_mode;
   uint16_t m_gws_opcode;
   cf_node_cf_word1 m_word1;
};

class cf_mem_node : public cf_node {
public:
   cf_mem_node(uint64_t bc);
protected:
   void print_mem_detail(std::ostream& os) const;

   static const char *m_type_string[4];
   uint16_t m_type;
private:
   uint16_t m_rw_gpr;
   bool m_rw_rel;
   uint16_t m_index_gpr;
   uint16_t m_elem_size;
   uint16_t m_burst_count;
   bool m_valid_pixel_mode;
   bool m_mark;
};

class cf_export_node : public cf_mem_node {
public:
   cf_export_node(uint64_t bc);
protected:
   uint16_t m_array_size;
   uint16_t m_comp_mask;
   bool m_end_of_program;
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