#include <benchmark/benchmark.h>

import std;
import star.bi_map;
import star.hash;
import star.json;
import star.list;
import star.map;
import star.native_runtime;
import star.perlin;
import star.string;
import star.string_view;

namespace {

using Star::EnumMap;
using Star::Json;
using Star::JsonArray;
using Star::JsonObject;
using Star::PerlinType;
using Star::String;

// Fixture construction and semantic validation precede the State iteration
// range, so Google Benchmark excludes them from timing. Only the explicitly
// named ConstructDestroy case times table construction and destruction.
constexpr auto FixedPerlinNames = Star::sortedEnumNames(std::array{
    std::pair{PerlinType::Uninitialized, std::string_view("uninitialized")},
    std::pair{PerlinType::Perlin, std::string_view("perlin")},
    std::pair{PerlinType::Billow, std::string_view("billow")},
    std::pair{PerlinType::RidgedMulti, std::string_view("ridgedMulti")},
});

auto makePerlinMap() {
  std::array<std::pair<PerlinType, String>, FixedPerlinNames.size()> entries;
  for (std::size_t i = 0; i < entries.size(); ++i)
    entries[i] = {FixedPerlinNames[i].first, String(FixedPerlinNames[i].second)};
  return EnumMap<PerlinType>::from(entries);
}

// A borrowed optional array access candidate, not a replacement enumName API:
// enumName is checked and constructs a diagnostic on a missing enum value.
std::optional<std::string_view> arrayPerlinName(PerlinType value) {
  auto found = std::lower_bound(FixedPerlinNames.begin(), FixedPerlinNames.end(), value,
      [](auto const& pair, auto key) { return pair.first < key; });
  if (found == FixedPerlinNames.end() || found->first != value)
    return std::nullopt;
  return found->second;
}

struct PerlinQueries {
  std::array<PerlinType, 4> hits{
      PerlinType::Uninitialized, PerlinType::Perlin, PerlinType::Billow, PerlinType::RidgedMulti};
  std::array<PerlinType, 4> misses{
      static_cast<PerlinType>(4), static_cast<PerlinType>(5),
      static_cast<PerlinType>(6), static_cast<PerlinType>(7)};
  std::array<String, 12> reverseHits{
      "uninitialized", "perlin", "billow", "ridgedMulti",
      "UNINITIALIZED", "PERLIN", "BILLOW", "RIDGEDMULTI",
      "uNiNiTiAlIzEd", "pErLiN", "bIlLoW", "rIdGeDmUlTi"};
  std::array<String, 4> reverseMisses{"", "perlin-extra", "ridgedMult", "日本語"};
};

bool validatePerlin(benchmark::State& state, EnumMap<PerlinType> const& dynamic, PerlinQueries const& queries) {
  if (dynamic.size() != FixedPerlinNames.size()) {
    state.SkipWithError("Perlin table size differs from the four-entry candidate");
    return false;
  }
  for (auto const& [value, spelling] : FixedPerlinNames) {
    auto dynamicOptional = dynamic.maybeRight(value);
    auto dynamicChecked = dynamic.getRight(value);
    auto fixedChecked = Star::enumName<FixedPerlinNames>(value);
    auto fixedOptional = arrayPerlinName(value);
    if (!dynamicOptional || !dynamicChecked || !fixedChecked || !fixedOptional
        || dynamicOptional->utf8() != spelling || dynamicChecked->get().utf8() != spelling
        || *fixedChecked != spelling || *fixedOptional != spelling) {
      state.SkipWithError("Perlin forward candidates disagree");
      return false;
    }
  }
  for (auto value : queries.misses) {
    if (dynamic.maybeRight(value) || arrayPerlinName(value)) {
      state.SkipWithError("Perlin optional forward miss unexpectedly matched");
      return false;
    }
  }
  for (std::size_t i = 0; i < queries.reverseHits.size(); ++i) {
    auto dynamicValue = dynamic.maybeLeft(queries.reverseHits[i]);
    auto fixedValue = Star::maybeEnumValue<FixedPerlinNames>(Star::StringView(queries.reverseHits[i]));
    auto expected = queries.hits[i % queries.hits.size()];
    if (!dynamicValue || !fixedValue || !*dynamicValue || !*fixedValue
        || **dynamicValue != expected || **fixedValue != expected) {
      state.SkipWithError("Perlin reverse candidates disagree on case-insensitive hits");
      return false;
    }
  }
  for (auto const& text : queries.reverseMisses) {
    auto dynamicValue = dynamic.maybeLeft(text);
    auto fixedValue = Star::maybeEnumValue<FixedPerlinNames>(Star::StringView(text));
    if (!dynamicValue || !fixedValue || *dynamicValue || *fixedValue) {
      state.SkipWithError("Perlin reverse optional miss returned an error or a match");
      return false;
    }
  }
  return true;
}

template <bool Fixed, bool Hit>
void BM_PerlinForwardOptional(benchmark::State& state) {
  auto fixture = makePerlinMap();
  if (!fixture) {
    state.SkipWithError("Perlin EnumMap fixture construction failed");
    return;
  }
  auto const& dynamic = *fixture;
  PerlinQueries queries;
  if (!validatePerlin(state, dynamic, queries))
    return;
  auto const& values = Hit ? queries.hits : queries.misses;
  state.SetLabel(Fixed ? "borrowed string_view optional; prebuilt array"
                       : "owning String optional; existing EnumMap");
  for (auto _ : state) {
    (void)_;
    for (auto value : values) {
      benchmark::DoNotOptimize(value);
      if constexpr (Fixed) {
        auto result = arrayPerlinName(value);
        benchmark::DoNotOptimize(result);
      } else {
        auto result = dynamic.maybeRight(value);
        benchmark::DoNotOptimize(result);
      }
    }
  }
  state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(values.size()));
}

template <bool Fixed>
void BM_PerlinForwardCheckedHit(benchmark::State& state) {
  auto fixture = makePerlinMap();
  if (!fixture) {
    state.SkipWithError("Perlin EnumMap fixture construction failed");
    return;
  }
  auto const& dynamic = *fixture;
  PerlinQueries queries;
  if (!validatePerlin(state, dynamic, queries))
    return;
  state.SetLabel(Fixed ? "checked borrowed string_view; enumName helper"
                       : "checked borrowed String reference; EnumMap::getRight");
  for (auto _ : state) {
    (void)_;
    for (auto value : queries.hits) {
      benchmark::DoNotOptimize(value);
      if constexpr (Fixed) {
        auto result = Star::enumName<FixedPerlinNames>(value);
        if (!result) {
          state.SkipWithError("enumName failed for a validated hit");
          return;
        }
        benchmark::DoNotOptimize(*result);
      } else {
        auto result = dynamic.getRight(value);
        if (!result) {
          state.SkipWithError("EnumMap::getRight failed for a validated hit");
          return;
        }
        benchmark::DoNotOptimize(result->get().utf8());
      }
    }
  }
  state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(queries.hits.size()));
}

template <bool Fixed, bool Hit>
void BM_PerlinReverseOptional(benchmark::State& state) {
  auto fixture = makePerlinMap();
  if (!fixture) {
    state.SkipWithError("Perlin EnumMap fixture construction failed");
    return;
  }
  auto const& dynamic = *fixture;
  PerlinQueries queries;
  if (!validatePerlin(state, dynamic, queries))
    return;
  auto const& texts = [&]() -> auto const& {
    if constexpr (Hit)
      return queries.reverseHits;
    else
      return queries.reverseMisses;
  }();
  state.SetLabel(Hit ? "prebuilt String queries; canonical/upper/mixed ASCII case"
                     : "prebuilt String queries; successful empty optionals, no diagnostics");
  for (auto _ : state) {
    (void)_;
    for (auto const& text : texts) {
      benchmark::DoNotOptimize(text);
      auto result = [&] {
        if constexpr (Fixed)
          return Star::maybeEnumValue<FixedPerlinNames>(Star::StringView(text));
        else
          return dynamic.maybeLeft(text);
      }();
      if (!result) {
        state.SkipWithError("Perlin reverse lookup failed for validated UTF-8");
        return;
      }
      benchmark::DoNotOptimize(*result);
    }
  }
  state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(texts.size()));
}

void BM_PerlinDynamicConstructDestroy(benchmark::State& state) {
  std::array<std::pair<PerlinType, String>, 4> entries;
  for (std::size_t i = 0; i < entries.size(); ++i)
    entries[i] = {FixedPerlinNames[i].first, String(std::string(FixedPerlinNames[i].second))};
  auto validation = EnumMap<PerlinType>::from(entries);
  PerlinQueries queries;
  if (!validation || !validatePerlin(state, *validation, queries)) {
    state.SkipWithError("EnumMap construction validation failed");
    return;
  }
  state.SetLabel("EnumMap::from prebuilt owning Strings + destruction; NOT process startup");
  for (auto _ : state) {
    (void)_;
    auto result = EnumMap<PerlinType>::from(entries);
    if (!result) {
      state.SkipWithError("EnumMap::from failed for validated unique entries");
      return;
    }
    benchmark::DoNotOptimize(*result);
  }
  state.SetItemsProcessed(state.iterations());
}

Json numericArray(std::size_t count) {
  JsonArray values;
  values.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    auto value = static_cast<std::int64_t>((i * 7919) % 2000001) - 1000000;
    if (i % 2)
      values.append(Json(static_cast<double>(value) / 8.0));
    else
      values.append(Json(value));
  }
  return Json(std::move(values));
}

Json unicodeDocument(std::size_t count) {
  JsonArray rows;
  rows.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    rows.append(Json(JsonObject{
        {"index", Json(i)},
        {"日本語", Json("星の世界 — café é 😀")},
        {"escapes", Json("quote: \"; slash: \\; newline:\n tab:\t control:\x01")},
        {"active", Json(i % 2 == 0)},
        {"nested", Json(JsonObject{{"nothing", Json()}, {"numbers", numericArray(8)}})},
    }));
  }
  return Json(JsonObject{{"title", Json("Unicode mixed document / Ελληνικά")}, {"rows", Json(std::move(rows))}});
}

template <bool Unicode>
void BM_JsonSerialize(benchmark::State& state) {
  auto count = static_cast<std::size_t>(state.range(0));
  auto document = Unicode ? unicodeDocument(count) : numericArray(count);
  auto reference = document.repr(0, false);
  if (!reference) {
    state.SkipWithError("JSON fixture serialization failed");
    return;
  }
  auto parsed = Json::parseJson(*reference);
  if (!parsed || *parsed != document) {
    state.SkipWithError("JSON serialization did not round-trip the deterministic fixture");
    return;
  }
  auto repeated = document.repr(0, false);
  if (!repeated || *repeated != *reference) {
    state.SkipWithError("JSON serialization output is inconsistent");
    return;
  }
  auto bytes = static_cast<std::int64_t>(reference->utf8().size());
  state.SetLabel(Unicode ? "repr(0,false); nested Unicode/escaped strings/numbers/bools/null; rows"
                         : "repr(0,false); alternating signed integers and exact eighths; elements");
  for (auto _ : state) {
    (void)_;
    auto result = document.repr(0, false);
    if (!result) {
      state.SkipWithError("JSON serialization failed for a validated fixture");
      return;
    }
    benchmark::DoNotOptimize(result->utf8().data());
    benchmark::DoNotOptimize(result->utf8().size());
  }
  state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(count));
  state.SetBytesProcessed(state.iterations() * bytes);
}

using FlatLookupMap = Star::HashMap<String, std::uint64_t, Star::hash<String>, std::equal_to<>>;
using StandardLookupMap = std::unordered_map<String, std::uint64_t, Star::hash<String>, std::equal_to<>>;

struct MapQueries {
  std::vector<String> hits;
  std::vector<String> misses;

  explicit MapQueries(std::size_t count) {
    hits.reserve(count);
    misses.reserve(count);
    // Counts are powers of two. An odd multiplier permutes the complete set,
    // independently of map layout, without timing RNG or key construction.
    for (std::size_t i = 0; i < count; ++i) {
      auto key = (i * 4051) & (count - 1);
      hits.emplace_back(std::format("asset/{:08x}/deterministic-key", key));
      misses.emplace_back(std::format("absent/{:08x}/deterministic-key", key));
    }
  }
};

template <typename Map, bool Hit>
void BM_MapLookup(benchmark::State& state) {
  auto count = static_cast<std::size_t>(state.range(0));
  MapQueries queries(count);
  Map map;
  map.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    auto inserted = map.emplace(queries.hits[i], static_cast<std::uint64_t>(i + 1));
    if (!inserted.second) {
      state.SkipWithError("Deterministic map keys are not unique");
      return;
    }
  }
  if (map.size() != count) {
    state.SkipWithError("Map fixture cardinality differs from the dataset");
    return;
  }
  for (std::size_t i = 0; i < count; ++i) {
    auto found = map.find(queries.hits[i]);
    if (found == map.end() || found->second != i + 1 || map.find(queries.misses[i]) != map.end()) {
      state.SkipWithError("Map hit/miss semantics differ from the deterministic dataset");
      return;
    }
  }
  auto const& lookup = std::as_const(map);
  auto const& keys = Hit ? queries.hits : queries.misses;
  state.SetLabel("Star::hash<String>/std::equal_to<>; reserve(count); prebuilt permuted String queries");
  for (auto _ : state) {
    (void)_;
    std::uint64_t checksum = 0;
    for (auto const& key : keys) {
      benchmark::DoNotOptimize(key);
      auto found = lookup.find(key);
      if (found != lookup.end())
        checksum += found->second;
    }
    benchmark::DoNotOptimize(checksum);
  }
  state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(count));
}

BENCHMARK_TEMPLATE(BM_JsonSerialize, false)->Name("Core/Json/NumericArraySerialize")
    ->Arg(16)->Arg(256)->Arg(4096)->Arg(65536);
BENCHMARK_TEMPLATE(BM_JsonSerialize, true)->Name("Core/Json/UnicodeMixedSerialize")
    ->Arg(1)->Arg(16)->Arg(256);

BENCHMARK_TEMPLATE(BM_PerlinForwardOptional, false, true)->Name("Core/Perlin/DynamicMaybeRight/Hit");
BENCHMARK_TEMPLATE(BM_PerlinForwardOptional, false, false)->Name("Core/Perlin/DynamicMaybeRight/Miss");
BENCHMARK_TEMPLATE(BM_PerlinForwardOptional, true, true)->Name("Core/Perlin/ArrayOptionalName/Hit");
BENCHMARK_TEMPLATE(BM_PerlinForwardOptional, true, false)->Name("Core/Perlin/ArrayOptionalName/Miss");
BENCHMARK_TEMPLATE(BM_PerlinForwardCheckedHit, false)->Name("Core/Perlin/DynamicGetRight/Hit");
BENCHMARK_TEMPLATE(BM_PerlinForwardCheckedHit, true)->Name("Core/Perlin/EnumNameHelper/Hit");
BENCHMARK_TEMPLATE(BM_PerlinReverseOptional, false, true)->Name("Core/Perlin/DynamicMaybeLeft/Hit");
BENCHMARK_TEMPLATE(BM_PerlinReverseOptional, false, false)->Name("Core/Perlin/DynamicMaybeLeft/Miss");
BENCHMARK_TEMPLATE(BM_PerlinReverseOptional, true, true)->Name("Core/Perlin/MaybeEnumValueHelper/Hit");
BENCHMARK_TEMPLATE(BM_PerlinReverseOptional, true, false)->Name("Core/Perlin/MaybeEnumValueHelper/Miss");
BENCHMARK(BM_PerlinDynamicConstructDestroy)->Name("Core/Perlin/DynamicFromPrebuiltStringsConstructDestroy");

BENCHMARK_TEMPLATE(BM_MapLookup, FlatLookupMap, true)->Name("Core/Map/HashMap/Hit")
    ->Arg(4)->Arg(32)->Arg(4096)->Arg(65536);
BENCHMARK_TEMPLATE(BM_MapLookup, FlatLookupMap, false)->Name("Core/Map/HashMap/Miss")
    ->Arg(4)->Arg(32)->Arg(4096)->Arg(65536);
BENCHMARK_TEMPLATE(BM_MapLookup, StandardLookupMap, true)->Name("Core/Map/UnorderedMap/Hit")
    ->Arg(4)->Arg(32)->Arg(4096)->Arg(65536);
BENCHMARK_TEMPLATE(BM_MapLookup, StandardLookupMap, false)->Name("Core/Map/UnorderedMap/Miss")
    ->Arg(4)->Arg(32)->Arg(4096)->Arg(65536);

}

class CheckedReporter : public benchmark::ConsoleReporter {
public:
  void ReportRuns(std::vector<Run> const& reports) override {
    for (auto const& run : reports)
      failed |= run.skipped == benchmark::internal::SkippedWithError;
    benchmark::ConsoleReporter::ReportRuns(reports);
  }
  bool failed = false;
};

int main(int argc, char** argv) {
  auto nativeRuntime = Star::NativeRuntime::create();
  if (!nativeRuntime) {
    std::cerr << nativeRuntime.error().what() << '\n';
    return 1;
  }
  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
    benchmark::Shutdown();
    return 1;
  }
  CheckedReporter reporter;
  auto matched = benchmark::RunSpecifiedBenchmarks(&reporter);
  benchmark::Shutdown();
  return reporter.failed || matched == 0 ? 1 : 0;
}
