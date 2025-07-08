#include "esp32_wled_adapter.h"
#include "esphome/core/log.h"
#include <lwip/sockets.h>
#include <errno.h>

namespace esphome {
namespace esp32_wled_adapter {

static const char* const TAG = "esp32_wled_adapter";

void WLEDUDPComponent::setup() {
  ESP_LOGI(TAG, "Setting up UDP listener on port %d", this->port_);

  this->sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (this->sock_ < 0) {
    ESP_LOGE(TAG, "Failed to create socket: errno %d", errno);
    return;
  }

  struct sockaddr_in dest_addr;
  dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(this->port_);

  if (bind(this->sock_, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
    ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
    close(this->sock_);
    return;
  }

  fcntl(this->sock_, F_SETFL, O_NONBLOCK);
}

void WLEDUDPComponent::loop() {
  if (this->sock_ < 0) return;

  uint8_t recv_buf[2048];
  struct sockaddr_in source_addr;
  socklen_t socklen = sizeof(source_addr);

  int len = recvfrom(this->sock_, recv_buf, sizeof(recv_buf), 0, (struct sockaddr*)&source_addr, &socklen);
  if (len < 0) return;

  // Parse received data as RGB triplets
  int num_leds = this->strip_->size();
  int max_leds = len / 3;
  int leds_to_update = (num_leds < max_leds) ? num_leds : max_leds;

  for (int i = 0; i < leds_to_update; i++) {
    int idx = i * 3;
    Color c(recv_buf[idx], recv_buf[idx+1], recv_buf[idx+2]);
    this->strip_->set_pixel(i, c);
  }
  this->strip_->schedule_show();
}

}  // namespace esp32_wled_adapter
}  // namespace esphome
