#pragma once

#include "lua.hpp"// IWYU pragma: export

#include "StarDirectives.hpp"
#include "StarException.hpp"
#include "StarJson.hpp"
#include "StarLexicalCast.hpp"
#include "StarRefPtr.hpp"
#include "StarString.hpp"

import std;

namespace Star {

class LuaEngine;
using LuaEnginePtr = RefPtr<LuaEngine>;

// Basic unspecified lua exception
using LuaException = ExceptionDerived<"LuaException">;

// Thrown when trying to parse an incomplete statement, useful for implementing
// REPL loops, uses the incomplete statement marker '<eof>' as the standard lua
// repl does.
using LuaIncompleteStatementException = ExceptionDerived<"LuaIncompleteStatementException", LuaException>;

// Thrown when the instruction limit is reached, if the instruction limit is
// set.
using LuaInstructionLimitReached = ExceptionDerived<"LuaInstructionLimitReached", LuaException>;

// Thrown when the engine recursion limit is reached, if the recursion limit is
// set.
using LuaRecursionLimitReached = ExceptionDerived<"LuaRecursionLimitReached", LuaException>;

// Thrown when an incorrect lua type is passed to something in C++ expecting a
// different type.
using LuaConversionException = ExceptionDerived<"LuaConversionException", LuaException>;

// Error information for Lua conversions (C++23 std::expected support)
struct LuaConversionError {
  String message;
  String expectedType;
  String actualType;
  
  constexpr LuaConversionError() = default;
  constexpr LuaConversionError(String msg) : message(std::move(msg)) {}
  constexpr LuaConversionError(String msg, String expected, String actual)
    : message(std::move(msg)), expectedType(std::move(expected)), actualType(std::move(actual)) {}
};

using LuaNilType = Empty;
using LuaBoolean = bool;
using LuaInt = lua_Integer;
using LuaFloat = lua_Number;
class LuaString;
class LuaTable;
class LuaFunction;
class LuaThread;
class LuaUserData;
using LuaValue = Variant<LuaNilType, LuaBoolean, LuaInt, LuaFloat, LuaString, LuaTable, LuaFunction, LuaThread, LuaUserData>;

// Used to wrap multiple return values from calling a lua function or to pass
// multiple values as arguments to a lua function from a container.  If this is
// used as an argument to a lua callback function, it must be the final
// argument of the function!
template <typename T>
class LuaVariadic : public List<T> {
public:
  using List<T>::List;
};

// Unpack a container and apply each of the arguments separately to a lua
// function, similar to lua's unpack.
template <typename Container>
auto luaUnpack(Container&& c) -> LuaVariadic<typename std::decay<Container>::type::value_type>;

// Similar to LuaVariadic, but a tuple type so automatic per-entry type
// conversion is done.  This can only be used as the return value of a wrapped
// c++ function, or as a type for the return value of calling a lua function.
template <typename... Types>
class LuaTupleReturn : public std::tuple<Types...> {
public:
  using Base = std::tuple<Types...>;

  explicit LuaTupleReturn(Types const&... args);
  template <typename... UTypes>
  explicit LuaTupleReturn(UTypes&&... args);
  template <typename... UTypes>
  explicit LuaTupleReturn(UTypes const&... args);
  LuaTupleReturn(LuaTupleReturn const& rhs);
  LuaTupleReturn(LuaTupleReturn&& rhs);
  template <typename... UTypes>
  LuaTupleReturn(LuaTupleReturn<UTypes...> const& rhs);
  template <typename... UTypes>
  LuaTupleReturn(LuaTupleReturn<UTypes...>&& rhs);

  auto operator=(LuaTupleReturn const& rhs) -> LuaTupleReturn&;
  auto operator=(LuaTupleReturn&& rhs) -> LuaTupleReturn&;
  template <typename... UTypes>
  auto operator=(LuaTupleReturn<UTypes...> const& rhs) -> LuaTupleReturn&;
  template <typename... UTypes>
  auto operator=(LuaTupleReturn<UTypes...>&& rhs) -> LuaTupleReturn&;
};

// std::tie for LuaTupleReturn
template <typename... Types>
auto luaTie(Types&... args) -> LuaTupleReturn<Types&...>;

// Constructs a LuaTupleReturn from the given arguments similar to make_tuple
template <typename... Types>
auto luaTupleReturn(Types&&... args) -> LuaTupleReturn<std::decay_t<Types>...>;

namespace LuaDetail {
struct LuaHandle {
  LuaHandle(LuaEnginePtr engine, int handleIndex);
  ~LuaHandle();

  LuaHandle(LuaHandle const& other);
  LuaHandle(LuaHandle&& other);

  auto operator=(LuaHandle const& other) -> LuaHandle&;
  auto operator=(LuaHandle&& other) -> LuaHandle&;

  LuaEnginePtr engine;
  int handleIndex = 0;
};

// Not meant to be used directly, exposes a raw interface for wrapped C++
// functions to be wrapped with the least amount of overhead.  Arguments are
// passed non-const so that they can be moved into wrapped functions that
// take values without copying.
using LuaFunctionReturn = Variant<LuaValue, LuaVariadic<LuaValue>>;
using LuaWrappedFunction = std::function<LuaFunctionReturn(LuaEngine&, size_t argc, LuaValue* argv)>;
}// namespace LuaDetail

// Prints the lua value similar to lua's print function, except it makes an
// attempt at printing tables.
auto operator<<(std::ostream& os, LuaValue const& value) -> std::ostream&;

// Holds a reference to a LuaEngine and a value held internally inside the
// registry of that engine.  The lifetime of the LuaEngine will be extended
// until all LuaReferences referencing it are destroyed.
class LuaReference {
public:
  LuaReference(LuaDetail::LuaHandle handle);

  LuaReference(LuaReference&&) = default;
  auto operator=(LuaReference&&) -> LuaReference& = default;

  LuaReference(LuaReference const&) = default;
  auto operator=(LuaReference const&) -> LuaReference& = default;

  auto operator==(LuaReference const& rhs) const -> bool;
  auto operator!=(LuaReference const& rhs) const -> bool;

  [[nodiscard("Engine reference must be used")]] auto engine() const -> LuaEngine&;
  [[nodiscard("Handle index needed for Lua operations")]] auto handleIndex() const -> int;

private:
  LuaDetail::LuaHandle m_handle;
};

class LuaString : public LuaReference {
public:
  using LuaReference::LuaReference;

  [[nodiscard("String pointer needed for access")]] auto ptr() const -> char const*;
  [[nodiscard("String length needed for operations")]] auto length() const -> size_t;

  [[nodiscard("String conversion creates new object")]] auto toString() const -> String;
  [[nodiscard("String view provides access")]] auto view() const -> StringView;
};

auto operator==(LuaString const& s1, LuaString const& s2) -> bool;
auto operator==(LuaString const& s1, char const* s2) -> bool;
auto operator==(LuaString const& s1, std::string const& s2) -> bool;
auto operator==(LuaString const& s1, String const& s2) -> bool;
auto operator==(char const* s1, LuaString const& s2) -> bool;
auto operator==(std::string const& s1, LuaString const& s2) -> bool;
auto operator==(String const& s1, LuaString const& s2) -> bool;

auto operator!=(LuaString const& s1, LuaString const& s2) -> bool;
auto operator!=(LuaString const& s1, char const* s2) -> bool;
auto operator!=(LuaString const& s1, std::string const& s2) -> bool;
auto operator!=(LuaString const& s1, String const& s2) -> bool;
auto operator!=(char const* s1, LuaString const& s2) -> bool;
auto operator!=(std::string const& s1, LuaString const& s2) -> bool;
auto operator!=(String const& s1, LuaString const& s2) -> bool;

class LuaTable : public LuaReference {
public:
  using LuaReference::LuaReference;

  template <typename T = LuaValue, typename K>
  auto get(K key) const -> T;
  template <typename T = LuaValue>
  auto get(char const* key) const -> T;

  template <typename T, typename K>
  void set(K key, T t) const;
  template <typename T>
  void set(char const* key, T t) const;

  // Shorthand for get(path) != LuaNil
  template <typename K>
  auto contains(K key) const -> bool;
  auto contains(char const* key) const -> bool;

  // Shorthand for setting to LuaNil
  template <typename K>
  void remove(K key) const;
  void remove(char const* key) const;

  // Result of lua # operator
  [[nodiscard("Table length needed for iteration")]] auto length() const -> LuaInt;

  // If iteration function returns bool, returning false signals stopping.
  template <typename Function>
  void iterate(Function&& iterator) const;

  template <typename Return, typename... Args, typename Function>
  void iterateWithSignature(Function&& func) const;

  [[nodiscard("Metatable may be empty")]] auto getMetatable() const -> std::optional<LuaTable>;
  void setMetatable(LuaTable const& table) const;

  template <typename T = LuaValue, typename K>
  auto rawGet(K key) const -> T;
  template <typename T = LuaValue>
  auto rawGet(char const* key) const -> T;

  template <typename T, typename K>
  void rawSet(K key, T t) const;
  template <typename T>
  void rawSet(char const* key, T t) const;

  [[nodiscard("Raw table length needed for operations")]] auto rawLength() const -> LuaInt;
};

class LuaFunction : public LuaReference {
public:
  using LuaReference::LuaReference;

  template <typename Ret = LuaValue, typename... Args>
  auto invoke(Args const&... args) const -> Ret;
};

class LuaThread : public LuaReference {
public:
  using LuaReference::LuaReference;
  enum class Status {
    Dead,
    Active,
    Error
  };

  // Will return a value if the thread has yielded a value, and nothing if the
  // thread has finished execution
  // Using deducing this (C++23) to deduce const-correctness
  template <typename Self, typename Ret = LuaValue, typename... Args>
  [[nodiscard("Thread result must be checked")]] 
  auto resume(this Self&& self, Args const&... args) -> std::optional<Ret> {
    auto res = self.engine().resumeThread(self.handleIndex(), args...);
    if (res)
      return LuaDetail::FromFunctionReturn<Ret>::convert(self.engine(), std::move(*res));
    return std::nullopt;
  }
  
  void pushFunction(LuaFunction const& func) const;
  [[nodiscard("Thread status needed for control flow")]] auto status() const -> Status;
};

// Keeping LuaReferences in LuaUserData will lead to circular references to
// LuaEngine, in addition to circular references in Lua which the Lua
// garbage collector can't collect. Don't put LuaReferences in LuaUserData.
class LuaUserData : public LuaReference {
public:
  using LuaReference::LuaReference;

  template <typename T>
  [[nodiscard("Type check result must be used")]] auto is() const -> bool;

  template <typename T>
  [[nodiscard("UserData reference must be used")]] auto get() const -> T&;
};

inline constexpr LuaValue LuaNil = LuaValue();

class LuaCallbacks {
public:
  void copyCallback(String srcName, String dstName);

  template <typename Function>
  void registerCallback(String name, Function&& func);

  auto removeCallback(String name) -> bool;

  template <typename Return, typename... Args, typename Function>
  void registerCallbackWithSignature(String name, Function&& func);

  auto merge(LuaCallbacks const& callbacks) -> LuaCallbacks&;

  [[nodiscard("Callback map needed for registration")]] auto callbacks() const -> StringMap<LuaDetail::LuaWrappedFunction> const&;

private:
  StringMap<LuaDetail::LuaWrappedFunction> m_callbacks;
};

template <typename T>
class LuaMethods {
public:
  template <typename Function>
  void registerMethod(String name, Function&& func);

  template <typename Return, typename... Args, typename Function>
  void registerMethodWithSignature(String name, Function&& func);

  [[nodiscard("Method map needed for registration")]] auto methods() const -> StringMap<LuaDetail::LuaWrappedFunction> const&;

private:
  StringMap<LuaDetail::LuaWrappedFunction> m_methods;
};

// A single execution context from a LuaEngine that manages a (mostly) distinct
// lua environment.  Each LuaContext's global environment is separate and one
// LuaContext can (mostly) not affect any other.
class LuaContext : protected LuaTable {
public:
  using RequireFunction = std::function<void(LuaContext&, LuaString const&)>;

  using LuaTable::LuaTable;

  using LuaTable::contains;
  using LuaTable::engine;
  using LuaTable::get;
  using LuaTable::handleIndex;
  using LuaTable::remove;
  using LuaTable::set;

  // Splits the path by '.' character, so can get / set values in tables inside
  // other tables.  If any table in the path is not a table but is accessed as
  // one, instead returns LuaNil.
  template <typename T = LuaValue>
  auto getPath(String path) const -> T;
  // Shorthand for getPath != LuaNil
  [[nodiscard("Path existence check must be used")]] auto containsPath(String path) const -> bool;
  // Will create new tables if the key contains paths that are nil
  template <typename T>
  void setPath(String path, T value);

  // Load the given code (either source or bytecode) into this context as a new
  // chunk.  It is not necessary to provide the name again if given bytecode.
  void load(char const* contents, size_t size, char const* name = nullptr);
  void load(std::span<char const> contents, char const* name = nullptr);
  void load(String const& contents, String const& name = String());
  void load(ByteArray const& contents, String const& name = String());

  // Evaluate a piece of lua code in this context, similar to the lua repl.
  // Can evaluate both expressions and statements.
  template <typename T = LuaValue>
  auto eval(String const& lua) -> T;

  // Override the built-in require function with the given function that takes
  // this LuaContext and the module name to load.
  void setRequireFunction(RequireFunction requireFunction);

  void setCallbacks(String const& tableName, LuaCallbacks const& callbacks) const;

  // For convenience, invokePath methods are equivalent to calling getPath(key)
  // to get a function, and then invoking it.

  template <typename Ret = LuaValue, typename... Args>
  auto invokePath(String const& key, Args const&... args) const -> Ret;

  // For convenience, calls to LuaEngine conversion / create functions are
  // duplicated here.

  template <typename T>
  auto luaFrom(T&& t) -> LuaValue;
  template <typename T>
  auto luaFrom(T const& t) -> LuaValue;
  template <typename T>
  auto luaMaybeTo(LuaValue const& v) -> std::optional<T>;
  template <typename T>
  auto luaMaybeTo(LuaValue&& v) -> std::optional<T>;
  template <typename T>
  auto luaTo(LuaValue const& v) -> T;
  template <typename T>
  auto luaTo(LuaValue&& v) -> T;

  auto createString(String const& str) -> LuaString;
  auto createString(char const* str) -> LuaString;

  auto createTable() -> LuaTable;

  template <LuaContainer Container>
  auto createTable(Container const& map) -> LuaTable;

  template <LuaContainer Container>
  auto createArrayTable(Container const& array) -> LuaTable;

  template <typename Function>
  auto createFunction(Function&& func) -> LuaFunction;

  template <typename Return, typename... Args, typename Function>
  auto createFunctionWithSignature(Function&& func) -> LuaFunction;

  template <typename T>
  auto createUserData(T t) -> LuaUserData;
};

template <typename T>
struct LuaNullTermWrapper : T {
  constexpr LuaNullTermWrapper() : T() {}
  constexpr LuaNullTermWrapper(LuaNullTermWrapper const& nt) : T(nt) {}
  constexpr LuaNullTermWrapper(LuaNullTermWrapper&& nt) noexcept(std::is_nothrow_move_constructible_v<T>) : T(std::move(nt)) {}
  constexpr LuaNullTermWrapper(T const& bt) : T(bt) {}
  constexpr LuaNullTermWrapper(T&& bt) noexcept(std::is_nothrow_move_constructible_v<T>) : T(std::move(bt)) {}

  using T::T;

  constexpr auto operator=(LuaNullTermWrapper const& rhs) -> LuaNullTermWrapper& {
    T::operator=(rhs);
    return *this;
  }

  constexpr auto operator=(LuaNullTermWrapper&& rhs) noexcept(std::is_nothrow_move_assignable_v<T>) -> LuaNullTermWrapper& {
    T::operator=(std::move(rhs));
    return *this;
  }

  constexpr auto operator=(T&& other) -> LuaNullTermWrapper& {
    T::operator=(std::forward<T>(other));
    return *this;
  }
};

class LuaNullEnforcer {
public:
  LuaNullEnforcer(LuaEngine& engine);
  LuaNullEnforcer(LuaNullEnforcer const&) = delete;
  LuaNullEnforcer(LuaNullEnforcer&&);
  ~LuaNullEnforcer();

private:
  LuaEngine* m_engine;
};

// Types that want to participate in automatic lua conversion should specialize
// this template and provide static to and from methods on it.  The method
// signatures will be called like:
//   LuaValue from(LuaEngine& engine, T t);
//   std::expected<T, LuaConversionError> tryTo(LuaEngine& engine, LuaValue v); // Modern API
//   std::optional<T> to(LuaEngine& engine, LuaValue v);  // Legacy API
// The methods can also take 'T const&' or 'LuaValue const&' as parameters, and
// the 'to' method can also return a bare T if conversion cannot fail.
template <typename T>
struct LuaConverter;

// C++20 Concepts for Lua type constraints
template <typename T>
concept LuaConvertible = requires(LuaEngine& engine, T value, LuaValue luaValue) {
  { LuaConverter<std::decay_t<T>>::from(engine, value) } -> std::same_as<LuaValue>;
  { LuaConverter<std::decay_t<T>>::to(engine, luaValue) };
};

// Modern concept requiring std::expected support
template <typename T>
concept LuaConvertibleExpected = LuaConvertible && requires(LuaEngine& engine, LuaValue luaValue) {
  { LuaConverter<std::decay_t<T>>::tryTo(engine, luaValue) } -> std::same_as<std::expected<T, LuaConversionError>>;
};

template <typename T>
concept LuaContainer = requires(T container) {
  typename T::value_type;
  { container.begin() } -> std::input_iterator;
  { container.end() } -> std::input_iterator;
  { container.size() } -> std::convertible_to<std::size_t>;
};

// UserData types that want to expose methods to lua should specialize this
// template.
template <typename T>
struct LuaUserDataMethods {
  static auto make() -> LuaMethods<T>;
};

// Convenience converter that simply converts to/from LuaUserData, can be
// derived from by a declared converter.
template <typename T>
struct LuaUserDataConverter {
  static auto from(LuaEngine& engine, T t) -> LuaValue;
  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<T>;
};

struct LuaProfileEntry {
  // Source name of the chunk the function was defined in
  String source;
  // Line number in the chunk of the beginning of the function definition
  unsigned sourceLine;
  // Name of the function, if it can be determined
  std::optional<String> name;
  // Scope of the function, if it can be determined
  std::optional<String> nameScope;
  // Time taken within this function itself
  std::int64_t selfTime;
  // Total time taken within this function or sub functions
  std::int64_t totalTime;
  // Calls from this function
  HashMap<std::tuple<String, unsigned>, std::shared_ptr<LuaProfileEntry>> calls;
};

// This class represents one execution engine in lua, holding a single
// lua_State.  Multiple contexts can be created, and they will have separate
// global environments and cannot affect each other.  Individual LuaEngines /
// LuaContexts are not thread safe, use one LuaEngine per thread.
class LuaEngine : public RefCounter {
public:
  // If 'safe' is true, then creates a lua engine with all builtin lua
  // functions that can affect the real world disabled.
  static auto create(bool safe = true) -> LuaEnginePtr;

  ~LuaEngine() override;

  LuaEngine(LuaEngine const&) = delete;
  LuaEngine(LuaEngine&&) = default;

  auto operator=(LuaEngine const&) -> LuaEngine& = delete;
  auto operator=(LuaEngine&&) -> LuaEngine& = default;

  // Set the instruction limit for computation sequences in the engine.  During
  // any function invocation, thread resume, or code evaluation, an instruction
  // counter will be started.  In the event that the instruction counter
  // becomes greater than the given limit, a LuaException will be thrown.  The
  // count is only reset when the initial entry into LuaEngine is returned,
  // recursive entries into LuaEngine accumulate the same instruction counter.
  // 0 disables the instruction limit.
  void setInstructionLimit(std::uint64_t instructionLimit = 0);
  [[nodiscard("Instruction limit needed for monitoring")]] auto instructionLimit() const -> std::uint64_t;

  // If profiling is enabled, then every 'measureInterval' instructions, the
  // function call stack will be recorded, and a summary of function timing can
  // be printed using profileReport
  void setProfilingEnabled(bool profilingEnabled);
  [[nodiscard("Profiling state needed for conditional logic")]] auto profilingEnabled() const -> bool;

  // Print a summary of the profiling data gathered since profiling was last
  // enabled.
  [[nodiscard("Profile data must be used")]] auto getProfile() -> List<LuaProfileEntry>;

  // If an instruction limit is set or profiling is enabled, this field
  // describes the resolution of instruction count measurement, and affects the
  // accuracy of profiling and the instruction count limit.  Defaults to 1000
  void setInstructionMeasureInterval(unsigned measureInterval = 1000);
  [[nodiscard("Measure interval needed for configuration")]] auto instructionMeasureInterval() const -> unsigned;

  // Sets the LuaEngine recursion limit, limiting the number of times a
  // LuaEngine call may directly or indirectly trigger a call back into the
  // LuaEngine, preventing a C++ stack overflow.  0 disables the limit.
  void setRecursionLimit(unsigned recursionLimit = 0);
  [[nodiscard("Recursion limit needed for safety checks")]] auto recursionLimit() const -> unsigned;

  // Compile a given script into bytecode.  If name is given, then it will be
  // used as the internal name for the resulting chunk and will provide better
  // error messages.
  //
  // Unfortunately the only way to completely ensure that a single script will
  // execute in two separate contexts and truly be isolated is to compile the
  // script to bytecode and load once in each context as a separate chunk.
  auto compile(char const* contents, size_t size, char const* name = nullptr) -> ByteArray;
  auto compile(std::span<char const> contents, char const* name = nullptr) -> ByteArray;
  auto compile(String const& contents, String const& name = String()) -> ByteArray;
  auto compile(ByteArray const& contents, String const& name = String()) -> ByteArray;

  // Returns the debug info of the state.
  auto debugInfo(int level = 1, const char* what = "nSlu") -> lua_Debug const&;

  // Generic from/to lua conversion, calls template specialization of
  // LuaConverter for actual conversion.
  template <typename T>
  auto luaFrom(T&& t) -> LuaValue;
  template <typename T>
  auto luaFrom(T const& t) -> LuaValue;
  
  // Modern API: Returns std::expected for structured error handling (C++23)
  template <typename T>
  [[nodiscard("Conversion result must be checked")]]
  auto luaConvertTo(LuaValue const& v) -> std::expected<T, LuaConversionError>;
  template <typename T>
  [[nodiscard("Conversion result must be checked")]]
  auto luaConvertTo(LuaValue&& v) -> std::expected<T, LuaConversionError>;
  
  // Legacy API: Returns std::optional (deprecated - use luaConvertTo)
  template <typename T>
  [[deprecated("Use luaConvertTo() for better error messages")]]
  auto luaMaybeTo(LuaValue const& v) -> std::optional<T>;
  template <typename T>
  [[deprecated("Use luaConvertTo() for better error messages")]]
  auto luaMaybeTo(LuaValue&& v) -> std::optional<T>;

  // Wraps luaConvertTo, throws an exception if conversion fails.
  template <typename T>
  auto luaTo(LuaValue const& v) -> T;
  template <typename T>
  auto luaTo(LuaValue&& v) -> T;

  auto createString(std::string const& str) -> LuaString;
  auto createString(String const& str) -> LuaString;
  auto createString(char const* str) -> LuaString;

  auto createTable(int narr = 0, int nrec = 0) -> LuaTable;

  template <LuaContainer Container>
  auto createTable(Container const& map) -> LuaTable;

  template <LuaContainer Container>
  auto createArrayTable(Container const& array) -> LuaTable;

  // Creates a function and deduces the signature of the function using
  // FunctionTraits.  As a convenience, the given function may optionally take
  // a LuaEngine& parameter as the first parameter, and if it does, when called
  // the function will get a reference to the calling LuaEngine.
  template <typename Function>
  auto createFunction(Function&& func) -> LuaFunction;

  // If the function signature is not deducible using FunctionTraits, you can
  // specify the return and argument types manually using this createFunction
  // version.
  template <typename Return, typename... Args, typename Function>
  auto createFunctionWithSignature(Function&& func) -> LuaFunction;

  auto createWrappedFunction(LuaDetail::LuaWrappedFunction function) -> LuaFunction;

  auto createRawFunction(lua_CFunction func) -> LuaFunction;

  auto createFunctionFromSource(int handleIndex, char const* contents, size_t size, char const* name) -> LuaFunction;

  auto createThread() -> LuaThread;

  template <typename T>
  auto createUserData(T t) -> LuaUserData;

  auto createContext() -> LuaContext;

  // Global environment changes only affect newly created contexts

  template <typename T = LuaValue, typename K>
  auto getGlobal(K key) -> T;
  template <typename T = LuaValue>
  auto getGlobal(char const* key) -> T;

  template <typename T, typename K>
  void setGlobal(K key, T value);

  template <typename T>
  void setGlobal(char const* key, T value);

  // Perform either a full or incremental garbage collection.
  void collectGarbage(std::optional<unsigned> steps = {});

  // Stop / start automatic garbage collection
  void setAutoGarbageCollection(bool autoGarbageColleciton);

  // Tune the pause and step values of the lua garbage collector
  void tuneAutoGarbageCollection(float pause, float stepMultiplier);

  // Bytes in use by lua
  [[nodiscard("Memory usage information must be used")]] auto memoryUsage() const -> std::size_t;

  // Enforce null-terminated string conversion as long as the returned enforcer object is in scope.
  [[nodiscard("Null enforcer RAII object must be kept alive")]] auto nullTerminate() -> LuaNullEnforcer;
  // Disables null-termination enforcement
  void setNullTerminated(bool nullTerminated);
  void addImGui();

private:
  friend struct LuaDetail::LuaHandle;
  friend class LuaReference;
  friend class LuaString;
  friend class LuaTable;
  friend class LuaFunction;
  friend class LuaThread;
  friend class LuaUserData;
  friend class LuaContext;
  friend class LuaNullEnforcer;

  LuaEngine() = default;

  // Get the LuaEngine* out of the lua registry magic entry.  Uses 1 stack
  // space, and does not call lua_checkstack.
  static auto luaEnginePtr(lua_State* state) -> LuaEngine*;
  // Counts instructions when instruction limiting is enabled.
  static void countHook(lua_State* state, lua_Debug* ar);

  static auto allocate(void* userdata, void* ptr, size_t oldSize, size_t newSize) -> void*;

  // Pops lua error from stack and throws LuaException
  void handleError(lua_State* state, int res);

  // lua_pcall with a better message handler that includes a traceback.
  auto pcallWithTraceback(lua_State* state, int nargs, int nresults) -> int;

  // override for lua coroutine resume with traceback
  static auto coresumeWithTraceback(lua_State* state) -> int;
  // propagates errors from one state to another, i.e. past thread boundaries
  // pops error off the top of the from stack and pushes onto the to stack
  static void propagateErrorWithTraceback(lua_State* from, lua_State* to);

  auto stringPtr(int handleIndex) -> char const*;
  auto stringLength(int handleIndex) -> size_t;
  auto string(int handleIndex) -> String;
  auto stringView(int handleIndex) -> StringView;

  auto tableGet(bool raw, int handleIndex, LuaValue const& key) -> LuaValue;
  auto tableGet(bool raw, int handleIndex, char const* key) -> LuaValue;

  void tableSet(bool raw, int handleIndex, LuaValue const& key, LuaValue const& value);
  void tableSet(bool raw, int handleIndex, char const* key, LuaValue const& value);

  auto tableLength(bool raw, int handleIndex) -> LuaInt;

  void tableIterate(int handleIndex, std::function_ref<bool(LuaValue, LuaValue)> iterator);

  auto tableGetMetatable(int handleIndex) -> std::optional<LuaTable>;
  void tableSetMetatable(int handleIndex, LuaTable const& table);

  template <typename... Args>
  auto callFunction(int handleIndex, Args const&... args) -> LuaDetail::LuaFunctionReturn;

  template <typename... Args>
  auto resumeThread(int handleIndex, Args const&... args) -> std::optional<LuaDetail::LuaFunctionReturn>;
  void threadPushFunction(int threadIndex, int functionIndex);
  auto threadStatus(int handleIndex) -> LuaThread::Status;

  template <typename T>
  void registerUserDataType();

  template <typename T>
  auto userDataIsType(int handleIndex) -> bool;

  template <typename T>
  auto getUserData(int handleIndex) -> T*;

  void setContextRequire(int handleIndex, LuaContext::RequireFunction requireFunction);

  void contextLoad(int handleIndex, char const* contents, size_t size, char const* name);

  auto contextEval(int handleIndex, String const& lua) -> LuaDetail::LuaFunctionReturn;

  auto contextGetPath(int handleIndex, String path) -> LuaValue;
  void contextSetPath(int handleIndex, String path, LuaValue const& value);

  auto popHandle(lua_State* state) -> int;
  void pushHandle(lua_State* state, int handleIndex);
  auto copyHandle(int handleIndex) -> int;
  void destroyHandle(int handleIndex);

  auto placeHandle() -> int;

  void pushLuaValue(lua_State* state, LuaValue const& luaValue);
  auto popLuaValue(lua_State* state) -> LuaValue;

  template <typename T>
  auto pushArgument(lua_State* state, T const& arg) -> std::size_t;

  template <typename T>
  auto pushArgument(lua_State* state, LuaVariadic<T> const& args) -> std::size_t;

  auto doPushArguments(lua_State*) -> std::size_t;
  template <typename First, typename... Rest>
  auto doPushArguments(lua_State* state, First const& first, Rest const&... rest) -> std::size_t;

  template <typename... Args>
  auto pushArguments(lua_State* state, Args const&... args) -> std::size_t;

  void incrementRecursionLevel();
  void decrementRecursionLevel();

  void updateCountHook();

  // The following fields exist to use their addresses as unique lightuserdata,
  // as is recommended by the lua docs.
  static int s_luaInstructionLimitExceptionKey;
  static int s_luaRecursionLimitExceptionKey;

  lua_State* m_state;
  int m_pcallTracebackMessageHandlerRegistryId;
  int m_scriptDefaultEnvRegistryId;
  int m_wrappedFunctionMetatableRegistryId;
  int m_requireFunctionMetatableRegistryId;
  HashMap<std::type_index, int> m_registeredUserDataTypes;

  lua_State* m_handleThread;
  int m_handleStackSize;
  int m_handleStackMax;
  List<int> m_handleFree;

  std::uint64_t m_instructionLimit;
  bool m_profilingEnabled;
  unsigned m_instructionMeasureInterval;
  std::uint64_t m_instructionCount;
  unsigned m_recursionLevel;
  unsigned m_recursionLimit;
  int m_nullTerminated;
  HashMap<std::tuple<String, unsigned>, std::shared_ptr<LuaProfileEntry>> m_profileEntries;
  lua_Debug m_debugInfo;
};

// Built in conversions

template <>
struct LuaConverter<bool> {
  static auto from(LuaEngine&, bool v) -> LuaValue {
    return v;
  }

  static auto to([[maybe_unused]] LuaEngine& engine, LuaValue const& v) -> std::optional<bool> {
    if (auto b = v.ptr<LuaBoolean>())
      return *b;
    if (v == LuaNil)
      return false;
    return true;
  }
  
  // Modern API with structured error
  static auto tryTo([[maybe_unused]] LuaEngine& engine, LuaValue const& v) -> std::expected<bool, LuaConversionError> {
    if (auto b = v.ptr<LuaBoolean>())
      return *b;
    if (v == LuaNil)
      return false;
    // Truthy conversion always succeeds
    return true;
  }
};

template <typename T>
struct LuaIntConverter {
  static auto from(LuaEngine&, T v) -> LuaValue {
    return LuaInt(v);
  }

  static auto to(LuaEngine&, LuaValue const& v) -> std::optional<T> {
    if (auto n = v.ptr<LuaInt>())
      return (T)*n;
    if (auto n = v.ptr<LuaFloat>())
      return (T)*n;
    if (auto s = v.ptr<LuaString>()) {
      if (auto n = maybeLexicalCast<LuaInt>(s->ptr()))
        return (T)*n;
      if (auto n = maybeLexicalCast<LuaFloat>(s->ptr()))
        return (T)*n;
    }
    return std::nullopt;
  }
};

template <>
struct LuaConverter<char> : LuaIntConverter<char> {};

template <>
struct LuaConverter<unsigned char> : LuaIntConverter<unsigned char> {};

template <>
struct LuaConverter<short> : LuaIntConverter<short> {};

template <>
struct LuaConverter<unsigned short> : LuaIntConverter<unsigned short> {};

template <>
struct LuaConverter<long> : LuaIntConverter<long> {};

template <>
struct LuaConverter<unsigned long> : LuaIntConverter<unsigned long> {};

template <>
struct LuaConverter<int> : LuaIntConverter<int> {};

template <>
struct LuaConverter<unsigned int> : LuaIntConverter<unsigned int> {};

template <>
struct LuaConverter<long long> : LuaIntConverter<long long> {};

template <>
struct LuaConverter<unsigned long long> : LuaIntConverter<unsigned long long> {};

template <typename T>
struct LuaFloatConverter {
  static auto from(LuaEngine&, T v) -> LuaValue {
    return LuaFloat(v);
  }

  static auto to(LuaEngine&, LuaValue const& v) -> std::optional<T> {
    if (auto n = v.ptr<LuaFloat>())
      return (T)*n;
    if (auto n = v.ptr<LuaInt>())
      return (T)*n;
    if (auto s = v.ptr<LuaString>()) {
      if (auto n = maybeLexicalCast<LuaFloat>(s->ptr()))
        return (T)*n;
      if (auto n = maybeLexicalCast<LuaInt>(s->ptr()))
        return (T)*n;
    }
    return std::nullopt;
  }
};

template <>
struct LuaConverter<float> : LuaFloatConverter<float> {};

template <>
struct LuaConverter<double> : LuaFloatConverter<double> {};

template <>
struct LuaConverter<String> {
  static auto from(LuaEngine& engine, String const& v) -> LuaValue {
    return engine.createString(v);
  }

  static auto to([[maybe_unused]] LuaEngine& engine, LuaValue const& v) -> std::optional<String> {
    if (v.is<LuaString>())
      return v.get<LuaString>().toString();
    if (v.is<LuaInt>())
      return String(toString(v.get<LuaInt>()));
    if (v.is<LuaFloat>())
      return String(toString(v.get<LuaFloat>()));
    return {};
  }
};

template <>
struct LuaConverter<std::string> {
  static auto from(LuaEngine& engine, std::string const& v) -> LuaValue {
    return engine.createString(v);
  }

  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<std::string> {
    return engine.luaTo<String>(std::move(v)).takeUtf8();
  }
};

template <>
struct LuaConverter<char const*> {
  static auto from(LuaEngine& engine, char const* v) -> LuaValue {
    return engine.createString(v);
  }
};

template <size_t s>
struct LuaConverter<std::array<char, s>> {
  static auto from(LuaEngine& engine, const std::array<char, s>& v) -> LuaValue {
    return engine.createString(v);
  }
};

template <>
struct LuaConverter<Directives> {
  static auto from(LuaEngine& engine, Directives const& v) -> LuaValue {
    if (String const* ptr = v.stringPtr())
      return engine.createString(*ptr);
    else
      return engine.createString("");
  }
};

template <>
struct LuaConverter<LuaString> {
  static auto from(LuaEngine&, LuaString v) -> LuaValue {
    return {std::move(v)};
  }

  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<LuaString> {
    if (v.is<LuaString>())
      return LuaString(std::move(v.get<LuaString>()));
    if (v.is<LuaInt>())
      return engine.createString(toString(v.get<LuaInt>()));
    if (v.is<LuaFloat>())
      return engine.createString(toString(v.get<LuaFloat>()));
    return {};
  }
};

template <typename T>
struct LuaValueConverter {
  static auto from(LuaEngine&, T v) -> LuaValue {
    return v;
  }

  static auto to(LuaEngine&, LuaValue v) -> std::optional<T> {
    if (auto p = v.ptr<T>()) {
      return std::move(*p);
    }
    return std::nullopt;
  }
};

template <>
struct LuaConverter<LuaTable> : LuaValueConverter<LuaTable> {};

template <>
struct LuaConverter<LuaFunction> : LuaValueConverter<LuaFunction> {};

template <>
struct LuaConverter<LuaThread> : LuaValueConverter<LuaThread> {};

template <>
struct LuaConverter<LuaUserData> : LuaValueConverter<LuaUserData> {};

template <>
struct LuaConverter<LuaValue> {
  static auto from(LuaEngine&, LuaValue v) -> LuaValue {
    return v;
  }

  static auto to(LuaEngine&, LuaValue v) -> LuaValue {
    return v;
  }
};

template <typename T>
struct LuaConverter<std::optional<T>> {
  static auto from(LuaEngine& engine, std::optional<T> const& v) -> LuaValue {
    if (v)
      return engine.luaFrom<T>(*v);
    else
      return LuaNil;
  }

  static auto from(LuaEngine& engine, std::optional<T>&& v) -> LuaValue {
    if (v)
      return engine.luaFrom<T>(std::move(*v));
    else
      return LuaNil;
  }

  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<std::optional<T>> {
    if (v != LuaNil) {
      if (auto conv = engine.luaMaybeTo<T>(v))
        return conv;
      else
        return std::nullopt;
    } else {
      return std::optional<T>();
    }
  }

  static auto to(LuaEngine& engine, LuaValue&& v) -> std::optional<std::optional<T>> {
    if (v != LuaNil) {
      if (auto conv = engine.luaMaybeTo<T>(std::move(v)))
        return conv;
      else
        return std::nullopt;
    } else {
      return std::optional<T>();
    }
  }
};

template <typename T>
struct LuaMapConverter {
  static auto from(LuaEngine& engine, T const& v) -> LuaValue {
    return engine.createTable(v);
  }

  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<T> {
    auto table = v.ptr<LuaTable>();
    if (!table)
      return {};

    T result;
    bool failed = false;
    table->iterate([&result, &failed, &engine](LuaValue key, LuaValue value) -> auto {
      auto contKey = engine.luaMaybeTo<typename T::key_type>(std::move(key));
      auto contValue = engine.luaMaybeTo<typename T::mapped_type>(std::move(value));
      if (!contKey || !contValue) {
        failed = true;
        return false;
      }
      result[std::move(*contKey)] = std::move(*contValue);
      return true;
    });

    if (failed)
      return {};

    return result;
  }
};

template <typename T>
struct LuaContainerConverter {
  static auto from(LuaEngine& engine, T const& v) -> LuaValue {
    return engine.createArrayTable(v);
  }

  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<T> {
    auto table = v.ptr<LuaTable>();
    if (!table)
      return {};

    T result;
    bool failed = false;
    table->iterate([&result, &failed, &engine](LuaValue key, LuaValue value) -> auto {
      if (!key.is<LuaInt>()) {
        failed = true;
        return false;
      }
      auto contVal = engine.luaMaybeTo<typename T::value_type>(std::move(value));
      if (!contVal) {
        failed = true;
        return false;
      }
      result.insert(result.end(), std::move(*contVal));
      return true;
    });

    if (failed)
      return {};

    return result;
  }
};

template <typename T, typename Allocator>
struct LuaConverter<List<T, Allocator>> : LuaContainerConverter<List<T, Allocator>> {};

template <typename T, typename Allocator, typename Equals>
struct LuaConverter<HashSet<T, Allocator, Equals>> : LuaContainerConverter<HashSet<T, Allocator, Equals>> {};

template <typename T, size_t MaxSize>
struct LuaConverter<StaticList<T, MaxSize>> : LuaContainerConverter<StaticList<T, MaxSize>> {};

template <typename T, size_t MaxStackSize>
struct LuaConverter<SmallList<T, MaxStackSize>> : LuaContainerConverter<SmallList<T, MaxStackSize>> {};

template <>
struct LuaConverter<StringList> : LuaContainerConverter<StringList> {};

template <typename T, typename BaseSet>
struct LuaConverter<Set<T, BaseSet>> : LuaContainerConverter<Set<T, BaseSet>> {};

template <typename T, typename BaseSet>
struct LuaConverter<HashSet<T, BaseSet>> : LuaContainerConverter<HashSet<T, BaseSet>> {};

template <typename Key, typename Value, typename Compare, typename Allocator>
struct LuaConverter<Map<Key, Value, Compare, Allocator>> : LuaMapConverter<Map<Key, Value, Compare, Allocator>> {};

template <typename Key, typename Value, typename Hash, typename Equals, typename Allocator>
struct LuaConverter<HashMap<Key, Value, Hash, Equals, Allocator>> : LuaMapConverter<HashMap<Key, Value, Hash, Equals, Allocator>> {};

template <>
struct LuaConverter<Json> {
  static auto from(LuaEngine& engine, Json const& v) -> LuaValue;
  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<Json>;
};

template <>
struct LuaConverter<JsonObject> {
  static auto from(LuaEngine& engine, JsonObject v) -> LuaValue;
  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<JsonObject>;
};

template <>
struct LuaConverter<JsonArray> {
  static auto from(LuaEngine& engine, JsonArray v) -> LuaValue;
  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<JsonArray>;
};

namespace LuaDetail {
inline LuaHandle::LuaHandle(LuaEnginePtr engine, int handleIndex)
    : engine(std::move(engine)), handleIndex(handleIndex) {}

inline LuaHandle::~LuaHandle() {
  if (engine)
    engine->destroyHandle(handleIndex);
}

inline LuaHandle::LuaHandle(LuaHandle const& other) {
  engine = other.engine;
  if (engine)
    handleIndex = engine->copyHandle(other.handleIndex);
}

inline LuaHandle::LuaHandle(LuaHandle&& other) {
  engine = take(other.engine);
  handleIndex = take(other.handleIndex);
}

inline auto LuaHandle::operator=(LuaHandle const& other) -> LuaHandle& {
  if (engine)
    engine->destroyHandle(handleIndex);

  engine = other.engine;
  if (engine)
    handleIndex = engine->copyHandle(other.handleIndex);

  return *this;
}

inline auto LuaHandle::operator=(LuaHandle&& other) -> LuaHandle& {
  if (engine)
    engine->destroyHandle(handleIndex);

  engine = take(other.engine);
  handleIndex = take(other.handleIndex);

  return *this;
}

template <typename T>
struct FromFunctionReturn {
  static auto convert(LuaEngine& engine, LuaFunctionReturn const& ret) -> T {
    if (auto l = ret.ptr<LuaValue>()) {
      return engine.luaTo<T>(*l);
    } else if (auto vec = ret.ptr<LuaVariadic<LuaValue>>()) {
      return engine.luaTo<T>(vec->at(0));
    } else {
      return engine.luaTo<T>(LuaNil);
    }
  }
};

template <typename T>
struct FromFunctionReturn<LuaVariadic<T>> {
  static auto convert(LuaEngine& engine, LuaFunctionReturn const& ret) -> LuaVariadic<T> {
    if (auto l = ret.ptr<LuaValue>()) {
      return {engine.luaTo<T>(*l)};
    } else if (auto vec = ret.ptr<LuaVariadic<LuaValue>>()) {
      LuaVariadic<T> ret(vec->size());
      for (size_t i = 0; i < vec->size(); ++i)
        ret[i] = engine.luaTo<T>((*vec)[i]);
      return ret;
    } else {
      return {};
    }
  }
};

template <typename ArgFirst, typename... ArgRest>
struct FromFunctionReturn<LuaTupleReturn<ArgFirst, ArgRest...>> {
  static auto convert(LuaEngine& engine, LuaFunctionReturn const& ret) -> LuaTupleReturn<ArgFirst, ArgRest...> {
    if (auto l = ret.ptr<LuaValue>()) {
      return doConvertSingle(engine, *l, typename GenIndexSequence<0, sizeof...(ArgRest)>::type());
    } else if (auto vec = ret.ptr<LuaVariadic<LuaValue>>()) {
      return doConvertMulti(engine, *vec, typename GenIndexSequence<0, sizeof...(ArgRest)>::type());
    } else {
      return doConvertNone(engine, typename GenIndexSequence<0, sizeof...(ArgRest)>::type());
    }
  }

  template <size_t... Indexes>
  static auto doConvertSingle(
    LuaEngine& engine, LuaValue const& single, IndexSequence<Indexes...> const&) -> LuaTupleReturn<ArgFirst, ArgRest...> {
    return LuaTupleReturn<ArgFirst, ArgRest...>(engine.luaTo<ArgFirst>(single), engine.luaTo<ArgRest>(LuaNil)...);
  }

  template <size_t... Indexes>
  static auto doConvertMulti(
    LuaEngine& engine, LuaVariadic<LuaValue> const& multi, IndexSequence<Indexes...> const&) -> LuaTupleReturn<ArgFirst, ArgRest...> {
    return LuaTupleReturn<ArgFirst, ArgRest...>(
      engine.luaTo<ArgFirst>(multi.at(0)), engine.luaTo<ArgRest>(multi.get(Indexes + 1))...);
  }

  template <size_t... Indexes>
  static auto doConvertNone(LuaEngine& engine, IndexSequence<Indexes...> const&) -> LuaTupleReturn<ArgFirst, ArgRest...> {
    return LuaTupleReturn<ArgFirst, ArgRest...>(engine.luaTo<ArgFirst>(LuaNil), engine.luaTo<ArgRest>(LuaNil)...);
  }
};

template <typename... Args, size_t... Indexes>
auto toVariadicReturn(
  LuaEngine& engine, LuaTupleReturn<Args...> const& vals, IndexSequence<Indexes...> const&) -> LuaVariadic<LuaValue> {
  return LuaVariadic<LuaValue>{engine.luaFrom(get<Indexes>(vals))...};
}

template <typename... Args>
auto toWrappedReturn(LuaEngine& engine, LuaTupleReturn<Args...> const& vals) -> LuaVariadic<LuaValue> {
  return toVariadicReturn(engine, vals, typename GenIndexSequence<0, sizeof...(Args)>::type());
}

template <typename T>
auto toWrappedReturn(LuaEngine& engine, LuaVariadic<T> const& vals) -> LuaVariadic<LuaValue> {
  LuaVariadic<LuaValue> ret(vals.size());
  for (size_t i = 0; i < vals.size(); ++i)
    ret[i] = engine.luaFrom(vals[i]);
  return ret;
}

template <typename T>
auto toWrappedReturn(LuaEngine& engine, T const& t) -> LuaValue {
  return engine.luaFrom(t);
}

template <typename T>
struct ArgGet {
  static auto get(LuaEngine& engine, size_t argc, LuaValue* argv, size_t index) -> T {
    if (index < argc)
      return engine.luaTo<T>(std::move(argv[index]));
    return engine.luaTo<T>(LuaNil);
  }
};

template <typename T>
struct ArgGet<LuaVariadic<T>> {
  static auto get(LuaEngine& engine, size_t argc, LuaValue* argv, size_t index) -> LuaVariadic<T> {
    if (index >= argc)
      return {};

    LuaVariadic<T> subargs(argc - index);
    for (size_t i = index; i < argc; ++i)
      subargs[i - index] = engine.luaTo<T>(std::move(argv[i]));
    return subargs;
  }
};

template <typename Return, typename... Args>
struct FunctionWrapper {
  template <typename Function, size_t... Indexes>
  static auto wrapIndexes(Function func, IndexSequence<Indexes...> const&) -> LuaWrappedFunction {
    return [func = std::move(func)](LuaEngine& engine, size_t argc, LuaValue* argv) -> auto {
      return toWrappedReturn(engine, func(ArgGet<Args>::get(engine, argc, argv, Indexes)...));
    };
  }

  template <typename Function>
  static auto wrap(Function func) -> LuaWrappedFunction {
    return wrapIndexes(std::forward<Function>(func), typename GenIndexSequence<0, sizeof...(Args)>::type());
  }
};

template <typename... Args>
struct FunctionWrapper<void, Args...> {
  template <typename Function, size_t... Indexes>
  static auto wrapIndexes(Function func, IndexSequence<Indexes...> const&) -> LuaWrappedFunction {
    return [func = std::move(func)](LuaEngine& engine, size_t argc, LuaValue* argv) -> auto {
      func(ArgGet<Args>::get(engine, argc, argv, Indexes)...);
      return LuaFunctionReturn();
    };
  }

  template <typename Function>
  static auto wrap(Function func) -> LuaWrappedFunction {
    return wrapIndexes(std::forward<Function>(func), typename GenIndexSequence<0, sizeof...(Args)>::type());
  }
};

template <typename Return, typename... Args>
struct FunctionWrapper<Return, LuaEngine, Args...> {
  template <typename Function, size_t... Indexes>
  static auto wrapIndexes(Function func, IndexSequence<Indexes...> const&) -> LuaWrappedFunction {
    return [func = std::move(func)](LuaEngine& engine, size_t argc, LuaValue* argv) -> auto {
      return toWrappedReturn(engine, (Return const&)func(engine, ArgGet<Args>::get(engine, argc, argv, Indexes)...));
    };
  }

  template <typename Function>
  static auto wrap(Function func) -> LuaWrappedFunction {
    return wrapIndexes(std::forward<Function>(func), typename GenIndexSequence<0, sizeof...(Args)>::type());
  }
};

template <typename... Args>
struct FunctionWrapper<void, LuaEngine, Args...> {
  template <typename Function, size_t... Indexes>
  static auto wrapIndexes(Function func, IndexSequence<Indexes...> const&) -> LuaWrappedFunction {
    return [func = std::move(func)](LuaEngine& engine, size_t argc, LuaValue* argv) -> auto {
      func(engine, ArgGet<Args>::get(engine, argc, argv, Indexes)...);
      return LuaFunctionReturn();
    };
  }

  template <typename Function>
  static auto wrap(Function func) -> LuaWrappedFunction {
    return wrapIndexes(std::forward<Function>(func), typename GenIndexSequence<0, sizeof...(Args)>::type());
  }
};

template <typename Return, typename... Args, typename Function>
auto wrapFunctionWithSignature(Function&& func) -> LuaWrappedFunction {
  return FunctionWrapper<Return, std::decay_t<Args>...>::wrap(std::forward<Function>(func));
}

template <typename Return, typename Function, typename... Args>
auto wrapFunctionArgs(Function&& func, VariadicTypedef<Args...> const&) -> LuaWrappedFunction {
  return wrapFunctionWithSignature<Return, Args...>(std::forward<Function>(func));
}

template <typename Function>
auto wrapFunction(Function&& func) -> LuaWrappedFunction {
  return wrapFunctionArgs<typename FunctionTraits<Function>::Return>(
    std::forward<Function>(func), typename FunctionTraits<Function>::Args());
}

template <typename Return, typename T, typename... Args>
struct MethodWrapper {
  template <typename Function, size_t... Indexes>
  static auto wrapIndexes(Function func, IndexSequence<Indexes...> const&) -> LuaWrappedFunction {
    return [func = std::move(func)](LuaEngine& engine, size_t argc, LuaValue* argv) mutable -> auto {
      if (argc == 0)
        throw LuaException("No object argument passed to wrapped method");
      return toWrappedReturn(engine,
                             (Return const&)func(argv[0].get<LuaUserData>().get<T>(), ArgGet<Args>::get(engine, argc - 1, argv + 1, Indexes)...));
    };
  }

  template <typename Function>
  static auto wrap(Function&& func) -> LuaWrappedFunction {
    return wrapIndexes(std::forward<Function>(func), typename GenIndexSequence<0, sizeof...(Args)>::type());
  }
};

template <typename T, typename... Args>
struct MethodWrapper<void, T, Args...> {
  template <typename Function, size_t... Indexes>
  static auto wrapIndexes(Function func, IndexSequence<Indexes...> const&) -> LuaWrappedFunction {
    return [func = std::move(func)](LuaEngine& engine, size_t argc, LuaValue* argv) -> auto {
      if (argc == 0)
        throw LuaException("No object argument passed to wrapped method");
      func(argv[0].get<LuaUserData>().get<T>(), ArgGet<Args>::get(engine, argc - 1, argv + 1, Indexes)...);
      return LuaFunctionReturn();
    };
  }

  template <typename Function>
  static auto wrap(Function func) -> LuaWrappedFunction {
    return wrapIndexes(std::forward<Function>(func), typename GenIndexSequence<0, sizeof...(Args)>::type());
  }
};

template <typename Return, typename T, typename... Args>
struct MethodWrapper<Return, T, LuaEngine, Args...> {
  template <typename Function, size_t... Indexes>
  static auto wrapIndexes(Function func, IndexSequence<Indexes...> const&) -> LuaWrappedFunction {
    return [func = std::move(func)](LuaEngine& engine, size_t argc, LuaValue* argv) -> auto {
      if (argc == 0)
        throw LuaException("No object argument passed to wrapped method");
      return toWrappedReturn(
        engine,
        (Return const&)func(argv[0].get<LuaUserData>().get<T>(), engine, ArgGet<Args>::get(engine, argc - 1, argv + 1, Indexes)...));
    };
  }

  template <typename Function>
  static auto wrap(Function func) -> LuaWrappedFunction {
    return wrapIndexes(std::forward<Function>(func), typename GenIndexSequence<0, sizeof...(Args)>::type());
  }
};

template <typename T, typename... Args>
struct MethodWrapper<void, T, LuaEngine, Args...> {
  template <typename Function, size_t... Indexes>
  static auto wrapIndexes(Function func, IndexSequence<Indexes...> const&) -> LuaWrappedFunction {
    return [func = std::move(func)](LuaEngine& engine, size_t argc, LuaValue* argv) -> auto {
      if (argc == 0)
        throw LuaException("No object argument passed to wrapped method");
      func(argv[0].get<LuaUserData>().get<T>(), engine, ArgGet<Args>::get(engine, argc - 1, argv + 1, Indexes)...);
      return LuaValue();
    };
  }

  template <typename Function>
  static auto wrap(Function func) -> LuaWrappedFunction {
    return wrapIndexes(std::forward<Function>(func), typename GenIndexSequence<0, sizeof...(Args)>::type());
  }
};

template <typename Return, typename... Args, typename Function>
auto wrapMethodWithSignature(Function&& func) -> LuaWrappedFunction {
  return MethodWrapper<Return, std::decay_t<Args>...>::wrap(std::forward<Function>(func));
}

template <typename Return, typename Function, typename... Args>
auto wrapMethodArgs(Function&& func, VariadicTypedef<Args...> const&) -> LuaWrappedFunction {
  return wrapMethodWithSignature<Return, Args...>(std::forward<Function>(func));
}

template <typename Function>
auto wrapMethod(Function&& func) -> LuaWrappedFunction {
  return wrapMethodArgs<typename FunctionTraits<Function>::Return>(
    std::forward<Function>(func), typename FunctionTraits<Function>::Args());
}

template <typename Ret, typename... Args>
struct TableIteratorWrapper;

template <typename Key, typename Value>
struct TableIteratorWrapper<bool, LuaEngine&, Key, Value> {
  template <typename Function>
  static auto wrap(LuaEngine& engine, Function&& func) -> std::function<bool(LuaValue, LuaValue)> {
    return [&engine, func = std::move(func)](LuaValue key, LuaValue value) -> bool {
      return func(engine, engine.luaTo<Key>(std::move(key)), engine.luaTo<Value>(std::move(value)));
    };
  }
};

template <typename Key, typename Value>
struct TableIteratorWrapper<void, LuaEngine&, Key, Value> {
  template <typename Function>
  static auto wrap(LuaEngine& engine, Function&& func) -> std::function<bool(LuaValue, LuaValue)> {
    return [&engine, func = std::move(func)](LuaValue key, LuaValue value) -> bool {
      func(engine, engine.luaTo<Key>(std::move(key)), engine.luaTo<Value>(std::move(value)));
      return true;
    };
  }
};

template <typename Key, typename Value>
struct TableIteratorWrapper<bool, Key, Value> {
  template <typename Function>
  static auto wrap(LuaEngine& engine, Function&& func) -> std::function<bool(LuaValue, LuaValue)> {
    return [&engine, func = std::move(func)](LuaValue key, LuaValue value) -> bool {
      return func(engine.luaTo<Key>(std::move(key)), engine.luaTo<Value>(std::move(value)));
    };
  }
};

template <typename Key, typename Value>
struct TableIteratorWrapper<void, Key, Value> {
  template <typename Function>
  static auto wrap(LuaEngine& engine, Function&& func) -> std::function<bool(LuaValue, LuaValue)> {
    return [&engine, func = std::move(func)](LuaValue key, LuaValue value) -> bool {
      func(engine.luaTo<Key>(std::move(key)), engine.luaTo<Value>(std::move(value)));
      return true;
    };
  }
};

template <typename Return, typename... Args, typename Function>
auto wrapTableIteratorWithSignature(LuaEngine& engine, Function&& func) -> std::function<bool(LuaValue, LuaValue)> {
  return TableIteratorWrapper<Return, std::decay_t<Args>...>::wrap(engine, std::forward<Function>(func));
}

template <typename Return, typename Function, typename... Args>
auto wrapTableIteratorArgs(
  LuaEngine& engine, Function&& func, VariadicTypedef<Args...> const&) -> std::function<bool(LuaValue, LuaValue)> {
  return wrapTableIteratorWithSignature<Return, Args...>(engine, std::forward<Function>(func));
}

template <typename Function>
auto wrapTableIterator(LuaEngine& engine, Function&& func) -> std::function<bool(LuaValue, LuaValue)> {
  return wrapTableIteratorArgs<typename FunctionTraits<Function>::Return>(
    engine, std::forward<Function>(func), typename FunctionTraits<Function>::Args());
}

// Like lua_setfield / lua_getfield but raw.
void rawSetField(lua_State* state, int index, char const* key);
void rawGetField(lua_State* state, int index, char const* key);

// Shallow copies a lua table at the given index into the table at the target
// index.
void shallowCopy(lua_State* state, int sourceIndex, int targetIndex);

auto insertJsonMetatable(LuaEngine& engine, LuaTable const& table, Json::Type type) -> LuaTable;

// Creates a custom lua table from a JsonArray or JsonObject that has
// slightly different behavior than a standard lua table.  The table
// remembers nil entries, as well as whether it was initially constructed
// from a JsonArray or JsonObject as a hint on how to convert it back into a
// Json.  The custom containers are meant to act nearly identical to standard
// lua tables, so iterating over the table with pairs or ipairs works exactly
// like a standard lua table, so will skip over nil entries and in the case
// of ipairs, stop at the first nil entry.
auto jsonContainerToTable(LuaEngine& engine, Json const& container) -> LuaTable;

// popJsonContainer must be called with a lua table on the top of the stack.
// Uses the table contents, as well as any hint entries if the table was
// created originally from a Json, to determine whether a JsonArray or
// JsonObject is more appropriate.
auto tableToJsonContainer(LuaTable const& table) -> std::optional<Json>;

// Special lua functions to operate on our custom jarray / jobject container
// types.  Should always do some "sensible" action if given a regular lua
// table instead of a custom json container one.

// Create a JsonList container table
auto jarrayCreate() -> Json;
// Create a JsonMap container table
auto jobjectCreate() -> Json;

// Adds the Json array metatable to a Lua table or creates one.
auto jarray(LuaEngine& engine, std::optional<LuaTable> table) -> LuaTable;
// Adds the Json object metatable to a Lua table or creates one.
auto jobject(LuaEngine& engine, std::optional<LuaTable> table) -> LuaTable;

// *Really* remove an entry from a JsonList or JsonMap container table,
// including removing it from the __nils table.  If the given table is not a
// special container table, is equivalent to setting the key entry to nil.
void jcontRemove(LuaTable const& t, LuaValue const& key);

// Returns the element count of the lua table argument, or, in the case of a
// special JsonList container table, returns the "true" element count
// including any nil entries.
auto jcontSize(LuaTable const& t) -> size_t;

// Resize the given lua table by removing any indexed entries greater than the
// target size, and in the case of a special JsonList container table, pads
// to the end of the new size with nil entries.
void jcontResize(LuaTable const& t, size_t size);

// Coerces a values (strings, floats, ints) into an integer, but fails if the
// number looks fractional (does not parse as int, float is not an exact
// integer)
auto asInteger(LuaValue const& v) -> std::optional<LuaInt>;
}// namespace LuaDetail

template <typename Container>
auto luaUnpack(Container&& c) -> LuaVariadic<typename std::decay<Container>::type::value_type> {
  LuaVariadic<typename std::decay<Container>::type::value_type> ret;
  if (std::is_rvalue_reference_v<Container&&>) {
    for (auto& e : c)
      ret.append(std::move(e));
  } else {
    for (auto const& e : c)
      ret.append(e);
  }
  return ret;
}

template <typename... Types>
LuaTupleReturn<Types...>::LuaTupleReturn(Types const&... args)
    : Base(args...) {}

template <typename... Types>
template <typename... UTypes>
LuaTupleReturn<Types...>::LuaTupleReturn(UTypes&&... args)
    : Base(std::move(args)...) {}

template <typename... Types>
template <typename... UTypes>
LuaTupleReturn<Types...>::LuaTupleReturn(UTypes const&... args)
    : Base(args...) {}

template <typename... Types>
LuaTupleReturn<Types...>::LuaTupleReturn(LuaTupleReturn const& rhs)
    : Base(rhs) {}

template <typename... Types>
LuaTupleReturn<Types...>::LuaTupleReturn(LuaTupleReturn&& rhs)
    : Base(std::move(rhs)) {}

template <typename... Types>
template <typename... UTypes>
LuaTupleReturn<Types...>::LuaTupleReturn(LuaTupleReturn<UTypes...> const& rhs)
    : Base(rhs) {}

template <typename... Types>
template <typename... UTypes>
LuaTupleReturn<Types...>::LuaTupleReturn(LuaTupleReturn<UTypes...>&& rhs)
    : Base(std::move(rhs)) {}

template <typename... Types>
auto LuaTupleReturn<Types...>::operator=(LuaTupleReturn const& rhs) -> LuaTupleReturn<Types...>& {
  Base::operator=(rhs);
  return *this;
}

template <typename... Types>
auto LuaTupleReturn<Types...>::operator=(LuaTupleReturn&& rhs) -> LuaTupleReturn<Types...>& {
  Base::operator=(std::move(rhs));
  return *this;
}

template <typename... Types>
template <typename... UTypes>
auto LuaTupleReturn<Types...>::operator=(LuaTupleReturn<UTypes...> const& rhs) -> LuaTupleReturn<Types...>& {
  Base::operator=((std::tuple<UTypes...> const&)rhs);
  return *this;
}

template <typename... Types>
template <typename... UTypes>
auto LuaTupleReturn<Types...>::operator=(LuaTupleReturn<UTypes...>&& rhs) -> LuaTupleReturn<Types...>& {
  Base::operator=((std::tuple<UTypes...>&&)std::move(rhs));
  return *this;
}

template <typename... Types>
auto luaTie(Types&... args) -> LuaTupleReturn<Types&...> {
  return LuaTupleReturn<Types&...>(args...);
}

template <typename... Types>
auto luaTupleReturn(Types&&... args) -> LuaTupleReturn<std::decay_t<Types>...> {
  return LuaTupleReturn<std::decay_t<Types>...>(std::forward<Types>(args)...);
}

inline LuaReference::LuaReference(LuaDetail::LuaHandle handle) : m_handle(std::move(handle)) {}

inline auto LuaReference::operator==(LuaReference const& rhs) const -> bool {
  return std::tie(m_handle.engine, m_handle.handleIndex) == std::tie(rhs.m_handle.engine, rhs.m_handle.handleIndex);
}

inline auto LuaReference::operator!=(LuaReference const& rhs) const -> bool {
  return std::tie(m_handle.engine, m_handle.handleIndex) != std::tie(rhs.m_handle.engine, rhs.m_handle.handleIndex);
}

inline auto LuaReference::engine() const -> LuaEngine& {
  return *m_handle.engine;
}

inline auto LuaReference::handleIndex() const noexcept -> int {
  return m_handle.handleIndex;
}

inline auto LuaString::ptr() const -> char const* {
  return engine().stringPtr(handleIndex());
}

inline auto LuaString::length() const -> size_t {
  return engine().stringLength(handleIndex());
}

inline auto LuaString::toString() const -> String {
  return engine().string(handleIndex());
}

inline auto LuaString::view() const -> StringView {
  return engine().stringView(handleIndex());
}

inline auto operator==(LuaString const& s1, LuaString const& s2) -> bool {
  return s1.view() == s2.view();
}

inline auto operator==(LuaString const& s1, char const* s2) -> bool {
  return s1.view() == s2;
}

inline auto operator==(LuaString const& s1, std::string const& s2) -> bool {
  return s1.view() == s2;
}

inline auto operator==(LuaString const& s1, String const& s2) -> bool {
  return s1.view() == s2;
}

inline auto operator==(char const* s1, LuaString const& s2) -> bool {
  return s2.view() == s1;
}

inline auto operator==(std::string const& s1, LuaString const& s2) -> bool {
  return s2.view() == s1;
}

inline auto operator==(String const& s1, LuaString const& s2) -> bool {
  return s2.view() == s1;
}

inline auto operator!=(LuaString const& s1, LuaString const& s2) -> bool {
  return !(s1 == s2);
}

inline auto operator!=(LuaString const& s1, char const* s2) -> bool {
  return !(s1 == s2);
}

inline auto operator!=(LuaString const& s1, std::string const& s2) -> bool {
  return !(s1 == s2);
}

inline auto operator!=(LuaString const& s1, String const& s2) -> bool {
  return !(s1 == s2);
}

inline auto operator!=(char const* s1, LuaString const& s2) -> bool {
  return !(s1 == s2);
}

inline auto operator!=(std::string const& s1, LuaString const& s2) -> bool {
  return !(s1 == s2);
}

inline auto operator!=(String const& s1, LuaString const& s2) -> bool {
  return !(s1 == s2);
}

template <typename T, typename K>
auto LuaTable::get(K key) const -> T {
  return engine().luaTo<T>(engine().tableGet(false, handleIndex(), engine().luaFrom(std::move(key))));
}

template <typename T>
auto LuaTable::get(char const* key) const -> T {
  return engine().luaTo<T>(engine().tableGet(false, handleIndex(), key));
}

template <typename T, typename K>
void LuaTable::set(K key, T value) const {
  engine().tableSet(false, handleIndex(), engine().luaFrom(std::move(key)), engine().luaFrom(std::move(value)));
}

template <typename T>
void LuaTable::set(char const* key, T value) const {
  engine().tableSet(false, handleIndex(), key, engine().luaFrom(std::move(value)));
}

template <typename K>
auto LuaTable::contains(K key) const -> bool {
  return engine().tableGet(false, handleIndex(), engine().luaFrom(std::move(key))) != LuaNil;
}

template <typename K>
void LuaTable::remove(K key) const {
  engine().tableSet(false, handleIndex(), engine().luaFrom(key), LuaValue());
}

template <typename Function>
void LuaTable::iterate(Function&& function) const {
  return engine().tableIterate(handleIndex(), LuaDetail::wrapTableIterator(engine(), std::forward<Function>(function)));
}

template <typename Return, typename... Args, typename Function>
void LuaTable::iterateWithSignature(Function&& func) const {
  return engine().tableIterate(handleIndex(), LuaDetail::wrapTableIteratorWithSignature<Return, Args...>(engine(), std::forward<Function>(func)));
}

template <typename T, typename K>
auto LuaTable::rawGet(K key) const -> T {
  return engine().luaTo<T>(engine().tableGet(true, handleIndex(), engine().luaFrom(key)));
}

template <typename T>
auto LuaTable::rawGet(char const* key) const -> T {
  return engine().luaTo<T>(engine().tableGet(true, handleIndex(), key));
}

template <typename T, typename K>
void LuaTable::rawSet(K key, T value) const {
  engine().tableSet(true, handleIndex(), engine().luaFrom(key), engine().luaFrom(value));
}

template <typename T>
void LuaTable::rawSet(char const* key, T value) const {
  engine().tableSet(true, handleIndex(), engine().luaFrom(key), engine().luaFrom(value));
}

template <typename Ret, typename... Args>
auto LuaFunction::invoke(Args const&... args) const -> Ret {
  return LuaDetail::FromFunctionReturn<Ret>::convert(engine(), engine().callFunction(handleIndex(), args...));
}

// LuaThread::resume is now defined inline with deducing this in the class definition

inline void LuaThread::pushFunction(LuaFunction const& func) const {
  engine().threadPushFunction(handleIndex(), func.handleIndex());
}

inline auto LuaThread::status() const -> LuaThread::Status {
  return engine().threadStatus(handleIndex());
}

template <typename T>
auto LuaUserData::is() const -> bool {
  return engine().userDataIsType<T>(handleIndex());
}

template <typename T>
auto LuaUserData::get() const -> T& {
  return *engine().getUserData<T>(handleIndex());
}

template <typename Function>
void LuaCallbacks::registerCallback(String name, Function&& func) {
  if (!m_callbacks.insert(name, LuaDetail::wrapFunction(std::forward<Function>(func))).second)
    throw LuaException::format(std::string_view("Lua callback '{}' was registered twice"), name);
}

template <typename Return, typename... Args, typename Function>
void LuaCallbacks::registerCallbackWithSignature(String name, Function&& func) {
  if (!m_callbacks.insert(name, LuaDetail::wrapFunctionWithSignature<Return, Args...>(std::forward<Function>(func))).second)
    throw LuaException::format(std::string_view("Lua callback '{}' was registered twice"), name);
}

template <typename T>
template <typename Function>
void LuaMethods<T>::registerMethod(String name, Function&& func) {
  if (!m_methods.insert(name, LuaDetail::wrapMethod(std::forward<Function>(std::move(func)))).second)
    throw LuaException::format(std::string_view("Lua method '{}' was registered twice"), name);
}

template <typename T>
template <typename Return, typename... Args, typename Function>
void LuaMethods<T>::registerMethodWithSignature(String name, Function&& func) {
  if (!m_methods.insert(name, LuaDetail::wrapMethodWithSignature<Return, Args...>(std::forward<Function>(std::move(func))))
         .second)
    throw LuaException::format(std::string_view("Lua method '{}' was registered twice"), name);
}

template <typename T>
auto LuaMethods<T>::methods() const -> StringMap<LuaDetail::LuaWrappedFunction> const& {
  return m_methods;
}

template <typename T>
[[nodiscard]] auto LuaContext::getPath(String path) const -> T {
  return engine().luaTo<T>(engine().contextGetPath(handleIndex(), std::move(path)));
}

template <typename T>
void LuaContext::setPath(String key, T value) {
  engine().contextSetPath(handleIndex(), std::move(key), engine().luaFrom<T>(std::move(value)));
}

template <typename Ret>
auto LuaContext::eval(String const& lua) -> Ret {
  return LuaDetail::FromFunctionReturn<Ret>::convert(engine(), engine().contextEval(handleIndex(), lua));
}

template <typename Ret, typename... Args>
auto LuaContext::invokePath(String const& key, Args const&... args) const -> Ret {
  auto p = getPath(key);
  if (auto f = p.ptr<LuaFunction>())
    return f->invoke<Ret>(args...);
  throw LuaException::format(std::string_view("invokePath called on path '{}' which is not function type"), key);
}

template <typename T>
auto LuaContext::luaFrom(T&& t) -> LuaValue {
  return engine().luaFrom(std::forward<T>(t));
}

template <typename T>
auto LuaContext::luaFrom(T const& t) -> LuaValue {
  return engine().luaFrom(t);
}

template <typename T>
auto LuaContext::luaMaybeTo(LuaValue&& v) -> std::optional<T> {
  return engine().luaFrom(std::move(v));
}

template <typename T>
auto LuaContext::luaMaybeTo(LuaValue const& v) -> std::optional<T> {
  return engine().luaFrom(v);
}

template <typename T>
auto LuaContext::luaTo(LuaValue&& v) -> T {
  return engine().luaTo<T>(std::move(v));
}

template <typename T>
auto LuaContext::luaTo(LuaValue const& v) -> T {
  return engine().luaTo<T>(v);
}

template <LuaContainer Container>
auto LuaContext::createTable(Container const& map) -> LuaTable {
  return engine().createTable(map);
}

template <LuaContainer Container>
auto LuaContext::createArrayTable(Container const& array) -> LuaTable {
  return engine().createArrayTable(array);
}

template <typename Function>
auto LuaContext::createFunction(Function&& func) -> LuaFunction {
  return engine().createFunction(std::forward<Function>(func));
}

template <typename Return, typename... Args, typename Function>
auto LuaContext::createFunctionWithSignature(Function&& func) -> LuaFunction {
  return engine().createFunctionWithSignature<Return, Args...>(std::forward<Function>(func));
}

template <typename T>
auto LuaContext::createUserData(T t) -> LuaUserData {
  return engine().createUserData(std::move(t));
}

template <typename T>
auto LuaUserDataMethods<T>::make() -> LuaMethods<T> {
  return LuaMethods<T>();
}

template <typename T>
auto LuaUserDataConverter<T>::from(LuaEngine& engine, T t) -> LuaValue {
  return engine.createUserData(std::move(t));
}

template <typename T>
auto LuaUserDataConverter<T>::to([[maybe_unused]] LuaEngine& engine, LuaValue const& v) -> std::optional<T> {
  if (auto ud = v.ptr<LuaUserData>()) {
    if (ud->is<T>())
      return ud->get<T>();
  }
  return {};
}

template <typename T>
auto LuaEngine::luaFrom(T&& t) -> LuaValue {
  return LuaConverter<std::decay_t<T>>::from(*this, std::forward<T>(t));
}

template <typename T>
auto LuaEngine::luaFrom(T const& t) -> LuaValue {
  return LuaConverter<T>::from(*this, t);
}

template <typename T>
auto LuaEngine::luaMaybeTo(LuaValue&& v) -> std::optional<T> {
  return LuaConverter<T>::to(*this, std::move(v));
}

template <typename T>
auto LuaEngine::luaMaybeTo(LuaValue const& v) -> std::optional<T> {
  return LuaConverter<T>::to(*this, v);
}

// Modern std::expected-based conversion (C++23)
template <typename T>
auto LuaEngine::luaConvertTo(LuaValue const& v) -> std::expected<T, LuaConversionError> {
  if constexpr (requires { LuaConverter<T>::tryTo(*this, v); }) {
    // Use modern tryTo if available
    return LuaConverter<T>::tryTo(*this, v);
  } else {
    // Fallback to legacy to() method
    if (auto result = LuaConverter<T>::to(*this, v)) {
      return std::expected<T, LuaConversionError>(std::move(*result));
    }
    return std::unexpected(LuaConversionError{
      strf("Failed to convert LuaValue to type '{}'", typeid(T).name())
    });
  }
}

template <typename T>
auto LuaEngine::luaConvertTo(LuaValue&& v) -> std::expected<T, LuaConversionError> {
  if constexpr (requires { LuaConverter<T>::tryTo(*this, std::move(v)); }) {
    return LuaConverter<T>::tryTo(*this, std::move(v));
  } else {
    if (auto result = LuaConverter<T>::to(*this, std::move(v))) {
      return std::expected<T, LuaConversionError>(std::move(*result));
    }
    return std::unexpected(LuaConversionError{
      strf("Failed to convert LuaValue to type '{}'", typeid(T).name())
    });
  }
}

template <typename T>
auto LuaEngine::luaTo(LuaValue&& v) -> T {
  auto res = luaConvertTo<T>(std::move(v));
  if (res)
    return std::move(*res);
  throw LuaConversionException::format("Error converting LuaValue: {}", res.error().message);
}

template <typename T>
auto LuaEngine::luaTo(LuaValue const& v) -> T {
  auto res = luaConvertTo<T>(v);
  if (res)
    return std::move(*res);
  throw LuaConversionException::format("Error converting LuaValue: {}", res.error().message);
}

template <LuaContainer Container>
auto LuaEngine::createTable(Container const& map) -> LuaTable {
  auto table = createTable(0, map.size());
  for (auto const& p : map)
    table.set(p.first, p.second);
  return table;
}

template <LuaContainer Container>
auto LuaEngine::createArrayTable(Container const& array) -> LuaTable {
  auto table = createTable(array.size(), 0);
  // Using std::views::enumerate from C++23 (included in C++26)
  // enumerate provides 0-based indices; +1 converts to Lua's 1-based indexing
  for (auto const& [i, elem] : std::views::enumerate(array)) {
    table.set(LuaInt(i + 1), elem);
  }
  return table;
}

template <typename Function>
auto LuaEngine::createFunction(Function&& func) -> LuaFunction {
  return createWrappedFunction(LuaDetail::wrapFunction(std::forward<Function>(func)));
}

template <typename Return, typename... Args, typename Function>
auto LuaEngine::createFunctionWithSignature(Function&& func) -> LuaFunction {
  return createWrappedFunction(LuaDetail::wrapFunctionWithSignature<Return, Args...>(std::forward<Function>(func)));
}

template <typename... Args>
auto LuaEngine::callFunction(int handleIndex, Args const&... args) -> LuaDetail::LuaFunctionReturn {
  lua_checkstack(m_state, 1);

  int stackSize = lua_gettop(m_state);
  pushHandle(m_state, handleIndex);

  size_t argSize = pushArguments(m_state, args...);

  incrementRecursionLevel();
  int res = pcallWithTraceback(m_state, argSize, LUA_MULTRET);
  decrementRecursionLevel();
  handleError(m_state, res);

  int returnValues = lua_gettop(m_state) - stackSize;
  if (returnValues == 0) {
    return {};
  } else if (returnValues == 1) {
    return popLuaValue(m_state);
  } else {
    LuaVariadic<LuaValue> ret(returnValues);
    for (int i = returnValues - 1; i >= 0; --i)
      ret[i] = popLuaValue(m_state);
    return ret;
  }
}

template <typename... Args>
auto LuaEngine::resumeThread(int handleIndex, Args const&... args) -> std::optional<LuaDetail::LuaFunctionReturn> {
  lua_checkstack(m_state, 1);

  pushHandle(m_state, handleIndex);
  lua_State* threadState = lua_tothread(m_state, -1);
  lua_pop(m_state, 1);

  if (lua_status(threadState) != LUA_YIELD && lua_gettop(threadState) == 0) {
    throw LuaException("cannot resume a dead or errored thread");
  }

  size_t argSize = pushArguments(threadState, args...);
  incrementRecursionLevel();
  int res = lua_resume(threadState, nullptr, argSize);
  decrementRecursionLevel();
  if (res != LUA_OK && res != LUA_YIELD) {
    propagateErrorWithTraceback(threadState, m_state);
    handleError(m_state, res);
  }

  int returnValues = lua_gettop(threadState);
  if (returnValues == 0) {
    return std::optional<LuaDetail::LuaFunctionReturn>(std::in_place);
  } else if (returnValues == 1) {
    return LuaDetail::LuaFunctionReturn(popLuaValue(threadState));
  } else {
    LuaVariadic<LuaValue> ret(returnValues);
    for (int i = returnValues - 1; i >= 0; --i)
      ret[i] = popLuaValue(threadState);
    return LuaDetail::LuaFunctionReturn(std::move(ret));
  }
}

template <typename T>
void LuaEngine::registerUserDataType() {
  if (m_registeredUserDataTypes.contains(typeid(T)))
    return;

  lua_checkstack(m_state, 2);

  lua_newtable(m_state);

  // Set the __index on the metatable to itself
  lua_pushvalue(m_state, -1);
  LuaDetail::rawSetField(m_state, -2, "__index");
  lua_pushboolean(m_state, 0);
  LuaDetail::rawSetField(m_state, -2, "__metatable");// protect metatable

  // Set the __gc function to the userdata destructor
  auto gcFunction = [](lua_State* state) -> auto {
    T& t = *(T*)(lua_touserdata(state, 1));
    t.~T();
    return 0;
  };
  lua_pushcfunction(m_state, gcFunction);
  LuaDetail::rawSetField(m_state, -2, "__gc");

  auto methods = LuaUserDataMethods<T>::make();
  for (auto& p : methods.methods()) {
    pushLuaValue(m_state, createWrappedFunction(p.second));
    LuaDetail::rawSetField(m_state, -2, p.first.utf8Ptr());
  }

  m_registeredUserDataTypes.add(typeid(T), luaL_ref(m_state, LUA_REGISTRYINDEX));
}

template <typename T>
auto LuaEngine::createUserData(T t) -> LuaUserData {
  registerUserDataType<T>();

  int typeMetatable = m_registeredUserDataTypes.get(typeid(T));

  lua_checkstack(m_state, 2);

  new (lua_newuserdata(m_state, sizeof(T))) T(std::move(t));

  lua_rawgeti(m_state, LUA_REGISTRYINDEX, typeMetatable);
  lua_setmetatable(m_state, -2);

  return {LuaDetail::LuaHandle(RefPtr<LuaEngine>(this), popHandle(m_state))};
}

template <typename T, typename K>
auto LuaEngine::getGlobal(K key) -> T {
  lua_checkstack(m_state, 1);
  lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_scriptDefaultEnvRegistryId);
  pushLuaValue(m_state, luaFrom(std::move(key)));
  lua_rawget(m_state, -2);

  LuaValue v = popLuaValue(m_state);
  lua_pop(m_state, 1);

  return luaTo<T>(v);
}

template <typename T>
auto LuaEngine::getGlobal(char const* key) -> T {
  lua_checkstack(m_state, 1);
  lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_scriptDefaultEnvRegistryId);
  lua_getfield(m_state, -1, key);

  LuaValue v = popLuaValue(m_state);
  lua_pop(m_state, 1);

  return luaTo<T>(v);
}

template <typename T, typename K>
void LuaEngine::setGlobal(K key, T value) {
  lua_checkstack(m_state, 1);

  lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_scriptDefaultEnvRegistryId);
  pushLuaValue(m_state, luaFrom(std::move(key)));
  pushLuaValue(m_state, luaFrom(std::move(value)));

  lua_rawset(m_state, -3);
  lua_pop(m_state, 1);
}

template <typename T>
void LuaEngine::setGlobal(char const* key, T value) {
  lua_checkstack(m_state, 1);

  lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_scriptDefaultEnvRegistryId);
  pushLuaValue(m_state, value);

  lua_setfield(m_state, -2, key);
  lua_pop(m_state, 1);
}

template <typename T>
auto LuaEngine::userDataIsType(int handleIndex) -> bool {
  int typeRef = m_registeredUserDataTypes.value(typeid(T), LUA_NOREF);
  if (typeRef == LUA_NOREF)
    return false;

  lua_checkstack(m_state, 3);

  pushHandle(m_state, handleIndex);
  if (lua_getmetatable(m_state, -1) == 0) {
    lua_pop(m_state, 1);
    throw LuaException("Userdata missing metatable in userDataIsType");
  }

  lua_rawgeti(m_state, LUA_REGISTRYINDEX, typeRef);
  bool typesEqual = lua_rawequal(m_state, -1, -2);
  lua_pop(m_state, 3);

  return typesEqual;
}

template <typename T>
auto LuaEngine::getUserData(int handleIndex) -> T* {
  int typeRef = m_registeredUserDataTypes.value(typeid(T), LUA_NOREF);
  if (typeRef == LUA_NOREF)
    throw LuaException::format(std::string_view("Cannot convert userdata type of {}, not registered"), typeid(T).name());

  lua_checkstack(m_state, 3);

  pushHandle(m_state, handleIndex);
  T* userdata = (T*)lua_touserdata(m_state, -1);
  if (lua_getmetatable(m_state, -1) == 0) {
    lua_pop(m_state, 1);
    throw LuaException("Cannot get userdata from lua type, no metatable found");
  }

  lua_rawgeti(m_state, LUA_REGISTRYINDEX, typeRef);
  if (!lua_rawequal(m_state, -1, -2)) {
    lua_pop(m_state, 3);
    throw LuaException::format(std::string_view("Improper conversion from userdata to type {}"), typeid(T).name());
  }

  lua_pop(m_state, 3);

  return userdata;
}

inline void LuaEngine::destroyHandle(int handleIndex) {
  // We don't bother setting the entry in the handle stack to nil, we just wait
  // for it to be reused and overwritten.  We could provide a way to
  // periodically ensure that the entire free list is niled out if this becomes
  // a memory issue?
  m_handleFree.append(handleIndex);
}

template <typename T>
auto LuaEngine::pushArgument(lua_State* state, T const& arg) -> std::size_t {
  pushLuaValue(state, luaFrom(arg));
  return 1;
}

template <typename T>
auto LuaEngine::pushArgument(lua_State* state, LuaVariadic<T> const& args) -> std::size_t {
  // If the argument list was empty then we've checked one extra space on the
  // stack, oh well.
  if (args.empty())
    return 0;

  // top-level pushArguments does a stack check of the total size of the
  // argument list, for variadic arguments, it could be more than one
  // argument so check the stack for the arguments in the variadic list minus
  // one.
  lua_checkstack(state, args.size() - 1);
  for (size_t i = 0; i < args.size(); ++i)
    pushLuaValue(state, luaFrom(args[i]));
  return args.size();
}

inline auto LuaEngine::doPushArguments(lua_State*) -> size_t {
  return 0;
}

template <typename First, typename... Rest>
auto LuaEngine::doPushArguments(lua_State* state, First const& first, Rest const&... rest) -> std::size_t {
  std::size_t s = pushArgument(state, first);
  return s + doPushArguments(state, rest...);
}

template <typename... Args>
auto LuaEngine::pushArguments(lua_State* state, Args const&... args) -> std::size_t {
  lua_checkstack(state, sizeof...(args));
  return doPushArguments(state, args...);
}

}// namespace Star

template <>
struct std::formatter<Star::LuaValue> : Star::ostream_formatter {};
