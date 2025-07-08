#pragma once

#include "esphome/core/component.h"
#include "esphome/components/light/addressable_light.h"
#include "esphome/components/network/util.h"
#include <vector>

namespace esphome {
namespace esp32_wled_adapter {

class WLEDUDPComponent : public Component {
 public:
  void set_strip(light::AddressableLight* strip) { this->strip_ = strip; }
  void set_port(uint16_t port) { this->port_ = port; }

  void setup() override;
  void loop() override;

 protected:
  light::AddressableLight* strip_;
  int sock_;
  uint16_t port_;
  std::vector<uint8_t> buffer_;
};

}  // namespace esp32_wled_adapter
}  // namespace esphome
