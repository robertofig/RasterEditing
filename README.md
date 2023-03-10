# Raster Tools

Library for editing and processing geographic raster images.

It provides functionalities that are either not commonly found on GIS programs, or that require several steps to achieve a result (which both consume a lot of time and memory). Its goal is to have dedicated functions with low memory footprint, no intermediary representation, and fast execution times, to either be used in real time applications or for bulk processing.

The tools currently included are:

* [raster-outline.h](src/raster-outline.h): Creates polygonal outlines around groups of pixels in an image.
* [raster-cur.h](src/raster-cut.h): Mosaics and cuts several rasters into a single image given a cut polygon.

#### 1) Raster Outline
The raster outline tool takes a target pixel value and a source raster, and outputs a list of polygons around groups of pixels that correspond to the target value in some way, such as being larger than the target value, or different from it.

**Example 1**: Creating an outline around valid pixels of the image by passing the image NoData as target value, and selecting all pixels that are *not* it.

![Outline of valid raster pixels](https://github.com/robertofig/RasterTools/blob/master/docs/outline-1.png?raw=true)

**Example 2**: Inspecting the raster interior for out-of-bounds pixels by passing a value that should not be present in it and outlining all matching pixels.

![Out-of-bounds values inside image rectangle](https://github.com/robertofig/RasterTools/blob/master/docs/outline-2.png?raw=true)

**Example 3**: Selecting region from DEM that stands above historical high tide level, by passing that level as target value and selecting all pixels above it.

![Polygons represent areas never before flooded](https://github.com/robertofig/RasterTools/blob/master/docs/outline-3.png?raw=true)

#### 2) Raster Cut

The raster cut tool takes a polygon and a list of rasters that overlap it, and returns a single raster of the shape of the polygon. If there is only one raster in the list, it cuts that raster to the polygon's boundary; if several are present, it also mosaics them into a single imagem.

**Example 1**: Two Landsat-8 images being cut and mosaicked to a starry shape.

![Rasters cut from polygon](https://github.com/robertofig/RasterTools/blob/master/docs/cut-1.png?raw=true)

## How to use?

The header files contain the struct and function declarations, and the .cpp files contain the implementations. The header files contain instructions on how to use the images

The library files can be moved in whole to the project directory, in which case the header files will include the .cpp files in the translation unit. They have very short compile times, even when linking with its dependencies, which makes this approach feasible.

Alternatively, one can build these into objects or static library, in which case passing `RASTER_TOOLS_STATIC_LINKING` as a preprocessing symbol when compiling the project will prevent the header files from including the implementation files.

## Dependencies

This library currently builds on top of [TinyBase](https://github.com/robertofig/TinyBase) and [GeoTypes](https://github.com/robertofig/GeoTypes) for basic memory operations and geometric manipulation. It also requires [GDAL](https://github.com/OSGeo/gdal) for opening and reading raster files (version 2.1 or above).

## Utilities

Some test utilities that showcase some usage of the tools are present in the [Utils](https://github.com/robertofig/RasterTools/utils) folder. To build them, open the build script and set the variables `GDALInc`, `TinyBaseInc` and `GeoTypesInc` to the path with the header files of these libraries, and `GDALLib` to the path with the import library for linking GDAL.

## License

MIT open source license.