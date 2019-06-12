set(CMAKE_RUNTIME_OUTPUT_DIRECTORY 			${PROJECT_BINARY_DIR}/output/bin/xsteg)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG 	${PROJECT_BINARY_DIR}/output/bin/xsteg)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE 	${PROJECT_BINARY_DIR}/output/bin/xsteg)

set(XSTEG_SOURCES
    src/main.cpp)

add_executable(xsteg ${XSTEG_SOURCES})
target_link_libraries(xsteg xsteg.core stb)