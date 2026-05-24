# GrowthPulse IoT System

Welcome to the GrowthPulse project repository. GrowthPulse is an IoT plant health and monitoring system utilizing ESP32 hardware and LoRaWAN protocols to track soil moisture, temperature, and other environmental metrics.

## 📂 Repository Structure

To keep our development environment clean and compatible with the Arduino IDE, please adhere to the following folder structure:

    GrowthPulse/
    ├── docs/                # Project documentation, risk matrices, and system diagrams
    ├── firmware/            # Production Arduino code (What actually runs on the ESP32s)
    │   ├── gateway/         # Code for the ESP32 LoRa-to-Wi-Fi gateway (gateway.ino)
    │   ├── node/            # Code for the ESP32 sensor nodes (node.ino)
    │   ├── lib/             # Custom, reusable project libraries (e.g., sensor drivers)
    │   └── common/          # Shared headers or constants used by both node and gateway
    ├── hardware/            # Schematics, PCB designs, and wiring diagrams
    ├── poc_stub/            # Experimental code, sandboxes, and proof-of-concept stubs
    ├── scripts/             # Utility/automation scripts (e.g., Python data viz, build scripts)
    └── README.md            # This file

## 🛠️ Development Guidelines

When contributing to this repository, please follow these rules to ensure everything compiles correctly:

### 1. Where does my code go?
* **Production Firmware:** If you are writing the main logic for the sensor node or the gateway, it goes in `firmware/node/` or `firmware/gateway/` respectively. 
* **Custom Libraries:** If you write a reusable class or driver (e.g., a specific LoRa helper function), put it in `firmware/lib/`.
* **Experimental Code:** If you are testing a new sensor or trying out a library but it isn't ready for the main system, put it in `poc_stub/`. **Do not put test sketches in the `firmware/` directory.**

### 2. Arduino IDE Compatibility
Arduino requires the main sketch file (`.ino`) to have the exact same name as the folder it resides in. 
* To work on the node, open `firmware/node/node.ino`
* To work on the gateway, open `firmware/gateway/gateway.ino`

If you are adding `.cpp` or `.h` files specifically for the node, keep them inside the `firmware/node/` folder alongside `node.ino`.

### 3. Gateway Limitations (Important Architecture Note)
Because our ESP32 gateway acts as a single-channel LoRa gateway (unlike commercial 8-channel gateways):
* Frequency hopping must be disabled on the nodes.
* Node transmissions must be staggered to prevent packet collisions at the gateway.

## 🚀 Getting Started
1. Clone this repository to your local machine.
2. Open the Arduino IDE (or PlatformIO).
3. Navigate to either the `node` or `gateway` folder depending on the hardware you are flashing.
4. Ensure you have the necessary board managers (ESP32) and third-party libraries installed before compiling.