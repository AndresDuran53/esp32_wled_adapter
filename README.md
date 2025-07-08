# ESPHome ESP32 WLED Adapter

This component allows an ESPHome device to receive control data for addressable LED strips via the UDP protocol compatible with WLED.

## Installation

To use this external component in your ESPHome configuration, add the following to your `yaml` file:

```yaml
external_components:
  - source: github://AndresDuran53/esp32_wled_adapter@main
    components: [esp32_wled_adapter]

# Component configuration
esp32_wled_adapter:
  port: 21324  # UDP port (default for WLED is 21324)
  light_id: my_addressable_light  # ID of your LED strip

# Your LED strip
light:
  - platform: neopixelbus
    id: my_addressable_light
    type: GRB
    pin: GPIO2
    num_leds: 60
    name: "My LED Strip"
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
