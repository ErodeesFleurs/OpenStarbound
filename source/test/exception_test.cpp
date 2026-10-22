import std;
import star.format;
import star.output_proxy;

#include "gtest/gtest.h"

using namespace Star;

static_assert(!std::is_base_of_v<std::exception, StarException>);
static_assert(!std::is_constructible_v<ExceptionDiagnostic, std::exception const&>);
static_assert(!std::is_constructible_v<StarException, std::string, std::exception const&>);

TEST(ExceptionTest, OwningCausePreservesKindsAndBothTraceModes) {
  std::string expectedShort;
  std::string expectedFull;
  bool sourceDestroyed = false;
  struct SourceException : OutOfRangeException {
    explicit SourceException(bool& destroyed)
      : OutOfRangeException("inner range failure"), destroyed(destroyed) {}
    ~SourceException() { destroyed = true; }
    bool& destroyed;
  };
  auto cause = [&] {
    SourceException source(sourceDestroyed);
    expectedShort = printException(source, false);
    expectedFull = printException(source, true);
    return ExceptionDiagnostic(source);
  }();
  ASSERT_TRUE(sourceDestroyed);
  EXPECT_EQ(printException(cause, false), expectedShort);
  EXPECT_EQ(printException(cause, true), expectedFull);
  auto wrapped = [&] {
    IOException source("outer io failure", cause);
    expectedShort = printException(source, false);
    expectedFull = printException(source, true);
    return ExceptionDiagnostic(source);
  }();
  EXPECT_EQ(printException(wrapped, false), expectedShort);
  EXPECT_EQ(printException(wrapped, true), expectedFull);
  auto output = [&] {
    IOException source("proxy owner", wrapped);
    return outputException(source, false);
  }();
  std::ostringstream os;
  os << output;
  auto text = os.str();
  auto outer = text.find("outer io failure");
  auto inner = text.find("inner range failure");
  ASSERT_NE(outer, std::string::npos);
  ASSERT_NE(inner, std::string::npos);
  EXPECT_LT(text.find("proxy owner"), outer);
  EXPECT_LT(outer, inner);
}

TEST(ExceptionTest, DisabledTraceSurvivesItsSource) {
  auto withoutTrace = [] {
    OutOfRangeException source("no trace", false);
    return ExceptionDiagnostic(source);
  }();
  EXPECT_EQ(printException(withoutTrace, true), printException(withoutTrace, false));
  EXPECT_NE(printException(withoutTrace, false).find("no trace"), std::string::npos);
}

TEST(ExceptionTest, IndependentDiagnosticValuesRetainExplicitCauses) {
  ExceptionDiagnostic const outer(IOException("outer captured object", false));
  ExceptionDiagnostic const inner(StarException("different diagnostic object", false));
  auto outerText = printException(outer, true);
  auto innerText = printException(inner, true);
  EXPECT_NE(outerText.find("outer captured object"), std::string::npos);
  EXPECT_EQ(outerText.find("different diagnostic object"), std::string::npos);
  EXPECT_NE(innerText.find("different diagnostic object"), std::string::npos);
  EXPECT_EQ(innerText.find("outer captured object"), std::string::npos);
  OutOfRangeException wrapped("explicit outer cause", outer);
  auto wrappedText = printException(wrapped, false);
  EXPECT_NE(wrappedText.find("outer captured object"), std::string::npos);
  EXPECT_EQ(wrappedText.find("different diagnostic object"), std::string::npos);
  IOException detached("independent third object", false);
  auto detachedText = printException(detached, true);
  EXPECT_NE(detachedText.find("independent third object"), std::string::npos);
  EXPECT_EQ(detachedText.find("outer captured object"), std::string::npos);
  EXPECT_EQ(detachedText.find("different diagnostic object"), std::string::npos);
}

TEST(ExceptionTest, ExpectedRetainsProjectErrorClassificationAndOwnership) {
  std::expected<int, StarException> captured = std::unexpected(OutOfRangeException("stored project error", false));
  ASSERT_FALSE(captured);
  EXPECT_TRUE(captured.error().is<OutOfRangeException>());
  EXPECT_TRUE(captured.error().is<StarException>());
  EXPECT_FALSE(captured.error().is<IOException>());
  auto text = printException(ExceptionDiagnostic(captured.error()), true);
  EXPECT_NE(text.find("OutOfRangeException"), std::string::npos);
  EXPECT_NE(text.find("stored project error"), std::string::npos);
  auto retained = std::move(captured);
  ASSERT_FALSE(retained);
  EXPECT_TRUE(retained.error().is<OutOfRangeException>());
  EXPECT_NE(std::string_view(retained.error().what()).find("stored project error"), std::string_view::npos);
}
