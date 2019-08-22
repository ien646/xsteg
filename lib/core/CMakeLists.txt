set(XSTEG_CORE_SOURCES    
    src/availability_map.cpp
    src/bit_tools.cpp
    src/bit_view.cpp
    src/image.cpp   
    src/runtime_settings.cpp
    src/steganographer.cpp
    src/synced_print.cpp
    src/task_queue.cpp
    src/visual_data.cpp)
    
find_package(Threads)

add_library(xsteg.core STATIC ${XSTEG_CORE_SOURCES})

target_link_libraries(xsteg.core 
    stb     
    ${CMAKE_THREAD_LIBS_INIT})
    
target_include_directories(xsteg.core PUBLIC include)