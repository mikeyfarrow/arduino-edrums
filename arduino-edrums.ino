// built-in libraries are included between <> brackets
#include <Control_Surface.h>

// local libraries in quotes
#include "CircularBuffer.h"

// #define _DEBUG_SERIAL

const uint8_t PIN_PIEZO = A0;

const uint16_t THRESHOLD = 70;
const uint16_t MS_DEBOUNCE = 50;
const uint16_t MS_GATE_LENGTH = 50;

struct Hit {
  uint32_t msTimeHit;
  int vel;
};

// USBDebugMIDI_Interface midi;
HardwareSerialMIDI_Interface midi { Serial1 };
CircularBuffer<Hit, 10> hits;


uint32_t msLastHit = 0; /* the last time that we detected a new hit */
bool prevSampleHit = false; /* whether or not the last sample from the sensor was above the threshold */
bool toggle = false;

void setup() {
  pinMode(PIN_PIEZO, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(6, OUTPUT);

#ifdef _DEBUG_SERIAL
  Serial.begin(9600); 
#endif

  Control_Surface.begin();
}

void loop() {
  toggle = !toggle;
  digitalWrite(6, toggle);

  checkSensorA0();
  checkNoteOffQueue();
}

/* Detects if there is a new hit on the sensor connected to pin A0
 * and sends a MIDI NoteOn and enqueues the corresponding NoteOff to
 * be sent in the future. */
void checkSensorA0()
{
  uint16_t sensorData = analogRead(PIN_PIEZO); /* in range 0 to 1023 */
  uint32_t now = millis();
  
  if (sensorData > THRESHOLD)
  {
#ifdef _DEBUG_SERIAL
    Serial.print("min:0,max:1024,sensor:");
    Serial.println(sensorData);
#endif
    if (!prevSampleHit && now - msLastHit > MS_DEBOUNCE)
    {
      // the threshold is being crossed
      msLastHit = now;
      Hit newHit = { now, min(127, sensorData) };
      midi.sendNoteOn(toMIDIAddress(), newHit.vel);
      digitalWrite(LED_BUILTIN, HIGH);
      hits.put(newHit);
    }
  }
#ifdef _DEBUG_SERIAL
  else if (prevSampleHit)
  {
    Serial.println("min:0,max:1024,sensor:0");
  }
#endif
  prevSampleHit = sensorData > THRESHOLD;
}

/* See if there is a pending NoteOff message due to be sent.
 * If there is, then dequeue it and send the NoteOff message */
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
