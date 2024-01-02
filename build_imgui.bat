@echo off
set LIB_VCPKG="F:\Env\vcpkg\installed\x64-windows\lib"
set INC_VCPKG="F:\Env\vcpkg\installed\x64-windows\include"
set INC_IMGUI=".\imgui"

set CommonCompileFlags=-MT -nologo -fp:fast -EHs -Od -WX- -W4 -Oi -GR- -Gm- -GS -wd4100 -wd4201 -wd4505 -FC -Z7 /I%INC_VCPKG% /I%INC_IMGUI%
set CommonLinkFlags=-opt:ref -incremental:no /NODEFAULTLIB:msvcrt /SUBSYSTEM:CONSOLE /LIBPATH:%LIB_VCPKG%  

if not exist build mkdir build
pushd build

cl %CommonCompileFlags% ..\imgui\imgui.cpp /c /EHsc
cl %CommonCompileFlags% ..\imgui\imgui_demo.cpp /c /EHsc
cl %CommonCompileFlags% ..\imgui\imgui_draw.cpp /c /EHsc
cl %CommonCompileFlags% ..\imgui\imgui_impl_dx11.cpp /c /EHsc
cl %CommonCompileFlags% ..\imgui\imgui_impl_win32.cpp /c /EHsc
cl %CommonCompileFlags% ..\imgui\imgui_tables.cpp /c /EHsc
cl %CommonCompileFlags% ..\imgui\imgui_widgets.cpp /c /EHsc

lib imgui.obj imgui_demo.obj imgui_draw.obj imgui_impl_dx11.obj imgui_impl_win32.obj imgui_tables.obj imgui_widgets.obj /OUT:imgui.lib /MACHINE:X64

popd