echo "-- Initializing git submodules -- "
git submodule init
git submodule update
md build_debug_win64
cd build_debug_win64
cmake -DCMAKE_GENERATOR_PLATFORM=x64 -DCMAKE_BUILD_TYPE=Debug -DBoost_NO_SYSTEM_PATHS=TRUE .. -Wdev
cmake --build . --config Debug
pause