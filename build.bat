"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build"\vcvarsall x64 && ^
cl win_saguinus.cpp /Zi /Fesaguinus /link User32.lib D3D11.lib DXGI.lib D3DCompiler.lib Xinput.lib Xaudio2.lib Ole32.lib && ^
cl saguinus.cpp /Zi /link /DLL /EXPORT:updateGameState /OUT:libsaguinus.dll