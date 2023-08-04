# RevilToolset

Revil Toolset is a collection of modding tools for RE Engine/MT Framework titles.

This toolset runs on Spike foundation.

Head to this **[Wiki](https://github.com/PredatorCZ/PreCore/wiki/Spike)** for more information on how to effectively use it.

**[Latest Release](https://github.com/PredatorCZ/RevilToolset/releases)**

## Release authenticity

Every release asset will contain corresponding `.sig` file, together with [Sign Key](sign_key.asc) can be used to verify asset via gpg.

Simple usage:

```bash
gpg --import sign_key.asc # Required only once
gpg --verify <asset_name>.sig
```

## Encrypt/Decrypt DDON SNGW

### Module command: ddon_sngw

Encrypts or decrypts `.sngw` sounds/music files from Dragons Dogma Online.

### Settings

- **encrypt**

  **CLI Long:** ***--encrypt***\
  **CLI Short:** ***-e***

  **Default value:** false

  Switch between encrypt or decrypt only.

## ARC Extract

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

  **Valid values:** Auto, Win32, PS3, X360, N3DS, CAFE, NSW, PS4, Android, IOS, Win64

  Set platform for correct archive handling.

## ARC Create

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

  **Valid values:** Auto, Win32, PS3, X360, N3DS, CAFE, NSW, PS4, Android, IOS, Win64

  Set platform for correct archive handling.

- **force-zlib-header**

  **CLI Long:** ***--force-zlib-header***\
  **CLI Short:** ***-z***

  **Default value:** false

  Force ZLIB header for files that won't be compressed. (Some platforms only)

## MOD to GLTF

### Module command: mod_to_gltf

Converts MT Framework `.mod` model into GLTF format.

## MTF TEX to DDS

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

  **Valid values:** Auto, Win32, PS3, X360, N3DS, CAFE, NSW, PS4, Android, IOS, Win64

  Set platform for correct texture handling.

## OBB Extract

### Module command: obb_extract

Extract Android .obb archives for Monster Hunter Stories.

## RE TEX to DDS

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

  **Default value:** false

  Will try to convert some matching formats from DX10 to DX9, for example: RG88 to AL88.

- **largest-mipmap-only**

  **CLI Long:** ***--largest-mipmap-only***\
  **CLI Short:** ***-m***

  **Default value:** true

  Will try to extract only highest mipmap.

## REAsset to GLTF

### Module command: reasset_to_gltf

Converts RE Engine various assets into GLTF format.
Currently only supports animations.

## SPAC Extract

### Module command: spac_conv

Extracts MT Framework `.spc` sound containers.

### Supported formats within SPAC archive

- FWSE, Modified IMA ADPCM, PC
- MSF, Various formats (16bit PCM, AT3, ...), PS3
- RIFF WAVE, Various formats (MS IMA ADPCM mostly), PC

## UDAS Extract

### Module command: udas_extract

Extract udas/das archives from RE4.

## XFS to XML

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

## License

This toolset is available under GPL v3 license. (See LICENSE.md)\
This toolset uses following libraries:

- RevilLib, Copyright (c) 2017-2022 Lukas Cone
