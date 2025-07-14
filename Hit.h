#ifndef HIT_H
#define HIT_H

const uint16_t THRESHOLD = 70;
const uint16_t MS_DEBOUNCE = 50;
const uint16_t MS_GATE_LENGTH = 50;

struct Hit {
  uint32_t msTimeHit;
  int vel;
  MIDIAddress address;
};

#endif  // HIT_H