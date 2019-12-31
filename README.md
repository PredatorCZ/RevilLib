# RevilToolset

Revil Toolset is a collection of modding tools for RE Engine/MT Framework titles.

## SPACConvert

Extracts and converts contents of SPAC archives.\
This app uses multithreading, so you can process multiple files at the same time. Best way is to drag'n'drop files onto app.
For this reason a .config file is placed alongside executable file, since app itself only takes file paths as arguments.
A .config file is in XML format. \
***Please do not create any spaces/tabs/uppercase letters/commas as decimal points within setting field. \
Program must run at least once to generate .config file.***

### Settings (.config file)

- ***Generate_Log:***\
        Will generate text log of console output next to application location.
- ***Convert_to_WAV:***\
        Convert non WAV formats into a 16bit PCM WAV.
- ***Force_WAV_Conversion:***\
        Force conversion of all sounds to a 16bit PCM WAV.

### Supported formats

- FWSE, Modified IMA ADPCM, PC
- MSF, Various formats (16bit PCM, AT3, ...), PS3
- RIFF WAVE, Various formats (MS IMA ADPCM mostly), PC

## [Latest Release](https://github.com/PredatorCZ/RevilToolset/releases)

## License

This toolset is available under GPL v3 license. (See LICENSE.md)\
This toolset uses following libraries:

- RevilLib, Copyright (c) 2017-2019 Lukas Cone
- vgmstream, Various Authors (See COPYING)
