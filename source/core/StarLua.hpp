#pragma once

#include <typeindex>
#include <type_traits>
#include <lua.hpp>

#include "StarLexicalCast.hpp"
#include "StarString.hpp"
#include "StarJson.hpp"
#include "StarRefPtr.hpp"
#include "StarDirectives.hpp"

namespace Star {

class LuaEngine;
using LuaEnginePtr = RefPtr<LuaEngine>;

// Basic unspecified lua exception
struct LuaExceptionTag { static constexpr char const* typeName = "LuaException"; };
using LuaException = TypedException<StarException, LuaExceptionTag>;

// Thrown when trying to parse an incomplete statement, useful for implementing
// REPL loops, uses the incomplete statement marker '<eof>' as the standard lua
// repl does.
struct LuaIncompleteStatementExceptionTag { static constexpr char const* typeName = "LuaIncompleteStatementException"; };
using LuaIncompleteStatementException = TypedException<LuaException, LuaIncompleteStatementExceptionTag>;

// Thrown when the instruction limit is reached, if the instruction limit is
// set.
struct LuaInstructionLimitReachedTag { static constexpr char const* typeName = "LuaInstructionLimitReached"; };
using LuaInstructionLimitReached = TypedException<LuaException, LuaInstructionLimitReachedTag>;

// Thrown when the engine recursion limit is reached, if the recursion limit is
// set.
struct LuaRecursionLimitReachedTag { static constexpr char const* typeName = "LuaRecursionLimitReached"; };
using LuaRecursionLimitReached = TypedException<LuaException, LuaRecursionLimitReachedTag>;

// Thrown when an incorrect lua type is passed to something in C++ expecting a
// different type.
struct LuaConversionExceptionTag { static constexpr char const* typeName = "LuaConversionException"; };
using LuaConversionException = TypedException<LuaException, LuaConversionExceptionTag>;

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
LuaVariadic<typename std::decay_t<Container>::value_type> luaUnpack(Container&& c);

// Similar to LuaVariadic, but a tuple type so automatic per-entry type
// conversion is done.  This can only be used as the return value of a wrapped
// c++ function, or as a type for the return value of calling a lua function.
template <typename... Types>
class LuaTupleReturn : public tuple<Types...> {
public:
  using Base = tuple<Types...>;

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

  LuaTupleReturn& operator=(LuaTupleReturn const& rhs);
  LuaTupleReturn& operator=(LuaTupleReturn&& rhs);
  template <typename... UTypes>
  LuaTupleReturn& operator=(LuaTupleReturn<UTypes...> const& rhs);
  template <typename... UTypes>
  LuaTupleReturn& operator=(LuaTupleReturn<UTypes...>&& rhs);
};

// std::tie for LuaTupleReturn
template <typename... Types>
LuaTupleReturn<Types&...> luaTie(Types&... args);

// Constructs a LuaTupleReturn from the given arguments similar to make_tuple
template <typename... Types>
LuaTupleReturn<std::decay_t<Types>...> luaTupleReturn(Types&&... args);

namespace LuaDetail {
  struct LuaHandle {
    LuaHandle(LuaEnginePtr engine, int handleIndex);
    ~LuaHandle();

    LuaHandle(LuaHandle const& other);
    LuaHandle(LuaHandle&& other);

    LuaHandle& operator=(LuaHandle const& other);
    LuaHandle& operator=(LuaHandle&& other);

    LuaEnginePtr engine;
    int handleIndex = 0;
  };

  // Not meant to be used directly, exposes a raw interface for wrapped C++
  // functions to be wrapped with the least amount of overhead.  Arguments are
  // passed non-const so that they can be moved into wrapped functions that
  // take values without copying.
  using LuaFunctionReturn = Variant<LuaValue, LuaVariadic<LuaValue>>;
  using LuaWrappedFunction = function<LuaFunctionReturn(LuaEngine&, size_t argc, LuaValue* argv)>;
}

// Prints the lua value similar to lua's print function, except it makes an
// attempt at printing tables.
std::ostream& operator<<(std::ostream& os, LuaValue const& value);

// Holds a reference to a LuaEngine and a value held internally inside the
// registry of that engine.  The lifetime of the LuaEngine will be extended
// until all LuaReferences referencing it are destroyed.
class LuaReference {
public:
  LuaReference(LuaDetail::LuaHandle handle);

  LuaReference(LuaReference&&) = default;
  LuaReference& operator=(LuaReference&&) = default;

  LuaReference(LuaReference const&) = default;
  LuaReference& operator=(LuaReference const&) = default;

  bool operator==(LuaReference const& rhs) const;
  bool operator!=(LuaReference const& rhs) const;

  LuaEngine& engine() const;
  int handleIndex() const;

private:
  LuaDetail::LuaHandle m_handle;
};

class LuaString : public LuaReference {
public:
  using LuaReference::LuaReference;

  char const* ptr() const;
  size_t length() const;

  String toString() const;
  StringView view() const;
};

bool operator==(LuaString const& s1, LuaString const& s2);
bool operator==(LuaString const& s1, char const* s2);
bool operator==(LuaString const& s1, std::string const& s2);
bool operator==(LuaString const& s1, String const& s2);
bool operator==(char const* s1, LuaString const& s2);
bool operator==(std::string const& s1, LuaString const& s2);
bool operator==(String const& s1, LuaString const& s2);

bool operator!=(LuaString const& s1, LuaString const& s2);
bool operator!=(LuaString const& s1, char const* s2);
bool operator!=(LuaString const& s1, std::string const& s2);
bool operator!=(LuaString const& s1, String const& s2);
bool operator!=(char const* s1, LuaString const& s2);
bool operator!=(std::string const& s1, LuaString const& s2);
bool operator!=(String const& s1, LuaString const& s2);

class LuaTable : public LuaReference {
public:
  using LuaReference::LuaReference;

  template <typename T = LuaValue, typename K>
  T get(K key) const;
  template <typename T = LuaValue>
  T get(char const* key) const;

  template <typename T, typename K>
  void set(K key, T t) const;
  template <typename T>
  void set(char const* key, T t) const;

  // Shorthand for get(path) != LuaNil
  template <typename K>
  bool contains(K key) const;
  bool contains(char const* key) const;

  // Shorthand for setting to LuaNil
  template <typename K>
  void remove(K key) const;
  void remove(char const* key) const;

  // Result of lua # operator
  LuaInt length() const;

  // If iteration function returns bool, returning false signals stopping.
  template <typename Function>
  void iterate(Function&& iterator) const;

  template <typename Return, typename... Args, typename Function>
  void iterateWithSignature(Function&& func) const;

  Maybe<LuaTable> getMetatable() const;
  void setMetatable(LuaTable const& table) const;

  template <typename T = LuaValue, typename K>
  T rawGet(K key) const;
  template <typename T = LuaValue>
  T rawGet(char const* key) const;

  template <typename T, typename K>
  void rawSet(K key, T t) const;
  template <typename T>
  void rawSet(char const* key, T t) const;

  LuaInt rawLength() const;
};

class LuaFunction : public LuaReference {
public:
  using LuaReference::LuaReference;

  template <typename Ret = LuaValue, typename... Args>
  Ret invoke(Args const&... args) const;
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
  template <typename Ret = LuaValue, typename... Args>
  Maybe<Ret> resume(Args const&... args) const;
  void pushFunction(LuaFunction const& func) const;
  Status status() const;
};

// Keeping LuaReferences in LuaUserData will lead to circular references to
// LuaEngine, in addition to circular references in Lua which the Lua
// garbage collector can't collect. Don't put LuaReferences in LuaUserData.
class LuaUserData : public LuaReference {
public:
  using LuaReference::LuaReference;

  template <typename T>
  bool is() const;

  template <typename T>
  T& get() const;
};

LuaValue const LuaNil = LuaValue();

class LuaCallbacks {
public:
  void copyCallback(String srcName, String dstName);

  template <typename Function>
  void registerCallback(String name, Function&& func);

  bool removeCallback(String name);

  template <typename Return, typename... Args, typename Function>
  void registerCallbackWithSignature(String name, Function&& func);

  LuaCallbacks& merge(LuaCallbacks const& callbacks);

  StringMap<LuaDetail::LuaWrappedFunction> const& callbacks() const;

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

  StringMap<LuaDetail::LuaWrappedFunction> const& methods() const;

private:
  StringMap<LuaDetail::LuaWrappedFunction> m_methods;
};

// A single execution context from a LuaEngine that manages a (mostly) distinct
// lua environment.  Each LuaContext's global environment is separate and one
// LuaContext can (mostly) not affect any other.
class LuaContext : protected LuaTable {
public:
  using RequireFunction = function<void(LuaContext&, LuaString const&)>;

  using LuaTable::LuaTable;

  using LuaTable::get;
  using LuaTable::set;
  using LuaTable::contains;
  using LuaTable::remove;
  using LuaTable::engine;
  using LuaTable::handleIndex;

  // Splits the path by '.' character, so can get / set values in tables inside
  // other tables.  If any table in the path is not a table but is accessed as
  // one, instead returns LuaNil.
  template <typename T = LuaValue>
  T getPath(String path) const;
  // Shorthand for getPath != LuaNil
  bool containsPath(String path) const;
  // Will create new tables if the key contains paths that are nil
  template <typename T>
  void setPath(String path, T value);

  // Load the given code (either source or bytecode) into this context as a new
  // chunk.  It is not necessary to provide the name again if given bytecode.
  void load(char const* contents, size_t size, char const* name = nullptr);
  void load(String const& contents, String const& name = String());
  void load(ByteArray const& contents, String const& name = String());

  // Evaluate a piece of lua code in this context, similar to the lua repl.
  // Can evaluate both expressions and statements.
  template <typename T = LuaValue>
  T eval(String const& lua);

  // Override the built-in require function with the given function that takes
  // this LuaContext and the module name to load.
  void setRequireFunction(RequireFunction requireFunction);

  void setCallbacks(String const& tableName, LuaCallbacks const& callbacks) const;

  // For convenience, invokePath methods are equivalent to calling getPath(key)
  // to get a function, and then invoking it.

  template <typename Ret = LuaValue, typename... Args>
  Ret invokePath(String const& key, Args const&... args) const;

  // For convenience, calls to LuaEngine conversion / create functions are
  // duplicated here.

  template <typename T>
  LuaValue luaFrom(T&& t);
  template <typename T>
  LuaValue luaFrom(T const& t);
  template <typename T>
  Maybe<T> luaMaybeTo(LuaValue&& v);
  template <typename T>
  Maybe<T> luaMaybeTo(LuaValue const& v);
  template <typename T>
  T luaTo(LuaValue const& v);
  template <typename T>
  T luaTo(LuaValue&& v);

  LuaString createString(String const& str);
  LuaString createString(char const* str);

  LuaTable createTable();

  template <typename Container>
  LuaTable createTable(Container const& map);

  template <typename Container>
  LuaTable createArrayTable(Container const& array);

  template <typename Function>
  LuaFunction createFunction(Function&& func);

  template <typename Return, typename... Args, typename Function>
  LuaFunction createFunctionWithSignature(Function&& func);

  template <typename T>
  LuaUserData createUserData(T t);
};

template <typename T>
struct LuaNullTermWrapper : T {
  LuaNullTermWrapper() : T() {}
  LuaNullTermWrapper(LuaNullTermWrapper const& nt) : T(nt) {}
  LuaNullTermWrapper(LuaNullTermWrapper&& nt) : T(std::move(nt)) {}
  LuaNullTermWrapper(T const& bt) : T(bt) {}
  LuaNullTermWrapper(T&& bt) : T(std::move(bt)) {}

  using T::T;

  LuaNullTermWrapper& operator=(LuaNullTermWrapper const& rhs) {
    T::operator=(rhs);
    return *this;
  }

  LuaNullTermWrapper& operator=(LuaNullTermWrapper&& rhs) {
    T::operator=(std::move(rhs));
    return *this;
  }

  LuaNullTermWrapper& operator=(T&& other) {
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
//   Maybe<T> to(LuaEngine& engine, LuaValue v);
// The methods can also take 'T const&' or 'LuaValue const&' as parameters, and
// the 'to' method can also return a bare T if conversion cannot fail.
template <typename T>
struct LuaConverter;

// UserData types that want to expose methods to lua should specialize this
// template.
template <typename T>
struct LuaUserDataMethods {
  static LuaMethods<T> make();
};

// Convenience converter that simply converts to/from LuaUserData, can be
// derived from by a declared converter.
template <typename T>
struct LuaUserDataConverter {
  static LuaValue from(LuaEngine& engine, T t);
  static Maybe<T> to(LuaEngine& engine, LuaValue const& v);
};

struct LuaProfileEntry {
  // Source name of the chunk the function was defined in
  String source;
  // Line number in the chunk of the beginning of the function definition
  unsigned sourceLine;
  // Name of the function, if it can be determined
  Maybe<String> name;
  // Scope of the function, if it can be determined
  Maybe<String> nameScope;
  // Time taken within this function itself
  int64_t selfTime;
  // Total time taken within this function or sub functions
  int64_t totalTime;
  // Calls from this function
  HashMap<tuple<String, unsigned>, shared_ptr<LuaProfileEntry>> calls;
};

// This class represents one execution engine in lua, holding a single
// lua_State.  Multiple contexts can be created, and they will have separate
// global environments and cannot affect each other.  Individual LuaEngines /
// LuaContexts are not thread safe, use one LuaEngine per thread.
class LuaEngine : public RefCounter {
public:
  // If 'safe' is true, then creates a lua engine with all builtin lua
  // functions that can affect the real world disabled.
  static LuaEnginePtr create(bool safe = true);

  ~LuaEngine();

  LuaEngine(LuaEngine const&) = delete;
  LuaEngine(LuaEngine&&) = default;

  LuaEngine& operator=(LuaEngine const&) = delete;
  LuaEngine& operator=(LuaEngine&&) = default;

  // Set the instruction limit for computation sequences in the engine.  During
  // any function invocation, thread resume, or code evaluation, an instruction
  // counter will be started.  In the event that the instruction counter
  // becomes greater than the given limit, a LuaException will be thrown.  The
  // count is only reset when the initial entry into LuaEngine is returned,
  // recursive entries into LuaEngine accumulate the same instruction counter.
  // 0 disables the instruction limit.
  void setInstructionLimit(uint64_t instructionLimit = 0);
  uint64_t instructionLimit() const;

  // If profiling is enabled, then every 'measureInterval' instructions, the
  // function call stack will be recorded, and a summary of function timing can
  // be printed using profileReport
  void setProfilingEnabled(bool profilingEnabled);
  bool profilingEnabled() const;

  // Print a summary of the profiling data gathered since profiling was last
  // enabled.
  List<LuaProfileEntry> getProfile();

  // If an instruction limit is set or profiling is neabled, this field
  // describes the resolution of instruction count measurement, and affects the
  // accuracy of profiling and the instruction count limit.  Defaults to 1000
  void setInstructionMeasureInterval(unsigned measureInterval = 1000);
  unsigned instructionMeasureInterval() const;

  // Sets the LuaEngine recursion limit, limiting the number of times a
  // LuaEngine call may directly or inderectly trigger a call back into the
  // LuaEngine, preventing a C++ stack overflow.  0 disables the limit.
  void setRecursionLimit(unsigned recursionLimit = 0);
  unsigned recursionLimit() const;

  // Compile a given script into bytecode.  If name is given, then it will be
  // used as the internal name for the resulting chunk and will provide better
  // error messages.
  //
  // Unfortunately the only way to completely ensure that a single script will
  // execute in two separate contexts and truly be isolated is to compile the
  // script to bytecode and load once in each context as a separate chunk.
  ByteArray compile(char const* contents, size_t size, char const* name = nullptr);
  ByteArray compile(String const& contents, String const& name = String());
  ByteArray compile(ByteArray const& contents, String const& name = String());

  // Returns the debug info of the state.
  lua_Debug const& debugInfo(int level = 1, const char* what = "nSlu");

  // Generic from/to lua conversion, calls template specialization of
  // LuaConverter for actual conversion.
  template <typename T>
  LuaValue luaFrom(T&& t);
  template <typename T>
  LuaValue luaFrom(T const& t);
  template <typename T>
  Maybe<T> luaMaybeTo(LuaValue&& v);
  template <typename T>
  Maybe<T> luaMaybeTo(LuaValue const& v);

  // Wraps luaMaybeTo, throws an exception if conversion fails.
  template <typename T>
  T luaTo(LuaValue const& v);
  template <typename T>
  T luaTo(LuaValue&& v);

  LuaString createString(std::string const& str);
  LuaString createString(String const& str);
  LuaString createString(char const* str);

  LuaTable createTable(int narr = 0, int nrec = 0);

  template <typename Container>
  LuaTable createTable(Container const& map);

  template <typename Container>
  LuaTable createArrayTable(Container const& array);

  // Creates a function and deduces the signature of the function using
  // FunctionTraits.  As a convenience, the given function may optionally take
  // a LuaEngine& parameter as the first parameter, and if it does, when called
  // the function will get a reference to the calling LuaEngine.
  template <typename Function>
  LuaFunction createFunction(Function&& func);

  // If the function signature is not deducible using FunctionTraits, you can
  // specify the return and argument types manually using this createFunction
  // version.
  template <typename Return, typename... Args, typename Function>
  LuaFunction createFunctionWithSignature(Function&& func);

  LuaFunction createWrappedFunction(LuaDetail::LuaWrappedFunction function);

  LuaFunction createRawFunction(lua_CFunction func);

  LuaFunction createFunctionFromSource(int handleIndex, char const* contents, size_t size, char const* name);

  LuaThread createThread();

  template <typename T>
  LuaUserData createUserData(T t);

  LuaContext createContext();

  // Global environment changes only affect newly created contexts

  template <typename T = LuaValue, typename K>
  T getGlobal(K key);
  template <typename T = LuaValue>
  T getGlobal(char const* key);

  template <typename T, typename K>
  void setGlobal(K key, T value);

  template <typename T>
  void setGlobal(char const* key, T value);

  // Perform either a full or incremental garbage collection.
  void collectGarbage(Maybe<unsigned> steps = {});

  // Stop / start automatic garbage collection
  void setAutoGarbageCollection(bool autoGarbageColleciton);

  // Tune the pause and step values of the lua garbage collector
  void tuneAutoGarbageCollection(float pause, float stepMultiplier);

  // Bytes in use by lua
  size_t memoryUsage() const;

  // Enforce null-terminated string conversion as long as the returned enforcer object is in scope.
  LuaNullEnforcer nullTerminate();
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
  static LuaEngine* luaEnginePtr(lua_State* state);
  // Counts instructions when instruction limiting is enabled.
  static void countHook(lua_State* state, lua_Debug* ar);

  static void* allocate(void* userdata, void* ptr, size_t oldSize, size_t newSize);

  // Pops lua error from stack and throws LuaException
  void handleError(lua_State* state, int res);

  // lua_pcall with a better message handler that includes a traceback.
  int pcallWithTraceback(lua_State* state, int nargs, int nresults);

  // override for lua coroutine resume with traceback
  static int coresumeWithTraceback(lua_State* state);
  // propagates errors from one state to another, i.e. past thread boundaries
  // pops error off the top of the from stack and pushes onto the to stack
  static void propagateErrorWithTraceback(lua_State* from, lua_State* to);

  char const* stringPtr(int handleIndex);
  size_t stringLength(int handleIndex);
  String string(int handleIndex);
  StringView stringView(int handleIndex);

  LuaValue tableGet(bool raw, int handleIndex, LuaValue const& key);
  LuaValue tableGet(bool raw, int handleIndex, char const* key);

  void tableSet(bool raw, int handleIndex, LuaValue const& key, LuaValue const& value);
  void tableSet(bool raw, int handleIndex, char const* key, LuaValue const& value);

  LuaInt tableLength(bool raw, int handleIndex);

  void tableIterate(int handleIndex, function<bool(LuaValue, LuaValue)> iterator);

  Maybe<LuaTable> tableGetMetatable(int handleIndex);
  void tableSetMetatable(int handleIndex, LuaTable const& table);

  template <typename... Args>
  LuaDetail::LuaFunctionReturn callFunction(int handleIndex, Args const&... args);

  template <typename... Args>
  Maybe<LuaDetail::LuaFunctionReturn> resumeThread(int handleIndex, Args const&... args);
  void threadPushFunction(int threadIndex, int functionIndex);
  LuaThread::Status threadStatus(int handleIndex);

  template <typename T>
  void registerUserDataType();

  template <typename T>
  bool userDataIsType(int handleIndex);

  template <typename T>
  T* getUserData(int handleIndex);

  void setContextRequire(int handleIndex, LuaContext::RequireFunction requireFunction);

  void contextLoad(int handleIndex, char const* contents, size_t size, char const* name);

  LuaDetail::LuaFunctionReturn contextEval(int handleIndex, String const& lua);

  LuaValue contextGetPath(int handleIndex, String path);
  void contextSetPath(int handleIndex, String path, LuaValue const& value);

  int popHandle(lua_State* state);
  void pushHandle(lua_State* state, int handleIndex);
  int copyHandle(int handleIndex);
  void destroyHandle(int handleIndex);

  int placeHandle();

  void pushLuaValue(lua_State* state, LuaValue const& luaValue);
  LuaValue popLuaValue(lua_State* state);

  template <typename T>
  size_t pushArgument(lua_State* state, T const& arg);

  template <typename T>
  size_t pushArgument(lua_State* state, LuaVariadic<T> const& args);

  size_t doPushArguments(lua_State*);
  template <typename First, typename... Rest>
  size_t doPushArguments(lua_State* state, First const& first, Rest const&... rest);

  template <typename... Args>
  size_t pushArguments(lua_State* state, Args const&... args);

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

  uint64_t m_instructionLimit;
  bool m_profilingEnabled;
  unsigned m_instructionMeasureInterval;
  uint64_t m_instructionCount;
  unsigned m_recursionLevel;
  unsigned m_recursionLimit;
  int m_nullTerminated;
  HashMap<tuple<String, unsigned>, shared_ptr<LuaProfileEntry>> m_profileEntries;
  lua_Debug m_debugInfo;
};

#include "StarLua_inl.hpp"

