#include "CircularBuffer.h"

const uint8_t PIN_PIEZO = A0;
const uint8_t PIN_LED_A  = 6;
const uint8_t PIN_LED_B = 7;
const uint8_t PIN_LED_C = 8;

const uint16_t THRESHOLD = 100;
const uint16_t MS_DEBOUNCE = 20;
const uint16_t MS_GATE_LENGTH = 50;

struct Strike {
  int msHit;
  int vel;
};

CircularBuffer<Strike, 10> hits;

int msLastHit = 0;
bool prevSampleHit = false;

void setup() {
  pinMode(PIN_PIEZO, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_LED_A, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);

  // disable all timer/counter interrupts while we configure the timers
  noInterrupts();

  setupTimer1();

  // re-enable interrupts now that timers are configured
  interrupts();
}

void loop() {
  int velocity = analogRead(PIN_PIEZO);
  digitalWrite(LED_BUILTIN, velocity > THRESHOLD);

  if (velocity > THRESHOLD)
  {
    int msSinceHit = millis() - msLastHit;
    if (!prevSampleHit && msSinceHit > MS_DEBOUNCE)
    {
      // the threshold is being crossed
      msLastHit = millis();
      hits.put((Strike){ msLastHit, 127 });
      digitalWrite(PIN_LED_B, HIGH);
    }

    prevSampleHit = true;
  }
  else
  {
    prevSampleHit = false;
  }
  
}

// this ISR runs every millisecond
ISR(TIMER1_COMPA_vect) {
  Strike hit;
  if (hits.peek(&hit))
  {
    if (millis() >= hit.msHit + MS_GATE_LENGTH)
    {
      // send note off and dequeue
      hits.dequeue();
      digitalWrite(PIN_LED_B, LOW);
    }
  }
}

// TIMER1: 1ms (1 kHz) using prescaler 64, OCR1A = 250
void setupTimer1() {
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= (1 << WGM12);                  // CTC mode
  TCCR1B |= (1 << CS11) | (1 << CS10);     // Prescaler = 64
  OCR1A = 250;                             // 1ms at 16MHz
  TIMSK1 |= (1 << OCIE1A);                 // Enable compare match interrupt
}