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


#ifndef r600_bc__test_h
#define r600_bc__test_h

#include <r600/bc_test.h>

#include <gtest/gtest.h>
#include <cstdint>
#include <vector>
#include <sstream>

namespace r600 {
/* Below tests strife to check whether the bits are properly arranged in the
 * byte code. Doing this is a pre-requisit for testing the disassembler part
 * properly.
 */
::testing::AssertionResult SameBitmap(const std::vector<uint8_t>& spacing,
                                      const char* m_expr,
                                      const char* n_expr,
                                      uint64_t m,
                                      uint64_t n) {
  if (m == n)
    return ::testing::AssertionSuccess();

  std::ostringstream msg;
  msg << "Expected:" << m_expr << " == " << n_expr << "\n got\n"
      << " -" << std::setbase(16) << std::setw(16) << std::setfill('0') << m << "\n"
      << " +" << std::setw(16) << n << "\n"
      << " delta: w1: ";
  uint64_t delta = m ^ n;
  int tabs = 0;
  for (int i = 63; i >= 0; --i) {
     msg << ((delta & (1ul << i)) ? '1' : '0');
     if (i == spacing[tabs]) {
        msg << " ";
        ++tabs;
     }
     if (i == 32)
         msg << "\n        w0: ";
  }

  return ::testing::AssertionFailure() << msg.str();
}

}



#endif // BC_TEST_H
