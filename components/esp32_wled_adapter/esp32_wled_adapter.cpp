#include "esp32_wled_adapter.h"
#include "esphome/core/log.h"
#include "esphome/components/esp32_rmt_led_strip/led_strip.h" // Incluir el encabezado para ESP32RMTLEDStripLightOutput
#include <lwip/sockets.h>
#include <errno.h>

namespace esphome {
namespace esp32_wled_adapter {

static const char* const TAG = "esp32_wled_adapter";

void WLEDUDPComponent::setup() {
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
    return;
  }

  fcntl(this->socket_fd_, F_SETFL, O_NONBLOCK);
}

void WLEDUDPComponent::loop() {
  if (this->socket_fd_ < 0) return;

  auto light_output = this->light_strip_->get_output();
  if (light_output == nullptr) return;

  auto led_strip = static_cast<esp32_rmt_led_strip::ESP32RMTLEDStripLightOutput*>(light_output);
  if (led_strip == nullptr) return;

  uint8_t udp_buffer[2048];
  struct sockaddr_in sender_address;
  socklen_t address_length = sizeof(sender_address);

  int received_bytes = recvfrom(this->socket_fd_, udp_buffer, sizeof(udp_buffer), 0, 
                                (struct sockaddr*)&sender_address, &address_length);
  if (received_bytes < 0) return;

  // Parse received data as RGB triplets
  int led_count = led_strip->size();
  int max_possible_leds = received_bytes / 3;
  int leds_to_update = std::min(led_count, max_possible_leds);

  for (int led_index = 0; led_index < leds_to_update; led_index++) {
    int buffer_position = led_index * 3;
    
    // Obtener una vista directa al LED usando el método específico de ESP32RMTLEDStripLightOutput
    auto color_view = led_strip->get_view_internal(led_index);
    
    // Establecer valores RGB directamente
    *color_view.red = udp_buffer[buffer_position];
    *color_view.green = udp_buffer[buffer_position + 1]; 
    *color_view.blue = udp_buffer[buffer_position + 2];
    
    // Si hay un canal blanco disponible, lo dejamos en 0
    if (color_view.white != nullptr) {
      *color_view.white = 0;
    }
  }
  
  // Programar la actualización de la tira LED
  led_strip->schedule_show();
}

}  // namespace esp32_wled_adapter
}  // namespace esphome
