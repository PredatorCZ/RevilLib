/*  Revil Format Library
    Copyright(C) 2021 Lukas Cone

    This program is free software : you can redistribute it and / or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once
#include "ext_base.hpp"

static const MtExtensionsStorage extMHSCommon{
    {"acd", 0x6CC8C051},      //
    {"acr", 0x488DAB8D},      //
    {"acs", 0x44A96149},      //
    {"ard", 0x7501E3C1},      //
    {"arp", 0x4EF8A3D0},      //
    {"bcmr", 0x2662A59D},     //
    {"bdy", 0x4DEF5367},      //
    {"bdy", 0x7063542F},      //
    {"bef", 0x4D87AEBA},      //
    {"bes", 0x759C9F51},      //
    {"bft", 0x23C997E4},      //
    {"blyr", 0x5400AB57},     //
    {"bnf", 0x7C409434},      //
    {"bnm", 0x4601A201},      //
    {"bnt", 0x2A25AF1B},      //
    {"bnt", 0x766F00AC},      //
    {"bnu", 0x77A69893},      //
    {"bplt", 0x14483CFB},     //
    {"brsb", 0x6F0CD860},     //
    {"bstr", 0x5001FCC8},     //
    {"btat", 0x3A298CCF},     //
    {"btatef", 0x60604467},   //
    {"bwpt", 0x1816EB57},     //
    {"ccinfo", 0x18D25FEC},   //
    {"ccl", 0x26E7FF},        //
    {"cfid", 0x7EF03563},     //
    {"clc", 0x21684FE4},      //
    {"cli", 0x2D4CF80A},      //
    {"cnd", 0x3E06712D},      //
    {"cndp", 0x52F032FA},     //
    {"ctc", 0x535D969F},      //
    {"d.l", 0x64BBEAA6},      //
    {"deep", 0x1A2B1A25},     //
    {"des", 0x6C980E22},      //
    {"dge", 0x74A4D35F},      //
    {"dgf", 0x459AFA94},      //
    {"dgp", 0x5699B3FE},      //
    {"dmd", 0x3077D00E},      //
    {"dpd", 0x2FE088C0},      //
    {"dpt", 0x48920641},      //
    {"dptev", 0x20E0A843},    //
    {"dptch", 0x3718B9C7},    //
    {"dptra", 0x5C89DEE},     //
    {"dptrm", 0x677EDC52},    //
    {"dpttr", 0x2AC7B904},    //
    {"dqt", 0x344FD684},      //
    {"dtt", 0xAF34367},       //
    {"e2d", 0x276DE8B7},      //
    {"ean", 0x4E397417},      //
    {"ebc", 0x284FCD6E},      //
    {"eepd", 0x751ACAA0},     //
    {"efl", 0x6D5AE854},      //
    {"efs", 0x2833703},       //
    {"epd", 0x3A8B6D04},      //
    {"epd", 0x5F975A38},      //
    {"equr", 0x2B40AE8F},     //
    {"esc", 0x1431ED71},      //
    {"ext", 0x336D826C},      //
    {"ext", 0x70FDFB5B},      //
    {"fas", 0x34BDFC5B},      //
    {"fas", 0x5D0CAD3},       //
    {"fas", 0x707698C},       //
    {"fbd", 0x310C90B8},      //
    {"fbd", 0x3F1513D0},      //
    {"fcr", 0x3C6F8994},      //
    {"fed", 0x36C92C9A},      //
    {"fed", 0xB452BD2},       //
    {"fesd", 0x3A873D57},     //
    {"fgd", 0x57C5F1D},       //
    {"fgi", 0x3718238F},      //
    {"fisd", 0x61382649},     //
    {"fld", 0x6A6AFD9E},      //
    {"fmd", 0x720EF393},      //
    {"fmpd", 0x31095696},     //
    {"fnbd", 0x122CA603},     //
    {"fnd", 0x2EF9EEAF},      //
    {"fned", 0x2EC99875},     //
    {"fnmd", 0x47165A39},     //
    {"fntl", 0x18C80D7A},     //
    {"fol", 0x1D8CC9B7},      //
    {"fosd", 0x562C6C5D},     //
    {"fpd", 0x134F74D8},      //
    {"fpd", 0x2DECBCA6},      //
    {"fpgl", 0x74B583F5},     //
    {"fppar", 0x2ECFC102},    //
    {"fppgr", 0x78956684},    //
    {"fppnr", 0x2957DDCD},    //
    {"fppwr", 0x325774D5},    //
    {"fsesl", 0x74EA83B0},    //
    {"fslm", 0x7370D1FC},     //
    {"fsm", 0x66B45610},      //
    {"fsosl", 0x228736BF},    //
    {"fssd", 0xA3FE3F4},      //
    {"fssl", 0x4042405A},     //
    {"ftd", 0x1479E09D},      //
    {"gar", 0x2178810},       //
    {"gcd", 0x148FA98A},      //
    {"ged", 0x5E1A909B},      //
    {"gfx", 0x193175EA},      //
    {"ghlt", 0x306945DB},     //
    {"glt", 0x75E48071},      //
    {"gmt", 0xA164982},       //
    {"gstd", 0x6A28E7C5},     //
    {"gtb", 0x3E4BFC33},      //
    {"itm", 0x7C4883A8},      //
    {"jnj", 0x5332520},       //
    {"lacd", 0x162E07FB},     //
    {"lad", 0x236C1AAB},      //
    {"lanl", 0x708E0028},     //
    {"lcm", 0x39C52040},      //
    {"lfd", 0x3516C3D2},      //
    {"lfx", 0x682B1925},      //
    {"lmd", 0x62440501},      //
    {"lmt", 0x76820D81},      //
    {"lsnl", 0x5A7A72DE},     //
    {"lyt", 0x15302EF4},      //
    {"mca", 0x79C47B59},      //
    {"mcd", 0x6CACB310},      //
    {"mdl", 0x7A23F10F},      //
    {"mea", 0x7534679E},      //
    {"mectd", 0x2325C7A0},    //
    {"mix", 0x3FB52996},      //
    {"mkr", 0x2009C23},       //
    {"mmk", 0x59CD219},       //
    {"mod", 0x58A15856},      //
    {"mpd", 0x3F53ECC9},      //
    {"mqsd", 0x7DD2AFBE},     //
    {"mrd", 0x6E6A25E0},      //
    {"mrl", 0x2749C8A8},      //
    {"mrt", 0xB842B37},       //
    {"msgm", 0x48DFD78B},     //
    {"nasl", 0x1FA3FF7A},     //
    {"nasl", 0x4C2446BF},     //
    {"nhap", 0x31798063},     //
    {"nhapp", 0x3A244568},    //
    {"npd", 0x4F1544F5},      //
    {"npsl", 0x3876D6FA},     //
    {"nstera", 0x6BEEFB2A},   //
    {"nsterb", 0x72E7AA90},   //
    {"nstip", 0x7190B007},    //
    {"nstmsg", 0x7AF36DC1},   //
    {"pec", 0x6FCC7AD4},      //
    {"pel", 0x5A525C16},      //
    {"pep", 0x20ED9750},      //
    {"plt", 0x34886F87},      //
    {"poom", 0x1E01F870},     //
    {"por", 0x5A2E573A},      //
    {"psl", 0x254309C9},      //
    {"ptex", 0x3756EE15},     //
    {"pts", 0x5842F0B0},      //
    {"pvi", 0x54DD639F},      //
    {"rcd", 0x3C4BD0ED},      //
    {"rdp", 0x29A5C1D1},      //
    {"revr_ctr", 0x232E228C}, //
    {"rnd", 0x5CF33CD6},      //
    {"sad", 0x990B835},       //
    {"samd", 0x7DAAFFDC},     //
    {"sat", 0x550D05AA},      //
    {"sbc", 0x51FC779F},      //
    {"sbkr", 0x15D782FB},     //
    {"sbmd", 0x597812F5},     //
    {"sbsd", 0x3759B207},     //
    {"sbsdef", 0x2D0B48AC},   //
    {"scd", 0x44EE724E},      //
    {"sce", 0x3654EBA0},      //
    {"scs", 0xECD7DF4},       //
    {"sddc", 0x48B239C0},     //
    {"sddsc", 0x7C559D35},    //
    {"sdl", 0x4C0DB839},      //
    {"sdm", 0x5450C517},      //
    {"sdsr", 0x315E81F},      //
    {"sed", 0x28BAE197},      //
    {"sfcbd", 0x5249C19},     //
    {"sfcsd", 0x4A32252D},    //
    {"sfd", 0x60EEAE69},      //
    {"sisd", 0x136EBC7F},     //
    {"skf", 0x45CFC491},      //
    {"smd", 0x24F56BC0},      //
    {"smkd", 0x6622AB2D},     //
    {"snd", 0x1F8BAD22},      //
    {"sod", 0x23AC90FB},      //
    {"sosd", 0x5D4F7DBF},     //
    {"sptl", 0x1657A2D0},     //
    {"sqd", 0x4ED7C110},      //
    {"srcd", 0x2074FB1F},     //
    {"srqr", 0x1BCC4966},     //
    {"ssd", 0x125F7431},      //
    {"ssdc", 0x3C56A1F1},     //
    {"ssdsc", 0x44DC7515},    //
    {"ssed", 0x7DD2F109},     //
    {"stb", 0x2555081D},      //
    {"std", 0x6E5FB1CC},      //
    {"stdc", 0x40596F49},     //
    {"stdrd", 0x55C30579},    //
    {"stdsc", 0x141C421D},    //
    {"stdsrd", 0x6409628D},   //
    {"stqr", 0x167DBBFF},     //
    {"swd", 0x46AADC49},      //
    {"swp", 0x86AEE8E},       //
    {"tdd", 0x370AD68B},      //
    {"tdmact", 0x11B6FD48},   //
    {"tdmd", 0x663D73B2},     //
    {"tdmeff", 0x48AE1387},   //
    {"tdmfc", 0x2A06CC14},    //
    {"tdmmot", 0x5376F652},   //
    {"tdmpos", 0x433B8D7E},   //
    {"tdms", 0x28C32975},     //
    {"tdvs", 0x69D4A3E},      //
    {"tex", 0x241F5DEB},      //
    {"tstd", 0x1337721},      //
    {"vjr", 0x215E5305},      //
    {"wet", 0x6E58760},       //
    {"wgi", 0x49F01BD},       //
    {"wofb", 0x6ABA51B0},     //
    {"wpd", 0x7C067B17},      //
    {"wpp", 0x2141D3A9},      //
};

static const MtExtensions extMHS{&extMHSCommon, Platform::N3DS};
