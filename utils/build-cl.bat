@echo off

SET GDALInc=""
SET GDALLib=""

SET CompilerIncludes=/I ..\src /I %GDALInc% /I ..\include\TinyBase\src /I ..\include\GeoTypes\src
SET CompilerFlags=/EHsc /Oi /O2 /nologo
SET LinkerFlags=/INCREMENTAL:NO

if not exist "..\build" mkdir "..\build"
pushd ..\build
cl ..\utils\raster-cut-util.cpp /Fe:raster-cut.exe %CompilerIncludes% %CompilerFlags% /link %GDALLib% %LinkerFlags%
cl ..\utils\raster-outline-util.cpp /Fe:raster-outline.exe %CompilerIncludes% %CompilerFlags% /link %GDALLib% %LinkerFlags%
popd