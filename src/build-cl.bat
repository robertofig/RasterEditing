@echo off

SET GDALInc=""
SET TinyBaseInc=""
SET GeoTypesInc=""
SET GDALLib=""

SET CompilerIncludes=/I %GDALInc% /I %TinyBaseInc% /I %GeoTypesInc%
SET CompilerFlags=/EHsc /Zi /nologo
SET LinkerFlags=/INCREMENTAL:NO

pushd ..\build
cl ..\src\raster-cut-util.cpp %CompilerIncludes% %CompilerFlags% /link %GDALLib% %LinkerFlags%
cl ..\src\raster-outline-util.cpp %CompilerIncludes% %CompilerFlags% /link %GDALLib% %LinkerFlags%
popd