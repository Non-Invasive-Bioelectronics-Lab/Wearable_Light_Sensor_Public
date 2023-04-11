/*********************************************************************
 This is an example for our nRF52 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/
#include <bluefruit.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#include <Adafruit_AS7341.h>

// BLE Service
BLEDfu  bledfu;  // OTA DFU service
BLEDis  bledis;  // device information
BLEUart bleuart; // uart over ble
BLEBas  blebas;  // battery

Adafruit_AS7341 as7341;

void setup()
{
  Serial.begin(115200);

#if CFG_DEBUG
  // Blocking wait for connection when debug mode is enabled via IDE
  while ( !Serial ) yield();
#endif
  
  Serial.println("Bluefruit52 BLEUART Example");
  Serial.println("---------------------------\n");

  // Setup the BLE LED to be enabled on CONNECT
  // Note: This is actually the default behavior, but provided
  // here in case you want to control this LED manually via PIN 19
  Bluefruit.autoConnLed(true);

  // Config the peripheral connection with maximum bandwidth 
  // more SRAM required by SoftDevice
  // Note: All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  Bluefruit.setName("NIBS Light Sensor");
  //Bluefruit.setName(getMcuUniqueID()); // useful testing with multiple central connections
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // To be consistent OTA DFU should be added first if it exists
  bledfu.begin();

  // Configure and Start Device Information Service
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();

  // Configure and Start BLE Uart Service
  bleuart.begin();

  // Start BLE Battery Service
  blebas.begin();
  blebas.write(100);

  // Set up and start advertising
  startAdv();

  Serial.println("Please use Adafruit's Bluefruit LE app to connect in UART mode");
  Serial.println("Once connected, enter character(s) that you wish to send");

  if (!as7341.begin()){
    Serial.println("Could not find AS7341");
    while (1) { delay(10); }
  }

  as7341.setATIME(100);
  as7341.setASTEP(999);
  as7341.setGain(AS7341_GAIN_256X);

  Serial.println("Configured AS7341");
  
}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void loop()
{
  // Forward data from HW Serial to BLEUART
  while (Serial.available())
  {
    // Delay to wait for enough input, since we have a limited transmission buffer
    delay(2);

    uint8_t buf[64];
    int count = Serial.readBytes(buf, sizeof(buf));
    bleuart.write( buf, count );
  }

  // Forward from BLEUART to HW Serial
  while ( bleuart.available() )
  {
    uint8_t ch;
    ch = (uint8_t) bleuart.read();
    Serial.write(ch);
    if (ch == 'r'){
      send_data();  
    }
    if (ch == 's'){
      
    }
   }
}

void set_integration(uint16_t atime){
  as7341.setATIME(100);
  as7341.setASTEP(999);
  as7341.setGain(AS7341_GAIN_256X);
}

void set_step(uint16_t astep){
  
}

void set_gain(uint16_t gain){
  
}

void send_data(void)
{
  Serial.println("");
  Serial.println("Data requested");
  if (!as7341.readAllChannels()){
    Serial.println("Error reading all channels!");
    return;
  }
  // Declare variables
  uint16_t data;
  String str_data;
  char buf[64]; 
     
  // Print out the stored values for each channel
  Serial.print("F1 415nm : ");
  Serial.println(as7341.getChannel(AS7341_CHANNEL_415nm_F1));
  bleuart.write("F1 415nm : ", 11);
  data = as7341.getChannel(AS7341_CHANNEL_415nm_F1);
  str_data = String(data);
  // Convert the string to an array of chars and store in buf
  str_data.toCharArray(buf, 6);
  bleuart.write(buf, 5);
  bleuart.write("\n", 1);

  Serial.print("F2 445nm : ");
  Serial.println(as7341.getChannel(AS7341_CHANNEL_445nm_F2));
  bleuart.write("F2 445nm : ", 11);
  data = as7341.getChannel(AS7341_CHANNEL_445nm_F2);
  str_data = String(data);
  // Convert the string to an array of chars and store in buf
  str_data.toCharArray(buf, 6);
  bleuart.write(buf, 5);
  bleuart.write("\n", 1);
 
  Serial.print("F3 480nm : ");
  Serial.println(as7341.getChannel(AS7341_CHANNEL_480nm_F3));
  bleuart.write("F3 480nm : ", 11);
  data = as7341.getChannel(AS7341_CHANNEL_480nm_F3);
  str_data = String(data);
  // Convert the string to an array of chars and store in buf
  str_data.toCharArray(buf, 6);
  bleuart.write(buf, 5);  
  bleuart.write("\n", 1);
  
  Serial.print("F4 515nm : ");
  Serial.println(as7341.getChannel(AS7341_CHANNEL_515nm_F4));
  bleuart.write("F4 515nm : ", 11);
  data = as7341.getChannel(AS7341_CHANNEL_515nm_F4);
  str_data = String(data);
  // Convert the string to an array of chars and store in buf
  str_data.toCharArray(buf, 6);
  bleuart.write(buf, 5);  
  bleuart.write("\n", 1);
  
  Serial.print("F5 555nm : ");
  Serial.println(as7341.getChannel(AS7341_CHANNEL_555nm_F5));
  bleuart.write("F5 555nm : ", 11);
  data = as7341.getChannel(AS7341_CHANNEL_555nm_F5);
  str_data = String(data);
  // Convert the string to an array of chars and store in buf
  str_data.toCharArray(buf, 6);
  bleuart.write(buf, 5);  
  bleuart.write("\n", 1);
  
  Serial.print("F6 590nm : ");
  Serial.println(as7341.getChannel(AS7341_CHANNEL_590nm_F6));
  bleuart.write("F6 590nm : ", 11);
  data = as7341.getChannel(AS7341_CHANNEL_590nm_F6);
  str_data = String(data);
  // Convert the string to an array of chars and store in buf
  str_data.toCharArray(buf, 6);
  bleuart.write(buf, 5);
  bleuart.write("\n", 1);  
  
  Serial.print("F7 630nm : ");
  Serial.println(as7341.getChannel(AS7341_CHANNEL_630nm_F7));
  bleuart.write("F7 630nm : ", 11);
  data = as7341.getChannel(AS7341_CHANNEL_630nm_F7);
  str_data = String(data);
  // Convert the string to an array of chars and store in buf
  str_data.toCharArray(buf, 6);
  bleuart.write(buf, 5);  
  bleuart.write("\n", 1);
  
  Serial.print("F8 680nm : ");
  Serial.println(as7341.getChannel(AS7341_CHANNEL_680nm_F8));
  bleuart.write("F8 680nm : ", 11);
  data = as7341.getChannel(AS7341_CHANNEL_680nm_F8);
  str_data = String(data);
  // Convert the string to an array of chars and store in buf
  str_data.toCharArray(buf, 6);
  bleuart.write(buf, 5);
  bleuart.write("\n", 1);  

  Serial.print("Clear    : ");
  Serial.println(as7341.getChannel(AS7341_CHANNEL_CLEAR));
  bleuart.write("Clear    : ", 11);
  data = as7341.getChannel(AS7341_CHANNEL_CLEAR);
  str_data = String(data);
  // Convert the string to an array of chars and store in buf
  str_data.toCharArray(buf, 6);
  bleuart.write(buf, 5);
  bleuart.write("\n", 1);  

  Serial.print("NIR    : ");
  Serial.println(as7341.getChannel(AS7341_CHANNEL_NIR));
  bleuart.write("NIR    : ", 11);
  data = as7341.getChannel(AS7341_CHANNEL_NIR);
  str_data = String(data);
  // Convert the string to an array of chars and store in buf
  str_data.toCharArray(buf, 6);
  bleuart.write(buf, 5);  
  bleuart.write("\n", 1);

  Serial.println("");
}


// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
}
