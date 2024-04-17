
<h1 align="center">Revil Toolset</h1>

<p align="center">
    Revil Toolset is a collection of modding tools for RE Engine/MT Framework titles.
</p>

<p align="center">
    This toolset runs on Spike foundation.
</p>

<p align="center">
    Head to this
    <b><a href="https://github.com/PredatorCZ/Spike/wiki/Spike">Wiki</a></b>
    for more information on how to effectively use it.
</p>

<p align="center">
<b><a href="https://github.com/PredatorCZ/RevilLib/releases">Latest Release</a></b>
</p><h2>Module list</h2>
<ul>
<li><a href="#Encrypt-or-Decrypt-DDON-SNGW">Encrypt or Decrypt DDON SNGW</a></li>
<li><a href="#ARC-Extract">ARC Extract</a></li>
<li><a href="#LMT-to-GLTF">LMT to GLTF</a></li>
<li><a href="#ARC-Create">ARC Create</a></li>
<li><a href="#MOD-to-GLTF">MOD to GLTF</a></li>
<li><a href="#MTF-TEX-to-DDS">MTF TEX to DDS</a></li>
<li><a href="#OBB-Extract">OBB Extract</a></li>
<li><a href="#RE-TEX-to-DDS">RE TEX to DDS</a></li>
<li><a href="#REAsset-to-GLTF">REAsset to GLTF</a></li>
<li><a href="#SDL-to-XML">SDL to XML</a></li>
<li><a href="#SPAC-Extract">SPAC Extract</a></li>
<li><a href="#UDAS-Extract">UDAS Extract</a></li>
<li><a href="#ValidateVFS">ValidateVFS</a></li>
<li><a href="#XFS-to-XML">XFS to XML</a></li>
<li><a href="#XML-to-SDL">XML to SDL</a></li>
</ul>

## Encrypt or Decrypt DDON SNGW

### Module command: ddon_sngw

Encrypts or decrypts `.sngw` sounds/music files from Dragons Dogma Online.

### Input file patterns: `.sngw$`

### Settings

- **encrypt**

  **CLI Long:** ***--encrypt***\
  **CLI Short:** ***-e***

  **Default value:** false

  Switch between encrypt or decrypt only.

### Input file patterns: `.dwm$`

## ARC Extract

### Module command: extract_arc

Extract MT Framework ARC archives.

### Input file patterns: `.arc$`

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

- **class-whitelist**

  **CLI Long:** ***--class-whitelist***\
  Extract only specified (comma separated) classes. Extract all if empty.

## LMT to GLTF

### Module command: lmt_to_gltf

Converts MT Framework `.lmt` motion list into GLTF format.

> [!NOTE]
> The following file patterns apply to `batch.json` which is described [HERE](https://github.com/PredatorCZ/Spike/wiki/Spike---Batching)

### Main file patterns: `.glb$`, `.gltf$`

### Secondary file patterns: `.lmt$`, `.bin$`

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

### Input file patterns: `.mod$`, `.dom$`

## MTF TEX to DDS

### Module command: mtf_tex_to_dds

Converts MT Framework `.tex` texture into DDS format.

### Input file patterns: `.tex$`

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

### Input file patterns: `.obb$`

## RE TEX to DDS

### Module command: re_tex_to_dds

Converts RE Engine `.tex` texture into DDS format.

### Input file patterns: `.tex$`

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

### Input file patterns: `.mot.43$`, `.mot.65$`, `.mot.78$`, `.mot.458$`, `.motlist.60$`, `.motlist.85$`, `.motlist.99$`, `.motlist.486$`

## SDL to XML

### Module command: sdl_to_xml

Converts MT Framework `.sdl` scheduler into XML format.

### Input file patterns: `.sdl$`

## SPAC Extract

### Module command: spac_conv

Extracts MT Framework `.spc` sound containers.

### Supported formats within SPAC archive

- FWSE, Modified IMA ADPCM, PC
- MSF, Various formats (16bit PCM, AT3, ...), PS3
- RIFF WAVE, Various formats (MS IMA ADPCM mostly), PC

### Input file patterns: `.spc$`

## UDAS Extract

### Module command: udas_extract

Extract udas/das archives from RE4.

### Input file patterns: `.udas.lfs$`

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

## XML to SDL

### Module command: xml_to_sdl

Converts XML format back to MT Framework `.sdl` scheduler.

### Input file patterns: `.xml$`

## License

This toolset is available under GPL v3 license. (See LICENSE.md)\
This toolset uses following libraries:

- RevilLib, Copyright (c) 2017-2024 Lukas Cone
