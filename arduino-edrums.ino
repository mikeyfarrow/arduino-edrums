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

// Create a queue of "hits" whose NoteOn messages have been sent but whose corresponding 
// NoteOff messages are still pending because the gate length has not yet elapsed
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

  // To accurately measure the "velocity" of a percussive strike against the piezo sensor
  // it does not work simply to use the first sample that crosses the threshold.
  // 
  // Instead we wait for 3 consecutive samples that are above the threshold and use the 
  // maximum of these samples to estime the velocity of the strike.
  //
  // We also must account for very soft strikes which only cross the threshold for 1-2
  // samples (i.e. we don't want to ignore these soft hits).
  // 
  // Therefore a hit is detected when both conditions are met:
  //      1. either this is the third consecutive sample that has been above THRESHOLD or
  //         this sample is below THRESHOLD and the previous sample was above THRESHOLD
  //      2. it has been more than MS_DEBOUNCE millis since the last hit was detected
  bool hitDetected =  now - msLastHit > MS_DEBOUNCE &&
      (numHighSamples == 3 || (numHighSamples == 0 && sensorDataPrev > THRESHOLD));

  if (hitDetected)
  {
    msLastHit = now;
    uint16_t highSample = max(sensorDataPrev, sensorDataCur);
    Hit newHit = { now, highSample / 8 }; /* now in range 0 to 127 */
    
    midi.sendNoteOn(toMIDIAddress(), newHit.vel);
    digitalWrite(LED_BUILTIN, HIGH);

    // add this hit to the CircularBuffer so that we can send the corresponding NoteOff
    // whenever the appropriate amount of time has passed. 
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
  return (MIDIAddress){MIDI_Notes::C[2], Channel_10};
}
