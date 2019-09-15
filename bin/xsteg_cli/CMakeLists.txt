set(CMAKE_RUNTIME_OUTPUT_DIRECTORY 			${PROJECT_BINARY_DIR}/output/bin/xsteg_cli)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG 	${PROJECT_BINARY_DIR}/output/bin/xsteg_cli)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE 	${PROJECT_BINARY_DIR}/output/bin/xsteg_cli)

set(XSTEG_CLI_SOURCES
    src/main.cpp
    src/program_args.cpp
    src/utils.cpp)

set(XSTEG_CLI_HEADERS
	src/*.hpp
)

add_executable(xsteg_cli ${XSTEG_CLI_SOURCES} ${XSTEG_CLI_HEADERS})
target_link_libraries(xsteg_cli xsteg.core stb strutils)