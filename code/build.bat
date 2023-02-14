@echo off

SET CompilerIncludes=/I "C:\SIG\GDAL-305\include" /I Z:\TinyBase\src /I Z:\GeoTypes\src
SET CompilerFlags=/EHsc /Zi
SET LinkerIncludes="C:\SIG\GDAL-305\lib\gdal_i.lib"
SET LinkerFlags=/INCREMENTAL:NO

pushd ..\build
REM cl ..\code\raster-cut-util.cpp %CompilerIncludes% %CompilerFlags% /link %LinkerIncludes% %LinkerFlags%
cl ..\code\raster-outline-util.cpp %CompilerIncludes% %CompilerFlags% /link %LinkerIncludes% %LinkerFlags%
popd