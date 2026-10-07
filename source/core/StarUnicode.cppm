module;
#include "StarUnicode.inc"

export module star.unicode;

export namespace Star {
  using ::Star::UnicodeExceptionTag;
  using ::Star::UnicodeException;
  using ::Star::Utf8Type;
  using ::Star::Utf32Type;
  using ::Star::STAR_UTF32_REPLACEMENT_CHAR;
  using ::Star::throwInvalidUtf8Sequence;
  using ::Star::throwMissingUtf8End;
  using ::Star::throwInvalidUtf32CodePoint;
  using ::Star::utf8Length;
  using ::Star::utf8DecodeChar;
  using ::Star::utf8EncodeChar;
  using ::Star::hexStringToUtf32;
  using ::Star::hexStringFromUtf32;
  using ::Star::isUtf16LeadSurrogate;
  using ::Star::isUtf16TrailSurrogate;
  using ::Star::utf32FromUtf16SurrogatePair;
  using ::Star::utf32ToUtf16SurrogatePair;
  using ::Star::U8ToU32Iterator;
  using ::Star::Utf8OutputIterator;
}
