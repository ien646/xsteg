set(XSTEG_CORE_SOURCES
    src/image.cpp
    src/availability_map.cpp
    src/visual_data.cpp
    src/steganographer.cpp
    src/bit_tools.cpp
    src/bit_view.cpp)
    
add_library(xsteg.core STATIC ${XSTEG_CORE_SOURCES})
target_link_libraries(xsteg.core stb)
target_include_directories(xsteg.core PUBLIC include)