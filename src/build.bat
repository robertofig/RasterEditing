@echo off

SET GDALInclude=""
SET TinyBaseInclude=""
SET GeoTypesInclude=""
SET GDALLib=""

SET CompilerIncludes=/I %GDALInclude% /I %TinyBaseInclude% /I %GeoTypesInclude%
SET CompilerFlags=/EHsc /Zi
SET LinkerFlags=/INCREMENTAL:NO

pushd ..\build
cl ..\src\raster-cut-util.cpp %CompilerIncludes% %CompilerFlags% /link %GDALLib% %LinkerFlags%
cl ..\src\raster-outline-util.cpp %CompilerIncludes% %CompilerFlags% /link %GDALLib% %LinkerFlags%
popd