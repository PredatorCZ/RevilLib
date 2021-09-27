# RevilLib

[![build](https://github.com/PredatorCZ/RevilLib/actions/workflows/cmake.yml/badge.svg)](https://github.com/PredatorCZ/RevilLib/actions/workflows/cmake.yml)
[![Coverage Status](https://coveralls.io/repos/github/PredatorCZ/RevilLib/badge.svg?branch=master)](https://coveralls.io/github/PredatorCZ/RevilLib?branch=master)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

RevilLib is an independent library for various formats, used by RE Engine and MT Framework.

Library is written for C++17 and compilable under MSVC 2017+, GCC 7+, Clang 5+.

## Supported titles/formats

* RE 7: motlist.60, mot.43 (PC only tested)
* REMake 2: motlist.85, mot.65 (PC only tested)
* REMake 3/Resistance: motlist.99, mot.78 (PC only tested)
* MT Framework: LMT, MOD, TEX, XFS, ARC (Cross platform, Multiversion)

## License

This library is available under GPL v3 license. (See LICENSE.md)

This library uses following libraries:

* PugiXml, Copyright (c) 2006-2020 Arseny Kapoulkine
* PreCore, Copyright (c) 2016-2020 Lukas Cone
