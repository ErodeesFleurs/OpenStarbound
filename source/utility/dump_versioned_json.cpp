import std;
import star.format;
import star.file;
import star.json;

import star.versioning_database;
import star.native_runtime;

using namespace Star;

int main(int argc, char** argv) {
  [[maybe_unused]] Star::NativeRuntime nativeRuntime;
  try {
    if (argc != 3) {
      coutf("Usage, {} <versioned_json_binary> <versioned_json_json>\n", argv[0]);
      return -1;
    }

    auto versionedJson = VersionedJson::readFile(argv[1]);
    File::writeFile(versionedJson.toJson().printJson(2), argv[2]);
    return 0;
  } catch (std::exception const&) {
    ExceptionDiagnostic const e(std::current_exception());
    coutf("Error! Caught exception {}\n", outputException(e, true));
    return 1;
  }
}
