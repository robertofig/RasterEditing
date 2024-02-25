#!/bin/sh

set -u

CUT='raster-cut'
OUT='raster-outline'
CompileOpts='-I../src -I../include -g -Wall -fpermissive'
LinkerOpts='-lm -lgdal -w'

mkdir -p ../build
cd ../build
g++ -o ${CUT} ../tests/${CUT}-util.cpp ${CompileOpts} ${LinkerOpts}
g++ -o ${OUT} ../tests/${OUT}-util.cpp ${CompileOpts} ${LinkerOpts}
cd ../tests