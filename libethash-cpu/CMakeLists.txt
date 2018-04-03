if (NOT MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
endif()

set(FILES
	CPUMiner.h
	CPUMiner.cpp
)

add_library(ethash-cpu ${FILES})

target_link_libraries(ethash-cpu ethcore ethash)
target_include_directories(ethash-cpu PUBLIC ${CUDA_INCLUDE_DIRS})
target_include_directories(ethash-cpu PRIVATE .. ${CMAKE_CURRENT_BINARY_DIR})
