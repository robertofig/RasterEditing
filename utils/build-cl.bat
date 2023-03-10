@echo off

SET GDALInc=""
SET TinyBaseInc=""
SET GeoTypesInc=""
SET GDALLib=""

SET CompilerIncludes=/I ..\src /I %GDALInc% /I %TinyBaseInc% /I %GeoTypesInc%
SET CompilerFlags=/EHsc /Oi /O2 /nologo
SET LinkerFlags=/INCREMENTAL:NO

pushd ..\build
cl ..\utils\raster-cut-util.cpp /Fe:raster-cut.exe %CompilerIncludes% %CompilerFlags% /link %GDALLib% %LinkerFlags%
cl ..\utils\raster-outline-util.cpp /Fe:raster-outline.exe %CompilerIncludes% %CompilerFlags% /link %GDALLib% %LinkerFlags%
popd