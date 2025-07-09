#include "esp32_wled_adapter.h"
#include "esphome/core/log.h"
#include "esphome/components/esp32_rmt_led_strip/led_strip.h"
#include <lwip/sockets.h>
#include <errno.h>
#include <unistd.h>

namespace esphome {
namespace esp32_wled_adapter {

static const char* const TAG = "esp32_wled_adapter";

WLEDUDPComponent::~WLEDUDPComponent() {
  if (this->socket_fd_ >= 0) {
    close(this->socket_fd_);
    this->socket_fd_ = -1;
  }
}

void WLEDUDPComponent::setup() {
  ESP_LOGI(TAG, "WLEDUDPComponent setup() called");
  // No socket creation here, will be done in loop()
}

void WLEDUDPComponent::open_udp_socket_() {
  if (this->socket_fd_ >= 0) {
    close(this->socket_fd_);
    this->socket_fd_ = -1;
  }
  ESP_LOGI(TAG, "Setting up UDP listener on port %d", this->port_);
  this->socket_fd_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (this->socket_fd_ < 0) {
    ESP_LOGE(TAG, "Failed to create socket: errno %d", errno);
    return;
  }
  struct sockaddr_in listen_address;
  listen_address.sin_addr.s_addr = htonl(INADDR_ANY);
  listen_address.sin_family = AF_INET;
  listen_address.sin_port = htons(this->port_);
  if (bind(this->socket_fd_, (struct sockaddr*)&listen_address, sizeof(listen_address)) < 0) {
    ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
    close(this->socket_fd_);
    this->socket_fd_ = -1;
    return;
  }
  fcntl(this->socket_fd_, F_SETFL, O_NONBLOCK);
}

void WLEDUDPComponent::loop() {
  // If the socket is not open, try to open it
  if (this->socket_fd_ < 0) {
    this->open_udp_socket_();
  }
  if (this->socket_fd_ < 0) return;

  // Get the LED strip output
  auto light_output = this->light_strip_->get_output();
  if (light_output == nullptr) return;

  // Get the addressable light from the esp32_rmt_led_strip output
  auto addressable_light = static_cast<light::AddressableLight*>(light_output);
  if (addressable_light == nullptr) return;

  uint8_t udp_buffer[2048];
  struct sockaddr_in sender_address;
  socklen_t address_length = sizeof(sender_address);

  // Receive UDP data
  int received_bytes = recvfrom(this->socket_fd_, udp_buffer, sizeof(udp_buffer), 0, 
                                (struct sockaddr*)&sender_address, &address_length);

  // If there is a serious error, close the socket to try to reopen it in the next loop
  if (received_bytes < 0 && errno != EWOULDBLOCK && errno != EAGAIN) {
    ESP_LOGE(TAG, "Socket error on recvfrom: errno %d", errno);
    close(this->socket_fd_);
    this->socket_fd_ = -1;
    return;
  }
  if (received_bytes <= 0) return;

  // Calculate how many LEDs can be updated
  int led_count = addressable_light->size();
  int data_offset = 0;

  // Detect UDP packet type
  // WLED packets start with 0x01 and have a specific header format
  // 0xE0 indicates a WLED packet with RGB data and 0xF0 indicates the packet type
  if (received_bytes >= 2 && udp_buffer[0] == 0x01 && (udp_buffer[1] == 0xE0 && ((received_bytes - 2) % 3 == 0))) {
    ESP_LOGD(TAG, "Received WLED UDP packet");
    data_offset = 2;  // Skip header
  } else {
    ESP_LOGD(TAG, "Received raw UDP packet");
    data_offset = 0;  // No header to skip
  }

  int leds_received = (received_bytes - data_offset) / 3;
  int leds_to_update = std::min(led_count, leds_received);

  // Update LEDs with received RGB data
  for (int led_index = 0; led_index < leds_to_update; led_index++) {
    int buffer_position = data_offset + (led_index * 3);
    auto pixel = addressable_light->get(led_index);
    pixel.set(Color(
      udp_buffer[buffer_position],
      udp_buffer[buffer_position + 1],
      udp_buffer[buffer_position + 2]
    ));
  }

  // Schedule LED strip update
  addressable_light->schedule_show();
}

}  // namespace esp32_wled_adapter
}  // namespace esphome