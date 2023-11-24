/*  SDLConvert
    Copyright(C) 2023 Lukas Cone

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

#include "project.h"
#include "re_common.hpp"
#include "revil/sdl.hpp"
#include "spike/io/binreader_stream.hpp"
#include "spike/util/pugiex.hpp"

#include "spike/io/binwritter_stream.hpp"
#include <cassert>
#include <sstream>

std::string_view filters[]{
    ".sdl$",
};

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = SDLConvert_DESC " v" SDLConvert_VERSION ", " SDLConvert_COPYRIGHT
                              "Lukas Cone",
    .filters = filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

// Function to calculate maximum
// overlap in two given strings
int findOverlappingPair(std::string str1, std::string str2, std::string &str) {

  // Max will store maximum
  // overlap i.e maximum
  // length of the matching
  // prefix and suffix
  int max = INT_MIN;
  int len1 = str1.length();
  int len2 = str2.length();

  // Check suffix of str1 matches
  // with prefix of str2
  for (int i = 1; i <= std::min(len1, len2); i++) {

    // Compare last i characters
    // in str1 with first i
    // characters in str2
    if (str1.compare(len1 - i, i, str2, 0, i) == 0) {
      if (max < i) {
        // Update max and str
        max = i;
        str = str1 + str2.substr(i);
      }
    }
  }

  // Check prefix of str1 matches
  // with suffix of str2
  for (int i = 1; i <= std::min(len1, len2); i++) {

    // compare first i characters
    // in str1 with last i
    // characters in str2
    if (str1.compare(0, i, str2, len2 - i, i) == 0) {
      if (max < i) {

        // Update max and str
        max = i;
        str = str2 + str1.substr(i);
      }
    }
  }

  return max;
}

// Function to calculate
// smallest string that contains
// each string in the given
// set as substring.
std::string findShortestSuperstring(std::string arr[], int len) {

  // Run len-1 times to
  // consider every pair
  while (len != 1) {

    // To store  maximum overlap
    int max = INT_MIN;

    // To store array index of strings
    int l, r;

    // Involved in maximum overlap
    std::string resStr;

    // Maximum overlap
    for (int i = 0; i < len; i++) {
      for (int j = i + 1; j < len; j++) {
        std::string str;

        // res will store maximum
        // length of the matching
        // prefix and suffix str is
        // passed by reference and
        // will store the resultant
        // string after maximum
        // overlap of arr[i] and arr[j],
        // if any.
        int res = findOverlappingPair(arr[i], arr[j], str);

        // check for maximum overlap
        if (max < res) {
          max = res;
          resStr.assign(str);
          l = i, r = j;
        }
      }
    }

    // Ignore last element in next cycle
    len--;

    // If no overlap, append arr[len] to arr[0]
    if (max == INT_MIN)
      arr[0] += arr[len];
    else {

      // Copy resultant string to index l
      arr[l] = resStr;

      // Copy string at last index to index r
      arr[r] = arr[len];
    }
  }
  return arr[0];
}

#include "revil/hashreg.hpp"
#include "spike/master_printer.hpp"

void AppProcessFile(AppContext *ctx) {
  std::vector<std::string> strs{"settings", "array",   "node_data",
                                "raycast",  "dataset", "castnode"};

  std::string str = findShortestSuperstring(strs.data(), strs.size());

  SDL sdl;

  try {
    sdl.Load(ctx->GetStream());
  } catch (const es::InvalidHeaderError &r) {
    return;
  }

  auto &outStr = ctx->NewFile(ctx->workingFile.ChangeExtension(".xml")).str;
  std::stringstream sstr;

  pugi::xml_document doc;
  sdl.ToXML(doc);
  doc.save(outStr);
  /*doc.save(sstr);

  std::string buff0 = std::move(sstr).str();

  auto &str = ctx->NewFile(ctx->workingFile.ChangeExtension(".xml.sdl")).str;
  revil::SDLFromXML(str, doc);
  sstr = {};
  revil::SDLFromXML(sstr, doc);

  SDL sdl2;
  sdl2.Load(sstr);
  auto &str2 =
      ctx->NewFile(ctx->workingFile.ChangeExtension(".xml.sdl.xml")).str;
  pugi::xml_document doc2;
  sdl2.ToXML(doc2);
  doc2.save(str2);
  sstr = {};
  doc2.save(sstr);

  std::string buff1 = std::move(sstr).str();

  assert(buff0 == buff1);*/
}
