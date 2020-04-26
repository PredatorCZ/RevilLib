
#include "lmt_codecs.inl"
#include "datas/tchar.hpp"

int main() {
  printer.AddPrinterFunction(UPrintf);

  TEST_CASES(int testResult, TEST_FUNC(test_lmt_codec00),
             TEST_FUNC(test_lmt_codec01), TEST_FUNC(test_lmt_codec02),
             TEST_FUNC(test_lmt_codec03), TEST_FUNC(test_lmt_codec04),
             TEST_FUNC(test_lmt_codec05), TEST_FUNC(test_lmt_codec06),
             TEST_FUNC(test_lmt_codec07), TEST_FUNC(test_lmt_codec08),
             TEST_FUNC(test_lmt_codec09), TEST_FUNC(test_lmt_codec10),
             TEST_FUNC(test_lmt_codec11), TEST_FUNC(test_lmt_codec12));

  return testResult;
}