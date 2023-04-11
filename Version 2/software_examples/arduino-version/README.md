# Arduino programming
Code to allow programming of the light sensor board/nRF52832 chipset using the Arduino IDE.
This makes use of the [Adafruit Feather nRF52 Bluefruit LE - nRF52832](https://www.adafruit.com/product/3406) product and the associated Adafruit libaries

## Setting up enviroment
Prerequisites:
 - Arduino IDE 1.8.13 (do NOT install Windows Store version)
 - [J-Link software V6.88](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack)
 - nRF Connect
 - nRF52 SDK 17.0.2 (this may not be required)
 - [Adafruit nRF52 by Adafruit Arduino Board Support Package (see below)](https://github.com/adafruit/Adafruit_nRF52_Arduino)
 
 To install the Adafruit nRF52 library follow the following steps:
 1. Go into Preferences
 2. Add https://www.adafruit.com/package_adafruit_index.json as an 'Additional Board Manager URL'
 3. Restart the Arduino IDE
 4. Open the Boards Manager from the Tools -> Board menu and install 'Adafruit nRF52 by Adafruit'
 5. Delete the core folder `nrf52` installed by Board Manager in Adruino15, depending on your OS. It could be
  * macOS  : `~/Library/Arduino15/packages/adafruit/hardware`
  * Linux  : `~/.arduino15/packages/adafruit/hardware`
  * Windows: `%LOCALAPPDATA%\Arduino15\packages\adafruit\hardware`
 6. `cd <SKETCHBOOK>`, where `<SKETCHBOOK>` is your Arduino Sketch folder:
  * macOS  : `~/Documents/Arduino`
  * Linux  : `~/Arduino`
  * Windows: `~/Documents/Arduino`
 7. Create a folder named `hardware/Adafruit`, if it does not exist, and change directories to it
 8. Clone the repo from Adafruit & its submodules: `git clone --recurse-submodules https://github.com/adafruit/Adafruit_nRF52_Arduino.git`
 9. Within the repo you downloaded navigate to \variants\feather_nrf52832
 10. Edit the file variant.h and comment out the line #define USE_LFXO 
 11. Change the pin definitions on line 98 - 100 to:
   ```
   #define PIN_SPI_MISO         (24)
   #define PIN_SPI_MOSI         (23)
   #define PIN_SPI_SCK          (25)
   ```
 12. Change the pin defitions on line 112 - 113 to:
   ```
   #define PIN_WIRE_SDA         (26u)
   #define PIN_WIRE_SCL         (27u)
   ```
 14. Uncomment the line #define USE_LFRC (you may need to add in '#'). This sets up the device to use the internal RC oscillator to control the BLE radio
 
Next step is to burn the bootloader onto the board. Plug in the board (or development board), you should be able to programme the board using either the dev kit or the dedicated J-LINK. 
Now open the Arduino IDE, go to Tools > Board > Adafruit nRF52 > Adafruit Feather nRF52832
Go to Tools > Port, and select the COM port where your debugger is connected. Note if using the dedicated J-LINK you may need to enable the COM port (J-Link Configurator, right click on device, configure, enable virtual COM port)
To flash the bootloader go to Tools > Burn Bootloader. If this fails try changing settings in Tools > Programmer and switching between J-Link for Bluefruit nRF52 and Bootloader DFU for Bluefruit nRF52
Once this has succeeded you should then be able to flash Arduino files onto the board.

To test, I recommend flashing the controller example. File > Examples > Adafruit Bluefruit nRF52 Libraries > Peripheral > controller
Hit programme. It may fail as you have to put the board in DFU mode. To do this short P0.20/SIO_20/pin 8 to ground, press and release the reset button and then remove the ground short.
Try programming again, if it fails again try switching Tools > Programmer settings as above and putting in DFU mode.

Once successfully programmed with controller example, open serial terminal and download Bluefruit Connect to your phone. Connect to Bluefruit52 > Controller > Control Pad. Press the buttons and you should see the commands appear on the serial terminal on the PC.
