#pragma once

#include "StarJson.hpp"
#include "StarString.hpp"

namespace Star {

class IConfiguration {
public:
  virtual ~IConfiguration() = default;

  virtual Json get(String const& key, Json def = {}) const = 0;
  virtual Json getPath(String const& path, Json def = {}) const = 0;

  virtual void set(String const& key, Json const& value) = 0;
  virtual void setPath(String const& path, Json const& value) = 0;

  virtual String printConfiguration() const = 0;
};

using IConfigurationPtr = SharedPtr<IConfiguration>;
using IConfigurationConstPtr = SharedPtr<IConfiguration const>;

}

