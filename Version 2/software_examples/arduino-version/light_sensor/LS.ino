#include <bluefruit.h>
#include <Adafruit_AS7341.h>
#include <LibPrintf.h>
#include <SPI.h>
#include <SD.h>
#include "Time.h"
#include "TimeLib.h"
#include <Wire.h>
#include <ADXL362.h>


// BLE Service
//BLEDfu  bledfu;  // OTA DFU service
BLEDis  bledis;  // device information
BLEUart bleuart; // uart over ble

TwoWire *wi = &Wire1;
ADXL362 ACC; 
Adafruit_AS7341 as7341;
uint8_t light_status;
uint8_t temp_status;
bool Connected = false;
bool sendData = false;
bool state = false;
bool ACC_failed = false;
float correctionGain [12] = {1.06, 1.05, 1.05, 1.05, 1.02, 1.02, 1.01, 1.00, 1.00, 0.99, 0.96};
float correctionFactor [12] = {1,1,1,1,1,1,1,1,1,1,1};
float offsetValues [12] = {0,0,0,0,0,0,0,0,0,0,0};
uint16_t readings[12];
bool timer_start = false;
uint8_t sd_first;
unsigned int A_gain;
uint8_t currentGain = 5;
uint8_t maxGain = 10;
bool RawValueSaturated = false;
bool RawValueNoise = false;
bool iPhone_device = false;
uint8_t newGain = 0;
char central_name[32] = { 0 };
unsigned int A_int;
uint16_t ch[12];
const int chipSelect = 1;
char dataStr[220];
char dataStr1[170];
char num_sam[11];
char filename[13] = "LS9_1.csv";
char num[5];
char timeStr[19] = "";
int acc_status = 0;
char SoC_buf[25];
char time_cent[22];
char setting[35];
volatile bool event = false; 
bool st_check = false;
bool dis_check = true;
bool TS_Failed = false;
char *col_names[] = {"Date", "Time","SoC","Temp", "Gain", "F1", "F2", "F3",
                         "F4", "F5", "F6", "F7" , "F8" , "Clear", "NIR", "accX" , "accY", "accZ",
                        };
char *channel_names[13] = {"F1", "F2", "F3",
                         "F4"," "," ", "F5", "F6", "F7" , "F8" , "Clr", "NIR"
                        };
float corr_counts[12];
char data_buf[25];
int j = 1;
uint8_t vbat_per;
String yearr;
int finder;
String date_parse;
char * str;
char * str_cent;
bool s_time;
uint32_t vbat_pin = A4;
time_t timeNow;
uint8_t presecond;         
#define VBAT_MV_PER_LSB   (0.73242188F)   // 3.0V ADC range and 12-bit ADC resolution = 3000mV/4096
#define VBAT_DIVIDER      (0.71387696F)   // 2M(499) + 0.806M (200k) voltage divider on VBAT = (2M / (0.806M + 2M))
#define VBAT_DIVIDER_COMP (1.400F)        // Compensation factor for the VBAT divider
#define WAKE_HIGH_PIN CHARG_DETECT
#define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)

void configBLE()
{
  // Setup the BLE LED to be enabled on CONNECT
  // Note: This is actually the default behavior, but provided

  
  // here in case you want to control this LED manually via PIN 11
  
  Bluefruit.autoConnLed(true);
  
  // Config the peripheral connection with maximum bandwidth
  // more SRAM required by SoftDevice
  // Note: All config***() function must be called before begin() 
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

  Bluefruit.begin();
  // TX power -40, -20, -16, -12, -8, -4, 0, 3, 4
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  Bluefruit.setName("NIBS LS9");
  //Bluefruit.setName(getMcuUniqueID()); // useful testing with multiple central connections
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // To be consistent OTA DFU should be added first if it exists
  //bledfu.begin();

  // Configure and Start Device Information Service
  bledis.setManufacturer("NIBS");
  bledis.setModel("LS v3.0");
  bledis.begin();
}
void configBLEUart() {
  // Configure and Start BLE Uart Service
  bleuart.begin();
}
void configAdv() {
  // Set up and start advertising
  startAdv();
}

void set_gain(uint8_t gain) {
  switch (gain) {
  case 0:
  as7341.setGain(AS7341_GAIN_0_5X);
    break;
  case 1:
  as7341.setGain(AS7341_GAIN_1X);
    break;
  case 2:
  as7341.setGain(AS7341_GAIN_2X);
    break;
  case 3:
  as7341.setGain(AS7341_GAIN_4X);
    break;
  case 4:
  as7341.setGain(AS7341_GAIN_8X);
    break;
  case 5:
  as7341.setGain(AS7341_GAIN_16X);
    break;
  case 6:
  as7341.setGain(AS7341_GAIN_32X);
    break;
  case 7:
  as7341.setGain(AS7341_GAIN_64X);
    break;
  case 8:
  as7341.setGain(AS7341_GAIN_128X);
    break;
  case 9:
  as7341.setGain(AS7341_GAIN_256X);
    break;
  case 10:
  as7341.setGain(AS7341_GAIN_512X);
    break;
  }
}
void default_set() {
  as7341.setATIME(0);
  as7341.setASTEP(65466);
  set_gain(currentGain);
}
void find_gain() {
  as7341_gain_t gain = as7341.getGain();
  switch (gain) {
  case AS7341_GAIN_0_5X:
  A_gain = 0;
    break;
  case AS7341_GAIN_1X:
    A_gain = 1;
    break;
  case AS7341_GAIN_2X:
    A_gain = 2;
    break;
  case AS7341_GAIN_4X:
    A_gain = 4;
    break;
  case AS7341_GAIN_8X:
    A_gain = 8;
    break;
  case AS7341_GAIN_16X:
    A_gain = 16;
    break;
  case AS7341_GAIN_32X:
    A_gain = 32;
    break;
  case AS7341_GAIN_64X:
    A_gain = 64;
    break;
  case AS7341_GAIN_128X:
    A_gain = 128;
    break;
  case AS7341_GAIN_256X:
    A_gain = 256;
    break;
  case AS7341_GAIN_512X:
    A_gain = 512;
    break;
  }
}

void AGC(){
uint8_t saturationGain = 11;
if (currentGain > maxGain){
currentGain = maxGain;
}

while (true){
  uint16_t AGC_readings[12];
  if (!as7341.readAllChannels(AGC_readings)){
    //Serial.println("Error reading all channels!");
    return;
  } 
     for(uint8_t i = 0; i < 12; i++) {
    if(i == 4 || i == 5) continue;
    if (AGC_readings[i] >= 65467) {
      RawValueSaturated = true;
    } 
    if (AGC_readings[i] <= 10000) {
      RawValueNoise = true;  
    } 
}
  if(RawValueSaturated){
    if (currentGain == 0){
      RawValueSaturated = false;
    break;
    }
   
    if (currentGain < saturationGain){
    saturationGain = currentGain;
    }
    currentGain--;
    set_gain(currentGain);
    RawValueSaturated = false;
  }
else if (RawValueNoise){
  if (currentGain == maxGain){
    RawValueNoise = false;
    break;
    }
      newGain = ((maxGain + currentGain) / 2.0 + 0.5);
      if (newGain == currentGain)
      {
      newGain++;
      }

      if (newGain >= saturationGain){
        RawValueNoise = false;
      break;
      }
    currentGain = newGain;
    set_gain(currentGain);
    RawValueNoise = false;
} 
}
}


void configAS7431() {
  if (!as7341.begin()) {
    //Serial.println("Could not find AS7341");
    while (1) {
      delay(10);
    }
  }
  default_set();
}

void configTimer() {
  // Configure TIMER2 which controls light sensor sampling (TIMER0 is used by softdevice)
  NRF_TIMER2->TASKS_STOP = 1;                         // Stop timer
  NRF_TIMER2->MODE = TIMER_MODE_MODE_Timer;           // Set timer to timer mode
  NRF_TIMER2->TASKS_CLEAR = 1;                        // Clear the task first so it's usable later
  NRF_TIMER2->PRESCALER = 9;                          // Prescaler value maximum is 9, f_tick = 16MHz / 2 ^ n
  NRF_TIMER2->BITMODE = TIMER_BITMODE_BITMODE_24Bit;  // Set 16 bit timer resolution
  NRF_TIMER2->CC[0] = 31250 * 30;                      // Set value for compare first compare register (f_ticks * CC)
  // NRF_TIMER2->CC[1] = 65535;                       // Can set up to 6 compare registers
  NRF_TIMER2->INTENSET = TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos;          // Enable COMAPRE0 Interrupt
  NRF_TIMER2->SHORTS = (TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos);  // Count then Complete mode enabled
  NVIC_EnableIRQ(TIMER2_IRQn);                        // Enable interrupts
  //NRF_TIMER2->TASKS_START = 1;                        // Start timer
}



void setup(){ 
  nrf_gpio_cfg(30, NRF_GPIO_PIN_DIR_INPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_S0D1, NRF_GPIO_PIN_NOSENSE);
  nrf_gpio_cfg(29, NRF_GPIO_PIN_DIR_INPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_S0D1, NRF_GPIO_PIN_NOSENSE);
  nrf_gpio_cfg_input(18,NRF_GPIO_PIN_NOPULL);
  Serial.begin(115200);
  pinMode(SD_DETECT, INPUT);
  Wire1.begin();
  configBLE();
  configBLEUart();
  configAdv();
  configAS7431();
  configTimer();
  pinMode(4, OUTPUT);
  for (int i = 0; i < 18; i++) {
  strcat(dataStr1, col_names[i]);
  strcat( dataStr1, ", ");
  s_time = false;
  }
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
     - Enable auto advertising if disconnected
     - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
     - Timeout for fast mode is 30 seconds
     - Start(timeout) with timeout = 0 will advertise forever (until connected)

     For recommended advertising interval
     https://developer.apple.com/library/content/qa/qa1931/_index.html
  */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(2);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
}

void loop()
{
   
   if(Connected && s_time == false){
   init_set_time();
   }
   
  

    
   
  if(state){
    Bluefruit.Advertising.start(2);
  }


  
  unsigned long startloop = millis();
  if(state == false && Connected){
  
  bleuart.write("Send ""s/"" to start the measurement\n", 36);
  delay(4000);
  }
  if(Connected){
  read_cent();
  }
  if (sendData && state) {
  sendData = false;
  enable_per();
  send_data();
  //Serial.println("after send_data");
  }
  check_batt();
if(state){
  
  unsigned long datasend = millis();
  unsigned long Millis = datasend - startloop;
  if(Millis <= 2000){
  uint32_t delaytime = 2000 - (int)Millis;
  delay(delaytime);
  
  
  
  }
  else{
    
  }
  unsigned long endloop = millis();
  unsigned long Millisend = endloop - startloop;
  if(Millisend == 1999){
    delay(1);
  }
    else{
    
  }
 
  unsigned long endlooop = millis();
  unsigned long Millisendd = endlooop - startloop;
  //printf(" end loop time: %d\n",Millisendd);

    if(Millisendd < 30000){
  uint32_t sleeptime = 30000 - (int)Millisendd;
  //Serial.print("sleeptime: ");
  //Serial.println(sleeptime);
  disable_per(); 
  delay(sleeptime);
  
    }
    else{
      
      }
}
/*LED turns ON when connected to a charger
  if(nrf_gpio_pin_read(18) == HIGH){
  Bluefruit.autoConnLed(false);
  digitalWrite(11, HIGH);
  delay(500);
  digitalWrite(11, LOW);
  delay(500);
  }
  */

}

void send_data(void)
{

  //vbatt_per();
  uint16_t data;
  String str_data;
  char buf[12];
  //check_SD();

  memset(dataStr, 0, 210);
  timeNow = now();
  //showTime(timeNow);
  //--------date
  //Serial.println(second());
  if(presecond == second()){
    //Serial.println("presecond");
    delay(60);
    timeNow = now();
    //showTime(timeNow);
    //Serial.println("I'm in the second if");
  }
  
  sprintf(timeStr,"%d/%d/%d, %d:%d:%d",day(),month(),year(),hour(),minute(),second());
  if(Connected){ 
  bleuart.write(timeStr, 20);
  bleuart.write("\n", 1);
  }
  presecond = second();
  strcat(dataStr, timeStr);
  strcat( dataStr, ", ");
  memset(timeStr, 0, 20);
  
  
//----------bat level
  vbatt_per();
  strcat( dataStr,SoC_buf);
  strcat( dataStr, ", ");
        
  memset(SoC_buf, 0, 25);
  if(vbat_per < 20){
    sprintf(SoC_buf, "SoC = Low(%d%%)", vbat_per);
  }
  else{
    sprintf(SoC_buf, "SoC = %d%%", vbat_per);
  }
  if(Connected){ 
  bleuart.write(SoC_buf, 25);
  bleuart.write("\n", 1);
  }
             
  //Serial.println(SoC_buf);
//----------------------------------batt end
//------------------ temp
float temp;
if(TS_Failed == true){
  temp = Read_ACCTemp();
}
else{
  temp = showtemperature();
  }
  //Serial.println(temp);
  char temp_buf[13];
  sprintf(temp_buf, "%.1f", temp);
  strcat( dataStr,temp_buf);
  strcat( dataStr, ", ");
  memset(temp_buf, 0, 13);            
  sprintf(temp_buf,"T: %.1f °C", temp);
  if(Connected){ 
  bleuart.write(temp_buf, 13);
  bleuart.write("\n", 1);
  }
  
  //----------------------temp end
  read_corrCounts();  
  accelerometer();
  //----------------------------------save data
  if(sd_first < 2){
  sd_first++; 
  } 
  else{  
  check_SD();
  SdFile::dateTimeCallback(dateTime);
  File dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    dataFile.println(dataStr);
    dataFile.close();

  }
  else {
    //printf("error opening %s\n", filename);
    //printf("error 1");
  }
  SPI.end();
  SPI1.end();
  //Serial.println("after saving");
  }
  
}

// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
  
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

memset(central_name, 0, 32);
  connection->getPeerName(central_name, sizeof(central_name));
  
  //Serial.print("Connected to ");
  //Serial.println(central_name);
  Connected = true;
  Bluefruit.Advertising.stop();

}

/**
   Callback invoked when a connection is dropped
   @param conn_handle connection where this event happens
   @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
*/
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;
    dis_check = true;
    Connected = false;
  if (s_time == false){
  NVIC_SystemReset();  
  }

  if(state == false){
   Bluefruit.Advertising.start(0); 
  }
            
          
  
}

  void check_SD(){
//Serial.println("check_sd");
    if (!SD.begin(chipSelect)){  
    if(Connected){   
    bleuart.write("Card failed, or not present\n", 30);
    bleuart.write("Insert the microSD to continue:\n", 32);
    //Serial.println("Card failed, or not present");
    }
    else if (Connected == false){ 
      //Serial.println("Card failed, or not present");
      
    Bluefruit.Advertising.start(0);
    //Serial.println("adv start");
    }

    
    while(1){
      if(SD.begin(chipSelect))
      {
       if(Connected){ 
        bleuart.write("microSD OK!\n",12);
       }
        else{
        Bluefruit.Advertising.stop();
        //Serial.println("adv stop");
            }
       break;
       
      }
      digitalWrite(11, !digitalRead(11));
      delay(2000);
           
    }
    digitalWrite(11, 0);
    delay(200);
    
    }
   //Serial.println("end of check_sd"); 
  }
void file_exists(){
  itoa (j, num, 10);
  strcat(num, ".csv");
  strcpy(filename, "LS9_");
  strcat(filename, num);
  check_SD();
  if (SD.exists(filename)) {
    //printf("%s exists.\n", filename);
    j++;
    file_exists();
  } else {
    //printf("%s doesn't exist.\n", filename);
  }
}

void read_cent() {
  char cent[10];
  char * st;
  //memset(cent, 0, 20); 
 while ( bleuart.available() )
  {


    bleuart.read(cent, 20);
    //Serial.write(ch);
    //printf("%s", cent);

    st = strtok(cent, "/");
    int p = 1;
    while (st != NULL) {


      switch (p)
      {

          case 1:
          if (strcmp(st, "s") == 0) {
          Bluefruit.autoConnLed(false);
          if(dis_check && state == false){
          check_sensor();
          dis_check = false;
          }
          //Serial.println(s_time); 
          if(s_time == false){
          bleuart.write("Please set date/time first.\n",30);
          bleuart.write("Format: dd/mm/yyyy hh:mm:ss\n",30);
          bleuart.write("\n",1);        
          set_time();
          }
          if(s_time == true && state == false)
          {
          sd_first = 0;
          file_exists();
          File dataFile = SD.open(filename, FILE_WRITE);
          if (dataFile) {
          dataFile.println(dataStr1);
          dataFile.close();
          }
          }
          state = true;
          //vbatt_per();
          //bleuart.write("SoC:", 6);
          //bleuart.write(SoC_buf, 25);
          //bleuart.write("\n", 1);
          if(!timer_start){
           timer_start = true; 
          NRF_TIMER2->TASKS_START = 1;  
          }
              
          }
          else if (strcmp(st, "r") == 0) {
          state = false;
          Bluefruit.autoConnLed(true);
          bleuart.write("\n", 1);
          bleuart.write("Now you may disconnect.\n",25);
          bleuart.write("\n", 1);
          bleuart.write("\n", 1);
          }
          else if (strcmp(st, "st") == 0) {
          s_time = false;
          st_check = true;
          bleuart.write("\n", 1);
          bleuart.write("Please set date/time:\n",22);
          bleuart.write("Format: dd/mm/yyyy hh:mm:ss\n",28);
          memset(time_cent, 0, 22);
          delay(200);
          set_time(); 
          }
          else if (strcmp(st, "off") == 0) {
          gotoSleep();
          }
          else if (strcmp(st, "reset") == 0) {
          NVIC_SystemReset();
          }
          
      }
      p++;
      //printf ("%s\n",st);
      st = strtok(NULL, "/");
    }

  }

}

void init_set_time() {
  if(strcmp(central_name, "iPhone") == 0){
    iPhone_device = true;
    //printf("iPhone\n");
  }
else{
  //printf("Not iPhone\n");
  set_time();
  }
}


void set_time() {
int hour_1 = 0;
int minute_1 = 0;
int second_1 = 0;
int day_1 = 0;
int month_1 = 0;
int year_1 = 0;

  while ( bleuart.available() )
  {

    //uint8_t ch;
    bleuart.read(time_cent, 22);
    //Serial.write(ch);
    //printf("%s\n\n", time_cent);

    str = strtok(time_cent, " /:");
    int k = 1;
    if (strcmp(str, "r") == 0 && st_check) {
          state = false;
          st_check = false;
          s_time = true;
          memset(time_cent, 0, 22);
          delay(200);
          return;   
      }
    else{
    
    while (str != NULL) {


      switch (k)
      {

          case 1:
          day_1 = atoi(str);
          //printf ("day: %d\n",day_1);
          break;
          
          case 2:
          month_1 = atoi(str);
          //printf ("month: %d\n",month_1);
          break;
          
          case 3:
          year_1 = atoi(str);
          //printf ("year: %d\n",year_1);
          break;

          case 4:
          hour_1 = atoi(str);
          //printf ("hour: %d\n",hour_1);
          break;

          case 5:
          minute_1 = atoi(str);
          //printf ("minute: %d\n",minute_1);
          break;

          case 6:
          second_1 = atoi(str);
          //printf ("second: %d\n",second_1);
          break;
          
      }
      k++;
      //printf ("%s\n",str);
      str = strtok(NULL, " /:");
    }
    }
  }
  
      if (s_time == false){
      if(year_1 >= 2022 && month_1 <=12 && month_1 > 0 && day_1 <= 31 && day_1 > 0 && hour_1<=23 && hour_1>=0
      && minute_1<=59 && minute_1>=0 && second_1<= 59 && second_1>=0){
      
      //configTimer();
      setTime(hour_1, minute_1, second_1, day_1, month_1, year_1);
      //NRF_TIMER2->TASKS_START = 1;     
      s_time = true;
      bleuart.write("time is set.\n",15);
      bleuart.write("\n",1);
      //showTime(timeNow);
      //
      if(iPhone_device){
      bleuart.write("You can send: st/",17);
      bleuart.write(" to readjust the date/time again.\n",34);
      bleuart.write("\n", 1);
      }
        }
      else{
        
        if(time_cent[0]!= 0){
          if(time_cent[0]== 's'){}
          else{
        memset(time_cent, 0, 22);  
       //Serial.println("invalid time format, try again!");
       bleuart.write("Invalid format/value, try again!\n",34);
       bleuart.write("e.g., 01/01/2022 11:30:45\n",28);
       bleuart.write("\n", 1); 
          }
        }
      set_time();
    }
      }      
    else{
      return;
    }
    
}

void showTime(time_t t)
{
  // display the given time
  Serial.print(hour(t));
  printDigits(minute(t));
  printDigits(second(t));
  Serial.print(" ");
  Serial.print(day(t));
  Serial.print("/");
  Serial.print(month(t));
  Serial.print("/");
  Serial.println(year(t));
  Serial.println();
}

void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

float readVBAT(void) {

  float raw;

  // Set the analog reference to 3.0V (default = 3.6V)
  analogReference(AR_INTERNAL_3_0);

  // Set the resolution to 12-bit (0..4095)
  analogReadResolution(12); // Can be 8, 10, 12 or 14

  // Let the ADC settle
  delay(1);

  // Get the raw 12-bit, 0..3000mV ADC value
  raw = analogRead(vbat_pin);

  // Set the ADC back to the default settings
  analogReference(AR_DEFAULT);
  analogReadResolution(10);

  // Convert the raw value to compensated mv, taking the resistor-
  // divider into account (providing the actual LIPO voltage)
  // ADC range is 0..3000mV and resolution is 12-bit (0..4095)
  return raw * REAL_VBAT_MV_PER_LSB;
}

uint8_t mvToPercent(float mvolts) {
  
  if(mvolts<3680)
    return 0;

  if(mvolts <3850) {
     mvolts -= 3662;
  return mvolts/3;  //10%-60%
  }
 if(mvolts >4141) {
  return 100;
  }
  mvolts -= 3391;
  return mvolts/7.5;  //60%-100%
}

 void vbatt_per(){
  vbat_per = 0;
  memset(SoC_buf, 0, 25);
float vbat_mv;

 for(int i = 0 ;i < 100 ;i++){
 vbat_mv += readVBAT();
 delayMicroseconds(250);
  }
  vbat_mv /= 100;
//Serial.println(vbat_mv);
  // Convert from raw mv to percentage (based on LIPO chemistry)
  vbat_per = round(mvToPercent(vbat_mv));
//Serial.println(vbat_per);

  if (vbat_per < 20){
  sprintf(SoC_buf,"Low(%d%%)", vbat_per);
  
  }
  else{
  sprintf(SoC_buf,"%d%%", vbat_per);
  
  
  }

}

uint16_t val;
float showtemperature() {
float temp;
  //SETUP
  Wire1.beginTransmission(0x50); // Start condition
  Wire1.write(0x0A);             //FIFO Configuration 2
  Wire1.write(0x1A);             // set FLUSH_FIFO to '1' 0b00011010
  Wire1.endTransmission(true);   //Stop

  //WRITE
  Wire1.beginTransmission(0x50);   // Start condition
  Wire1.write(0x14);               //TEMP SENSOR SETUP register(0x14)
  Wire1.write(0xC1);               //Set CONVERT_T to '1'
  Wire1.endTransmission(true);     //Stop
  delay(100);                     //delay > 50ms

  //READ
  Wire1.beginTransmission(0x50);   //start condition
  Wire1.write(0x08);               //FIFO_DATA register
  Wire1.endTransmission(false);
  Wire1.requestFrom(0x50, 2); // read address 0xA1
  
  byte buff[2];
  if (Wire1.available()) {
    Wire1.readBytes(buff, 2);  //read byte 1 & 2

  }
  Wire1.endTransmission(true);
  //printf("buff0 :%d\n",buff[0]);
  //printf("buff1: %d\n",buff[1]);
  //Print
  val = ((buff[0] << 8) | buff[1]);

  if (bitRead(val, 16) == 1) {
    val = ~val + 1;
    temp = val * -0.005;
    //printf("Temp(in) = %0.1f °C\n",temp);
    //Serial.print("temp = ");
    //Serial.print(temp);
    //Serial.println(" °C");
  }
  else {
    temp = val * 0.005;
    //printf("Temp(in) = %0.1f °C\n",temp);
    //Serial.print("temp = ");
    //Serial.print(temp);
    //Serial.println(" °C");

  }
  temp -= 2.8;
  return temp;
}

void readChannel(){
  as7341.powerEnable(true);
  memset(readings, 0, 12);
  AGC();
  find_gain();
  if (!as7341.readAllChannels(readings)){
    //Serial.println("Error reading all channels!");
    return;
  }
  
 as7341.powerEnable(false);
  }

void gotoSleep()
{                    

  nrf_gpio_cfg_sense_input(18, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_HIGH);
  
 if(Connected){         
 bleuart.write("DEEP SLEEP MODE ON", 20);
 bleuart.write("\n", 1);
 }
    state = false;
    s_time = false;  
    if(dis_check == true){
    digitalWrite(11, LOW); 
    } 
    //SPI.end();
    //SPI1.end();   
    sd_power_system_off();
}

void check_batt(){
  
    float vbat_mv;
    for(int i = 0 ;i < 100 ;i++){
    vbat_mv += readVBAT();
    delayMicroseconds(250);
    }
  vbat_mv /= 100;
  int vbat_threshold = (int)vbat_mv;
  //Serial.print("vbat_threshold  = ");
  //Serial.println(vbat_threshold);
  /*
  if(nrf_gpio_pin_read(18) == LOW){
    Serial.println("LOW");
  }
  if(nrf_gpio_pin_read(18) == HIGH){ 
    Serial.println("HIGH");
  }
*/
  
 if (vbat_threshold <= 3450){
  if(nrf_gpio_pin_read(18) == LOW){
  gotoSleep();  
  }
   else{
    
   }
 }
 //if state delay must be higher 4000
 delay(100);
}


void read_corrCounts(){
  readChannel();
  memset(corr_counts, 0, 12);
  uint16_t reading_copy[12];
  char buf_corr[24];
  char buf[12];
  
  find_gain();
  sprintf(buf, "%d, ", A_gain);
  strcat( dataStr, buf);
  
  if(Connected){ 
  sprintf(buf, "Gain: %dx", A_gain);
  bleuart.write(buf, 12);
  bleuart.write("\n", 1);
  } 
  
  // memset(buf, 0, 12); // 1    1 or 2
  //sprintf(buf, "%d, ", A_int);
  //strcat( dataStr, buf);
  //if(Connected){ 
  //memset(buf, 0, 12);  // 2
  //sprintf(buf,"Tint: %d ms", A_int);
  //bleuart.write(buf, 12);
  //bleuart.write("\n", 1);
  //}
  
for(uint8_t i = 0; i < 12; i++) {
  if(i == 4 || i == 5) continue;
  reading_copy[i] = readings[i];
  corr_counts[i] = (float)readings[i];
  //------------------
  switch (A_gain) {
  case 0:
    corr_counts[i] = (corr_counts[i] / (182 * 0.5));
    corr_counts[i] *= correctionGain[0];
    break;
  case 1:
    corr_counts[i] = (corr_counts[i] / (182 * 1));
    corr_counts[i] *= correctionGain[1];
    break;
  case 2:
    corr_counts[i] = (corr_counts[i] / (182 * 2));
    corr_counts[i] *= correctionGain[2];
    break;
  case 4:
    corr_counts[i] = (corr_counts[i] / (182 * 4));
    corr_counts[i] *= correctionGain[3];
    break;
  case 8:
    corr_counts[i] = (corr_counts[i] / (182 * 8));
    corr_counts[i] *= correctionGain[4];
    break;
  case 16:
    corr_counts[i] = (corr_counts[i] / (182 * 16));
    corr_counts[i] *= correctionGain[5];
    break;
  case 32:
    corr_counts[i] = (corr_counts[i] / (182 * 32));
    corr_counts[i] *= correctionGain[6];
    break;
  case 64:
    corr_counts[i] = (corr_counts[i] / (182 * 64));
    corr_counts[i] *= correctionGain[7];
    break;
  case 128:
    corr_counts[i] = (corr_counts[i] / (182 * 128));
    corr_counts[i] *= correctionGain[8];
    break;
  case 256:
    corr_counts[i] = (corr_counts[i] / (182 * 256));
    corr_counts[i] *= correctionGain[9];
    break;
  case 512:
    corr_counts[i] = (corr_counts[i] / (182 * 512));
    corr_counts[i] *= correctionGain[10];
    break;
  }
  //-----------------
  sprintf(data_buf, "%.6f, ", corr_counts[i]);
  strcat( dataStr, data_buf);
  if(Connected){       
  sprintf(buf_corr,"%s: %.6f",channel_names[i] ,corr_counts[i]);
  bleuart.write(buf_corr, 13);
  bleuart.write("\n", 1);
  
  } 
  memset(buf_corr, 0, 24);
  memset(data_buf, 0, 25);
  }
  
}
void one_space_sd(){
  check_SD();
  File dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    dataFile.println("\n\n");
    dataFile.close();

  }
}

void dateTime(uint16_t* date_sd, uint16_t* time_sd) {

 // return date using FAT_DATE macro to format fields
 *date_sd = FAT_DATE(year(), month(), day());

 // return time using FAT_TIME macro to format fields
 *time_sd = FAT_TIME(hour(), minute(), second());
}

void accelerometer(){

  int16_t accX, accY, accZ, accTemp;
  char accdata [4];
  char xyz_acc[32];
  
  ACC.begin(22);
  ACC.beginMeasure();              // Switch ADXL362 to measure mode 
  ACC.readXYZTData(accX, accY, accZ, accTemp);
  sprintf(xyz_acc,"%d, %d, %d", accX, accY, accZ);
  //printf("%s\n",xyz_acc);
  strcat(dataStr, xyz_acc);
  //printf("%s\n",dataStr);
  if(Connected){       
  memset(xyz_acc, 0, 32);
  sprintf(xyz_acc,"X = %d Y = %d Z = %d", accX, accY, accZ);
  bleuart.write(xyz_acc, 32); 
  bleuart.write("\n", 1);
  bleuart.write("\n", 1);
  }
  //printf("%s\n",xyz_acc);
  

  SPI.end();
  SPI1.end();
  
}

  void check_sensor(){ 
  int16_t accX, accY, accZ, accTemp; 
  ACC.begin(22);
  ACC.beginMeasure();              // Switch ADXL362 to measure mode 
  ACC.readXYZTData(accX, accY, accZ, accTemp);
  if (accX !=0 && accY !=0 && accZ !=0){
   acc_status = 1; 
  }
  SPI.end();
  SPI1.end();
  
  if(acc_status == 0){
    bleuart.write("ACC failed.\n",13);
    ACC_failed = true;
  }
  else{
    bleuart.write("ACC OK!\n",9);
  }
  if (!as7341.begin()){
  bleuart.write("LS failed.\n",12);
  }
  else{
    bleuart.write("LS OK!\n",8);
  }

 Wire1.begin(); 
if (showtemperature() > 60.0){
  bleuart.write("TS failed.\n",12);
  if(!ACC_failed){
  bleuart.write("ACC_TS is activated.\n",21);
  TS_Failed = true;
  }
  }
  else{
    bleuart.write("TS OK!\n",8);
  }

  
    if (!SD.begin(chipSelect)){     
    bleuart.write("Card failed, or not present\n", 30);
    bleuart.write("Please insert the microSD.\n", 28);
    bleuart.write("\n",1);
    while(1){
      if(SD.begin(chipSelect))
      {
        bleuart.write("microSD OK!\n",12);
        bleuart.write("\n",1);
        Bluefruit.Advertising.stop();
        break;
      }
      delay(3000);
    }
    }
      else{
       bleuart.write("microSD OK!\n",12);
       Bluefruit.Advertising.stop(); 
      }
bleuart.write("\n",1);
}
 float Read_ACCTemp(){
  ACC.begin(22);
  ACC.beginMeasure();
  int16_t ATemp = 0;
  float Treal = 0;
  ATemp = ACC.readTemp();
  if(bitRead(ATemp, 13) == 1){
   ATemp -= 0xFFFFF000; 
   Treal = ((float)ATemp * 0.065/10) - 5.8;
   //Treal = ((float)ATemp * -0.065) -5;
  }
  else{
  Treal = ((float)ATemp * -0.065/10) - 5.8;
  //Treal = ((float)ATemp * 0.065) - 5;  
  }
  SPI.end();
  SPI1.end();
  return Treal;
}

void disable_per(){
NRF_SPIS2->ENABLE = (SPIS_ENABLE_ENABLE_Disabled << SPIS_ENABLE_ENABLE_Pos);  
NRF_UART0->TASKS_STOPTX = 1;
NRF_UART0->TASKS_STOPRX = 1;
NRF_UART0->ENABLE = (UART_ENABLE_ENABLE_Disabled << UART_ENABLE_ENABLE_Pos);


Wire.end();
Wire1.end();
SPI.end();
SPI1.end();
}


void enable_per(){
as7341.begin();
Wire1.begin();  
}



// ISR that handles TIMER2 interrupts (does sampling of sensor)
extern "C" {
  void TIMER2_IRQHandler() {
    if (NRF_TIMER2->EVENTS_COMPARE[0] != 0) {
      NRF_TIMER2->EVENTS_COMPARE[0] = 0;    //Clear compare register 0 event
      digitalToggle(4);
      //if (Connected) {
        sendData = true;        
     // }

    }

  }

}
