#ifndef CFFETCHNODE_H
#define CFFETCHNODE_H

#include <r600/alu_defines.h>
#include <r600/node.h>
#include <r600/value.h>

namespace r600 {

class FetchNode : public node
{
public:
   using Pointer=std::shared_ptr<FetchNode>;

   enum EBufferIndexMode {
      bim_none,
      bim_zero,
      bim_one,
      bim_invalid
   };

   FetchNode(uint64_t bc0);

   static Pointer decode(uint64_t bc0, uint64_t bc1);
protected:
   void set_dst_sel(const std::vector<int>& dsel);
   void encode_src(uint64_t& result) const;
   void encode_dst(uint64_t& result) const;
   void encode_dst_sel(uint64_t& result) const;
   void print_dst(std::ostream& os) const;
   void print_src(std::ostream& os) const;
   void print_src_sel(std::ostream& os) const;
private:
   GPRValue m_src;
   GPRValue m_dst;
   std::vector<int> m_dst_swizzle;
};

using PFetchNode=FetchNode::Pointer;

class VertexFetchNode: public FetchNode{
public:

   VertexFetchNode(uint64_t bc0, uint64_t bc1);

   enum EFetchInstr {
      vc_fetch = 0,
      vc_semantic = 1,
      vc_get_buf_resinfo = 14
   };

   enum EFetchType {
      vertex_data = 0,
      instance_data = 1,
      no_index_offset = 2
   };

   enum EVTXDataFormat {
      fmt_invalid = 0,
      fmt_8 = 1,
      fmt_4_4 = 2,
      fmt_3_3_2 = 3,
      fmt_reserved_4 = 4,
      fmt_16 = 5,
      fmt_16_float = 6,
      fmt_8_8 = 7,
      fmt_5_6_5 = 8,
      fmt_6_5_5 = 9,
      fmt_1_5_5_5 = 10,
      fmt_4_4_4_4 = 11,
      fmt_5_5_5_1 = 12,
      fmt_32 = 13,
      fmt_32_float = 14,
      fmt_16_16 = 15,
      fmt_16_16_float = 16,
      fmt_8_24 = 17,
      fmt_8_24_float = 18,
      fmt_24_8 = 19,
      fmt_24_8_float = 20,
      fmt_10_11_11 = 21,
      fmt_10_11_11_float = 22,
      fmt_11_11_10 = 23,
      fmt_11_11_10_float = 24,
      fmt_2_10_10_10 = 25,
      fmt_8_8_8_8 = 26,
      fmt_10_10_10_2 = 27,
      fmt_x24_8_32_float = 28,
      fmt_32_32 = 29,
      fmt_32_32_float = 30,
      fmt_16_16_16_16 = 31,
      fmt_16_16_16_16_float = 32,
      fmt_reserved_33 = 33,
      fmt_32_32_32_32 = 34,
      fmt_32_32_32_32_float = 35,
      fmt_reserved_36 = 36,
      fmt_1 = 37,
      fmt_1_reversed = 38,
      fmt_gb_gr = 39,
      fmt_bg_rg = 40,
      fmt_32_as_8 = 41,
      fmt_32_as_8_8 = 42,
      fmt_5_9_9_9_sharedexp = 43,
      fmt_8_8_8 = 44,
      fmt_16_16_16 = 45,
      fmt_16_16_16_float = 46,
      fmt_32_32_32 = 47,
      fmt_32_32_32_float = 48,
      fmt_bc1 = 49,
      fmt_bc2 = 50,
      fmt_bc3 = 51,
      fmt_bc4 = 52,
      fmt_bc5 = 53,
      fmt_apc0 = 54,
      fmt_apc1 = 55,
      fmt_apc2 = 56,
      fmt_apc3 = 57,
      fmt_apc4 = 58,
      fmt_apc5 = 59,
      fmt_apc6 = 60,
      fmt_apc7 = 61,
      fmt_ctx1 = 62,
      fmt_reserved_63 = 63
   };

   enum ENumFormat {
      nf_norm = 0,
      nf_int = 1,
      nf_scaled = 2
   };

   enum EEndianSwap {
      es_none = 0,
      es_8in16 = 1,
      es_8in32 = 1
   };

   enum EFlagShift {
      vtx_fetch_whole_quad,
      vtx_use_const_field,
      vtx_format_comp_signed,
      vtx_srf_mode,
      vtx_buf_no_stride,
      vtx_alt_const,
      vtx_unknwon
   };

   static const std::vector<std::pair<int, uint64_t>> ms_flag_bits;

   static const uint64_t vtx_mega_fetch_bit = 1ul << 19;

private:
   void print(std::ostream& os) const override;
   uint64_t create_bytecode_byte(int i) const override;

   EFetchInstr m_vc_opcode;
   uint32_t m_offset;
   EVTXDataFormat m_data_format;
   ENumFormat m_num_format;
   bool m_is_mega_fetch;
   uint32_t m_mega_fetch_count;
   uint32_t m_buffer_id;
   EFetchType m_fetch_type;
   EEndianSwap m_endian_swap;
   EBufferIndexMode m_buffer_index_mode;
   std::bitset<16> m_flags;
   uint32_t m_semantic_id;
};

class TexFetchNode: public FetchNode {
public:
   enum ETexInst {
      tex_ld = 3,
      tex_get_res_info = 4,
      tex_get_num_samples = 5,
      tex_get_comp_lod = 6,
      tex_get_grad_h = 7,
      tex_get_grad_v = 8,
      tex_set_offs = 9,
      tex_keep_grad = 10,
      tex_set_grad_h = 11,
      tex_set_grad_v = 12,
      tex_sample = 16,
      tex_sample_l = 17,
      tex_sample_lb = 18,
      tex_sample_lz = 19,
      tex_sample_g = 20,
      tex_gather4 = 21,
      tex_sample_g_lb = 22,
      tex_gather4_o = 23,
      tex_sample_c = 24,
      tex_sample_c_l = 25,
      tex_sample_c_lb = 26,
      tex_sample_c_lz = 27,
      tex_sample_c_g = 28,
      tex_gather4_c = 29,
      tex_sample_c_g_lb = 30,
      tex_sample_c_o = 31
   };
   enum EInstMod  {
      im_ld_normal = 0,
      im_ldptr = 1,
      im_grad_coarse = 0,
      im_grad_fine = 1,
      im_gather4_x = 0,
      im_gather4_y = 1,
      im_gather4_z = 2,
      im_gather4_w = 3
   };

   enum ETexFlags {
      fetch_whole_quad,
      alt_const,
      coord_type_x,
      coord_type_y,
      coord_type_z,
      coord_type_w,
      tex_flag_last
   };

   TexFetchNode(uint64_t bc0, uint64_t bc1);

private:
   uint64_t create_bytecode_byte(int i) const override;
   void print(std::ostream& os) const override;
   const char *opname_from_opcode() const;

   static const std::vector<uint64_t> sm_tex_flag_bit;

   ETexInst m_tex_opcode;
   EInstMod m_inst_mode;
   uint32_t m_resource_id;
   uint32_t m_sampler_id;
   uint32_t m_load_bias;
   EBufferIndexMode m_resource_index_mode;
   EBufferIndexMode m_sampler_index_mode;
   std::vector<int> m_offset;
   std::vector<int> m_src_swizzle;
   std::bitset<8> m_flags;

};

class MemoryReadNode: public FetchNode {
public:
   enum EMemOp {
      rd_scratch = 0,
      rd_scatter = 2
   };

   enum EFlags {
      rd_whole_quad_mode,
      rd_uncached,
      rd_indexed,
      rd_signed,
      rd_srf_mode
   };

   MemoryReadNode(uint64_t bc0, uint64_t bc1);

private:
   uint64_t create_bytecode_byte(int i) const override;
   void print(std::ostream& os) const override;
   EMemOp m_mem_op;
   int m_elm_size;
   int m_mem_req_size;
   int m_burst_cnt;
   int m_data_format;
   int m_num_format_all;
   int m_address;
   int m_endian_swap;
   int m_array_size;

};

class GDSOpNode: public FetchNode {
public:

   GDSOpNode(uint64_t bc0, uint64_t bc1);

private:
   uint64_t create_bytecode_byte(int i) const override;
   void print(std::ostream& os) const override;

   int m_src_rel_mode;
   int m_dst_rel_mode;
   std::vector<int> m_src_sel;
   ESDOp m_gds_op;
};


}

#endif // CFFETCHNODE_H
