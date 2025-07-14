#ifndef DRUM_PAD_H
#define DRUM_PAD_H

#include <Control_Surface.h>

#include "Hit.h"

class DrumPad {
public:
  /* arduino pin that the sensor is connected to */
  uint8_t piezoPin;
  
private:

  /* MIDI info for this pad */
  MIDIAddress addr;
  Channel channel;

  uint32_t msLastHit; /* the last time that we detected a new hit */
  uint8_t numHighSamples;
  uint16_t sensorDataCur;
  uint16_t sensorDataPrev;

public:
  DrumPad(uint8_t pin, Channel ch, MIDIAddress addr);
  bool readSensor(Hit* valPtr);
};

/* DrumPad constructor */
DrumPad::DrumPad(uint8_t pin, Channel ch, MIDIAddress addr)
  : addr(addr), channel(ch) {
  piezoPin = pin;

  msLastHit = 0;
  numHighSamples = 0;
  sensorDataCur = 0;
  sensorDataPrev = 0;
}

bool DrumPad::readSensor(Hit* valPtr) {
  sensorDataCur = analogRead(piezoPin); /* in range 0 to 1023 */
  uint32_t now = millis();

  if (sensorDataCur > THRESHOLD)
    numHighSamples++;
  else
    numHighSamples = 0;

  bool hitDetected = now - msLastHit > MS_DEBOUNCE && (numHighSamples == 3 || (numHighSamples == 0 && sensorDataPrev > THRESHOLD));

  if (hitDetected) {
    msLastHit = now;
    uint16_t highSample = max(sensorDataPrev, sensorDataCur);
    *valPtr = { now, highSample / 8, addr };
  }

  sensorDataPrev = sensorDataCur;
  return hitDetected;
}
#endif  // DRUM_PAD_H