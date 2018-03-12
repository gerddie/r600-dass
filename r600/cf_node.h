#ifndef CF_NODE_H
#define CF_NODE_H

#include <r600/node.h>
#include <r600/defines.h>

#include <string>

class cf_node : public node {
public:
        cf_node(unsigned bytecode_size, uint32_t *bytecode, bool alu_node);
protected:
        static const char *index_mode_string;
private:
        void print(std::ostream& os) const override;

        virtual std::string op_from_opcode(uint32_t opcode) const;
        virtual void print_detail(std::ostream& os) const = 0;

        uint32_t opcode;
        bool barrier;
};

class cf_node_cf_word1 {
public:
        cf_node_cf_word1(uint32_t word1);
        void print(std::ostream& os) const;
private:
        static const char *condition;
        uint8_t pop_count;
        uint8_t cf_const;
        uint8_t cond;
        uint8_t count;
        bool valid_pixel_mode;
        bool end_of_program;
        bool whole_quad_mode;
};

class cf_node_with_address : public cf_node {
public:
        cf_node_with_address(unsigned bytecode_size, uint32_t *bytecode, bool alu_node);
private:
        uint32_t addr;
};

class cf_alu_node : public cf_node_with_address {
public:
        cf_alu_node(uint32_t *bytecode, bool alu_ext);
private:
        std::string op_from_opcode(uint32_t opcode) const override final;
        void print_detail(std::ostream& os) const override;

        uint16_t nkcache;
        uint16_t kcache_bank_idx_mode[4];
        uint16_t kcache_bank[4];
        uint16_t kcache_mode[4];
        uint16_t kcache_addr[4];
        uint16_t count;
        bool alt_const;
        bool whole_quad_mode;
};

class cf_native_node : public cf_node_with_address {
public:
        cf_native_node(uint32_t *bytecode);
private:
        void print_detail(std::ostream& os) const override;

        uint8_t jumptable_se;
        cf_node_cf_word1 word1;

        static const char jts_names[6][3];
};

class cf_gws_node : public cf_node {
public:
        cf_gws_node(uint32_t *bytecode);
private:
        void print_detail(std::ostream& os) const override;
        static const char *opcode_as_string[4];

        uint16_t value;
        uint8_t resource;
        uint8_t val_index_mode;
        uint8_t rsrc_index_mode;
        uint8_t gws_opcode;
        cf_node_cf_word1 word1;
};

class cf_mem_node : public cf_node {
public:
        cf_mem_node(uint32_t *bytecode);
protected:
        static const char *type_string[4];
        void print_mem_detail(std::ostream& os) const;
        uint8_t type;
private:
        uint16_t rw_gpr;
        bool rw_rel;
        uint16_t index_gpr;
        uint8_t elem_size;
        uint8_t burst_count;
        bool valid_pixel_mode;
        bool mark;
};

class cf_export_node : public cf_mem_node {
public:
        cf_export_node(uint32_t *bytecode);
protected:
        uint16_t array_size;
        uint8_t comp_mask;
        bool end_of_program;
};

class cf_rat_node : public cf_export_node {
public:
        cf_rat_node(uint32_t *bytecode);
private:
        static const char *type_string[4];
        void print_detail(std::ostream& os) const override;
        const char *rat_inst_string(int opcode) const;
        uint8_t rat_id;
        uint8_t rat_inst;
        uint8_t rat_index_mode;
};

class cf_export_mem_node: public cf_export_node {
public:
        cf_export_mem_node(uint32_t *bytecode);
private:
        void print_detail(std::ostream& os) const override;

        uint16_t array_base;
        uint8_t sel[4];
};

class cf_mem_stream_node: public cf_mem_node {
public:
        cf_mem_stream_node(uint32_t *bytecode);
private:
        void print_detail(std::ostream& os) const override;

        uint16_t array_base;
};

#endif // CF_NODE_H