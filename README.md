# ESP32 CAN Web Sniffer

A web-based CAN bus sniffer using the ESP32 microcontroller. This project allows you to monitor and analyze CAN bus traffic via a web interface.

## Features

- Real-time CAN bus data capture
- Web interface for live monitoring
- Filter and search CAN messages
- Data logging and export options

## Hardware Recommended

 - [IOXESP32 (ESP32 Dev Board)](https://www.artronshop.co.th/p/59) + [IOXESP32 CAN bus shield](https://www.artronshop.co.th/p/26) - Connect the A, B wires manually.
 - [ESP-OBD2](https://www.artronshop.co.th/p/733) - ESP32 with OBD2 connector (CAN bus) connect it to your car then ready to sniffer !

## Software Requirements

- PlatformIO
- ESP32 board support
- Required libraries: `ESPAsyncWebServer`, `AsyncTCP`

## Setup

1. Connect the CAN transceiver to the ESP32 according to your hardware.
2. Clone this repository:
    ```bash
    git clone https://github.com/yourusername/ESP32-CAN-Web-Sniffer.git
    ```
3. Open the project in your IDE (PlatformIO).
4. Upload the firmware to your ESP32.
5. To run tests (not request ESP32 hardware), execute the following command in the project directory:
    ```bash
    node test.js
    ```
6. After running the tests, go to [http://localhost:3000](http://localhost:3000) in your web browser to start the test.

- Power on the ESP32 and connect to the ESP32 over WiFi with name `ESP32-CAN`.
- Open a web browser and navigate to the ESP32's IP address 192.168.4.1
- View and analyze CAN messages in real time.

## License

This project is licensed under the MIT License.

