# ESP32 BLE controller/receiver library

> [!CAUTION]
> Right now the library has only been tested in a specific application and decoding the HID message is necessary for generalized use.

The controller side is a restricted wrapper of the [ESP32-BLE-Gamepad](https://github.com/lemmingDev/ESP32-BLE-Gamepad) repo and the receiver side is a custom [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) code to interact specifically with ESP32-BLE-Gamepad controllers.
