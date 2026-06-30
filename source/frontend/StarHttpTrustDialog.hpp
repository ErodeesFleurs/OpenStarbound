#pragma once

#include "StarIAssets.hpp"
#include "StarIConfiguration.hpp"
#include "StarPane.hpp"

namespace Star {

enum class HttpTrustReply {
  Allow,
  Deny
};

class HttpTrustDialog final : public Pane {
public:
  struct Services {
    IAssetsConstPtr assets;
    IConfigurationPtr configuration;
  };

  HttpTrustDialog(Services services = {});

  ~HttpTrustDialog() override = default;

  void displayRequest(String const& domain, function<void(HttpTrustReply, bool)> callback);

  void dismissed() override;

private:
  void reply(HttpTrustReply replyType);

  IAssetsConstPtr m_assets;
  IConfigurationPtr m_configuration;
  String m_domain;
  bool m_confirmed;
  function<void(HttpTrustReply, bool)> m_callback;
};

}
