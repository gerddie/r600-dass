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
                                      uint64_t n);

class BytecodeTest: public testing::Test {
protected:
   void check(const char *s_data, const char *s_expect,
              uint64_t data, uint64_t expect) const {
      GTEST_ASSERT_(SameBitmap(spacing, s_data, s_expect, data, expect),
                    GTEST_NONFATAL_FAILURE_);
   }
   void set_spacing(const std::vector<uint8_t>& s) {
      spacing = s;
   }
private:
   std::vector<uint8_t> spacing;
};

#define TEST_EQ(X, Y) check(#X, #Y, X, Y)

}



#endif // BC_TEST_H
