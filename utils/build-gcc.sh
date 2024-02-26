#!/bin/sh

set -u

CUT='raster-cut'
OUT='raster-outline'
CompileOpts='-I../src -I../include -g -Wall -fpermissive -mavx2'
LinkerOpts='-lm -lgdal -w'

mkdir -p ../build
cd ../build
g++ -o ${CUT} ../utils/${CUT}-util.cpp ${CompileOpts} ${LinkerOpts}
g++ -o ${OUT} ../utils/${OUT}-util.cpp ${CompileOpts} ${LinkerOpts}
cd ../utils