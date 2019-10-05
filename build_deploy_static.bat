git submodule update --init --recursive
md build_deploy_static
cd build_deploy_static
cmake -DCMAKE_GENERATOR_PLATFORM=x64 -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=on -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=on -DXSTEG_DEPLOY_STATIC=on .. -Wdev
cmake --build . --config Release
pause