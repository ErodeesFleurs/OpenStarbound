import std;

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "gtest/gtest.h"

TEST(LuaSyntaxTest, CompoundAssignmentOperators) {
  std::unique_ptr<lua_State, decltype(&lua_close)> state(luaL_newstate(), &lua_close);
  ASSERT_NE(state.get(), nullptr);
  luaL_openlibs(state.get());
  EXPECT_EQ(luaL_dostring(state.get(), R"LUA(
    local x = 10
    x += 2 * 3; assert(x == 16)
    x -= 3; assert(x == 13)
    x *= 2; assert(x == 26)
    x %= 7; assert(x == 5)
    x /= 2; assert(x == 2.5)
    x = 17; x //= 3; assert(x == 5)
    x = "a"; x ..= "b" .. "c"; assert(x == "abc")
    x = 3; x <<= 2; assert(x == 12)
    x >>= 1; assert(x == 6)
    x &= 3; assert(x == 2)
    x |= 8; assert(x == 10)

    local function update()
      x += 1
    end
    update(); assert(x == 11)
    extensionValue = 2
    extensionValue *= 4
    assert(extensionValue == 8)
  )LUA"), LUA_OK) << lua_tostring(state.get(), -1);
}

TEST(LuaSyntaxTest, CompoundAssignmentEvaluatesLvalueOnce) {
  std::unique_ptr<lua_State, decltype(&lua_close)> state(luaL_newstate(), &lua_close);
  ASSERT_NE(state.get(), nullptr);
  luaL_openlibs(state.get());
  EXPECT_EQ(luaL_dostring(state.get(), R"LUA(
    local values = {20, field = 7}
    local tableCalls, keyCalls, rhsCalls = 0, 0, 0
    local function receiver()
      tableCalls = tableCalls + 1
      return values
    end
    local function key()
      keyCalls = keyCalls + 1
      return 1
    end
    local function rhs()
      rhsCalls = rhsCalls + 1
      return 3
    end
    receiver()[key()] += rhs()
    assert(values[1] == 23)
    assert(tableCalls == 1 and keyCalls == 1 and rhsCalls == 1)
    receiver().field *= rhs()
    assert(values.field == 21)
    assert(tableCalls == 2 and keyCalls == 1 and rhsCalls == 2)
    values[1] //= 2
    values.field -= 1
    assert(values[1] == 11 and values.field == 20)

    local gets, sets, adds = 0, 0, 0
    local number = setmetatable({}, {
      __add = function(left, right)
        adds = adds + 1
        assert(left ~= nil and right == 4)
        return 99
      end
    })
    local proxy = setmetatable({}, {
      __index = function(_, name)
        gets = gets + 1
        assert(name == "value")
        return number
      end,
      __newindex = function(_, name, value)
        sets = sets + 1
        assert(name == "value" and value == 99)
      end
    })
    proxy.value += 4
    assert(gets == 1 and sets == 1 and adds == 1)
  )LUA"), LUA_OK) << lua_tostring(state.get(), -1);
}

TEST(LuaSyntaxTest, ForControlVariablesAreReadOnly) {
  std::unique_ptr<lua_State, decltype(&lua_close)> state(luaL_newstate(), &lua_close);
  ASSERT_NE(state.get(), nullptr);
  EXPECT_EQ(luaL_loadstring(state.get(), "for index = 1, 3 do index = 0 end"), LUA_ERRSYNTAX);
  lua_settop(state.get(), 0);
  EXPECT_EQ(luaL_loadstring(state.get(), "for key in pairs({}) do key += 1 end"), LUA_ERRSYNTAX);
  lua_settop(state.get(), 0);
  EXPECT_EQ(luaL_loadstring(state.get(), "for key, value in pairs({}) do key = 0 end"), LUA_ERRSYNTAX);
  lua_settop(state.get(), 0);
  luaL_openlibs(state.get());
  EXPECT_EQ(luaL_dostring(state.get(), R"LUA(
    local sum = 0
    for index = 1, 3 do
      local mutable = index
      mutable += 1
      sum += mutable
    end
    assert(sum == 9)
    for key, value in ipairs({10, 20}) do
      value += 1
      assert(value == key * 10 + 1)
    end
  )LUA"), LUA_OK) << lua_tostring(state.get(), -1);
}

TEST(LuaSyntaxTest, CompoundAssignmentHonorsConstDeclarations) {
  std::unique_ptr<lua_State, decltype(&lua_close)> state(luaL_newstate(), &lua_close);
  ASSERT_NE(state.get(), nullptr);
  EXPECT_EQ(luaL_loadstring(state.get(), "local value <const> = 3; value += 1"), LUA_ERRSYNTAX);
  lua_settop(state.get(), 0);
  EXPECT_EQ(luaL_loadstring(state.get(), "global value <const> = 3; value += 1"), LUA_ERRSYNTAX);
  lua_settop(state.get(), 0);
  EXPECT_EQ(luaL_loadstring(state.get(), "local value <const> = {}; value = {}"), LUA_ERRSYNTAX);
  lua_settop(state.get(), 0);
  EXPECT_EQ(luaL_loadstring(state.get(), "global value <const> = {}; value = {}"), LUA_ERRSYNTAX);
  lua_settop(state.get(), 0);
  EXPECT_EQ(luaL_loadstring(state.get(), "local value <const> = 3; for i = 1, 2 do value = i end"), LUA_ERRSYNTAX);
  lua_settop(state.get(), 0);
  EXPECT_EQ(luaL_loadstring(state.get(), "global value <const> = 3; for i in pairs({}) do value += i end"), LUA_ERRSYNTAX);
  lua_settop(state.get(), 0);
  EXPECT_EQ(luaL_loadstring(state.get(), "local value = {}; value?.field += 1"), LUA_ERRSYNTAX);
}

TEST(LuaSyntaxTest, SafeNavigationShortCircuitsAndPreservesValues) {
  std::unique_ptr<lua_State, decltype(&lua_close)> state(luaL_newstate(), &lua_close);
  ASSERT_NE(state.get(), nullptr);
  luaL_openlibs(state.get());
  EXPECT_EQ(luaL_dostring(state.get(), R"LUA(
    local absent, disabled = nil, false
    local receiverCalls, keyCalls, indexCalls = 0, 0, 0
    local function key()
      keyCalls = keyCalls + 1
      return "child"
    end
    local function receiver(value)
      receiverCalls = receiverCalls + 1
      return value
    end
    assert(absent?.child == nil)
    assert(disabled?.child == false)
    assert(absent?[key()] == nil)
    assert(disabled?[key()] == false)
    assert(keyCalls == 0)
    assert(receiver(nil)?[key()] == nil)
    assert(receiverCalls == 1 and keyCalls == 0)

    local value = {child = {answer = 42}}
    assert(receiver(value)?[key()]?.answer == 42)
    assert(receiverCalls == 2 and keyCalls == 1)
    assert(value?.missing?.answer == nil)
    assert((absent?.child or value)?.child?.answer == 42)
    assert(value?.child?.answer + 1 == 43)
    local proxy = setmetatable({}, {
      __index = function(_, name)
        indexCalls = indexCalls + 1
        assert(name == "child")
        return value.child
      end
    })
    assert(proxy?.child?.answer == 42)
    assert(indexCalls == 1)
    local a, b, c = 7, absent?[key()], value?.child?.answer
    assert(a == 7 and b == nil and c == 42 and keyCalls == 1)
    local function checkArguments(a, b, c)
      assert(a == 1 and b == nil and c == 42)
    end
    checkArguments(1, absent?.child, value?.child?.answer)
  )LUA"), LUA_OK) << lua_tostring(state.get(), -1);
}

TEST(LuaSyntaxTest, MathExtensionsPreserveNumericSemantics) {
  std::unique_ptr<lua_State, decltype(&lua_close)> state(luaL_newstate(), &lua_close);
  ASSERT_NE(state.get(), nullptr);
  luaL_openlibs(state.get());
  EXPECT_EQ(luaL_dostring(state.get(), R"LUA(
    assert(math.round(1.5) == 2 and math.round(-1.5) == -2)
    assert(math.type(math.round(1.5)) == "integer")
    assert(math.round(math.maxinteger) == math.maxinteger)
    assert(math.round(math.mininteger) == math.mininteger)
    assert(math.round(math.huge) == math.huge)
    assert(math.copysign(-3) == 3)
    assert(math.copysign(3, -1) == -3)
    assert(1 / math.copysign(0, -1) == -math.huge)
  )LUA"), LUA_OK) << lua_tostring(state.get(), -1);
}
