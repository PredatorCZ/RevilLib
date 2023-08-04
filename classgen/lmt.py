import sys
import os.path as pt
sys.path.append(pt.dirname(pt.abspath(__file__)) + '/../3rd_party/spike/classgen')
from classgen import *

BASE = MainSettings()
BASE.permutators = [
  'LMT22',
  'LMT40',
  'LMT49',
  'LMT50',
  'LMT51',
  'LMT56',
  'LMT57',
  'LMT66',
  'LMT67',
  'LMT92',
  'LMT95',
]
BASE.pointer_x86 = True
BASE.pointer_x64 = True
BASE.class_layout_msvc = True

def dump_classes(CLASSES):
    for cls in CLASSES:
        print('namespace clgen::%s {' % cls.name)
        print(
            cls.gen_enum_cpp())
        print(
            cls.gen_table_cpp(BASE))
        print(
            cls.gen_interface_cpp(BASE))
        print('}')
