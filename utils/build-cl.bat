@echo off

SET GDALInc="E:\Ferramentas\GDAL-305\include"
SET GDALLib="E:\Ferramentas\GDAL-305\lib\gdal_i.lib"

REM SET CompilerIncludes=/I ..\src /I %GDALInc% /I ..\include\TinyBase\src /I ..\include\GeoTypes\src
SET CompilerIncludes=/I ..\src /I %GDALInc% /I Z:\TinyBase\src /I Z:\GeoTypes\src
SET CompilerFlags=/EHsc /Oi /O2 /nologo
SET LinkerFlags=/INCREMENTAL:NO

if not exist "..\build" mkdir "..\build"
pushd ..\build
REM cl ..\utils\raster-cut-util.cpp /Fe:raster-cut.exe %CompilerIncludes% %CompilerFlags% /link %GDALLib% %LinkerFlags%
cl ..\utils\raster-outline-util.cpp /Fe:raster-outline.exe %CompilerIncludes% %CompilerFlags% /link %GDALLib% %LinkerFlags%
popd