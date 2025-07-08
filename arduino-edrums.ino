#include <Control_Surface.h> // built-in libraries are included between <> brackets
#include "CircularBuffer.h"  // local libraries in quotes

const uint8_t PIN_PIEZO = A0;

const uint16_t THRESHOLD = 70;
const uint16_t MS_DEBOUNCE = 50;
const uint16_t MS_GATE_LENGTH = 50;

struct Hit {
  uint32_t msTimeHit;
  int vel;
};

HardwareSerialMIDI_Interface midi { Serial1 };
CircularBuffer<Hit, 10> hits;

uint16_t sensorDataCur = 0;
uint16_t sensorDataPrev = 0;

uint32_t msLastHit = 0;       /* the last time that we detected a new hit */
bool prevSampleHit = false;   /* whether or not the last sample from the sensor was above the threshold */

uint8_t numHighSamples = 0;

void setup() {
  pinMode(PIN_PIEZO, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(6, OUTPUT);

  Control_Surface.begin();
  // Serial.begin(9600);
}

void loop() {
  checkSensorA0();
  checkNoteOffQueue();
}

/* Detects if there is a new hit on the sensor connected to pin A0
 * and sends a MIDI NoteOn and enqueues the corresponding NoteOff to
 * be sent in the future. */
void checkSensorA0()
{
  sensorDataCur = analogRead(PIN_PIEZO); /* in range 0 to 1023 */
  uint32_t now = millis();

  if (sensorDataCur > THRESHOLD)
    numHighSamples++;
  else
    numHighSamples = 0;

  bool hitDetected =  now - msLastHit > MS_DEBOUNCE &&
      (numHighSamples == 3 || (numHighSamples == 0 && sensorDataPrev > THRESHOLD));

  if (hitDetected)
  {
    msLastHit = now;
    uint16_t highSample = max(sensorDataPrev, sensorDataCur);
    Hit newHit = { now, highSample / 8 };
    
    midi.sendNoteOn(toMIDIAddress(), newHit.vel);
    digitalWrite(LED_BUILTIN, HIGH);
    hits.put(newHit);
  }
  
  sensorDataPrev = sensorDataCur;
}

/* See if there is a pending NoteOff message due to be sent.
 * If there is, dequeue it and send the NoteOff message */
void checkNoteOffQueue()
{
  Hit hit;
  if (hits.peek(&hit))
  {
    if (millis() >= hit.msTimeHit + MS_GATE_LENGTH)
    {
      hits.dequeue();
      midi.sendNoteOff(toMIDIAddress(), hit.vel);
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}

MIDIAddress toMIDIAddress()
{
  return (MIDIAddress){MIDI_Notes::C[2], Channel_1};
}
