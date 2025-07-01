const uint8_t PIN_PIEZO = A0;
const uint8_t PIN_LED_A  = 6;
const uint8_t PIN_LED_B = 7;
const uint8_t PIN_LED_C = 8;

const uint16_t THRESHOLD = 100;

void setup() {
  pinMode(PIN_PIEZO, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_LED_A, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  pinMode(PIN_LED_C, OUTPUT);

  // disable all timer/counter interrupts while we configure the timers
  noInterrupts();

  setupTimer1();
  setupTimer3();
  setupTimer4();

  // re-enable interrupts now that timers are configured
  interrupts();
}

void loop() {
  int sensorVal = analogRead(PIN_PIEZO);
  digitalWrite(LED_BUILTIN, sensorVal > THRESHOLD ? HIGH : LOW);
}

// === Timer Setup Functions ===

// TIMER1: 1ms (1 kHz) using prescaler 64, OCR1A = 250
void setupTimer1() {
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= (1 << WGM12);                  // CTC mode
  TCCR1B |= (1 << CS11) | (1 << CS10);     // Prescaler = 64
  OCR1A = 250;                             // 1ms at 16MHz
  TIMSK1 |= (1 << OCIE1A);                 // Enable compare match interrupt
}

// TIMER3: 20ms (50 Hz) using prescaler 64, OCR3A = 5000
void setupTimer3() {
  TCCR3A = 0;
  TCCR3B = 0;
  TCCR3B |= (1 << WGM32);                  // CTC mode
  TCCR3B |= (1 << CS31) | (1 << CS30);     // Prescaler = 64
  OCR3A = 5000;                            // 20ms at 16MHz
  TIMSK3 |= (1 << OCIE3A);
}

// TIMER4: 50ms (20 Hz) using prescaler 256, OCR4A = 3125
void setupTimer4() {
  TCCR4A = 0;
  TCCR4B = 0;
  TCCR4B |= (1 << WGM42);                  // CTC mode
  TCCR4B |= (1 << CS42);                   // Prescaler = 256
  OCR4A = 3125;                            // 50ms at 16MHz
  TIMSK4 |= (1 << OCIE4A);
}

/**************************************************************************************************
 ***** Timer "Interrupt Service Routines" AKA ISRs *******
 * 
 *    These code blocks will run repeatedly according to the settings of their associated timer.
 *    
 *    The settings are determined by the configuration of the... 
 *        ..."output compare registers" (e.g. OCR1A, OCR3A, OCR4A)
 *              - in CTC mode, once the timer reaches the value in the output compare register
 *                the ISR is triggered and the timer restarts at 0.
 *        ..."timer/counter control register" (e.g. TCCR*) sets the timer/counter's mode
 **************************************************************************************************/

// Interrupt Service Routine for Timer1
ISR(TIMER1_COMPA_vect) {
  bool ledA = digitalRead(PIN_LED_A);
  if (ledA)
  {
    digitalWrite(PIN_LED_A, LOW);
  }
  else
  {
    digitalWrite(PIN_LED_A, HIGH);
  }
}

// Interrupt Service Routine for Timer3
ISR(TIMER3_COMPA_vect) {
  digitalWrite(PIN_LED_B, !digitalRead(PIN_LED_B));
}


// Interrupt Service Routine for Timer4
ISR(TIMER4_COMPA_vect) {
  digitalWrite(PIN_LED_C, !digitalRead(PIN_LED_C));
}