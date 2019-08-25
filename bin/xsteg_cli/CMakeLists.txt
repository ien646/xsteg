set(CMAKE_RUNTIME_OUTPUT_DIRECTORY 			${PROJECT_BINARY_DIR}/output/bin/xsteg_cli)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG 	${PROJECT_BINARY_DIR}/output/bin/xsteg_cli)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE 	${PROJECT_BINARY_DIR}/output/bin/xsteg_cli)

set(XSTEG_CLI_SOURCES
    src/main.cpp
    src/program_args.cpp
    src/utils.cpp)

add_executable(xsteg_cli ${XSTEG_CLI_SOURCES})
target_link_libraries(xsteg_cli xsteg.core stb strutils)