# Arduino programming
Code to allow programming of the light monitoring device based on Laird Connectivity BL652 (Nordic semi nRF52832) bluetooth module/MCU using the Arduino IDE.
This makes use of the [Adafruit Feather nRF52 Bluefruit LE - nRF52832](https://www.adafruit.com/product/3406) product and the associated Adafruit libaries.

## Setting up enviroment
Prerequisites:
 - [Arduino IDE 1.8.15](https://www.arduino.cc/en/software) (do NOT install Windows Store version) (tested version: 1.8.15)
 - [Git](https://git-scm.com/downloads)
 - [J-Link software V6.88a and V7.20b](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack) (Tested version: V6.88a)
 - [nRF Connect](https://www.nordicsemi.com/Products/Development-tools/nrf-connect-for-desktop/download) (optional)
 - [nrf command line tools](https://www.nordicsemi.com/Products/Development-tools/nRF-Command-Line-Tools/Download?lang=en#infotabs) (Tested version: 10.12.1)
 - nRF52 SDK 17.0.2 (this may not be required)
 - [Adafruit nRF52 by Adafruit Arduino Board Support Package (see below)](https://github.com/adafruit/Adafruit_nRF52_Arduino)


 
 To install the Adafruit nRF52 library follow the following steps:
 1. Go into Preferences
 2. Add https://adafruit.github.io/arduino-board-index/package_adafruit_index.json as an 'Additional Board Manager URL'
 3. Restart the Arduino IDE
 4. Open the Boards Manager from the Tools -> Board menu and install 'Adafruit nRF52 by Adafruit' and once it is installed, select 'Adafruit Feather nRF52832' from the Tools -> Board menu, which will update your system config to use the right compiler and settings for the nRF52.
 
 5. Delete the core folder `nrf52` installed by Board Manager in Adruino15, depending on your OS. It could be
  * macOS  : `~/Library/Arduino15/packages/adafruit/hardware`
  * Linux  : `~/.arduino15/packages/adafruit/hardware`
  * Windows: `%LOCALAPPDATA%\Arduino15\packages\adafruit\hardware`
 6. `cd <SKETCHBOOK>`, where `<SKETCHBOOK>` is your Arduino Sketch folder:
  * macOS  : `~/Documents/Arduino`
  * Linux  : `~/Arduino`
  * Windows: `~/Documents/Arduino`
 7. Create a folder named `hardware/Adafruit`, if it does not exist, and change directories to it
 8. Clone the repo from Adafruit & its submodules: 
 9. `git clone https://github.com/adafruit/Adafruit_nRF52_Arduino.git`
    
    `cd Adafruit_nRF52_Arduino`
    
    `git submodule update --init`
    
    or
    
    `git clone --recurse-submodules https://github.com/adafruit/Adafruit_nRF52_Arduino.git`

 10. Within the repo you downloaded navigate to \variants\feather_nrf52832
 11. Edit the file variant.h and comment out the line #define USE_LFXO 
 12. Change the pin definitions on line 98 - 100 to:
   ```
   #define PIN_SPI_MISO         (24)
   #define PIN_SPI_MOSI         (23)
   #define PIN_SPI_SCK          (25)
   ```
 13. Change the pin defitions on line 112 - 113 to:
   ```
   #define PIN_WIRE_SDA         (26u)
   #define PIN_WIRE_SCL         (27u)
   ```
 14. Uncomment the line #define USE_LFRC (you may need to add in '#'). This sets up the device to use the internal RC oscillator to control the BLE radio
 15. Navigate to `hardware\Adafruit\Adafruit_nRF52_Arduino` and open `programmers.txt` and change line 6 and 7 to:
 
```
nrfjprog.program.cmd=nrfjprog
nrfjprog.program.cmd.windows=nrfjprog.exe
```
 
Next step is to burn the bootloader onto the board. Plug in the board (or development board), you should be able to programme the board using either the dev kit or the dedicated J-LINK. 
Now open the Arduino IDE, go to Tools > Board > Adafruit nRF52 > Adafruit Feather nRF52832
Go to Tools > Port, and select the COM port where your debugger is connected. Note if using the dedicated J-LINK you may need to enable the COM port (J-Link Configurator, right click on device, configure, enable virtual COM port)
To flash the bootloader go to Tools > Burn Bootloader. If this fails try changing settings in Tools > Programmer and switching between J-Link for Bluefruit nRF52 and Bootloader DFU for Bluefruit nRF52
Once this has succeeded you should then be able to flash Arduino files onto the board.

To test, I recommend flashing the controller example. File > Examples > Adafruit Bluefruit nRF52 Libraries > Peripheral > controller
Hit programme. It may fail as you have to put the board in DFU mode. To do this short P0.20/SIO_20/pin 8 to ground, press and release the reset button and then remove the ground short.
Try programming again, if it fails again try switching Tools > Programmer settings as above and putting in DFU mode.

Once successfully programmed with controller example, open serial terminal and download Bluefruit Connect to your phone. Connect to Bluefruit52 > Controller > Control Pad. Press the buttons and you should see the commands appear on the serial terminal on the PC.

# Configuration 

The light monitoring device is developed based on the Adafruit Feather nRF52 Bluefruit LE [nRF52832]. Therefore, in terms of firmware and hardware configurations, it needed to be modified to meet the application requirements and allow the development of firmware based on the Adafruit nRF52 library using Arduino IDE. For example, by default, only one SPI and I2C are active on the Adafruit nRF52 library, although this project needed 2 SPI and 2 I2C according to the hardware routing.
To make the Adafruit nRF52 library ready for firmware development, follow the following steps:

First, set up the environment, then go to:

~\Documents\Arduino\hardware\adafruit\Adafruit_nRF52_Arduino\variants\feather_nrf52832
Open the variant.h with a text editor software (e.g., notepad++) and change the pin definitions on the lines below. Alternatively, you can download the pre-prepared variant.h in this folder and replace the original variant.h:

Defining pins that are different from the nrf52 feather

```
#define PIN_LED2           (11)
#define PIN_A4             (28)
#define PIN_AREF           PIN_A7
#define PIN_VBAT           PIN_A4
#define CHARG_DETECT       (18)
#define SD_DETECT          (17)
```

Adding a second SPI interface

```
#define SPI_INTERFACES_COUNT 2
#define PIN_SPI_MISO         (15)
#define PIN_SPI_MOSI         (0)
#define PIN_SPI_SCK          (13)
static const uint8_t SS    =  1 ;
static const uint8_t MOSI  = PIN_SPI_MOSI ;
static const uint8_t MISO  = PIN_SPI_MISO ;
static const uint8_t SCK   = PIN_SPI_SCK ;
#define PIN_SPI1_MISO        (23)
#define PIN_SPI1_MOSI        (24)
#define PIN_SPI1_SCK         (25)
static const uint8_t SS1   = 22 ;
static const uint8_t MOSI1 = PIN_SPI1_MOSI ;
static const uint8_t MISO1 = PIN_SPI1_MISO ;
static const uint8_t SCK1  = PIN_SPI1_SCK ;
```
Adding a second wire/I2C interface
```
#define WIRE_INTERFACES_COUNT 2
#define PIN_WIRE_SDA         (2u)
#define PIN_WIRE_SCL         (4u)
#define PIN_WIRE1_SDA        (29u)
#define PIN_WIRE1_SCL        (30u)
```
Navigate to `~\Documents\Arduino\hardware\adafruit\Adafruit_nRF52_Arduino\libraries\SPI` and open `SPI_nrf52832.cpp` with a source code editor and add the line below after `SPIClass SPI(NRF_SPIM2,  PIN_SPI_MISO,  PIN_SPI_SCK,  PIN_SPI_MOSI);` (line 227):

```
SPIClass SPI1(NRF_SPIM2,  PIN_SPI1_MISO,  PIN_SPI1_SCK,  PIN_SPI1_MOSI);
```

Navigate to `~\Documents\Arduino\hardware\adafruit\Adafruit_nRF52_Arduino\libraries\Bluefruit52Lib\src` and open `bluefruit.cpp` with a source code editor and comment line 799 (`_setConnLed(true);`) to turn off the LED during the light measurement.

In terms of hardware configuration, due to space constraints on the board, it is necessary to use a programming adaptor as the programming and UART headers are 1.27mm through hole micro terminal strips. Also, in addition to jtag pins, UART pins need to connect to the j-link for debugging/programming the Bl652 Bluetooth module/MCU.

The main firmware features are including:

•	Automatic gain control (AGC)

•	Date/time stamp

•	Real-time battery state of charge (SoC)

•	MicroSD detection

•	Precision timing

•	Command-based communication with smart devices (e.g., smartphones)

•	Auto-checking peripherals (light sensor, temp sensor, microSD card, accelerometer)

•	Saving data on a comma-separated values (CSV) file

•	Auto set date/time using a companion android app (customised Bluefruit LE Connect)

•	Built-in calibration for ADC raw values

•	Saving measurement files over BLE or USB
