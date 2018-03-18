SET(SRC
   alu_node.cpp
   cf_node.cpp
   disassembler.cpp
   node.cpp
   value.cpp)

SET(HEADERS
   alu_node.h   
   cf_node.h
   defines.h
   disassembler.h
   node.h
   value.h)

ADD_LIBRARY(r600-disass SHARED ${SRC})


MACRO(NEW_TEST name)
ADD_EXECUTABLE(test-${name} test_${name}.cpp)
ADD_TEST(${name} test-${name})
TARGET_LINK_LIBRARIES(test-${name} r600-disass
   ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY})
ENDMACRO(NEW_TEST)

NEW_TEST(cf_parsing)
NEW_TEST(bc_create)

