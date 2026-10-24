import std;
import star.format;
import star.string;
import star.list;
import star.json;
import star.lua;
import star.lua_converters;
import star.lexical_cast;
import star.rpc_promise;

#include "gtest/gtest.h"

import star.vector;
import star.rect;
import star.poly;
import star.math_common;
import star.algorithm;
using namespace Star;

namespace {
template <typename T, typename U>
void expectLuaAlternative(std::expected<LuaValue, StarException> result, U const& expected) {
  ASSERT_TRUE(result) << result.error().what();
  auto value = std::get_if<T>(&*result);
  ASSERT_NE(value, nullptr);
  EXPECT_EQ(*value, expected);
}

void expectLuaNil(std::expected<LuaValue, StarException> result) {
  ASSERT_TRUE(result) << result.error().what();
  if (!std::holds_alternative<LuaNilType>(*result)) {
    auto rendered = printLuaValue(*result);
    ASSERT_TRUE(rendered) << rendered.error().what();
    ADD_FAILURE() << "Expected Lua nil; actual result: " << *rendered;
  }
}
}

TEST(LuaTest, BasicGetSet) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;
  auto luaContextResult = luaEngine->createContext();
  ASSERT_TRUE(luaContextResult);
  auto luaContext = std::move(*luaContextResult);
  ASSERT_TRUE((luaContext.load(R"SCRIPT(
      data1 = 1.0
      data2 = 3.0 > 2.0
      data3 = "hello"
    )SCRIPT")));

  ASSERT_TRUE((luaContext.set("data4", 4.0)));

  EXPECT_EQ(luaContext.get<double>("data1"), 1.0);
  EXPECT_EQ(luaContext.get<bool>("data2"), true);
  EXPECT_EQ(luaContext.get<String>("data3"), "hello");
  EXPECT_EQ(luaContext.get<double>("data4"), 4.0);
}

TEST(LuaTest, TableReferences) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;
  auto luaContextResult = luaEngine->createContext();
  ASSERT_TRUE(luaContextResult);
  auto luaContext = std::move(*luaContextResult);
  ASSERT_TRUE((luaContext.load(R"SCRIPT(
      table = {foo=1, bar=2}
      tableRef = table
    )SCRIPT")));

  auto tableResult = luaContext.get<LuaTable>("table");
  ASSERT_TRUE(tableResult);
  auto table = std::move(*tableResult);
  auto tableRef1Result = luaContext.get<LuaTable>("tableRef");
  ASSERT_TRUE(tableRef1Result);
  auto tableRef1 = std::move(*tableRef1Result);
  auto tableRef2 = table;

  EXPECT_EQ(table.get<double>("foo"), 1.0);
  EXPECT_EQ(table.get<double>("bar"), 2.0);

  ASSERT_TRUE((table.set("baz", 3.0)));
  EXPECT_EQ(tableRef1.get<double>("baz"), 3.0);

  ASSERT_TRUE((tableRef1.set("baf", 4.0)));
  EXPECT_EQ(table.get<double>("baf"), 4.0);
  EXPECT_EQ(tableRef2.get<double>("baf"), 4.0);
}

TEST(LuaTest, FunctionCallTest) {
  std::weak_ptr<int> destructionObserver;

  {
    auto luaEngineResult = LuaEngine::create();
    ASSERT_TRUE(luaEngineResult);
    auto& luaEngine = *luaEngineResult;
    auto luaContextResult = luaEngine->createContext();
    ASSERT_TRUE(luaContextResult);
    auto luaContext = std::move(*luaContextResult);
    ASSERT_TRUE((luaContext.load(R"SCRIPT(
        function testFunc(arg1, arg2)
          return callback(3) + arg1 + arg2
        end

        function testEmpty()
          return emptyCallback()
        end
      )SCRIPT")));

    auto toDestruct = std::make_shared<int>();
    destructionObserver = toDestruct;
    auto callbackResult1 = luaEngine->createFunction([toDestruct](double n) { return n * 2; });
    ASSERT_TRUE(callbackResult1);
    ASSERT_TRUE((luaContext.set("callback", *callbackResult1)));

    auto callbackResult2 = luaEngine->createFunction([]() { return "heyooo"; });
    ASSERT_TRUE(callbackResult2);
    ASSERT_TRUE((luaContext.set("emptyCallback", *callbackResult2)));

    EXPECT_EQ(luaContext.invokePath<double>("testFunc", 5.0, 10.0), 21.0);
    EXPECT_EQ(luaContext.invokePath<String>("emptyCallback"), "heyooo");
  }

  EXPECT_TRUE(destructionObserver.expired());
}

TEST(LuaTest, CoroutineTest) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;
  auto luaContextResult = luaEngine->createContext();
  ASSERT_TRUE(luaContextResult);
  auto luaContext = std::move(*luaContextResult);

  ASSERT_TRUE((luaContext.load(R"SCRIPT(
      function accumulate(sum)
        return sum + callback(coroutine.yield(sum))
      end

      function run()
          local sum = 0
          for i=1,4 do
            sum = accumulate(sum)
          end
          return sum
      end

      co = coroutine.create(run)
    )SCRIPT")));

  auto callbackResult3 = luaEngine->createFunction([](double num) { return num * 2; });
  ASSERT_TRUE(callbackResult3);
  ASSERT_TRUE((luaContext.set("callback", *callbackResult3)));

  auto threadResult = luaEngine->createThread();
  ASSERT_TRUE(threadResult);
  auto thread = std::move(*threadResult);
  EXPECT_EQ(thread.status(), LuaThread::Status::Dead);
  auto funcResult = luaContext.get<LuaFunction>("run");
  ASSERT_TRUE(funcResult);
  auto func = std::move(*funcResult);
  ASSERT_TRUE((thread.pushFunction(func)));
  EXPECT_EQ(thread.status(), LuaThread::Status::Active);
  EXPECT_EQ(thread.resume<double>(), 0.0);
  EXPECT_EQ(thread.resume<double>(1.0), 2.0);
  EXPECT_EQ(thread.resume<double>(3.0), 8.0);
  EXPECT_EQ(thread.resume<double>(5.0), 18.0);
  EXPECT_EQ(thread.resume<double>(7.0), 32.0);
  // manually created threads are empty after execution is finished
  EXPECT_EQ(thread.status(), LuaThread::Status::Dead);

  ASSERT_TRUE((thread.pushFunction(func)));
  EXPECT_EQ(thread.resume<double>(), 0.0);
  EXPECT_EQ(thread.resume<double>(1.0), 2.0);
  {
    auto failure = thread.pushFunction(func);
    ASSERT_FALSE(failure);
    EXPECT_TRUE(failure.error().is<LuaException>());
  }; // pushing function to suspended or errored thread

  auto coroutineResult = luaContext.get<LuaThread>("co");
  ASSERT_TRUE(coroutineResult);
  auto coroutine = std::move(*coroutineResult);
  EXPECT_EQ(coroutine.status(), LuaThread::Status::Active);
  EXPECT_EQ(coroutine.resume<double>(), 0.0);
  EXPECT_EQ(coroutine.resume<double>(1.0), 2.0);
  EXPECT_EQ(coroutine.resume<double>(3.0), 8.0);
  EXPECT_EQ(coroutine.resume<double>(5.0), 18.0);
  EXPECT_EQ(coroutine.resume<double>(7.0), 32.0);
  EXPECT_EQ(coroutine.status(), LuaThread::Status::Dead);
  {
    auto failure = coroutine.resume();
    ASSERT_FALSE(failure);
    EXPECT_TRUE(failure.error().is<LuaException>());
  };
  EXPECT_EQ(coroutine.status(), LuaThread::Status::Dead);

  auto thread2Result = luaEngine->createThread();
  ASSERT_TRUE(thread2Result);
  auto thread2 = std::move(*thread2Result);
  EXPECT_EQ(thread2.status(), LuaThread::Status::Dead);
  ASSERT_TRUE((thread2.pushFunction(func)));
  EXPECT_EQ(thread2.status(), LuaThread::Status::Active);
  EXPECT_EQ(thread2.resume<double>(), 0.0);
  EXPECT_EQ(thread2.resume<double>(1.0), 2.0);
  {
    auto failure = thread2.resume<String>("not_a_number");
    ASSERT_FALSE(failure);
    EXPECT_TRUE(failure.error().is<LuaException>());
  };
  EXPECT_EQ(thread2.status(), LuaThread::Status::Error);
  {
    auto failure = thread2.resume();
    ASSERT_FALSE(failure);
    EXPECT_TRUE(failure.error().is<LuaException>());
  };
  EXPECT_EQ(thread2.status(), LuaThread::Status::Error);
}

template <typename T>
testing::AssertionResult roundTripEqual(LuaContext const& context, T t) {
  auto result = context.invokePath<T>("roundTrip", t);
  if (!result)
    return testing::AssertionFailure() << result.error().what();
  if (*result != t)
    return testing::AssertionFailure() << "Lua round trip returned a different value";
  return testing::AssertionSuccess();
}

TEST(LuaTest, Converters) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;
  auto luaContextResult = luaEngine->createContext();
  ASSERT_TRUE(luaContextResult);
  auto luaContext = std::move(*luaContextResult);

  ASSERT_TRUE((luaContext.load( R"SCRIPT(
      function makeVec()
        return {1, 2}
      end

      function makePoly()
        return {{1, 2}, {3, 4}, {5, 6}}
      end

      function roundTrip(ret)
        return ret
      end
    )SCRIPT")));

  auto vecCompare = Vec2F(1.0f, 2.0f);
  auto polyCompare = PolyF({Vec2F(1.0f, 2.0f), Vec2F(3.0f, 4.0f), Vec2F(5.0f, 6.0f)});

  EXPECT_TRUE(luaContext.invokePath<Vec2F>("makeVec") == vecCompare);
  EXPECT_TRUE(luaContext.invokePath<PolyF>("makePoly") == polyCompare);
  EXPECT_TRUE(luaContext.invokePath<Vec2F>("roundTrip", vecCompare) == vecCompare);
  EXPECT_TRUE(luaContext.invokePath<PolyF>("roundTrip", polyCompare) == polyCompare);

  EXPECT_TRUE(roundTripEqual(luaContext, PolyF()));
  EXPECT_TRUE(roundTripEqual(luaContext, List<int>{1, 2, 3, 4}));
  EXPECT_TRUE(roundTripEqual(luaContext, List<PolyF>{PolyF(), PolyF()}));
  EXPECT_TRUE(roundTripEqual(luaContext, std::optional<int>(1)));
  EXPECT_TRUE(roundTripEqual(luaContext, std::optional<int>()));

  auto listCompare = List<int>{1, 2, 3, 4};
  EXPECT_TRUE(luaContext.invokePath<List<int>>("roundTrip", JsonArray{1, 2, 3, 4}) == listCompare);
  auto mapCompare = StringMap<String>{{"one", "two"}, {"three", "four"}};
  EXPECT_TRUE(luaContext.invokePath<StringMap<String>>("roundTrip", JsonObject{{"one", "two"}, {"three", "four"}}) == mapCompare);
}

TEST(LuaTest, OptionalConversionDistinguishesNilFromFailure) {
  auto engineResult = LuaEngine::create();
  ASSERT_TRUE(engineResult);
  auto& engine = *engineResult;
  auto contextResult = engine->createContext();
  ASSERT_TRUE(contextResult);
  auto context = std::move(*contextResult);
  auto nilValue = context.eval<LuaValue>("nil");
  ASSERT_TRUE(nilValue);
  auto nil = engine->luaMaybeTo<std::optional<int>>(*nilValue);
  ASSERT_TRUE(nil);
  ASSERT_TRUE(*nil);
  EXPECT_FALSE(**nil);
  auto numberValue = context.eval<LuaValue>("17");
  ASSERT_TRUE(numberValue);
  auto number = engine->luaMaybeTo<std::optional<int>>(*numberValue);
  ASSERT_TRUE(number);
  ASSERT_TRUE(number);
  ASSERT_TRUE(*number);
  ASSERT_TRUE(**number);
  EXPECT_EQ(***number, 17);
  auto booleanValue = context.eval<LuaValue>("false");
  ASSERT_TRUE(booleanValue);
  auto boolean = engine->luaMaybeTo<std::optional<bool>>(*booleanValue);
  ASSERT_TRUE(boolean);
  ASSERT_TRUE(boolean);
  ASSERT_TRUE(*boolean);
  ASSERT_TRUE(**boolean);
  EXPECT_FALSE(***boolean);
  auto invalidValue = context.eval<LuaValue>("{}");
  ASSERT_TRUE(invalidValue);
  auto invalid = engine->luaMaybeTo<std::optional<int>>(*invalidValue);
  ASSERT_TRUE(invalid);
  EXPECT_FALSE(*invalid);
  {
    auto failure = context.eval<std::optional<int>>("{}");
    ASSERT_FALSE(failure);
    EXPECT_TRUE(failure.error().is<LuaConversionException>());
  };
}

struct TestUserData1 {
  int field;
};

struct TestUserData2 {
  int field;
};

namespace Star {
template <>
struct LuaConverter<TestUserData1> : LuaUserDataConverter<TestUserData1> {};

template <>
struct LuaConverter<TestUserData2> : LuaUserDataConverter<TestUserData2> {};
}

TEST(LuaTest, UserDataTest) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;

  auto luaContextResult = luaEngine->createContext();
  ASSERT_TRUE(luaContextResult);
  auto luaContext = std::move(*luaContextResult);
  ASSERT_TRUE((luaContext.load(
      R"SCRIPT(
        function doit(ref)
          heldValue = ref
        end
      )SCRIPT")));

  auto userdata1Result = luaEngine->createUserData(TestUserData1{1});
  ASSERT_TRUE(userdata1Result);
  auto userdata1 = std::move(*userdata1Result);
  auto userdata2Result = luaEngine->createUserData(TestUserData2{2});
  ASSERT_TRUE(userdata2Result);
  auto userdata2 = std::move(*userdata2Result);

  ASSERT_TRUE((luaContext.invokePath("doit", userdata1)));
  auto userdata3Result = luaContext.get<LuaUserData>("heldValue");
  ASSERT_TRUE(userdata3Result);
  auto userdata3 = std::move(*userdata3Result);

  EXPECT_EQ(userdata2.is<TestUserData2>(), true);
  EXPECT_EQ(userdata2.is<TestUserData1>(), false);

  EXPECT_EQ(userdata1.is<TestUserData1>(), true);
  EXPECT_EQ(userdata1.is<TestUserData2>(), false);

  EXPECT_EQ(userdata3.is<TestUserData1>(), true);
  EXPECT_EQ(userdata3.is<TestUserData2>(), false);

  auto data1 = userdata1.get<TestUserData1>();
  auto data2 = userdata2.get<TestUserData2>();
  auto data3 = userdata3.get<TestUserData1>();
  ASSERT_TRUE(data1);
  ASSERT_TRUE(data2);
  ASSERT_TRUE(data3);
  EXPECT_EQ((*data1)->field, 1);
  EXPECT_EQ((*data2)->field, 2);
  EXPECT_EQ((*data3)->field, 1);

  (*data1)->field = 3;
  EXPECT_EQ((*data1)->field, 3);
  EXPECT_EQ((*data3)->field, 3);

  ASSERT_TRUE((luaContext.invokePath("doit", TestUserData1())));
  auto userdata4Result = luaContext.get("heldValue");
  ASSERT_TRUE(userdata4Result);
  auto userdata4 = std::move(*userdata4Result);
  auto convertedUserdata = luaEngine->luaMaybeTo<TestUserData1>(userdata4);
  ASSERT_TRUE(convertedUserdata);
  EXPECT_TRUE(*convertedUserdata);

  ASSERT_TRUE((luaContext.invokePath("doit", "notuserdata")));
  auto notuserdataResult = luaContext.get("heldValue");
  ASSERT_TRUE(notuserdataResult);
  auto notuserdata = std::move(*notuserdataResult);
  auto rejectedUserdata = luaEngine->luaMaybeTo<TestUserData1>(notuserdata);
  ASSERT_TRUE(rejectedUserdata);
  EXPECT_FALSE(*rejectedUserdata);
}

namespace Star {
template <>
struct LuaConverter<Vec3F> : LuaUserDataConverter<Vec3F> {};

template <>
struct LuaUserDataMethods<Vec3F> {
  [[nodiscard]] static std::expected<LuaMethods<Vec3F>, StarException> make() {
    LuaMethods<Vec3F> methods;
    auto registered = methods.registerMethodWithSignature<float, Vec3F const&>("magnitude", std::mem_fn(&Vec3F::magnitude));
    if (!registered)
      return std::unexpected(std::move(registered.error()));
    return methods;
  }
};
}

TEST(LuaTest, UserMethodTest) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;
  auto callbackResult4 = luaEngine->createFunctionWithSignature<Vec3F, float, float, float>(construct<Vec3F>());
  ASSERT_TRUE(callbackResult4);
  ASSERT_TRUE((luaEngine->setGlobal("vec3", *callbackResult4)));

  auto luaContextResult = luaEngine->createContext();
  ASSERT_TRUE(luaContextResult);
  auto luaContext = std::move(*luaContextResult);
  ASSERT_TRUE((luaContext.load(R"SCRIPT(
      v = vec3(3, 2, 1)
      function testMagnitude(v2)
        return v:magnitude() + v2:magnitude()
      end
    )SCRIPT")));

  auto magnitudeResult = luaContext.invokePath<float>("testMagnitude", Vec3F(5, 5, 5));
  ASSERT_TRUE(magnitudeResult);
  auto magnitude = std::move(*magnitudeResult);
  EXPECT_TRUE(std::fabs(magnitude - (Vec3F(3, 2, 1).magnitude() + Vec3F(5, 5, 5).magnitude())) < 0.00001f);
}

TEST(LuaTest, GlobalTest) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;
  ASSERT_TRUE((luaEngine->setGlobal("globalfoo", LuaInt(42))));
  expectLuaAlternative<LuaInt>(luaEngine->getGlobal("globalfoo"), 42);

  auto luaContextResult = luaEngine->createContext();
  ASSERT_TRUE(luaContextResult);
  auto luaContext = std::move(*luaContextResult);
  ASSERT_TRUE((luaContext.load(R"SCRIPT(
      function test()
        return globalfoo
      end
    )SCRIPT")));

  expectLuaAlternative<LuaInt>(luaContext.invokePath("test"), 42);
}

TEST(LuaTest, ArgTest) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;

  auto luaContextResult = luaEngine->createContext();
  ASSERT_TRUE(luaContextResult);
  auto luaContext = std::move(*luaContextResult);
  ASSERT_TRUE((luaContext.load(R"SCRIPT(
      function test()
        callback("2", 3, nil)
      end
    )SCRIPT")));

  auto callbackResult5 = luaEngine->createFunction([](LuaFloat n, LuaString s, LuaBoolean b, LuaValue o) {
        EXPECT_EQ(n, 2.0);
        EXPECT_EQ(s.toString(), String("3"));
        EXPECT_EQ(b, false);
        EXPECT_TRUE(std::holds_alternative<LuaNilType>(o));
      });
  ASSERT_TRUE(callbackResult5);
  ASSERT_TRUE((luaContext.set("callback",
      *callbackResult5)));

  ASSERT_TRUE((luaContext.invokePath("test")));
}

TEST(LuaTest, ArrayTest) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;
  auto luaContextResult = luaEngine->createContext();
  ASSERT_TRUE(luaContextResult);
  auto luaContext = std::move(*luaContextResult);
  ASSERT_TRUE((luaContext.load(R"SCRIPT(
      function test()
        return {2, 4, 6, 8, 10}
      end
    )SCRIPT")));

  auto arrayValue = luaContext.invokePath("test");
  ASSERT_TRUE(arrayValue);
  auto arrayTablePointer = std::get_if<LuaTable>(&*arrayValue);
  ASSERT_NE(arrayTablePointer, nullptr);
  auto& arrayTable = *arrayTablePointer;

  EXPECT_EQ(arrayTable.length(), 5);
  expectLuaAlternative<LuaInt>(arrayTable.get(2), 4);
  expectLuaAlternative<LuaInt>(arrayTable.get(5), 10);

  List<std::pair<int, int>> values;
  ASSERT_TRUE((arrayTable.iterate([&values](LuaValue const& key, LuaValue const& value) -> std::expected<void, StarException> {
    auto ikey = std::get_if<LuaInt>(&key);
    auto ivalue = std::get_if<LuaInt>(&value);
    if (!ikey || !ivalue)
      return std::unexpected(LuaConversionException("Expected integer array key and value"));
    values.append({*ikey, *ivalue});
    return {};
  })));
  List<std::pair<int, int>> compare = {{1, 2}, {2, 4}, {3, 6}, {4, 8}, {5, 10}};
  EXPECT_EQ(values, compare);
}

TEST(LuaTest, PathTest) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;
  auto luaContextResult = luaEngine->createContext();
  ASSERT_TRUE(luaContextResult);
  auto luaContext = std::move(*luaContextResult);
  ASSERT_TRUE((luaContext.load(R"SCRIPT(
      foo = {
          bar = {
            baz = 1
          }
        }

      function test()
        return foo.bar.baf
      end
    )SCRIPT")));

  EXPECT_EQ(luaContext.containsPath("foo.bar.baz"), true);
  expectLuaAlternative<LuaInt>(luaContext.getPath("foo.bar.baz"), 1);
  EXPECT_EQ(luaContext.containsPath("foo.nothing.at.all"), false);

  ASSERT_TRUE((luaContext.setPath("foo.bar.baf", LuaInt(5))));
  expectLuaAlternative<LuaInt>(luaContext.invokePath("test"), 5);

  ASSERT_TRUE((luaContext.setPath("new.table.value", LuaInt(5))));
  expectLuaAlternative<LuaInt>(luaContext.getPath("new.table.value"), 5);
}

TEST(LuaTest, CallbackTest) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;

  LuaCallbacks callbacks;
  ASSERT_TRUE((callbacks.registerCallback("add", [](LuaInt a, LuaInt b) { return a + b; })));
  ASSERT_TRUE((callbacks.registerCallbackWithSignature<LuaInt, LuaInt, LuaInt>("subtract", [](int a, int b) { return a - b; })));
  ASSERT_TRUE((callbacks.registerCallbackWithSignature<LuaInt, LuaInt>("multiply2", std::bind([](int a, int b) { return a * b; }, 2, std::placeholders::_1))));
  ASSERT_TRUE((callbacks.registerCallbackWithSignature<void, LuaValue>("nothing", [](LuaValue v) { return v; })));

  auto luaContextResult = luaEngine->createContext();
  ASSERT_TRUE(luaContextResult);
  auto luaContext = std::move(*luaContextResult);
  ASSERT_TRUE((luaContext.setCallbacks("callbacks", callbacks)));
  ASSERT_TRUE((luaContext.load(R"SCRIPT(
      function test1()
        return callbacks.multiply2(callbacks.add(5, 10) + callbacks.subtract(3, 10))
      end

      function test2()
        return callbacks.nothing(1)
      end
    )SCRIPT")));

  expectLuaAlternative<LuaInt>(luaContext.invokePath("test1"), 16);
  expectLuaNil(luaContext.invokePath("test2"));
}

TEST(LuaTest, VariableParameters) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;
  auto context1Result = luaEngine->createContext();
  ASSERT_TRUE(context1Result);
  auto context1 = std::move(*context1Result);

  ASSERT_TRUE((context1.load(R"SCRIPT(
      function variableArgsCount(...)
        local arg = {...}
        return #arg
      end
    )SCRIPT")));

  auto res1Result = context1.invokePath<int>("variableArgsCount");
  ASSERT_TRUE(res1Result);
  auto res1 = std::move(*res1Result);
  auto res2Result = context1.invokePath<int>("variableArgsCount", 1);
  ASSERT_TRUE(res2Result);
  auto res2 = std::move(*res2Result);
  auto res3Result = context1.invokePath<int>("variableArgsCount", 1, 1);
  ASSERT_TRUE(res3Result);
  auto res3 = std::move(*res3Result);
  auto res4Result = context1.invokePath<int>("variableArgsCount", 1, 1, 1);
  ASSERT_TRUE(res4Result);
  auto res4 = std::move(*res4Result);

  EXPECT_EQ(res1, 0);
  EXPECT_EQ(res2, 1);
  EXPECT_EQ(res3, 2);
  EXPECT_EQ(res4, 3);
}

TEST(LuaTest, Scope) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;
  auto script1Result = luaEngine->compile(R"SCRIPT(
      function create(param)
        local self = {}
        local foo = param

        local getValue = function()
          return foo
        end

        function self.get()
          return getValue()
        end

        return self
      end
    )SCRIPT");
  ASSERT_TRUE(script1Result);
  auto script1 = std::move(*script1Result);

  auto script2Result = luaEngine->compile(R"SCRIPT(
      function init()
        obj = create(param)
      end

      function produce()
        return obj.get()
      end
    )SCRIPT");
  ASSERT_TRUE(script2Result);
  auto script2 = std::move(*script2Result);

  auto context1Result = luaEngine->createContext();
  ASSERT_TRUE(context1Result);
  auto context1 = std::move(*context1Result);
  ASSERT_TRUE((context1.load(script1)));
  ASSERT_TRUE((context1.load(script2)));

  auto context2Result = luaEngine->createContext();
  ASSERT_TRUE(context2Result);
  auto context2 = std::move(*context2Result);
  ASSERT_TRUE((context2.load(script1)));
  ASSERT_TRUE((context2.load(script2)));

  ASSERT_TRUE((context1.setPath("param", 1)));
  ASSERT_TRUE((context1.invokePath("init")));

  ASSERT_TRUE((context2.setPath("param", 2)));
  ASSERT_TRUE((context2.invokePath("init")));

  EXPECT_EQ(context1.invokePath<int>("produce"), 1);
  EXPECT_EQ(context2.invokePath<int>("produce"), 2);

  ASSERT_TRUE((context1.setPath("param", 2)));
  ASSERT_TRUE((context1.invokePath("init")));

  ASSERT_TRUE((context2.setPath("param", 1)));
  ASSERT_TRUE((context2.invokePath("init")));

  EXPECT_EQ(context1.invokePath<int>("produce"), 2);
  EXPECT_EQ(context2.invokePath<int>("produce"), 1);
}

TEST(LuaTest, Scope2) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;

  auto context1Result = luaEngine->createContext();
  ASSERT_TRUE(context1Result);
  auto context1 = std::move(*context1Result);
  ASSERT_TRUE((context1.load(R"SCRIPT(
      function init1()
        scopeState = {}
        scopeState.val = 10
      end
    )SCRIPT")));

  auto context2Result = luaEngine->createContext();
  ASSERT_TRUE(context2Result);
  auto context2 = std::move(*context2Result);
  ASSERT_TRUE((context2.load(R"SCRIPT(
      function init2()
        scopeState = {}
        scopeState.val = 20
      end
    )SCRIPT")));

  EXPECT_EQ(context1.contains("init1"), true);
  EXPECT_EQ(context2.contains("init2"), true);
  EXPECT_EQ(context1.contains("init2"), false);
  EXPECT_EQ(context2.contains("init1"), false);

  ASSERT_TRUE((context1.invokePath("init1")));
  EXPECT_EQ(context1.getPath<int>("scopeState.val"), 10);

  expectLuaNil(context2.getPath("scopeState"));

  ASSERT_TRUE((context2.invokePath("init2")));
  EXPECT_EQ(context2.getPath<int>("scopeState.val"), 20);

  EXPECT_EQ(context1.getPath<int>("scopeState.val"), 10);
}

TEST(LuaTest, MetaTable) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;

  auto contextResult = luaEngine->createContext();
  ASSERT_TRUE(contextResult);
  auto context = std::move(*contextResult);
  ASSERT_TRUE((context.load(R"SCRIPT(
      function add(a, b)
        return a + b
      end
    )SCRIPT")));

  auto mtResult = luaEngine->createTable();
  ASSERT_TRUE(mtResult);
  auto mt = std::move(*mtResult);
  auto callbackResult6 = luaEngine->createFunction([](LuaEngine& engine, LuaTable const& a, LuaTable const& b) -> std::expected<LuaTable, StarException> {
        auto ax = a.get<double>(1);
        if (!ax) return std::unexpected(std::move(ax.error()));
        auto ay = a.get<double>(2);
        if (!ay) return std::unexpected(std::move(ay.error()));
        auto bx = b.get<double>(1);
        if (!bx) return std::unexpected(std::move(bx.error()));
        auto by = b.get<double>(2);
        if (!by) return std::unexpected(std::move(by.error()));
        return engine.createArrayTable(std::initializer_list<double>{*ax + *bx, *ay + *by});
      });
  ASSERT_TRUE(callbackResult6);
  ASSERT_TRUE((mt.set("__add",
      *callbackResult6)));
  ASSERT_TRUE((mt.set("test", "hello")));

  auto t1Result = luaEngine->createArrayTable(std::initializer_list<double>{1, 2});
  ASSERT_TRUE(t1Result);
  auto t1 = std::move(*t1Result);
  ASSERT_TRUE(t1.setMetatable(mt));

  auto t2Result = luaEngine->createArrayTable(std::initializer_list<double>{5, 6});
  ASSERT_TRUE(t2Result);
  auto t2 = std::move(*t2Result);
  ASSERT_TRUE(t2.setMetatable(mt));

  auto trResult = context.invokePath<LuaTable>("add", t1, t2);
  ASSERT_TRUE(trResult);
  auto tr = std::move(*trResult);
  EXPECT_EQ(tr.get<double>(1), 6);
  EXPECT_EQ(tr.get<double>(2), 8);
  auto metatable1 = t1.getMetatable();
  ASSERT_TRUE(metatable1);
  ASSERT_TRUE(*metatable1);
  EXPECT_EQ((*metatable1)->get<String>("test"), "hello");
  auto metatable2 = t2.getMetatable();
  ASSERT_TRUE(metatable2);
  ASSERT_TRUE(*metatable2);
  EXPECT_EQ((*metatable2)->get<String>("test"), "hello");
}

TEST(LuaTest, Integers) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;
  auto contextResult = luaEngine->createContext();
  ASSERT_TRUE(contextResult);
  auto context = std::move(*contextResult);
  ASSERT_TRUE((context.load(R"SCRIPT(
      n1 = 0
      n2 = 1
      n3 = 1.0
      n4 = 1.1
      n5 = 5.0
      n6 = 5
    )SCRIPT")));

  expectLuaAlternative<LuaInt>(context.get("n1"), 0);
  expectLuaAlternative<LuaInt>(context.get("n2"), 1);
  expectLuaAlternative<LuaFloat>(context.get("n3"), 1.0);
  expectLuaAlternative<LuaFloat>(context.get("n4"), 1.1);
  expectLuaAlternative<LuaFloat>(context.get("n5"), 5.0);
  expectLuaAlternative<LuaInt>(context.get("n6"), 5);
}

TEST(LuaTest, Require) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;
  auto contextResult = luaEngine->createContext();
  ASSERT_TRUE(contextResult);
  auto context = std::move(*contextResult);
  ASSERT_TRUE((context.setRequireFunction([](LuaContext& context, LuaString const& arg) -> std::expected<void, StarException> {
      auto callback = context.createFunction([arg]() { return arg; });
      if (!callback)
        return std::unexpected(std::move(callback.error()));
      return context.set(arg, *callback);
    })));

  ASSERT_TRUE((context.load(R"SCRIPT(
      require "a"
      require "b"
      require "c"

      function res()
        return a() .. b() .. c()
      end
    )SCRIPT")));

  auto required = context.invokePath<LuaString>("res");
  ASSERT_TRUE(required);
  EXPECT_EQ(required->toString(), String("abc"));

  // The require function above stored closures that capture a Lua reference
  // (LuaString), and such a closure keeps the engine alive from inside its own
  // Lua state: the state would never be destroyed and everything it allocated
  // leaks (LeakSanitizer reports ~22KB for this test alone).  Drop the globals
  // and collect before the engine goes out of scope.
  ASSERT_TRUE((context.load("a = nil b = nil c = nil res = nil")));
  ASSERT_TRUE(luaEngine->collectGarbage());
}

TEST(LuaTest, Eval) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;
  auto contextResult = luaEngine->createContext();
  ASSERT_TRUE(contextResult);
  auto context = std::move(*contextResult);

  ASSERT_TRUE((context.eval("i = 3")));
  // Make sure statements and expressions both work in eval.
  EXPECT_EQ(context.eval<int>("i + 1"), 4);
  EXPECT_EQ(context.eval<int>("return i + 1"), 4);
}

TEST(LuaTest, Multi) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;
  auto scriptResult = luaEngine->compile(R"SCRIPT(
      function entry()
        return callbacks.func(2, 4)
      end

      function sum(...)
        local sum = 0
        for i,v in ipairs(arg) do
          sum = sum + v
        end
        return sum
      end
    )SCRIPT");
  ASSERT_TRUE(scriptResult);
  auto script = std::move(*scriptResult);

  auto context1Result = luaEngine->createContext();
  ASSERT_TRUE(context1Result);
  auto context1 = std::move(*context1Result);
  auto context2Result = luaEngine->createContext();
  ASSERT_TRUE(context2Result);
  auto context2 = std::move(*context2Result);
  auto context3Result = luaEngine->createContext();
  ASSERT_TRUE(context3Result);
  auto context3 = std::move(*context3Result);

  ASSERT_TRUE((context1.load(script)));
  ASSERT_TRUE((context2.load(script)));
  ASSERT_TRUE((context3.load(script)));

  LuaCallbacks addCallbacks;
  ASSERT_TRUE((addCallbacks.registerCallback("func",
      [](LuaVariadic<int> const& args) -> int {
        int sum = 0.0;
        for (auto arg : args)
          sum += arg;
        return sum;
      })));

  LuaCallbacks multCallbacks;
  ASSERT_TRUE((multCallbacks.registerCallback("func",
      [](LuaVariadic<int> const& args) -> int {
        int mult = 1.0;
        for (auto arg : args)
          mult *= arg;
        return mult;
      })));

  ASSERT_TRUE((context1.setCallbacks("callbacks", addCallbacks)));
  ASSERT_TRUE((context2.setCallbacks("callbacks", multCallbacks)));
  ASSERT_TRUE((context3.setCallbacks("callbacks", addCallbacks)));

  EXPECT_EQ(context1.invokePath<int>("entry"), 6);
  EXPECT_EQ(context2.invokePath<int>("entry"), 8);
  EXPECT_EQ(context3.invokePath<int>("entry"), 6);
  EXPECT_EQ(context1.invokePath<int>("entry"), 6);
  EXPECT_EQ(context2.invokePath<int>("entry"), 8);
  EXPECT_EQ(context3.invokePath<int>("entry"), 6);
  EXPECT_EQ(context1.invokePath<int>("entry"), 6);

  auto context4Result = luaEngine->createContext();
  ASSERT_TRUE(context4Result);
  auto context4 = std::move(*context4Result);
  ASSERT_TRUE((context4.load(R"SCRIPT(
      function sum(...)
        local args = {...}
        local sum = 0
        for i = 1, #args do
          sum = sum + args[i]
        end
        return sum
      end

      function mreturn(...)
        return ...
      end

      function callbacktest(...)
        local x, y = callback()
        return x, y
      end

      function emptycallbacktest(...)
        return emptycallback()
      end
    )SCRIPT")));
  EXPECT_EQ(context4.invokePath<int>("sum", LuaVariadic<int>{1, 2, 3}), 6);
  EXPECT_EQ(context4.invokePath<int>("sum", 5, LuaVariadic<int>{1, 2, 3}, 10), 21);
  EXPECT_EQ(context4.invokePath<LuaVariadic<int>>("mreturn", 1, 2, 3), LuaVariadic<int>({1, 2, 3}));

  int a;
  float b;
  String c;
  auto tupleResult1 = context4.invokePath<LuaTupleReturn<int, float, String>>("mreturn", 1, 2.0f, "foo");
  ASSERT_TRUE(tupleResult1);
  luaTie(a, b, c) = std::move(*tupleResult1);
  EXPECT_EQ(a, 1);
  EXPECT_EQ(b, 2.0f);
  EXPECT_EQ(c, "foo");

  auto callbackResult7 = context4.createFunction([]() { return luaTupleReturn(5, 10); });
  ASSERT_TRUE(callbackResult7);
  ASSERT_TRUE((context4.set("callback", *callbackResult7)));

  auto callbackResult8 = context4.createFunction([]() { return luaTupleReturn(); });
  ASSERT_TRUE(callbackResult8);
  ASSERT_TRUE((context4.set("emptycallback", *callbackResult8)));

  int d;
  int e;
  auto tupleResult2 = context4.invokePath<LuaTupleReturn<int, int>>("callbacktest");
  ASSERT_TRUE(tupleResult2);
  luaTie(d, e) = std::move(*tupleResult2);
  EXPECT_EQ(d, 5);
  EXPECT_EQ(e, 10);

  expectLuaNil(context4.invokePath("emptycallbacktest"));
}

TEST(LuaTest, Limits) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;
  luaEngine->setInstructionLimit(500000);
  luaEngine->setRecursionLimit(64);
  auto contextResult = luaEngine->createContext();
  ASSERT_TRUE(contextResult);
  auto context = std::move(*contextResult);
  ASSERT_TRUE((context.load(R"SCRIPT(
      function toinfinityandbeyond()
        while true do
        end
      end

      function toabignumberandthenstop()
        for i = 0, 50000 do
        end
      end
    )SCRIPT")));

  // Make sure infinite loops trigger the instruction limit.
  {
    auto failure = context.invokePath("toinfinityandbeyond");
    ASSERT_FALSE(failure);
    EXPECT_TRUE(failure.error().is<LuaInstructionLimitReached>());
  };

  // Make sure the instruction count is reset after each call.
  ASSERT_TRUE((context.invokePath("toabignumberandthenstop")));

  auto infLoop = R"SCRIPT(
        while true do
        end
      )SCRIPT";

  // Make sure loading code into context with infinite loops in their
  // evaluation triggers instruction limit.
  {
    auto failure = context.load(infLoop);
    ASSERT_FALSE(failure);
    EXPECT_TRUE(failure.error().is<LuaInstructionLimitReached>());
  };

  // And the same for eval
  {
    auto failure = context.eval(infLoop);
    ASSERT_FALSE(failure);
    EXPECT_TRUE(failure.error().is<LuaInstructionLimitReached>());
  };

  auto call1 = [&context]() { return context.invokePath("call2"); };

  auto call2 = [&context]() { return context.invokePath("call1"); };

  auto callbackResult9 = context.createFunction(call1);
  ASSERT_TRUE(callbackResult9);
  ASSERT_TRUE((context.set("call1", *callbackResult9)));
  auto callbackResult10 = context.createFunction(call2);
  ASSERT_TRUE(callbackResult10);
  ASSERT_TRUE((context.set("call2", *callbackResult10)));

  {
    auto failure = context.invokePath("call1");
    ASSERT_FALSE(failure);
    EXPECT_TRUE(failure.error().is<LuaRecursionLimitReached>());
  };

  // Make sure the context still functions properly after these previous
  // errors.
  EXPECT_EQ(context.eval<int>("1 + 1"), 2);
}

TEST(LuaTest, Errors) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;
  auto contextResult = luaEngine->createContext();
  ASSERT_TRUE(contextResult);
  auto context = std::move(*contextResult);

  {
    auto failure = context.eval("while true do");
    ASSERT_FALSE(failure);
    EXPECT_TRUE(failure.error().is<LuaIncompleteStatementException>());
  };
  ASSERT_TRUE((context.setPath("val", 1.0)));
  {
    auto failure = context.getPath<Vec2D>("val");
    ASSERT_FALSE(failure);
    EXPECT_TRUE(failure.error().is<LuaConversionException>());
  };
  auto val = context.get("val");
  ASSERT_TRUE(val);
  auto rect = luaEngine->luaMaybeTo<RectF>(*val);
  ASSERT_TRUE(rect);
  EXPECT_FALSE(*rect);

  auto callbackResult11 = luaEngine->createFunction([]() -> std::expected<void, StarException> {
      return std::unexpected(StarException("lua caught the error!"));
    });
  ASSERT_TRUE(callbackResult11);
  ASSERT_TRUE((context.set("throwException", *callbackResult11)));

  ASSERT_TRUE((context.load(R"SCRIPT(
      function throwError()
        return throwException()
      end
      function catchError()
        return pcall(throwException)
      end
    )SCRIPT")));

  {
    auto failure = context.invokePath("throwError");
    ASSERT_FALSE(failure);
    EXPECT_TRUE(failure.error().is<LuaException>());
  };

  bool status;
  LuaValue error;
  auto tupleResult3 = context.invokePath<LuaTupleReturn<bool, LuaValue>>("catchError");
  ASSERT_TRUE(tupleResult3);
  luaTie(status, error) = std::move(*tupleResult3);
  EXPECT_EQ(status, false);
  auto errorText = luaEngine->luaTo<String>(error);
  ASSERT_TRUE(errorText);
  EXPECT_EQ(errorText->contains("lua caught the error"), true);
}

TEST(LuaTest, ErrorDiagnosticsLeaveBothCBoundariesClean) {
  auto engineResult = LuaEngine::create();
  ASSERT_TRUE(engineResult);
  auto& engine = *engineResult;
  auto contextResult = engine->createContext();
  ASSERT_TRUE(contextResult);
  auto context = std::move(*contextResult);
  std::weak_ptr<int> wrappedLifetime;
  std::weak_ptr<int> requireLifetime;
  auto callbackResult12 = engine->createFunction([&]() -> std::expected<void, StarException> {
    auto lifetime = std::make_shared<int>(1);
    wrappedLifetime = lifetime;
    return std::unexpected(IOException("wrapped callback failure",
        ExceptionDiagnostic(StarException("Lua callback cause"))));
  });
  ASSERT_TRUE(callbackResult12);
  ASSERT_TRUE((context.set("failWrapped", *callbackResult12)));
  ASSERT_TRUE((context.setRequireFunction([&](LuaContext&, LuaString const&) -> std::expected<void, StarException> {
    auto lifetime = std::make_shared<int>(2);
    requireLifetime = lifetime;
    return std::unexpected(IOException("require callback failure",
        ExceptionDiagnostic(StarException("Lua callback cause"))));
  })));
  ASSERT_TRUE((context.load(R"SCRIPT(
    function captureWrapped() return pcall(failWrapped) end
    function captureRequire() return pcall(require, "broken") end
    function healthy(value) return value * 3 end
  )SCRIPT")));

  bool status;
  LuaValue error;
  auto tupleResult4 = context.invokePath<LuaTupleReturn<bool, LuaValue>>("captureWrapped");
  ASSERT_TRUE(tupleResult4);
  luaTie(status, error) = std::move(*tupleResult4);
  EXPECT_FALSE(status);
  auto wrappedText = engine->luaTo<String>(error);
  ASSERT_TRUE(wrappedText);
  EXPECT_EQ(wrappedText->contains("(IOException) wrapped callback failure"), true);
  EXPECT_EQ(wrappedText->contains("Caused by:"), true);
  EXPECT_EQ(wrappedText->contains("Lua callback cause"), true);
  EXPECT_TRUE(wrappedLifetime.expired());
  EXPECT_EQ(context.invokePath<int>("healthy", 7), 21);

  auto tupleResult5 = context.invokePath<LuaTupleReturn<bool, LuaValue>>("captureRequire");
  ASSERT_TRUE(tupleResult5);
  luaTie(status, error) = std::move(*tupleResult5);
  EXPECT_FALSE(status);
  auto requireText = engine->luaTo<String>(error);
  ASSERT_TRUE(requireText);
  EXPECT_EQ(requireText->contains("(IOException) require callback failure"), true);
  EXPECT_EQ(requireText->contains("Caused by:"), true);
  EXPECT_EQ(requireText->contains("Lua callback cause"), true);
  EXPECT_TRUE(requireLifetime.expired());
  EXPECT_EQ(context.invokePath<int>("healthy", 9), 27);

  // Lua errors not caught by pcall still return the project's error value.
  {
    auto failure = context.load("failWrapped()");
    ASSERT_FALSE(failure);
    EXPECT_TRUE(failure.error().is<LuaException>());
  };
  EXPECT_TRUE(wrappedLifetime.expired());
  EXPECT_EQ(context.invokePath<int>("healthy", 10), 30);
}

TEST(LuaTest, GarbageCollection) {
  auto engineResult = LuaEngine::create();
  ASSERT_TRUE(engineResult);
  auto& engine = *engineResult;
  auto contextResult = engine->createContext();
  ASSERT_TRUE(contextResult);
  auto context = std::move(*contextResult);

  auto ptr = std::make_shared<int>(5);
  engine->setAutoGarbageCollection(false);
  {
    auto userdata = context.createUserData(ptr);
    ASSERT_TRUE(userdata);
    ASSERT_TRUE(context.setPath("ref", *userdata));
  }
  EXPECT_EQ(ptr.use_count(), 2);
  ASSERT_TRUE((context.setPath("ref", LuaNil)));
  EXPECT_EQ(ptr.use_count(), 2);
  ASSERT_TRUE(engine->collectGarbage());
  EXPECT_EQ(ptr.use_count(), 1);
}

TEST(LuaTest, AutomaticCollectionCanBeStoppedAndRestarted) {
  auto engine = LuaEngine::create(false);
  ASSERT_TRUE(engine);
  auto context = (*engine)->createContext();
  ASSERT_TRUE(context);
  (*engine)->setAutoGarbageCollection(false);
  EXPECT_EQ(context->eval<bool>("collectgarbage('isrunning')"), false);
  (*engine)->setAutoGarbageCollection(true);
  EXPECT_EQ(context->eval<bool>("collectgarbage('isrunning')"), true);
  (*engine)->setAutoGarbageCollection(false);
  ASSERT_TRUE((*engine)->collectGarbage(1));
  EXPECT_EQ(context->eval<bool>("collectgarbage('isrunning')"), false);
}

TEST(LuaTest, OrdinaryCallsReleaseExpiredReferencesWithoutHostCollection) {
  auto engine = LuaEngine::create(false);
  ASSERT_TRUE(engine);
  auto context = (*engine)->createContext();
  ASSERT_TRUE(context);
  ASSERT_TRUE(context->load(R"SCRIPT(
    observed = setmetatable({}, {__mode = "v"})
    function remember(index)
      local object = {index = index}
      observed[index] = object
      return object
    end
    function alive()
      collectgarbage("collect")
      local count = 0
      for _ in pairs(observed) do count = count + 1 end
      return count
    end
  )SCRIPT"));
  std::optional<LuaTable> retained;
  {
    auto first = context->invokePath<LuaTable>("remember", 1);
    ASSERT_TRUE(first);
    retained = std::move(*first);
  }
  for (int i = 2; i <= 4096; ++i) {
    auto temporary = context->invokePath<LuaTable>("remember", i);
    ASSERT_TRUE(temporary);
  }
  EXPECT_EQ(context->invokePath<int>("alive"), 1);
  EXPECT_EQ(retained->get<int>("index"), 1);
  retained.reset();
  EXPECT_EQ(context->invokePath<int>("alive"), 0);
}

TEST(LuaTest, FinalizerWarningPreservesContextAndReleasesUserData) {
  auto engineResult = LuaEngine::create();
  ASSERT_TRUE(engineResult);
  auto& engine = *engineResult;
  engine->setAutoGarbageCollection(false);
  auto contextResult = engine->createContext();
  ASSERT_TRUE(contextResult);
  auto context = std::move(*contextResult);

  auto resource = std::make_shared<int>(7);
  std::weak_ptr<int> lifetime = resource;
  {
    auto userdata = context.createUserData(resource);
    ASSERT_TRUE(userdata);
    ASSERT_TRUE(context.setPath("trackedResource", *userdata));
  }
  ASSERT_TRUE(context.load(R"SCRIPT(
    finalized = false
    do
      local doomed = {resource = trackedResource}
      setmetatable(doomed, {
        __gc = function(object)
          finalized = true
          object.resource = nil
          error("lua-gc-finalizer-recovery-marker")
        end
      })
    end
    trackedResource = nil
  )SCRIPT"));
  resource.reset();
  EXPECT_FALSE(lifetime.expired());

  ASSERT_TRUE(engine->collectGarbage());
  EXPECT_EQ(context.get<bool>("finalized"), true);
  EXPECT_EQ(context.eval<int>("6 * 7"), 42);

  // Finalized userdata can retain objects until a subsequent GC cycle.
  ASSERT_TRUE(engine->collectGarbage());
  ASSERT_TRUE(engine->collectGarbage());
  EXPECT_TRUE(lifetime.expired());
}

TEST(LuaTest, IntTest) {
  auto engineResult = LuaEngine::create();
  ASSERT_TRUE(engineResult);
  auto& engine = *engineResult;
  auto contextResult = engine->createContext();
  ASSERT_TRUE(contextResult);
  auto context = std::move(*contextResult);
  ASSERT_TRUE((context.setPath("test", (std::uint64_t)-1)));
  EXPECT_EQ(context.getPath<std::uint64_t>("test"), (std::uint64_t)-1);
}

TEST(LuaTest, VariantTest) {
  auto engineResult = LuaEngine::create();
  ASSERT_TRUE(engineResult);
  auto& engine = *engineResult;
  auto contextResult = engine->createContext();
  ASSERT_TRUE(contextResult);
  auto context = std::move(*contextResult);

  using IntOrString = std::variant<int, String>;

  EXPECT_EQ(context.eval<IntOrString>("'foo'"), IntOrString(String("foo")));
  EXPECT_EQ(context.eval<IntOrString>("'1'"), IntOrString(1));

  using StringOrInt = std::variant<String, int>;
  EXPECT_EQ(context.eval<StringOrInt>("'1'"), StringOrInt(String("1")));

  using MIntOrString = std::variant<std::monostate, std::optional<int>, String>;

  EXPECT_EQ(context.eval<MIntOrString>("'foo'"), MIntOrString(String("foo")));
  EXPECT_EQ(context.eval<MIntOrString>("'1'"), MIntOrString(std::optional<int>(1)));
  auto nilValue = context.eval<LuaValue>("nil");
  ASSERT_TRUE(nilValue);
  auto nil = engine->luaMaybeTo<MIntOrString>(*nilValue);
  ASSERT_TRUE(nil);
  ASSERT_TRUE(*nil);
  EXPECT_TRUE(std::holds_alternative<std::monostate>(**nil));
  ASSERT_TRUE((context.set("variantNil", MIntOrString{})));
  EXPECT_EQ(context.eval<bool>("variantNil == nil"), true);

  using OptionalOrString = std::variant<std::optional<int>, String>;
  auto optionalNilValue = context.eval<LuaValue>("nil");
  ASSERT_TRUE(optionalNilValue);
  auto optionalNil = engine->luaMaybeTo<OptionalOrString>(*optionalNilValue);
  ASSERT_TRUE(optionalNil);
  ASSERT_TRUE(*optionalNil);
  ASSERT_TRUE(std::holds_alternative<std::optional<int>>(**optionalNil));
  EXPECT_FALSE(std::get<std::optional<int>>(**optionalNil));
}

TEST(LuaTest, ProfilingTest) {
  auto luaEngineResult = LuaEngine::create();
  ASSERT_TRUE(luaEngineResult);
  auto& luaEngine = *luaEngineResult;
  luaEngine->setProfilingEnabled(true);
  luaEngine->setInstructionMeasureInterval(1000);

  auto contextResult = luaEngine->createContext();
  ASSERT_TRUE(contextResult);
  auto context = std::move(*contextResult);
  ASSERT_TRUE((context.eval(R"SCRIPT(
      function function1()
        for i = 1, 1000 do
        end
      end

      function function2()
        for i = 1, 1000 do
        end
      end

      function function3()
        for i = 1, 1000 do
        end
      end

      for i = 1, 10000 do
        function1()
        function2()
        function3()
      end
    )SCRIPT")));

  StringSet names;
  List<LuaProfileEntry> profile = luaEngine->getProfile();
  for (auto const& p : profile[0].calls)
    names.add(p.second->name.value_or(String()));

  EXPECT_TRUE(names.contains("function1"));
  EXPECT_TRUE(names.contains("function2"));
  EXPECT_TRUE(names.contains("function3"));
}

TEST(LuaTest, PromiseUserdataObservesSharedOutcome) {
  auto engineResult = LuaEngine::create();
  ASSERT_TRUE(engineResult);
  auto& engine = *engineResult;
  auto contextResult = engine->createContext();
  ASSERT_TRUE(contextResult);
  auto context = std::move(*contextResult);
  auto [promise, keeper] = RpcPromise<Json>::createPair();
  ASSERT_TRUE((context.setPath("promise", promise)));
  ASSERT_TRUE((context.setPath("keeper", std::make_shared<AutoFailRpcPromiseKeeper<Json>>(keeper))));
  EXPECT_EQ(context.eval<bool>("promise:finished()"), false);
  EXPECT_EQ(context.eval<bool>("promise:succeeded()"), false);
  EXPECT_EQ(context.eval<bool>("promise:result() == nil and promise:error() == nil"), true);
  ASSERT_TRUE((context.load("keeper:fulfill({answer = 42})")));
  EXPECT_EQ(context.eval<bool>("promise:finished() and promise:succeeded()"), true);
  EXPECT_EQ(context.eval<Json>("promise:result()"), JsonObject({{"answer", 42}}));
  EXPECT_EQ(context.eval<bool>("promise:error() == nil"), true);
  auto promiseResult = promise.result();
  ASSERT_TRUE(promiseResult);
  ASSERT_TRUE(*promiseResult);
  EXPECT_EQ((**promiseResult).getInt("answer"), 42);
  {
    auto failure = context.load("keeper:fail('second resolution')");
    ASSERT_FALSE(failure);
    EXPECT_TRUE(failure.error().is<LuaException>());
  };
  ASSERT_TRUE((context.load("keeper = nil promise = nil")));
  ASSERT_TRUE(engine->collectGarbage());
  EXPECT_EQ(promise.succeeded(), true);
}

TEST(LuaTest, PromiseKeeperGarbageCollectionFailsOnlyLastOwner) {
  auto engineResult = LuaEngine::create();
  ASSERT_TRUE(engineResult);
  auto& engine = *engineResult;
  engine->setAutoGarbageCollection(false);
  auto contextResult = engine->createContext();
  ASSERT_TRUE(contextResult);
  auto context = std::move(*contextResult);
  auto [promise, keeper] = RpcPromise<Json>::createPair();
  auto automatic = std::make_shared<AutoFailRpcPromiseKeeper<Json>>(keeper);
  ASSERT_TRUE((context.setPath("keeper", automatic)));
  ASSERT_TRUE((context.setPath("promise", promise)));
  ASSERT_TRUE((context.load("alias = keeper keeper = nil")));
  automatic.reset();
  ASSERT_TRUE(engine->collectGarbage());
  EXPECT_EQ(promise.finished(), false);
  ASSERT_TRUE((context.load("alias = nil")));
  ASSERT_TRUE(engine->collectGarbage());
  EXPECT_EQ(promise.failed(), true);
  auto promiseError = promise.error();
  ASSERT_TRUE(promiseError);
  ASSERT_TRUE(*promiseError);
  EXPECT_EQ(**promiseError, "Keeper destroyed");
  EXPECT_EQ(context.eval<bool>("promise:finished() and not promise:succeeded()"), true);
  EXPECT_EQ(context.eval<String>("promise:error()"), "Keeper destroyed");
  ASSERT_TRUE((context.load("promise = nil")));
  ASSERT_TRUE(engine->collectGarbage());
}

TEST(LuaTest, PromiseKeeperCollectionPreservesChainAndFailure) {
  auto engineResult = LuaEngine::create();
  ASSERT_TRUE(engineResult);
  auto& engine = *engineResult;
  auto contextResult = engine->createContext();
  ASSERT_TRUE(contextResult);
  auto context = std::move(*contextResult);
  auto [source, sourceKeeper] = RpcPromise<Json>::createPair();
  auto [promise, keeper] = RpcPromise<Json>::createPair();
  ASSERT_TRUE((context.setPath("source", source)));
  ASSERT_TRUE((context.setPath("promise", promise)));
  ASSERT_TRUE((context.setPath("keeper", std::make_shared<AutoFailRpcPromiseKeeper<Json>>(keeper))));
  ASSERT_TRUE((context.load("keeper:chain(source) keeper = nil")));
  ASSERT_TRUE(engine->collectGarbage());
  EXPECT_EQ(promise.finished(), false);
  ASSERT_TRUE((sourceKeeper.fulfill(JsonArray{1, 2, 3})));
  EXPECT_EQ(context.eval<Json>("promise:result()"), JsonArray({1, 2, 3}));

  auto [failed, failedKeeper] = RpcPromise<Json>::createPair();
  ASSERT_TRUE((context.setPath("failedKeeper", std::make_shared<AutoFailRpcPromiseKeeper<Json>>(failedKeeper))));
  ASSERT_TRUE((context.load("failedKeeper:fail('from Lua') failedKeeper = nil")));
  ASSERT_TRUE(engine->collectGarbage());
  auto failedError = failed.error();
  ASSERT_TRUE(failedError);
  ASSERT_TRUE(*failedError);
  EXPECT_EQ(**failedError, "from Lua");
  ASSERT_TRUE((context.load("source = nil promise = nil")));
  ASSERT_TRUE(engine->collectGarbage());
}
