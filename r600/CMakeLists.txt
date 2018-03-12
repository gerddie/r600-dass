SET(SRC
  cf_node.cpp
  node.cpp
  value.cpp)

SET(HEADERS 
  cf_node.h
  defines.h
  node.h
  value.h)

ADD_LIBRARY(r600-disass SHARED ${SRC})




