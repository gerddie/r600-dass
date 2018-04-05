SET(SRC
   alu_defines.cpp
   alu_node.cpp
   cf_node.cpp
   cffetchnode.cpp
   disassembler.cpp
   node.cpp
   value.cpp)

SET(HEADERS
   alu_node.h
   alu_defines.h
   cf_node.h
   cffetchnode.h
   defines.h
   disassembler.h
   node.h
   value.h)

ADD_LIBRARY(r600-disass SHARED ${SRC})


ADD_LIBRARY(r600-test-helper SHARED bc_test.cpp)
TARGET_LINK_LIBRARIES(r600-test-helper
  ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY})

MACRO(NEW_TEST name)
ADD_EXECUTABLE(test-${name} test_${name}.cpp)
ADD_TEST(${name} test-${name})
TARGET_LINK_LIBRARIES(test-${name} r600-disass
  r600-test-helper)
ENDMACRO(NEW_TEST)

NEW_TEST(cf_parsing)
NEW_TEST(bc_create)
NEW_TEST(fetch_node)
NEW_TEST(alu_bc)
NEW_TEST(program_disass)
