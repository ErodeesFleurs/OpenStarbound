import std;
import star.json;
import star.string;
import star.list;
import star.lexical_cast;

import star.formatted_json;
import star.json_path;

#include "gtest/gtest.h"

using namespace Star;

TEST(FormattedJsonTest, JsonInterop) {
  Json array1 = JsonArray{1, 2, 3, 4};
  Json array2 = JsonArray{4, 3, 2, 1};
  auto result1 = FormattedJson::create(array1);
  ASSERT_TRUE(result1);
  FormattedJson farray1 = (*result1);
  auto result2 = FormattedJson::create(array2);
  ASSERT_TRUE(result2);
  FormattedJson farray2 = (*result2);

  EXPECT_EQ(farray1.toJson(), array1);
  EXPECT_EQ(farray2.toJson(), array2);
  EXPECT_NE(farray1.toJson(), array2);
  EXPECT_NE(farray2.toJson(), array1);
}

TEST(FormattedJsonTest, Parsing) {
  auto result3 = FormattedJson::parse(R"JSON(
    {
      "foo": "bar",
      "hello" : "world",
      "abc" :123
      // Comment
      ,"wat": {
        "thing": [
          49,
          27]
      }
    }
)JSON");
  ASSERT_TRUE(result3);
  FormattedJson json1 = (*result3);
  auto barElement = FormattedJson::create(Json{"bar"});
  ASSERT_TRUE(barElement);
  auto worldElement = FormattedJson::create(Json{"world"});
  ASSERT_TRUE(worldElement);
  auto numberElement = FormattedJson::create(Json{123});
  ASSERT_TRUE(numberElement);
  auto objectElement = FormattedJson::create(Json{JsonObject{{"thing", JsonArray{49, 27}}}});
  ASSERT_TRUE(objectElement);
  FormattedJson::ElementList expectedElements = {
    WhitespaceElement{"\n      "},
    ObjectKeyElement{"foo"},
    ColonElement{},
    WhitespaceElement{" "},
    ValueElement{*barElement},
    CommaElement{},
    WhitespaceElement{"\n      "},
    ObjectKeyElement{"hello"},
    WhitespaceElement{" "},
    ColonElement{},
    WhitespaceElement{" "},
    ValueElement{*worldElement},
    CommaElement{},
    WhitespaceElement{"\n      "},
    ObjectKeyElement{"abc"},
    WhitespaceElement{" "},
    ColonElement{},
    ValueElement{*numberElement},
    WhitespaceElement{"\n      // Comment\n      "},
    CommaElement{},
    ObjectKeyElement{"wat"},
    ColonElement{},
    WhitespaceElement{" "},
    ValueElement{*objectElement},
    WhitespaceElement{"\n    "}
  };
  EXPECT_EQ(json1.elements(), expectedElements);
  auto result4 = json1.get("foo");
  ASSERT_TRUE(result4);
  auto result5 = FormattedJson::create(Json{"bar"});
  ASSERT_TRUE(result5);
  EXPECT_EQ((*result4), (*result5));
  auto result6 = json1.get("abc");
  ASSERT_TRUE(result6);
  auto result7 = FormattedJson::create(Json{123});
  ASSERT_TRUE(result7);
  EXPECT_EQ((*result6), (*result7));
  auto result8 = json1.get("wat");
  ASSERT_TRUE(result8);
  auto result9 = (*result8).get("thing");
  ASSERT_TRUE(result9);
  auto result10 = FormattedJson::create(Json{27});
  ASSERT_TRUE(result10);
  auto result11 = (*result9).get(1);
  ASSERT_TRUE(result11);
  EXPECT_EQ((*result11), (*result10));
  auto result12 = json1.get("wat");
  ASSERT_TRUE(result12);
  auto result13 = (*result12).get("thing");
  ASSERT_TRUE(result13);
  auto result14 = FormattedJson::create(Json{66});
  ASSERT_TRUE(result14);
  auto result15 = (*result13).get(0);
  ASSERT_TRUE(result15);
  EXPECT_NE((*result15), (*result14));

  auto error16 = FormattedJson::parse(" ");
  ASSERT_FALSE(error16);
  EXPECT_TRUE(error16.error().is<JsonParsingException>());
  auto error17 = FormattedJson::parse("/* */");
  ASSERT_FALSE(error17);
  EXPECT_TRUE(error17.error().is<JsonParsingException>());
  auto error18 = FormattedJson::parse("x");
  ASSERT_FALSE(error18);
  EXPECT_TRUE(error18.error().is<JsonParsingException>());
  auto error19 = FormattedJson::parseJson("123");
  ASSERT_FALSE(error19);
  EXPECT_TRUE(error19.error().is<JsonParsingException>());
  auto error20 = FormattedJson::parseJson("\"foo\"");
  ASSERT_FALSE(error20);
  EXPECT_TRUE(error20.error().is<JsonParsingException>());
  auto result21 = FormattedJson::parse("123");
  ASSERT_TRUE(result21);
  EXPECT_TRUE((*result21).isType(Json::Type::Int));
  auto result22 = FormattedJson::parse("\"foo\"");
  ASSERT_TRUE(result22);
  EXPECT_TRUE((*result22).isType(Json::Type::String));
}

List<String> keyOrder(FormattedJson const& json) {
  List<String> keys;
  for (JsonElement const& elem : json.elements()) {
    if (std::holds_alternative<ObjectKeyElement>(elem))
      keys.append(std::get<ObjectKeyElement>(elem).key);
  }
  return keys;
}

TEST(FormattedJsonTest, ObjectInsertion) {
  FormattedJson json = FormattedJson::ofType(Json::Type::Object);
  List<String> expectedKeys;

  EXPECT_EQ(keyOrder(json), expectedKeys);

  auto result201 = FormattedJson::create(Json{"bar"});
  ASSERT_TRUE(result201);
  auto result23 = json.set("foo", (*result201));
  ASSERT_TRUE(result23);
  json = (*result23);
  expectedKeys.append("foo");
  EXPECT_EQ(keyOrder(json), expectedKeys);

  auto result202 = FormattedJson::create(Json{"..."});
  ASSERT_TRUE(result202);
  auto result24 = json.set("baz", (*result202));
  ASSERT_TRUE(result24);
  json = (*result24);
  expectedKeys.append("baz");
  EXPECT_EQ(keyOrder(json), expectedKeys);

  auto result203 = FormattedJson::create(Json{"world"});
  ASSERT_TRUE(result203);
  auto result25 = json.prepend("hello", (*result203));
  ASSERT_TRUE(result25);
  json = (*result25);
  expectedKeys.insertAt(0, "hello");
  EXPECT_EQ(keyOrder(json), expectedKeys);

  auto result204 = FormattedJson::create(Json{"alal"});
  ASSERT_TRUE(result204);
  auto result26 = json.insertBefore("lala", (*result204), "foo");
  ASSERT_TRUE(result26);
  json = (*result26);
  expectedKeys.insertAt(1, "lala");
  EXPECT_EQ(keyOrder(json), expectedKeys);

  auto result205 = FormattedJson::create(Json{"ipsum"});
  ASSERT_TRUE(result205);
  auto result27 = json.insertAfter("lorem", (*result205), "foo");
  ASSERT_TRUE(result27);
  json = (*result27);
  expectedKeys.insertAt(3, "lorem");
  EXPECT_EQ(keyOrder(json), expectedKeys);

  auto result206 = FormattedJson::create(Json{"sit amet"});
  ASSERT_TRUE(result206);
  auto result28 = json.append("dolor", (*result206));
  ASSERT_TRUE(result28);
  json = (*result28);
  expectedKeys.append("dolor");
  EXPECT_EQ(keyOrder(json), expectedKeys);

  // If the key already exists, the key order doesn't change.
  auto result207 = FormattedJson::create(Json{123});
  ASSERT_TRUE(result207);
  auto result29 = json.set("foo", (*result207));
  ASSERT_TRUE(result29);
  auto result208 = FormattedJson::create(Json{123});
  ASSERT_TRUE(result208);
  auto result30 = (*result29).append("hello", (*result208));
  ASSERT_TRUE(result30);
  auto result209 = FormattedJson::create(Json{123});
  ASSERT_TRUE(result209);
  auto result31 = (*result30).insertAfter("dolor", (*result209), "baz");
  ASSERT_TRUE(result31);
  json = (*result31);
  EXPECT_EQ(keyOrder(json), expectedKeys);

  auto numberElement = FormattedJson::create(Json{123});
  ASSERT_TRUE(numberElement);
  auto alalElement = FormattedJson::create(Json{"alal"});
  ASSERT_TRUE(alalElement);
  auto ipsumElement = FormattedJson::create(Json{"ipsum"});
  ASSERT_TRUE(ipsumElement);
  auto dotsElement = FormattedJson::create(Json{"..."});
  ASSERT_TRUE(dotsElement);
  FormattedJson::ElementList expectedElements = {ObjectKeyElement{"hello"},
      ColonElement{},
      ValueElement{*numberElement},
      CommaElement{},
      ObjectKeyElement{"lala"},
      ColonElement{},
      ValueElement{*alalElement},
      CommaElement{},
      ObjectKeyElement{"foo"},
      ColonElement{},
      ValueElement{*numberElement},
      CommaElement{},
      ObjectKeyElement{"lorem"},
      ColonElement{},
      ValueElement{*ipsumElement},
      CommaElement{},
      ObjectKeyElement{"baz"},
      ColonElement{},
      ValueElement{*dotsElement},
      CommaElement{},
      ObjectKeyElement{"dolor"},
      ColonElement{},
      ValueElement{*numberElement}};
  EXPECT_EQ(json.elements(), expectedElements);

  FormattedJson emptyObject = FormattedJson::ofType(Json::Type::Object);
  auto result210 = FormattedJson::create(Json{});
  ASSERT_TRUE(result210);
  auto error32 = emptyObject.insertBefore("foo", (*result210), "bar");
  ASSERT_FALSE(error32);
  EXPECT_TRUE(error32.error().is<JsonException>());
  auto result211 = FormattedJson::create(Json{});
  ASSERT_TRUE(result211);
  auto error33 = emptyObject.insertAfter("foo", (*result211), "bar");
  ASSERT_FALSE(error33);
  EXPECT_TRUE(error33.error().is<JsonException>());
  auto result212 = FormattedJson::create(Json{});
  ASSERT_TRUE(result212);
  auto error34 = FormattedJson::ofType(Json::Type::Array).set("foo", (*result212));
  ASSERT_FALSE(error34);
  EXPECT_TRUE(error34.error().is<JsonException>());
}

TEST(FormattedJsonTest, ObjectInsertionWithWhitespace) {
  auto result35 = FormattedJson::parse(" {  \"foo\": 123  } ");
  ASSERT_TRUE(result35);
  FormattedJson json = (*result35);
  auto result213 = FormattedJson::create(Json{"world"});
  ASSERT_TRUE(result213);
  auto result36 = json.append("hello", (*result213));
  ASSERT_TRUE(result36);
  json = (*result36);
  auto result214 = FormattedJson::create(Json{"ipsum"});
  ASSERT_TRUE(result214);
  auto result37 = json.prepend("lorem", (*result214));
  ASSERT_TRUE(result37);
  json = (*result37);
  auto result38 = json.repr();
  ASSERT_TRUE(result38);
  EXPECT_EQ((*result38),
      R"JSON({  "lorem": "ipsum",  "foo": 123,  "hello": "world"  })JSON");
}

TEST(FormattedJsonTest, ArrayInsertion) {
  auto result39 = FormattedJson::parse(R"JSON([
    12,
    34
  ])JSON");
  ASSERT_TRUE(result39);
  FormattedJson json = (*result39);
  auto result215 = FormattedJson::create(Json{23});
  ASSERT_TRUE(result215);
  auto result40 = json.insert(1, (*result215));
  ASSERT_TRUE(result40);
  json = (*result40);
  auto result216 = FormattedJson::create(Json{45});
  ASSERT_TRUE(result216);
  auto result41 = json.append((*result216));
  ASSERT_TRUE(result41);
  json = (*result41);
  auto result217 = FormattedJson::create(Json{"01"});
  ASSERT_TRUE(result217);
  auto result42 = json.set(0, (*result217));
  ASSERT_TRUE(result42);
  json = (*result42);
  auto result218 = FormattedJson::create(Json{0});
  ASSERT_TRUE(result218);
  auto result43 = json.insert(0, (*result218));
  ASSERT_TRUE(result43);
  json = (*result43);
  std::string_view expected = R"JSON([
    0,
    "01",
    23,
    34,
    45
  ])JSON";
  auto result44 = json.repr();
  ASSERT_TRUE(result44);
  EXPECT_EQ((*result44), expected);

  FormattedJson emptyArray = FormattedJson::ofType(Json::Type::Array);
  auto result219 = FormattedJson::create(Json{});
  ASSERT_TRUE(result219);
  auto result45 = emptyArray.insert(0, (*result219));
  ASSERT_TRUE(result45);
  auto result46 = (*result45).size();
  ASSERT_TRUE(result46);
  EXPECT_EQ((*result46), 1u);
  auto result220 = FormattedJson::create(Json{});
  ASSERT_TRUE(result220);
  auto error47 = emptyArray.insert(1, (*result220));
  ASSERT_FALSE(error47);
  EXPECT_TRUE(error47.error().is<JsonException>());
  auto result221 = FormattedJson::create(Json{});
  ASSERT_TRUE(result221);
  auto error48 = FormattedJson::ofType(Json::Type::Object).insert(0, (*result221));
  ASSERT_FALSE(error48);
  EXPECT_TRUE(error48.error().is<JsonException>());
}

TEST(FormattedJsonTest, ObjectErase) {
  auto result49 = FormattedJson::parse(R"JSON({
    "zzz": 123,
    "mmm": 456,
    "aaa": 789
  })JSON");
  ASSERT_TRUE(result49);
  FormattedJson json = (*result49);
  auto result50 = json.eraseKey("mmm");
  ASSERT_TRUE(result50);
  json = (*result50);
  std::string_view expected = R"JSON({
    "zzz": 123,
    "aaa": 789
  })JSON";
  auto result51 = json.repr();
  ASSERT_TRUE(result51);
  EXPECT_EQ((*result51), expected);

  auto result52 = json.eraseKey("zzz");
  ASSERT_TRUE(result52);
  FormattedJson jsonNoZ = (*result52);
  expected = R"JSON({
    "aaa": 789
  })JSON";
  auto result53 = jsonNoZ.repr();
  ASSERT_TRUE(result53);
  EXPECT_EQ((*result53), expected);

  auto result54 = json.eraseKey("aaa");
  ASSERT_TRUE(result54);
  FormattedJson jsonNoA = (*result54);
  expected = R"JSON({
    "zzz": 123
  })JSON";
  auto result55 = jsonNoA.repr();
  ASSERT_TRUE(result55);
  EXPECT_EQ((*result55), expected);

  auto result56 = json.eraseKey("bbb");
  ASSERT_TRUE(result56);
  ASSERT_EQ((*result56), json);
  auto error57 = FormattedJson::ofType(Json::Type::Array).eraseKey("foo");
  ASSERT_FALSE(error57);
  EXPECT_TRUE(error57.error().is<JsonException>());
}

TEST(FormattedJsonTest, ArrayErase) {
  auto result58 = FormattedJson::parse("[123, 456, 789]");
  ASSERT_TRUE(result58);
  FormattedJson json = (*result58);
  auto result59 = json.eraseIndex(0);
  ASSERT_TRUE(result59);
  auto result60 = (*result59).repr();
  ASSERT_TRUE(result60);
  EXPECT_EQ((*result60), "[456, 789]");
  auto result61 = json.eraseIndex(1);
  ASSERT_TRUE(result61);
  auto result62 = (*result61).repr();
  ASSERT_TRUE(result62);
  EXPECT_EQ((*result62), "[123, 789]");
  auto result63 = json.eraseIndex(2);
  ASSERT_TRUE(result63);
  auto result64 = (*result63).repr();
  ASSERT_TRUE(result64);
  EXPECT_EQ((*result64), "[123, 456]");
  auto result65 = json.eraseIndex(0);
  ASSERT_TRUE(result65);
  auto result66 = (*result65).eraseIndex(0);
  ASSERT_TRUE(result66);
  auto result67 = (*result66).repr();
  ASSERT_TRUE(result67);
  EXPECT_EQ((*result67), "[789]");
  auto result68 = json.eraseIndex(0);
  ASSERT_TRUE(result68);
  auto result69 = (*result68).eraseIndex(0);
  ASSERT_TRUE(result69);
  auto result70 = (*result69).eraseIndex(0);
  ASSERT_TRUE(result70);
  auto result71 = (*result70).repr();
  ASSERT_TRUE(result71);
  EXPECT_EQ((*result71), "[]");

  auto error72 = FormattedJson::ofType(Json::Type::Object).eraseIndex(0);
  ASSERT_FALSE(error72);
  EXPECT_TRUE(error72.error().is<JsonException>());
}

TEST(FormattedJsonTest, CommentPreservation) {
  auto result73 = FormattedJson::parse(R"JSON({
    // This is a comment
    "hello": 1,
    "world": 2
  })JSON");
  ASSERT_TRUE(result73);
  FormattedJson json = (*result73);
  auto result222 = FormattedJson::create(Json{1});
  ASSERT_TRUE(result222);
  auto result74 = json.insertBefore("goodbye", (*result222), "world");
  ASSERT_TRUE(result74);
  json = (*result74);
  auto result75 = json.eraseKey("hello");
  ASSERT_TRUE(result75);
  json = (*result75);
  std::string_view expected = R"JSON({
    // This is a comment
    "goodbye": 1,
    "world": 2
  })JSON";
  auto result76 = json.repr();
  ASSERT_TRUE(result76);
  EXPECT_EQ((*result76), expected);
}

TEST(FormattedJsonTest, StylePreservation) {
  auto result77 = FormattedJson::parse(R"JSON({

    "hello"        :          1234

  })JSON");
  ASSERT_TRUE(result77);
  FormattedJson json = (*result77);
  auto result223 = FormattedJson::create(Json{5678});
  ASSERT_TRUE(result223);
  auto result78 = json.append("world", (*result223));
  ASSERT_TRUE(result78);
  json = (*result78);
  std::string_view expected = R"JSON({

    "hello"        :          1234,

    "world"        :          5678

  })JSON";
  auto result79 = json.repr();
  ASSERT_TRUE(result79);
  EXPECT_EQ((*result79), expected);
}

TEST(FormattedJsonTest, Queries) {
  auto result80 = FormattedJson::parse("[]");
  ASSERT_TRUE(result80);
  FormattedJson json0 = (*result80);
  auto result81 = FormattedJson::parse("{\"a\":1}");
  ASSERT_TRUE(result81);
  FormattedJson json1 = (*result81);
  auto result82 = FormattedJson::parse("[1,2]");
  ASSERT_TRUE(result82);
  FormattedJson json2 = (*result82);
  auto result83 = FormattedJson::parse(R"({"a":1,"b":2,"c":3})");
  ASSERT_TRUE(result83);
  FormattedJson json3 = (*result83);

  auto result84 = json0.size();
  ASSERT_TRUE(result84);
  EXPECT_EQ((*result84), 0u);
  auto result85 = json1.size();
  ASSERT_TRUE(result85);
  EXPECT_EQ((*result85), 1u);
  auto result86 = json2.size();
  ASSERT_TRUE(result86);
  EXPECT_EQ((*result86), 2u);
  auto result87 = json3.size();
  ASSERT_TRUE(result87);
  EXPECT_EQ((*result87), 3u);

  auto result88 = json1.contains("a");
  ASSERT_TRUE(result88);
  EXPECT_TRUE((*result88));
  auto result89 = json1.contains("b");
  ASSERT_TRUE(result89);
  EXPECT_FALSE((*result89));
  auto result90 = json3.contains("c");
  ASSERT_TRUE(result90);
  EXPECT_TRUE((*result90));
  auto result91 = json3.contains("b");
  ASSERT_TRUE(result91);
  EXPECT_TRUE((*result91));

  auto result92 = FormattedJson::parse("123");
  ASSERT_TRUE(result92);
  auto error93 = (*result92).size();
  ASSERT_FALSE(error93);
  EXPECT_TRUE(error93.error().is<JsonException>());
  auto error94 = json2.contains("1");
  ASSERT_FALSE(error94);
  EXPECT_TRUE(error94.error().is<JsonException>());
}

TEST(FormattedJsonTest, Types) {
  auto result95 = FormattedJson::create(Json{});
  ASSERT_TRUE(result95);
  FormattedJson jsonNull = (*result95);
  auto result96 = FormattedJson::create(Json{true});
  ASSERT_TRUE(result96);
  FormattedJson jsonBool = (*result96);
  auto result97 = FormattedJson::create(Json{1});
  ASSERT_TRUE(result97);
  FormattedJson jsonInt = (*result97);
  auto result98 = FormattedJson::create(Json{2.0});
  ASSERT_TRUE(result98);
  FormattedJson jsonFloat = (*result98);
  auto result99 = FormattedJson::create(Json{"foo"});
  ASSERT_TRUE(result99);
  FormattedJson jsonString = (*result99);
  auto result100 = FormattedJson::create(Json::ofType(Json::Type::Array));
  ASSERT_TRUE(result100);
  FormattedJson jsonArray = (*result100);
  auto result101 = FormattedJson::create(Json::ofType(Json::Type::Object));
  ASSERT_TRUE(result101);
  FormattedJson jsonObject = (*result101);

  EXPECT_EQ(jsonNull.type(), Json::Type::Null);
  EXPECT_TRUE(jsonNull.isType(Json::Type::Null));
  EXPECT_EQ(jsonNull.typeName(), "null");

  EXPECT_EQ(jsonBool.type(), Json::Type::Bool);
  EXPECT_TRUE(jsonBool.isType(Json::Type::Bool));
  EXPECT_EQ(jsonBool.typeName(), "bool");

  EXPECT_EQ(jsonInt.type(), Json::Type::Int);
  EXPECT_TRUE(jsonInt.isType(Json::Type::Int));
  EXPECT_EQ(jsonInt.typeName(), "int");

  EXPECT_EQ(jsonFloat.type(), Json::Type::Float);
  EXPECT_TRUE(jsonFloat.isType(Json::Type::Float));
  EXPECT_EQ(jsonFloat.typeName(), "float");

  EXPECT_EQ(jsonString.type(), Json::Type::String);
  EXPECT_TRUE(jsonString.isType(Json::Type::String));
  EXPECT_EQ(jsonString.typeName(), "string");

  EXPECT_EQ(jsonArray.type(), Json::Type::Array);
  EXPECT_TRUE(jsonArray.isType(Json::Type::Array));
  EXPECT_EQ(jsonArray.typeName(), "array");

  EXPECT_EQ(jsonObject.type(), Json::Type::Object);
  EXPECT_TRUE(jsonObject.isType(Json::Type::Object));
  EXPECT_EQ(jsonObject.typeName(), "object");
}

TEST(FormattedJsonTest, JsonPath) {
  auto result102 = FormattedJson::parse(R"JSON({
      "foo": {
        "bar": [
          12,
          {
            "hello": "world"
          },
          45
        ]
      },
      "baz": [{"a":1}, {"a":2}, {"a":3}]
    })JSON");
  ASSERT_TRUE(result102);
  FormattedJson json = (*result102);

  auto result103 = FormattedJson::create(Json{JsonObject{{"hello", "world"}}});
  ASSERT_TRUE(result103);
  FormattedJson expectedHelloWorld = (*result103);
  auto result104 = JsonPath::Pointer("/foo/bar/1").get(json);
  ASSERT_TRUE(result104);
  EXPECT_EQ((*result104), expectedHelloWorld);
  auto result105 = JsonPath::QueryPath("foo.bar[1]").get(json);
  ASSERT_TRUE(result105);
  EXPECT_EQ((*result105), expectedHelloWorld);
  auto result106 = FormattedJson::create(Json{1});
  ASSERT_TRUE(result106);
  auto result107 = JsonPath::Pointer("/baz/0/a").get(json);
  ASSERT_TRUE(result107);
  EXPECT_EQ((*result107), (*result106));
  auto result108 = FormattedJson::create(Json{1});
  ASSERT_TRUE(result108);
  auto result109 = JsonPath::QueryPath("baz[0].a").get(json);
  ASSERT_TRUE(result109);
  EXPECT_EQ((*result109), (*result108));

  auto result110 = FormattedJson::create(Json{0});
  ASSERT_TRUE(result110);
  auto result111 = JsonPath::Pointer("/baz/0/a").set(json, (*result110));
  ASSERT_TRUE(result111);
  json = (*result111);
  auto result112 = FormattedJson::create(Json{4});
  ASSERT_TRUE(result112);
  auto result113 = JsonPath::QueryPath("baz[1].a").set(json, (*result112));
  ASSERT_TRUE(result113);
  json = (*result113);

  auto result114 = FormattedJson::parse("{\"b\":1}");
  ASSERT_TRUE(result114);
  auto result115 = JsonPath::Pointer("/baz/1").add(json, (*result114));
  ASSERT_TRUE(result115);
  json = (*result115);
  auto result116 = FormattedJson::parse("{\"c\":0.5}");
  ASSERT_TRUE(result116);
  auto result117 = JsonPath::QueryPath("baz[1]").add(json, (*result116));
  ASSERT_TRUE(result117);
  json = (*result117);

  auto result118 = json.get("baz");
  ASSERT_TRUE(result118);
  auto result119 = (*result118).repr();
  ASSERT_TRUE(result119);
  EXPECT_EQ((*result119),
      R"([{"a":0},{"c":0.5},{"b":1}, {"a":4}, {"a":3}])");

  auto result120 = FormattedJson::create(JsonArray{});
  ASSERT_TRUE(result120);
  auto result121 = JsonPath::Pointer("/plz").set(json, (*result120));
  ASSERT_TRUE(result121);
  json = (*result121);
  auto result122 = FormattedJson::create(Json{"thx"});
  ASSERT_TRUE(result122);
  auto result123 = JsonPath::Pointer("/plz/-").set(json, (*result122));
  ASSERT_TRUE(result123);
  json = (*result123);
  auto result124 = FormattedJson::create(Json{"bye"});
  ASSERT_TRUE(result124);
  auto result125 = JsonPath::Pointer("/plz/-").add(json, (*result124));
  ASSERT_TRUE(result125);
  json = (*result125);
  auto result126 = FormattedJson::create(Json{JsonArray{"thx", "bye"}});
  ASSERT_TRUE(result126);
  FormattedJson expectedThxBye = (*result126);
  auto result127 = json.get("plz");
  ASSERT_TRUE(result127);
  EXPECT_EQ((*result127), expectedThxBye);

  // Set and add are almost the same, but:
  //    Set 0 => replaces the first array element
  auto result128 = FormattedJson::create(Json{"kthx"});
  ASSERT_TRUE(result128);
  auto result129 = JsonPath::Pointer("/plz/0").set(json, (*result128));
  ASSERT_TRUE(result129);
  json = (*result129);
  //    Add 0 => inserts a new element at the beginning
  auto result130 = FormattedJson::create(Json{"kbye"});
  ASSERT_TRUE(result130);
  auto result131 = JsonPath::Pointer("/plz/0").add(json, (*result130));
  ASSERT_TRUE(result131);
  json = (*result131);
  auto result132 = FormattedJson::create(Json{JsonArray{"kbye", "kthx", "bye"}});
  ASSERT_TRUE(result132);
  FormattedJson expectedKByeKThxBye = (*result132);
  auto result133 = json.get("plz");
  ASSERT_TRUE(result133);
  EXPECT_EQ((*result133), expectedKByeKThxBye);

  auto result134 = JsonPath::Pointer("/foo/bar/1").remove(json);
  ASSERT_TRUE(result134);
  json = (*result134);
  auto result135 = FormattedJson::create(Json{JsonArray{12, 45}});
  ASSERT_TRUE(result135);
  FormattedJson expectedBar = (*result135);
  auto result136 = JsonPath::Pointer("/foo/bar").get(json);
  ASSERT_TRUE(result136);
  EXPECT_EQ((*result136), expectedBar);
  auto result137 = JsonPath::QueryPath("foo.bar[1]").remove(json);
  ASSERT_TRUE(result137);
  json = (*result137);
  auto result138 = JsonPath::QueryPath("foo.bar").get(json);
  ASSERT_TRUE(result138);
  EXPECT_EQ((*result138).toJson(), JsonArray{12});
}

TEST(FormattedJsonTest, NumberFormatPreservation) {
  auto result139 = FormattedJson::parse("1.0");
  ASSERT_TRUE(result139);
  auto result140 = (*result139).repr();
  ASSERT_TRUE(result140);
  EXPECT_EQ((*result140), "1.0");
  auto result141 = FormattedJson::parse("1");
  ASSERT_TRUE(result141);
  auto result142 = (*result141).repr();
  ASSERT_TRUE(result142);
  EXPECT_EQ((*result142), "1");
  auto result143 = FormattedJson::parse("-0");
  ASSERT_TRUE(result143);
  auto result144 = (*result143).repr();
  ASSERT_TRUE(result144);
  EXPECT_EQ((*result144), "-0");
  auto result145 = FormattedJson::parse("0");
  ASSERT_TRUE(result145);
  auto result146 = (*result145).repr();
  ASSERT_TRUE(result146);
  EXPECT_EQ((*result146), "0");

  auto result147 = FormattedJson::parseJson("[-0.0,1.0,-0]");
  ASSERT_TRUE(result147);
  auto result148 = (*result147).repr();
  ASSERT_TRUE(result148);
  EXPECT_EQ((*result148), "[-0.0,1.0,-0]");
  auto result149 = FormattedJson::parseJson("[1,0]");
  ASSERT_TRUE(result149);
  auto result150 = (*result149).repr();
  ASSERT_TRUE(result150);
  EXPECT_EQ((*result150), "[1,0]");
}
