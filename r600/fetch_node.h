#ifndef CFFETCHNODE_H
#define CFFETCHNODE_H

#include <r600/node.h>
#include <r600/value.h>

namespace r600 {

class FetchNode : public node
{
public:
   enum EBufferIndexMode {
      bim_none,
      bim_zero,
      bim_one,
      bim_invalid
   };

   FetchNode(uint64_t bc0);
protected:
   void set_dst_sel(const std::vector<int>& dsel);
   void encode_src(uint64_t& result) const;
   void encode_dst(uint64_t& result) const;
   void encode_dst_sel(uint64_t& result) const;
   void print_dst(std::ostream& os) const;
   void print_src(std::ostream& os) const;
private:
   GPRValue m_src;
   GPRValue m_dst;
   std::vector<int> m_dst_swizzle;
};

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
      FMT_INVALID = 0,
      FMT_8 = 1,
      FMT_4_4 = 2,
      FMT_3_3_2 = 3,
      FMT_RESERVED_4 = 4,
      FMT_16 = 5,
      FMT_16_FLOAT = 6,
      FMT_8_8 = 7,
      FMT_5_6_5 = 8,
      FMT_6_5_5 = 9,
      FMT_1_5_5_5 = 10,
      FMT_4_4_4_4 = 11,
      FMT_5_5_5_1 = 12,
      FMT_32 = 13,
      FMT_32_FLOAT = 14,
      FMT_16_16 = 15,
      FMT_16_16_FLOAT = 16,
      FMT_8_24 = 17,
      FMT_8_24_FLOAT = 18,
      FMT_24_8 = 19,
      FMT_24_8_FLOAT = 20,
      FMT_10_11_11 = 21,
      FMT_10_11_11_FLOAT = 22,
      FMT_11_11_10 = 23,
      FMT_11_11_10_FLOAT = 24,
      FMT_2_10_10_10 = 25,
      FMT_8_8_8_8 = 26,
      FMT_10_10_10_2 = 27,
      FMT_X24_8_32_FLOAT = 28,
      FMT_32_32 = 29,
      FMT_32_32_FLOAT = 30,
      FMT_16_16_16_16 = 31,
      FMT_16_16_16_16_FLOAT = 32,
      FMT_RESERVED_33 = 33,
      FMT_32_32_32_32 = 34,
      FMT_32_32_32_32_FLOAT = 35,
      FMT_RESERVED_36 = 36,
      FMT_1 = 37,
      FMT_1_REVERSED = 38,
      FMT_GB_GR = 39,
      FMT_BG_RG = 40,
      FMT_32_AS_8 = 41,
      FMT_32_AS_8_8 = 42,
      FMT_5_9_9_9_SHAREDEXP = 43,
      FMT_8_8_8 = 44,
      FMT_16_16_16 = 45,
      FMT_16_16_16_FLOAT = 46,
      FMT_32_32_32 = 47,
      FMT_32_32_32_FLOAT = 48,
      FMT_BC1 = 49,
      FMT_BC2 = 50,
      FMT_BC3 = 51,
      FMT_BC4 = 52,
      FMT_BC5 = 53,
      FMT_APC0 = 54,
      FMT_APC1 = 55,
      FMT_APC2 = 56,
      FMT_APC3 = 57,
      FMT_APC4 = 58,
      FMT_APC5 = 59,
      FMT_APC6 = 60,
      FMT_APC7 = 61,
      FMT_CTX1 = 62,
      FMT_RESERVED_63 = 63
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
      vtx_mega_fetch,
      vtx_alt_const,
      vtx_unknwon
   };

   static const std::vector<std::pair<int, uint64_t>> ms_flag_bits;

   static const uint64_t vtx_fetch_whole_quad_bit = 1ul << 7;
   static const uint64_t vtx_use_const_field_bit = 1ul << 53;
   static const uint64_t vtx_format_comp_signed_bit = 1ul << 62;
   static const uint64_t vtx_srf_mode_bit = 1ul << 63;
   static const uint64_t vtx_buf_no_stride_bit = 1ul << 18;
   static const uint64_t vtx_mega_fetch_bit = 1ul << 19;
   static const uint64_t vtx_alt_const_bit = 1ul << 20;


private:
   void print(std::ostream& os) const override;
   uint64_t create_bytecode_byte(int i) const override;

   EFetchInstr m_vc_opcode;
   uint32_t m_offset;
   EVTXDataFormat m_data_format;
   ENumFormat m_num_format;
   uint32_t m_mega_fetch_count;
   uint32_t m_buffer_id;
   EFetchType m_fetch_type;
   EEndianSwap m_endian_swap;
   EBufferIndexMode m_buffer_index_mode;
   std::bitset<16> m_flags;
   uint32_t m_semantic_id;
};

class TexFetchNode: public FetchNode{
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

}

#endif // CFFETCHNODE_H