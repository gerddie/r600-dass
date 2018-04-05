#include "cffetchnode.h"
#include <iostream>
#include <cassert>

namespace r600 {

using std::vector;
using std::string;

FetchNode::FetchNode(const GPRValue& src, const GPRValue& dst):
   m_src(src), m_dst(dst)
{
}

void FetchNode::encode_src(uint64_t& result) const
{
   result |= m_src.sel();
   result |= m_src.chan() << 24;
   if (m_src.rel())
      result |= 1 << 7;
}

void FetchNode::encode_dst(uint64_t& result) const
{
   result |= static_cast<uint64_t>(m_dst.sel()) << 32;
   if (m_dst.rel())
      result |= 1ul << 39;
}

void FetchNode::encode_dst_sel(uint64_t& result) const
{
   for (int i = 0; i < 4; ++i)
      result |= static_cast<uint64_t>(m_dst_swizzle[i]) << (41 + i*3);
}

void FetchNode::print_dst(std::ostream& os) const
{
   os << m_dst.sel();
   if (m_dst.rel())
      os << "[LoopIDX]";

   os << ".";
   for (auto& s: m_dst_swizzle) {
      os << Value::component_names[s];
   }
}

void FetchNode::print_src(std::ostream& os) const
{
   Value::PrintFlags flags;
   flags.index_mode = 4;
   m_src.print(os, flags);
}

void FetchNode::set_dst_sel(const std::vector<int>& dsel)
{
   m_dst_swizzle = dsel;
}

VertexFetchNode::VertexFetchNode(uint64_t bc0, uint64_t bc1):
   FetchNode(GPRValue((bc0 >> 17) & 0x3f, (bc0 >> 24) & 0x3, false, bc0 & (1 << 23), false),
             GPRValue((bc0 >> 32) & 0x3f, 0, false, bc0 & (1ul << 39), false)),
   m_vc_opcode(static_cast<EFetchInstr>(bc0 & 0x1f)),
   m_offset(bc1 & 0xffff),
   m_data_format(static_cast<EVTXDataFormat>((bc0 >> 54) & 0x3f)),
   m_num_format(static_cast<ENumFormat>((bc0 >> 60) & 3)),
   m_mega_fetch_count((bc0 >> 26) & 0x3f),
   m_buffer_id((bc0 >> 8) & 0xff),
   m_fetch_type(static_cast<EFetchType>((bc0 >> 5) & 3)),
   m_endian_swap(static_cast<EEndianSwap>((bc1 >> 16) & 3)),
   m_buffer_index_mode(static_cast<EBufferIndexMode>((bc1 >> 21) & 3))
{
   vector<int> dst_sel(4);
   for (int i = 0; i < 4; ++i)
      dst_sel[i] = (bc0 >> (41 + 3*i)) & 7;
   set_dst_sel(dst_sel);

   if (m_vc_opcode == vc_semantic) {
      m_semantic_id = (bc0 >> 32) & 0xff;
   }

#define COPY_BIT(BYTECODE, FLAG) \
   if (BYTECODE & FLAG ## _bit) \
      m_flags.set(FLAG)

   COPY_BIT(bc0, vtx_fetch_whole_quad);
   COPY_BIT(bc0, vtx_use_const_field);
   COPY_BIT(bc0, vtx_format_comp_signed);
   COPY_BIT(bc0, vtx_srf_mode);
   COPY_BIT(bc1, vtx_mega_fetch);
   COPY_BIT(bc1, vtx_buf_no_stride);
   COPY_BIT(bc1, vtx_alt_const);
#undef COPY_BIT

}

void VertexFetchNode::print(std::ostream& os) const
{
   static const string num_format_char[] = {"norm", "int", "scaled"};
   static const string endian_swap_code[] = {
      "noswap", "8in16", "8in32"
   };
   static const char buffer_index_mode_char[] = "_01E";
   static const char flag_char[] = "QCSZMBA";

   switch (m_vc_opcode) {
   case vc_fetch:
      os << "Fetch VTX R";
      print_dst(os);
      break;
   case vc_semantic:
      os << "Fetch VTX Semantic SID:" << m_semantic_id;
      break;
   case vc_get_buf_resinfo:
      os << "Fetch BufResinfo:";
      print_dst(os);
      break;
   default:
      os << "Fetch ERROR";
      return;
   }

   os << ", ";
   print_src(os);

   if (m_offset)
      os << "+" << m_offset;

   os << " BUFID:" << m_buffer_id
      << " FMT:(" << m_data_format
      << " " << num_format_char[m_num_format]
      << " " << endian_swap_code[m_endian_swap]
      << ")";
   if (m_buffer_index_mode > 0)
      os << " IndexMode:" << buffer_index_mode_char[m_buffer_index_mode];


   if (m_flags.test(vtx_mega_fetch))
      os << " MFC:" << m_mega_fetch_count;

   os << " Flags: ";
   for( int i = 0; i < vtx_unknwon; ++i) {
      if (i != vtx_mega_fetch)
         os << (m_flags.test(i) ? flag_char[i] : '_');
   }
}

uint64_t VertexFetchNode::create_bytecode_byte(int i) const
{
   assert(i < 2);
   uint64_t result = 0;

#define COPY_BIT(BYTECODE, FLAG) \
   if (m_flags.test(FLAG)) \
      BYTECODE |= FLAG ## _bit

   if (i == 0) {
      result |= static_cast<uint64_t>(m_vc_opcode);
      result |= static_cast<uint64_t>(m_fetch_type) << 5;
      result |= m_buffer_id << 8;
      result |= static_cast<uint64_t>(m_data_format) << 54;
      result |= static_cast<uint64_t>(m_num_format) << 60;

      encode_src(result);
      encode_dst_sel(result);
      if (m_vc_opcode == vc_semantic)
         encode_dst(result);
      else
         result |= static_cast<uint64_t>(m_semantic_id) << 32;

      COPY_BIT(result, vtx_fetch_whole_quad);
      COPY_BIT(result, vtx_use_const_field);
      COPY_BIT(result, vtx_format_comp_signed);
      COPY_BIT(result, vtx_srf_mode);
   } else {

      result |= m_offset;
      result |= static_cast<uint64_t>(m_endian_swap) << 16;
      result |= static_cast<uint64_t>(m_buffer_index_mode) << 21;

      COPY_BIT(result, vtx_mega_fetch);
      COPY_BIT(result, vtx_buf_no_stride);
      COPY_BIT(result, vtx_alt_const);
   }
#undef COPY_BIT
   return result;
}

}