@echo off

set GYP_MSVS_VERSION=2017
set DEPOT_TOOLS_WIN_TOOLCHAIN=0
set GYP_MSVS_OVERRIDE_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community

gn gen out\Debug --args="target_cpu=\"x86\" is_clang=false is_debug=true" --ide="vs2017" --sln="Example" --winsdk="10.0.16299.0"

pause
