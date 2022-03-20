# RevilToolset

Revil Toolset is a collection of modding tools for RE Engine/MT Framework titles.

This toolset runs on Spike foundation.

Head to this **[Wiki](https://github.com/PredatorCZ/PreCore/wiki/Spike)** for more information on how to effectively use it.

## DDONSngw

### Module command: ddon_sngw

Encrypts or decrypts `.sngw` sounds/music files from Dragons Dogma Online.

### Settings

- **encrypt**

  **CLI Long:** ***--encrypt***\
  **CLI Short:** ***-e***

  **Default value:** false

  Switch between encrypt or decrypt only.

## ARCExtract

### Module command: extract_arc

Extract MT Framework ARC archives.

### Settings

- **title**

  **CLI Long:** ***--title***\
  **CLI Short:** ***-t***

  Set title for correct archive handling.

- **platform**

  **CLI Long:** ***--platform***\
  **CLI Short:** ***-p***

  **Default value:** Auto

  Set platform for correct archive handling.

## ARCMake

### Module command: make_arc

Create MT Framework ARC archives.

### Settings

- **title**

  **CLI Long:** ***--title***\
  **CLI Short:** ***-t***

  Set title for correct archive handling.

- **platform**

  **CLI Long:** ***--platform***\
  **CLI Short:** ***-p***

  **Default value:** Auto

  Set platform for correct archive handling.

- **force-zlib-header**

  **CLI Long:** ***--force-zlib-header***\
  **CLI Short:** ***-z***

  **Default value:** false

  Force ZLIB header for files that won't be compressed. (Some platforms only)

## MOD2GLTF

### Module command: mod_to_gltf

Converts MT Framework `.mod` model into GLTF format.

## TEXConvert

### Module command: mtf_tex_to_dds

Converts MT Framework `.tex` texture into DDS format.

### Settings

- **legacy-dds**

  **CLI Long:** ***--legacy-dds***\
  **CLI Short:** ***-l***

  **Default value:** false

  Tries to convert texture into legacy (DX9) DDS format.

- **force-legacy-dds**

  **CLI Long:** ***--force-legacy-dds***\
  **CLI Short:** ***-f***

  **Default value:** false

  Will try to convert some matching formats from DX10 to DX9, for example: RG88 to AL88.

- **largest-mipmap-only**

  **CLI Long:** ***--largest-mipmap-only***\
  **CLI Short:** ***-m***

  **Default value:** false

  Will try to extract only highest mipmap.

- **platform**

  **CLI Long:** ***--platform***\
  **CLI Short:** ***-p***

  **Default value:** Auto

  Set platform for correct texture handling.

## TEXConvert

### Module command: re_tex_to_dds

Converts RE Engine `.tex` texture into DDS format.

### Settings

- **legacy-dds**

  **CLI Long:** ***--legacy-dds***\
  **CLI Short:** ***-l***

  **Default value:** true

  Tries to convert texture into legacy (DX9) DDS format.

- **force-legacy-dds**

  **CLI Long:** ***--force-legacy-dds***\
  **CLI Short:** ***-f***

  **Default value:** true

  Will try to convert some matching formats from DX10 to DX9, for example: RG88 to AL88.

- **largest-mipmap-only**

  **CLI Long:** ***--largest-mipmap-only***\
  **CLI Short:** ***-m***

  **Default value:** true

  Will try to extract only highest mipmap.

## REAsset2GLTF

### Module command: reasset_to_gltf

Converts RE Engine various assets into GLTF format.
Currently only supports animations.

## SPACConvert

### Module command: spac_conv

Extracts MT Framework `.spc` sound containers and converts their files into WAV format.

### Supported formats within SPAC archive

- FWSE, Modified IMA ADPCM, PC
- MSF, Various formats (16bit PCM, AT3, ...), PS3
- RIFF WAVE, Various formats (MS IMA ADPCM mostly), PC

### Settings

- **convert-wav**

  **CLI Long:** ***--convert-wav***\
  **CLI Short:** ***-w***

  **Default value:** true

  Convert sounds to WAV format.

- **force-wav**

  **CLI Long:** ***--force-wav***\
  **CLI Short:** ***-W***

  **Default value:** false

  Convert ADPCM WAV files into PCM WAV if SPAC contains then.

## XFS2XML

### Module command: xfs_to_xml

Converts MT Framework generic binary data table format into XML.

### Settings

- **save-rtti**

  **CLI Long:** ***--save-rtti***\
  **CLI Short:** ***-r***

  **Default value:** true

  Save layout information.

- **save-data**

  **CLI Long:** ***--save-data***\
  **CLI Short:** ***-d***

  **Default value:** true

  Save data.

## [Latest Release](https://github.com/PredatorCZ/RevilToolset/releases)

## License

This toolset is available under GPL v3 license. (See LICENSE.md)\
This toolset uses following libraries:

- RevilLib, Copyright (c) 2017-2022 Lukas Cone
- vgmstream, Various Authors (See COPYING)
