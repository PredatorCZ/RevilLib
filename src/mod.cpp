#include "datas/binreader.hpp"
#include "datas/binwritter.hpp"
#include "datas/bitfield.hpp"
#include "datas/flags.hpp"
#include "datas/matrix44.hpp"
#include "datas/vectors_simd.hpp"
#include <map>
#include <vector>

#include "uni/list_vector.hpp"
#include "uni/skeleton.hpp"

/*static std::ofstream ofs("report");
#include <datas/fileinfo.hpp>*/
/*
void LoadMOD(const std::string &fileName) {
  BinReader rd(fileName);
  MODHeaderCommon header;
  rd.Push();
  rd.Read(header);
  rd.Pop();

  if (header.id == DOMID) {
    rd.SwapEndian(true);
    header.SwapEndian();
  } else if (header.id != MODID) {
    throw es::InvalidHeaderError(header.id);
  }
  es::string_view sw(fileName);
  MODMaker mk;
  mk.version = header.version;
  mk.swappedEndian = rd.SwappedEndian();

  auto found = modLoaders.find(mk);

  if (es::IsEnd(modLoaders, found)) {
    throw es::InvalidVersionError(mk.version);
  }

  auto main = found->second(rd);
  auto sMain = static_cast<MOD<MODTraitsXC5> *>(main.get());

  es::Dispose(rd);

  /*AFileInfo info(fileName);

  ofs << info.GetFilename() << ' ';

  for (auto &m : sMain->materials) {
    ofs << m.vshData.Get<MODMaterialXC5::Unk00>() << ' '
        << m.vshData.Get<MODMaterialXC5::MultiTextureType>() << ' '
        << m.vshData.Get<MODMaterialXC5::Unk01>() << ' '
        << m.vshData.Get<MODMaterialXC5::Unk02>() << ' ';
  }

  ofs << std::endl;*/

  // printf("%s", fileName.data());

 /* for (auto &m : sMain->materials) {

    m.pshData.Set<MODMaterialXC5::Unk04>(3); //
    m.envMapPower = 10;

    // printf("%s\n", fileName.data());
  }

  // printf("\n");

  BinWritter wr(fileName);
  SaveMODXC5(*sMain, wr);
}
*/
