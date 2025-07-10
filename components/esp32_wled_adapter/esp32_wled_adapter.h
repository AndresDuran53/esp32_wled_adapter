#pragma once

#include "esphome/core/component.h"
#include "esphome/components/light/addressable_light.h"
#include "esphome/components/light/light_state.h"
#include "esphome/components/network/util.h"
#include <vector>

namespace esphome {
namespace esp32_wled_adapter {

class WLEDUDPComponent : public Component {
 public:
  bool is_effect_active = false;

  void set_strip(light::AddressableLightState* light_strip) { this->light_strip_ = light_strip; }
  void set_port(uint16_t port) { this->port_ = port; }
  void set_effect_active(bool active) { this->is_effect_active = active; }

  void setup() override;
  void loop() override;
  ~WLEDUDPComponent();

 protected:
  void open_udp_socket_();
  light::AddressableLightState* light_strip_{nullptr};
  int socket_fd_{-1};
  uint16_t port_{0};
  uint32_t effect_end_time_{0};
  bool waiting_udp_transition{false};
};

}  // namespace esp32_wled_adapter
}  // namespace esphome
