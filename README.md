# ESPHome ESP32 WLED Adapter

This component allows an ESPHome device to receive control data for addressable LED strips via the UDP protocol compatible with WLED.

## Installation

To use this external component in your ESPHome configuration, add the following to your `yaml` file:

```yaml
external_components:
  source:
    type: git
    url: https://github.com/AndresDuran53/esp32_wled_adapter


# Component configuration
esp32_wled_adapter:
  port: 21324  # Standard WLED UDP port
  light_id: led_strip

# Your LED strip
light:
  - platform: esp32_rmt_led_strip
    id: led_strip
    name: "Strip Lights"
    chipset: ws2811
    pin: GPIO47
    num_leds: 61
    rgb_order: GRB
```

## Usage

Once configured, the component will create a UDP server that listens on the specified port. 
Any device that sends data in RGB format (3 bytes per LED) to the ESP32/ESP8266 will be able to control the LED strip.

This component is compatible with:
- WLED applications
- Other controllers that send RGB data via UDP
- Music/video light synchronization software

## Configuration Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| port      | int  | Yes      | UDP port to listen on |
| light_id  | id   | Yes      | ID of the addressable LED strip |

## License

This project is licensed under the MIT License.
