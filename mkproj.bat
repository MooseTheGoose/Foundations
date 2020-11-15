@echo off

PUSHD src
CL /Zi /EHsc /Fe:..\build\foundations.exe /Fd:..\build\foundations.pdb /I ..\include *.cpp
DEL *.obj
POPD

