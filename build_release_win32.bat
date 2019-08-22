git submodule update --init --recursive
md build_release_win32
cd build_release_win32
cmake -DCMAKE_GENERATOR_PLATFORM=WIN32 -DCMAKE_BUILD_TYPE=Release .. -Wdev
cmake --build . --config Release
pause