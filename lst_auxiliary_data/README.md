## Land Surface Temperature Auxiliary Archive Generation 0.0.4 Release Notes
Release Date: August 2016

See git tag [2016_Aug-1]

The scripts in this directory facilitate building an archive of NARR data utilized by the Land Surface Temperature processing during product generation.  <b>Note:</b> Significant disk space is required to archive all of the NARR data, even for the reduced files this software produces and archives.

## Script Descriptions

#### build_narr_aux_archive_from_CISL_RDA.py

This script is used to retrieve data from the NARR archive located at http://rda.ucar.edu   This data is provided in 1 to 4 day increments depending on the year and days in the month.  Most files are 3day files.  When a file is downloaded and processed, all days within the file will be processed and added to (or will update) the local archive.  See http://rda.ucar.edu for more details about the contents of the data.

#### update_narr_aux_data.py

This script is used to update the archive on a daily basis from http://ftp.cpc.ncep.noaa.gov/NARR/archive/rotating_3hour where files are available for the current year in 3 hour increments.  However there is a delay before new files are provided as time is needed to wait for input data and to generate the parameters.  See http://rda.ucar.edu for more details about the contents of the data.

## Installation

### Dependencies
* Python 2.7+
* wgrib [Found here](http://www.cpc.ncep.noaa.gov/products/wesley/wgrib.html)
  - The command line tool is used to extract required parameters from original NARR source files.
* Login credentials for https://rda.ucar.edu
  - The source for archived NARR data.
* Additional Python libraries
  - requests: ```pip install requests``` 

### Configuration Files
* lst_auxiliary.config placed into the executable installation location.  Typically ```$PREFIX/bin```.  See [example](example-lst_auxiliary.config) file.

### Environment Variables
* Required for installing this software
```
export PREFIX="path_to_Installation_Directory"
```

### Build Steps
* Clone the repository and replace the defaulted version(master) with this version of the software
```
git clone https://github.com/USGS-EROS/espa-land-surface-temperature.git
cd espa-land-surface-temperature
git checkout version_<version>
```
* Build and install the application specific software
```
cd lst_auxiliary_data
make install
```
## Usage
See `build_narr_aux_archive_from_CISL_RDA.py --help` for command line details.

See `update_narr_aux_data.py --help` for command line details.

### Environment Variables
* PATH - May need to be updated to include the following
  - `$PREFIX/bin`
* LST_AUX_DIR - Points to the local NARR data archive.
  - `export LST_AUX_DIR="/usr/local/auxiliaries/LST/NARR"`

## More Information
This project is provided by the US Geological Survey (USGS) Earth Resources Observation and Science (EROS) Land Satellite Data Systems (LSDS) Science Research and Development (LSRD) Project. For questions regarding products produced by this source code, please contact the Landsat Contact Us page and specify USGS CDR/ECV in the "Regarding" section. https://landsat.usgs.gov/contactus.php
