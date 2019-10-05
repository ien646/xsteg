set(XSTEG_CORE_SOURCES    
    src/availability_map.cpp
    src/bit_tools.cpp
    src/bit_view.cpp
    src/image.cpp   
    src/steganographer.cpp
    src/synced_print.cpp
    src/task_queue.cpp
    src/visual_data.cpp)
	
set(XSTEG_CORE_HEADERS
    include/xsteg/availability_map.hpp
    include/xsteg/bit_tools.hpp
    include/xsteg/bit_view.hpp
    include/xsteg/image.hpp
    include/xsteg/pixel_availability.hpp
    include/xsteg/steganographer.hpp
    include/xsteg/synced_print.hpp
    include/xsteg/task_queue.hpp
    include/xsteg/visual_data.hpp
)

find_package(Threads)

add_library(xsteg.core ${XSTEG_CORE_SOURCES} ${XSTEG_CORE_HEADERS})

target_link_libraries(xsteg.core 
    stb
    strutils
    ${CMAKE_THREAD_LIBS_INIT})
    
target_include_directories(xsteg.core PUBLIC include)

if(XSTEG_DEPLOY_STATIC)
	FILE(COPY include DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
endif()