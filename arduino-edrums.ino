#include <Control_Surface.h> // built-in libraries are included between <> brackets
#include "CircularBuffer.h"  // local libraries in quotes
#include "Hit.h" 
#include "DrumPad.h"

#define MIDI_DRUM_CH 10

// USBDebugMIDI_Interface midi;
HardwareSerialMIDI_Interface midi { Serial1 };
CircularBuffer<Hit, 10> hits;

DrumPad pads[] = {
    /* GPIO     MIDI note num        MIDI ch num    */
    {   A0,     36,                  MIDI_DRUM_CH },
    {   A1,     37,                  MIDI_DRUM_CH }
};

void setup() {
  // set all the piezo pins as inputs
  for (DrumPad &pad : pads) {
    pinMode(pad.piezoPin, INPUT);
  }

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(6, OUTPUT);

  Control_Surface.begin();
}

void loop() {
  for (DrumPad &pad : pads) {
    Hit hit;
    if (pad.readSensor(&hit))
    {
      // Serial.print("Hit on pad ");
      // Serial.println(pad.piezoPin);
      midi.sendNoteOn(hit.address, hit.vel);
      digitalWrite(LED_BUILTIN, HIGH);
      hits.put(hit);
    }
  }

  checkNoteOffQueue();
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
      midi.sendNoteOff(hit.address, hit.vel);
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}

