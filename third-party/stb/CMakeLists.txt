set(STB_SOURCES
    src/stb_image.impl.c
    src/stb_image_resize.impl.c
    src/stb_image_write.impl.c)
    
set(STB_HEADERS
    include/*.h
    include/*.hpp
)

add_library(stb ${STB_SOURCES})
target_include_directories(stb PUBLIC include)