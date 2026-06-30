#include "StarException.hpp"

#include "gtest/gtest.h"

#include <sstream>

using namespace Star;

TEST(ExceptionTest, OutputProxyOutlivesException) {
  unique_ptr<OutputProxy> proxy;

  {
    StarException cause("inner failure");
    StarException outer("outer failure", cause);
    proxy = make_unique<OutputProxy>(outputException(outer, false));
  }

  std::ostringstream os;
  os << *proxy;
  auto output = os.str();

  EXPECT_NE(output.find("outer failure"), std::string::npos);
  EXPECT_NE(output.find("inner failure"), std::string::npos);
}
