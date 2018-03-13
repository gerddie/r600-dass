#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <r600/cf_node.h>

#include <vector>
#include <memory>
#include <cstdint>
#include <string>

class disassembler
{
public:
   disassembler(const std::vector<uint64_t> &bc);

   std::string as_string() const;
private:
   enum ECFNodeType {
      nt_cf_native,
      nt_cf_alu,
      nt_cf_export,
      nt_cf_mem_export,
      nt_cf_mem_rat,
      nt_cf_mem_ring,
      nt_cf_mem_scratch,
      nt_cf_mem_stream,
      nt_cf_unknown
   };

   bool require_two_quadwords(uint64_t bc);
   ECFNodeType get_cf_node_type(uint64_t bc);


   std::vector<cf_node::pointer> programm;
};

#endif // DISASSEMBLER_H