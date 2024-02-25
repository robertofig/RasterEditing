@echo off

SET GDALInc=""
SET GDALLib=""

SET CompilerIncludes=/I ..\src /I ..\include /I %GDALInc%
SET CompilerFlags=/EHsc /Oi /O2 /nologo
SET LinkerFlags=/INCREMENTAL:NO

if not exist "..\build" mkdir "..\build"
pushd ..\build
REM cl ..\utils\raster-cut-util.cpp /Fe:raster-cut.exe %CompilerIncludes% %CompilerFlags% /link %GDALLib% %LinkerFlags%
cl ..\utils\raster-outline-util.cpp /Fe:raster-outline.exe %CompilerIncludes% %CompilerFlags% /link %GDALLib% %LinkerFlags%
popd