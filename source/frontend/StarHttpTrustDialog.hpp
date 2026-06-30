#pragma once

#include "StarAssets.hpp"
#include "StarConfiguration.hpp"
#include "StarPane.hpp"

namespace Star {

enum class HttpTrustReply {
  Allow,
  Deny
};

class HttpTrustDialog final : public Pane {
public:
  struct Services {
    AssetsConstPtr assets;
    ConfigurationPtr configuration;
  };

  explicit HttpTrustDialog(Services services);

  ~HttpTrustDialog() override = default;

  void displayRequest(String const& domain, function<void(HttpTrustReply, bool)> callback);

  void dismissed() override;

private:
  void reply(HttpTrustReply replyType);

  AssetsConstPtr m_assets;
  ConfigurationPtr m_configuration;
  String m_domain;
  bool m_confirmed;
  function<void(HttpTrustReply, bool)> m_callback;
};

}
