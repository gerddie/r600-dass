#ifndef CF_NODE_H
#define CF_NODE_H

#include <r600/node.h>
#include <r600/defines.h>

#include <memory>
#include <string>
#include <vector>

class cf_node : public node {
public:
        cf_node(unsigned bytecode_size, uint32_t *bytecode, bool alu_node);
protected:
        static const char *m_index_mode_string;
private:
        void print(std::ostream& os) const override;

        virtual std::string op_from_opcode(uint32_t m_opcode) const;
        virtual void print_detail(std::ostream& os) const = 0;

        uint32_t m_opcode;
        bool m_barrier;

        using pointer=std::shared_ptr<cf_node>;
        pointer parent;
        std::vector<pointer> children;
};

class cf_node_cf_word1 {
public:
        cf_node_cf_word1(uint32_t word1);
        void print(std::ostream& os) const;
private:
        static const char *m_condition;
        uint8_t m_pop_count;
        uint8_t m_cf_const;
        uint8_t m_cond;
        uint8_t m_count;
        bool m_valid_pixel_mode;
        bool m_end_of_program;
        bool m_whole_quad_mode;
};

class cf_node_with_address : public cf_node {
public:
        cf_node_with_address(unsigned bytecode_size, uint32_t *bytecode, bool alu_node);
private:
        uint32_t m_addr;
};

class cf_alu_node : public cf_node_with_address {
public:
        cf_alu_node(uint32_t *bytecode, bool alu_ext);
private:
        std::string op_from_opcode(uint32_t m_opcode) const override final;
        void print_detail(std::ostream& os) const override;

        uint16_t m_nkcache;
        uint16_t m_kcache_bank_idx_mode[4];
        uint16_t m_kcache_bank[4];
        uint16_t m_kcache_mode[4];
        uint16_t m_kcache_addr[4];
        uint16_t m_count;
        bool m_alt_const;
        bool m_whole_quad_mode;
};

class cf_native_node : public cf_node_with_address {
public:
        cf_native_node(uint32_t *bytecode);
private:
        void print_detail(std::ostream& os) const override;

        uint8_t m_jumptable_se;
        cf_node_cf_word1 m_word1;

        static const char m_jts_names[6][3];
};

class cf_gws_node : public cf_node {
public:
        cf_gws_node(uint32_t *bytecode);
private:
        void print_detail(std::ostream& os) const override;

        static const char *m_opcode_as_string[4];
        uint16_t m_value;
        uint8_t m_resource;
        uint8_t m_val_index_mode;
        uint8_t m_rsrc_index_mode;
        uint8_t m_gws_opcode;
        cf_node_cf_word1 m_word1;
};

class cf_mem_node : public cf_node {
public:
        cf_mem_node(uint32_t *bytecode);
protected:
        void print_mem_detail(std::ostream& os) const;

        static const char *m_type_string[4];
        uint8_t m_type;
private:
        uint16_t m_rw_gpr;
        bool m_rw_rel;
        uint16_t m_index_gpr;
        uint8_t m_elem_size;
        uint8_t m_burst_count;
        bool m_valid_pixel_mode;
        bool m_mark;
};

class cf_export_node : public cf_mem_node {
public:
        cf_export_node(uint32_t *bytecode);
protected:
        uint16_t m_array_size;
        uint8_t m_comp_mask;
        bool m_end_of_program;
};

class cf_rat_node : public cf_export_node {
public:
        cf_rat_node(uint32_t *bytecode);
private:
        static const char *m_type_string[4];
        void print_detail(std::ostream& os) const override;
        const char *rat_inst_string(int m_opcode) const;
        uint8_t m_rat_id;
        uint8_t m_rat_inst;
        uint8_t m_rat_index_mode;
};

class cf_export_mem_node: public cf_export_node {
public:
        cf_export_mem_node(uint32_t *bytecode);
private:
        void print_detail(std::ostream& os) const override;

        uint16_t m_array_base;
        uint8_t m_sel[4];
};

class cf_mem_stream_node: public cf_mem_node {
public:
        cf_mem_stream_node(uint32_t *bytecode);
private:
        void print_detail(std::ostream& os) const override;

        uint16_t m_array_base;
};

#endif // CF_NODE_H