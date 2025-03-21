#pragma once

#include "logs/interfaces/logs.hpp"
#include "trigger/factory.hpp"

#include <tuple>

namespace trigger::lnx::oneshot
{

using config_t = std::tuple<std::shared_ptr<logs::LogIf>>;

class Trigger : public TriggerIf
{
  public:
    ~Trigger();
    bool name(std::string&) override;
    bool run() override;

  private:
    friend class trigger::Factory;
    explicit Trigger(const config_t&);

    struct Handler;
    std::unique_ptr<Handler> handler;
};

} // namespace trigger::lnx::oneshot
