# RevilLib

[![build](https://github.com/PredatorCZ/RevilLib/actions/workflows/cmake.yml/badge.svg)](https://github.com/PredatorCZ/RevilLib/actions/workflows/cmake.yml)
[![Coverage Status](https://coveralls.io/repos/github/PredatorCZ/RevilLib/badge.svg?branch=master)](https://coveralls.io/github/PredatorCZ/RevilLib?branch=master)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

RevilLib is an independent library for various formats, used by RE Engine and MT Framework.

Library is written for C++20/23 and compilable under GCC 12+ and Clang 13+.

## Toolset

Toolset can be found in [Toolset folder](https://github.com/PredatorCZ/RevilLib/tree/master/toolset)

[Toolset releases](https://github.com/PredatorCZ/RevilLib/releases)

## Supported titles/formats

* RE 7: motlist.60, mot.43 (PC only tested)
* REMake 2: motlist.85, mot.65 (PC only tested)
* REMake 3/Resistance: motlist.99, mot.78 (PC only tested)
* MT Framework: LMT, MOD, TEX, XFS, ARC (Cross platform, Multiversion)

## License

This library is available under GPL v3 license. (See LICENSE)

This library uses following libraries:

* Spike, Copyright (c) 2016-2023 Lukas Cone (Apache 2)
* libmspack, Copyright (c) 2003-2016 Stuart Caie (LGPLv2)
* zlib, Copyright (c) 1995-2022 Jean-loup Gailly and Mark Adler (Zlib)
* PoverVR Core, Copyright (c) Imagination Technologies Limited (MIT)
