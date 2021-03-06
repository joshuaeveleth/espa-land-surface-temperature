## NOTE - The products produced by this software have not been validated and are considered prototype.

## Land Surface Temperature 0.0.4 Release Notes
Release Date: August 2016

See git tag [2016_Aug-1]

This project contains application source code for producing Land Surface Temperature products.

## Product Descriptions
See the [lst_readme_vX.X_for_evaluators.pdf](http://espa.cr.usgs.gov/downloads/provisional/land_surface_temperature) product guide (which is an unofficial and provisional version) for information about the Land Surface Temperature products.

## Release Notes
* README.md Documentation Changes Only

## Installation

### Dependencies
* ESPA raw binary libraries, tools, and it's dependencies. [Found here](https://github.com/USGS-EROS/espa-product-formatter)
* Python 2.7+
* [GDAL](http://www.gdal.org/) 1.11.1
  - The command line tools are utilized for some of the processing steps.
* MODTRAN
  - Used to generate upwelling, downwelling and transmittance from NARR data points.
* Additional Python libraries
  - numpy: ```pip install numpy```
  - gdal: ```pip install gdal```
  - requests: ```pip install requests```

### Environment Variables
* Required for building this software (For an example see setup-build-environment.sh)
```
export PREFIX="path_to_Installation_Directory"
export XML2INC="path_to_LIBXML2_include_files"
export XML2LIB="path_to_LIBXML2_libraries"
export LZMALIB="path_to_LZMA_libraries_for_linking"
export ZLIBLIB="path_to_ZLIB_libraries_for_linking"
export ESPAINC="path_to_ESPA_PRODUCT_FORMATTER_include_files"
export ESPALIB="path_to_ESPA_PRODUCT_FORMATTER_libraries"
```

### Build Steps
* Clone the repository and replace the defaulted version(master) with this
  version of the software
```
git clone https://github.com/USGS-EROS/espa-land-surface-temperature.git
cd espa-land-surface-temperature
git checkout version_<version>
```
* Build and install the application specific software
```
make
make install
```

## Usage
See `land_surface_temperature.py --help` for command line details.

### Environment Variables
* PATH - May need to be updated to include the following
  - `$PREFIX/bin`
* LST_AUX_DIR - Points to the local NARR data archive.  See [LST Auxiliary Data](lst_auxiliary_data/README.md).
  - `export LST_AUX_DIR="/usr/local/auxiliaries/LST/NARR"`
* LST_DATA_DIR - Points to the installed static file
  - `export LST_DATA_DIR="/usr/local/espa-land-surface-temperature/lst/static_data"`
* MODTRAN_PATH - Points to the installed MODTRAN location
  - `export MODTRAN_PATH="/usr/local/bin"`
* MODTRAN_DATA_DIR - Points to the directory containing the MODTRAN "DATA" directory
  - `export MODTRAN_DATA_DIR="/usr/local/auxiliaries/MODTRAN_DATA"`
* ASTER_GED_SERVER_NAME
  - `export ASTER_GED_SERVER_NAME="e4ftl01.cr.usgs.gov"`
* ASTER_GED_SERVER_PATH
  - `export ASTER_GED_SERVER_PATH="/ASTT/AG100.003/2000.01.01/"`

### Data Processing Requirements
This version of the Land Surface Temperature application requires the input products to be in the ESPA internal file format.

The following input data are required to generate the Land Surface Temperature products:
* Level 1 Product
  - The thermal band from the level 1 product is also utilized.
* Top of Atmosphere Reflectance (TOA)
  - TOA products can be generated using the [LEDAPS](https://github.com/USGS-EROS/espa-surface-reflectance) or [L8_SR](https://github.com/USGS-EROS/espa-surface-reflectance) software found in our [espa-surface-reflectance](https://github.com/USGS-EROS/espa-surface-reflectance) project.  TOA products can also be generated through our on-demand processing system [ESPA](https://espa.cr.usgs.gov).  Be sure to select the ENVI output format if using our [ESPA](https://espa.cr.usgs.gov) on-demand system.
* Elevation
  - Elevation data is expected to be in the same projection, resolution, and image dimensions as the TOA products as well as identified in the XML metadata file and in the ENVI file format.  The filename is also expected be `<sceneid>_elevation.img`: where <b>sceneid</b> is the Landsat Scene ID of the data being processed.
* ASTER GED
  - ASTER GED data can be [found here](https://lpdaac.usgs.gov/data_access/data_pool).  However it will automatically be retrieved for you as needed and cleaned up.
* North American Regional Reanalysis (NARR)
  - NARR data, it would be best to utilize the Land Surface Temperature Auxiliary Archive Generation software provided in this project to download and build your own archive for the dates you require.  This software archives a reduced set of parameters from each source file.  (Only the parameters required for LST generation.)  See [LST Auxiliary Data](lst_auxiliary_data/README.md).

### Data Postprocessing
After compiling the [espa-product-formatter](https://github.com/USGS-EROS/espa-product-formatter) libraries and tools, the `convert_espa_to_gtif` and `convert_espa_to_hdf` command-line tools can be used to convert the ESPA internal file format to HDF or GeoTIFF.  Otherwise the data will remain in the ESPA internal file format, which includes each band in the ENVI file format (i.e. raw binary file with associated ENVI header file) and an overall XML metadata file.

## More Information
This project is provided by the US Geological Survey (USGS) Earth Resources Observation and Science (EROS) Land Satellite Data Systems (LSDS) Science Research and Development (LSRD) Project. For questions regarding products produced by this source code, please contact the Landsat Contact Us page and specify USGS CDR/ECV in the "Regarding" section. https://landsat.usgs.gov/contactus.php
