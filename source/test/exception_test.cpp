import std;
import star.format;
import star.output_proxy;

#include "gtest/gtest.h"

using namespace Star;

static_assert(!std::is_constructible_v<ExceptionDiagnostic, std::exception const&>);
static_assert(!std::is_constructible_v<StarException, std::string, std::exception const&>);

TEST(ExceptionTest, OwningCausePreservesKindsAndBothTraceModes) {
  std::string expectedShort;
  std::string expectedFull;
  bool sourceDestroyed = false;
  struct SourceException : OutOfRangeException {
    explicit SourceException(bool& destroyed)
      : OutOfRangeException("inner range failure"), destroyed(destroyed) {}
    ~SourceException() override { destroyed = true; }
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

TEST(ExceptionTest, DisabledTraceAndThirdPartyWhatSurviveTheirSources) {
  auto withoutTrace = [] {
    OutOfRangeException source("no trace", false);
    return ExceptionDiagnostic(source);
  }();
  EXPECT_EQ(printException(withoutTrace, true), printException(withoutTrace, false));
  std::string originalWhat;
  auto thirdParty = [&] {
    try {
      throw std::runtime_error("third party failure");
    } catch (std::exception const& e) {
      originalWhat = e.what();
      return ExceptionDiagnostic(std::current_exception());
    }
  }();
  EXPECT_NE(printException(thirdParty, false).find(originalWhat), std::string::npos);
  EXPECT_EQ(printException(thirdParty, true), printException(thirdParty, false));
  IOException wrapped("wrapped third party", thirdParty);
  auto text = printException(wrapped, true);
  auto cause = text.find(originalWhat);
  ASSERT_NE(cause, std::string::npos);
  EXPECT_LT(text.find("wrapped third party"), cause);
}

TEST(ExceptionTest, NestedHandlerDoesNotReplaceTheRequestedDiagnostic) {
  try {
    throw IOException("outer captured object", false);
  } catch (std::exception const&) {
    ExceptionDiagnostic const outer(std::current_exception());
    try {
      throw std::runtime_error("different active object");
    } catch (std::exception const&) {
      ExceptionDiagnostic const inner(std::current_exception());
      auto outerText = printException(outer, true);
      auto innerText = printException(inner, true);
      EXPECT_NE(outerText.find("outer captured object"), std::string::npos);
      EXPECT_EQ(outerText.find("different active object"), std::string::npos);
      EXPECT_NE(innerText.find("different active object"), std::string::npos);
      EXPECT_EQ(innerText.find("outer captured object"), std::string::npos);
      OutOfRangeException wrapped("explicit outer cause", outer);
      auto wrappedText = printException(wrapped, false);
      EXPECT_NE(wrappedText.find("outer captured object"), std::string::npos);
      EXPECT_EQ(wrappedText.find("different active object"), std::string::npos);
      IOException detached("unthrown third object", false);
      auto detachedText = printException(detached, true);
      EXPECT_NE(detachedText.find("unthrown third object"), std::string::npos);
      EXPECT_EQ(detachedText.find("outer captured object"), std::string::npos);
      EXPECT_EQ(detachedText.find("different active object"), std::string::npos);
    }
  }
}

TEST(ExceptionTest, ExceptionPtrRetainsProjectErrorClassification) {
  auto captured = std::make_exception_ptr(OutOfRangeException("stored project error", false));
  auto text = printException(captured, true);
  EXPECT_NE(text.find("OutOfRangeException"), std::string::npos);
  EXPECT_NE(text.find("stored project error"), std::string::npos);
  try {
    std::rethrow_exception(captured);
  } catch (OutOfRangeException const& e) {
    EXPECT_NE(std::string_view(e.what()).find("stored project error"), std::string_view::npos);
  } catch (...) {
    FAIL() << "The project exception kind was changed";
  }
}
