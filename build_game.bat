"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build"\vcvarsall x64 && ^
cl saguinus.cpp /Zi /link /DLL /EXPORT:updateGameState /OUT:libsaguinus.dll