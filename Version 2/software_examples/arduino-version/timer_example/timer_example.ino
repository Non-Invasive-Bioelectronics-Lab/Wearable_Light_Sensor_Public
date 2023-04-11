int flag = 0;

void setup() {
  Serial.begin(115200);
 
  pinMode(11, OUTPUT);
 
  //NRF52 Timer setup
  NRF_TIMER2->TASKS_STOP = 1;                         // Stop timer
  NRF_TIMER2->MODE = TIMER_MODE_MODE_Timer;           // Set timer to timer mode
  NRF_TIMER2->TASKS_CLEAR = 1;                        // Clear the task first so it's usable later
  NRF_TIMER2->PRESCALER = 9;                          // Prescaler value maximum is 9, f_tick = 16MHz / 2 ^ n
  NRF_TIMER2->BITMODE = TIMER_BITMODE_BITMODE_32Bit;  // Set 16 bit timer resolution
  NRF_TIMER2->CC[0] = 31250;                          // Set value for compare first compare register (f_ticks * CC)
  // NRF_TIMER2->CC[1] = 65535;                        // Can set up to 6 compare registers
  NRF_TIMER2->INTENSET = TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos;          // Enable COMAPRE0 Interrupt
  NRF_TIMER2->SHORTS = (TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos);  // Count then Complete mode enabled
  NVIC_EnableIRQ(TIMER2_IRQn);                        // Enable interrupts
  NRF_TIMER2->TASKS_START = 1;                        // Start timer

}

void loop() {

}

//Use extern C with some other stuff to make sure that the right stuff actually gets called
extern "C" {
  void TIMER2_IRQHandler() {
//    toggleLED();
    if (NRF_TIMER2->EVENTS_COMPARE[0] !=0) {
      NRF_TIMER2->EVENTS_COMPARE[0] = 0;    //Clear compare register 0 event
      flag++;
      toggleLED();
    }
  }
}

void toggleLED(){
  digitalToggle(11);
}
