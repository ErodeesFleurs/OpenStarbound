#pragma once

#include "StarString.hpp"
#include "StarByteArray.hpp"

import std;

namespace Star {

auto hexEncode(char const* data, std::size_t len, char* output, std::size_t outLen = std::numeric_limits<std::size_t>::max()) -> std::size_t;
auto hexDecode(char const* src, std::size_t len, char* output, std::size_t outLen = std::numeric_limits<std::size_t>::max()) -> std::size_t;
auto nibbleDecode(char const* src, std::size_t len, char* output, std::size_t outLen = std::numeric_limits<std::size_t>::max()) -> std::size_t;

auto base64Encode(char const* data, std::size_t len, char* output, std::size_t outLen = std::numeric_limits<std::size_t>::max()) -> std::size_t;
auto base64Decode(char const* src, std::size_t len, char* output, std::size_t outLen = std::numeric_limits<std::size_t>::max()) -> std::size_t;

auto hexEncode(char const* data, std::size_t len) -> String;
auto base64Encode(char const* data, std::size_t len) -> String;

auto hexEncode(ByteArray const& data) -> String;
auto hexDecode(String const& encodedData) -> ByteArray;

auto base64Encode(ByteArray const& data) -> String;
auto base64Decode(String const& encodedData) -> ByteArray;

}
