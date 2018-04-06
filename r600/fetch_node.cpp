#include <r600/fetch_node.h>
#include <iostream>
#include <iomanip>
#include <cassert>

namespace r600 {

using std::vector;
using std::string;
using std::ostringstream;

const char *fmt_descr[64] = {
   "INVALID",
   "8",
   "4_4",
   "3_3_2",
   "RESERVED_4",
   "16",
   "16F",
   "8_8",
   "5_6_5",
   "6_5_5",
   "1_5_5_5",
   "4_4_4_4",
   "5_5_5_1",
   "32",
   "32F",
   "16_16",
   "16_16F",
   "8_24",
   "8_24F",
   "24_8",
   "24_8F",
   "10_11_11",
   "10_11_11F",
   "11_11_10",
   "11_11_10F",
   "2_10_10_10",
   "8_8_8_8",
   "10_10_10_2",
   "X24_8_32F",
   "32_32",
   "32_32F",
   "16_16_16_16",
   "16_16_16_16F",
   "RESERVED_33",
   "32_32_32_32",
   "32_32_32_32F",
   "RESERVED_36",
   "1",
   "1_REVERSED",
   "GB_GR",
   "BG_RG",
   "32_AS_8",
   "32_AS_8_8",
   "5_9_9_9_SHAREDEXP",
   "8_8_8",
   "16_16_16",
   "16_16_16F",
   "32_32_32",
   "32_32_32F",
   "BC1",
   "BC2",
   "BC3",
   "BC4",
   "BC5",
   "APC0",
   "APC1",
   "APC2",
   "APC3",
   "APC4",
   "APC5",
   "APC6",
   "APC7",
   "CTX1",
   "RESERVED_63"
};

FetchNode::FetchNode(uint64_t bc0):
   m_src((bc0 >> 16) & 0x7f, (bc0 >> 24) & 0x3,
         false, bc0 & (1 << 23), false),
   m_dst((bc0 >> 32) & 0x7f, 0,
         false, bc0 & (1ul << 39), false),
   m_dst_swizzle(4)
{
   for (int i = 0; i < 4; ++i)
      m_dst_swizzle[i] = (bc0 >> (41 + 3*i)) & 7;
}

FetchNode::Pointer FetchNode::decode(uint64_t bc0, uint64_t bc1)
{
   int opcode = bc0 & 0x1f;
   switch (opcode) {
   case 0:
   case 1:
   case 14:
      return Pointer(new VertexFetchNode(bc0, bc1));
   default:
      return Pointer(new TexFetchNode(bc0, bc1));
   }
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

void FetchNode::print_src_sel(std::ostream& os) const
{
   os << 'R';
   if (m_src.rel())
      os << '[';
   os << m_src.sel();
   if (m_src.rel())
      os << "+LoopIdx]";
   os << ".";
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
   FetchNode(bc0),
   m_vc_opcode(static_cast<EFetchInstr>(bc0 & 0x1f)),
   m_offset(bc1 & 0xffff),
   m_data_format(static_cast<EVTXDataFormat>((bc0 >> 54) & 0x3f)),
   m_num_format(static_cast<ENumFormat>((bc0 >> 60) & 3)),
   m_is_mega_fetch(bc1 & vtx_mega_fetch_bit),
   m_mega_fetch_count((bc0 >> 26) & 0x3f),
   m_buffer_id((bc0 >> 8) & 0xff),
   m_fetch_type(static_cast<EFetchType>((bc0 >> 5) & 3)),
   m_endian_swap(static_cast<EEndianSwap>((bc1 >> 16) & 3)),
   m_buffer_index_mode(static_cast<EBufferIndexMode>((bc1 >> 21) & 3))
{
   if (m_vc_opcode == vc_semantic) {
      m_semantic_id = (bc0 >> 32) & 0xff;
   }

   const uint64_t bc[2] = {bc0, bc1};
   for (int i = 0; i < vtx_unknwon; ++i){
      if (bc[ms_flag_bits[i].first] & ms_flag_bits[i].second)
         m_flags.set(i);
   }
}

const std::vector<std::pair<int, uint64_t>>
VertexFetchNode::ms_flag_bits = {
   {0, 1ul << 7},
   {0, 1ul << 53},
   {0, 1ul << 62},
   {0, 1ul << 63},
   {1, 1ul << 18},
   {1, 1ul << 20},
};

void VertexFetchNode::print(std::ostream& os) const
{
   static const string num_format_char[] = {"norm", "int", "scaled"};
   static const string endian_swap_code[] = {
      "noswap", "8in16", "8in32"
   };
   static const char buffer_index_mode_char[] = "_01E";
   static const char *flag_string[] = {"WQM",  "CF", "signed", "no_zero",
                                       "nostride", "AC"};

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
      << " FMT:(" << fmt_descr[m_data_format]
      << " " << num_format_char[m_num_format]
      << " " << endian_swap_code[m_endian_swap]
      << ")";
   if (m_buffer_index_mode > 0)
      os << " IndexMode:" << buffer_index_mode_char[m_buffer_index_mode];


   if (m_is_mega_fetch)
      os << " MFC:" << m_mega_fetch_count;
   else
      os << " mfc*:" << m_mega_fetch_count;

   if (m_flags.any()) {
      os << " Flags:";
      for( int i = 0; i < vtx_unknwon; ++i) {
         if (m_flags.test(i))
            os << ' ' << flag_string[i];
      }
   }
}

uint64_t VertexFetchNode::create_bytecode_byte(int i) const
{
   assert(i < 2);
   uint64_t result = 0;

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

      for (int i = 0; i < vtx_buf_no_stride; ++i){
         assert(ms_flag_bits[i].first == 0);
         if (m_flags.test(i))
            result |= ms_flag_bits[i].second;
      }
   } else {

      result |= m_offset;
      result |= static_cast<uint64_t>(m_endian_swap) << 16;
      result |= static_cast<uint64_t>(m_buffer_index_mode) << 21;

      if (m_is_mega_fetch)
         result |= vtx_mega_fetch_bit;

      for (int i = vtx_buf_no_stride; i < vtx_unknwon; ++i){
         assert(ms_flag_bits[i].first == 1);
         if (m_flags.test(i))
            result |= ms_flag_bits[i].second;
      }
   }
   return result;
}

TexFetchNode::TexFetchNode(uint64_t bc0, uint64_t bc1):
   FetchNode(bc0),
   m_tex_opcode(static_cast<ETexInst>(bc0 & 0x1f)),
   m_inst_mode(static_cast<EInstMod>((bc0 >> 5) & 3)),
   m_resource_id((bc0 >> 8) & 0xff),
   m_sampler_id((bc1 >> 15) & 0x1f),
   m_load_bias((bc0 >> 21) & 0x3f),
   m_resource_index_mode(static_cast<EBufferIndexMode>((bc0 >> 25) & 0x3)),
   m_sampler_index_mode(static_cast<EBufferIndexMode>((bc0 >> 27) & 0x3)),
   m_offset(3),
   m_src_swizzle(4)
{
   for (int i = 0; i < tex_flag_last; ++i) {
      if (bc0 & sm_tex_flag_bit[i])
         m_flags.set(i);
   }

   for (int i = 0; i < 3; ++i)
      m_offset[i] = (bc1 >> 5*i) & 0x1f;

   for (int i = 0; i < 4; ++i)
      m_src_swizzle[i] = (bc1 >> (3*i + 20)) & 0x7;

}

void TexFetchNode::print(std::ostream& os) const
{
   ostringstream os_help;
   os_help << opname_from_opcode();
   switch (m_tex_opcode) {
   case tex_ld:
      if (m_inst_mode == im_ldptr)
         os_help << " (ptr)";
   break;
   case tex_gather4:
      os_help << " (" << Value::component_names[m_inst_mode] << ")";
   break;
   case tex_get_grad_h:
   case tex_get_grad_v:
      if (m_inst_mode == im_grad_fine)
         os_help << "(fine)";
      else
         os_help << "(coarse)";
      break;
   default:
      break;
   }

   os << std::setw(15) << std::left << os_help.str();
   os << "R";
   print_dst(os);
   os << ", ";
   print_src_sel(os);
   for (int i = 0; i < 4; ++i)
      os << Value::component_names[m_src_swizzle[i]];

   if (m_offset[0] != 0 || m_offset[1] != 0 || m_offset[2] != 0)  {
      os << "+[" << m_offset[0] << ", " << m_offset[1]
         << ", " << m_offset[2] << "]";
   }

   os << ", RID:" << m_resource_id;
   os << ", SID:" << m_sampler_id;

   os << " CT:";
   for (int i = coord_type_x; i < tex_flag_last; ++i)
      os << (m_flags.test(i) ? 'n' : 'u');

   if (m_tex_opcode == tex_sample_c_g_lb ||
       m_tex_opcode == tex_sample_c_lb ||
       m_tex_opcode == tex_sample_g_lb ||
       m_tex_opcode == tex_sample_lb) {
      os << " LB:" << std::setbase(16)  << m_load_bias << std::setbase(10);
   }

   if (m_flags.test(fetch_whole_quad))
      os << " WQM";
   if (m_flags.test(alt_const))
      os << " AC";
}

const char *TexFetchNode::opname_from_opcode() const
{
   switch (m_tex_opcode) {
   case tex_ld: return "LD";
   case tex_get_res_info: return  "GET_RES_INFO";
   case tex_get_num_samples: return  "GET_NUM_SAMPLES";
   case tex_get_comp_lod: return "GET_COMP_LOD";
   case tex_get_grad_h: return "GET_GRAD_H";
   case tex_get_grad_v: return "GET_GRAD_V";
   case tex_set_offs: return "SET_OFFSET";
   case tex_keep_grad: return "KEEP_GRAD";
   case tex_set_grad_h: return "SET_GRAD_H";
   case tex_set_grad_v: return "SET_GRAD_V";
   case tex_sample: return "SAMPLE";
   case tex_sample_l: return "SAMPLE_L";
   case tex_sample_lb: return "SAMPLE_LB";
   case tex_sample_lz: return "SAMPLE_LZ";
   case tex_sample_g: return "SAMPLE_G";
   case tex_gather4: return "GATHER4";
   case tex_sample_g_lb: return "SAMPLE_G_LB";
   case tex_gather4_o: return "GATHER4_O";
   case tex_sample_c: return "SAMPLE_C";
   case tex_sample_c_l: return "SAMPLE_C_L";
   case tex_sample_c_lb: return "SAMPLE_C_LB";
   case tex_sample_c_lz: return "SAMPLE_C_LZ";
   case tex_sample_c_g: return "SAMPLE_C_G";
   case tex_gather4_c: return "GATHER4_C";
   case tex_sample_c_g_lb: return "SAMPLE_C_G_LB";
   case tex_sample_c_o: return "SAMPLE_C_O";
   default: return "UNKNOWN";
   };
}

uint64_t TexFetchNode::create_bytecode_byte(int i) const
{
   assert(i < 2);
   uint64_t result = 0;

   if (i == 0) {
      result |= m_tex_opcode;
      result |= m_inst_mode << 5;
      result |= m_resource_id << 8;
      result |= m_resource_index_mode << 25;
      result |= m_sampler_index_mode << 27;
      result |= static_cast<uint64_t>(m_load_bias) << 53;

      encode_src(result);
      encode_dst_sel(result);
      encode_dst(result);

      for (int i = 0; i < tex_flag_last; ++i){
         if (m_flags.test(i))
            result |= sm_tex_flag_bit[i];
      }

   } else {
      result |= static_cast<uint64_t>(m_sampler_id) << 15;

      for (int i = 0; i < 3; ++i)
         result |= m_offset[i] << (5*i);

      for (int i = 0; i < 4; ++i)
         result |= static_cast<uint64_t>(m_src_swizzle[i])
                   << ((3*i) + 20);
   }
   return result;
}

const vector<uint64_t> TexFetchNode::sm_tex_flag_bit = {
   1 << 7,
   1 << 24,
   1 << 28,
   1 << 29,
   1 << 30,
   1ul  << 31
};

}