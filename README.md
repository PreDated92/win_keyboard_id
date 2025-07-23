# win_keyboard_id
A C++ file to subscribe and push keyboard events from unique keyboards on Windows. This compiles to a dll which can be used by other wrappers

# Visual Studio Code Compiler
### 64-bit DLL
1. Open "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
2. Navigate and compile with "cl /LD raw_input.cpp user32.lib /Fe:raw_input.dll"
