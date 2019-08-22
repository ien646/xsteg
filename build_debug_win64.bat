git submodule update --init --recursive
md build_debug_win64
cd build_debug_win64
cmake -DCMAKE_GENERATOR_PLATFORM=x64 -DCMAKE_BUILD_TYPE=Debug .. -Wdev
cmake --build . --config Debug
pause