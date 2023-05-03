# nfc-checkpoints
This repository contains two projects: An NRF Connect application with implemented NFC and peripheral advertising functionalities, and a PlatformIO project that acts as a central that receives packets from the peripheral and outputs callback information regarding what NFC URL had been sent to the mobile device.

Navigate to the folders by accessing the "Projects" folder. You should see the following projects:
- writable-ndef-msg: The NRF Connect application that possesses NFC and ble peripheral advertising functionalities
- ble-central-connect: The PlatformIO application that is capable of detecting the advertisements from the peripheral functionality of the NRF Connect application and outputting callbacks when the NFC tag has been successfully read.
