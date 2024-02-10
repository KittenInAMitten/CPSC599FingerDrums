/*
  CPSC 599.88 | A1: Finger Drums | Dr. Lora Oehlberg
  Winter 2024 | UCID: 30089672 | C.S.
*/

#include <MIDI.h> // The MIDI library

// A bunch of notes to MIDI macros
#define C 24 //C1 KICK DRUM
#define C_ 25
#define D 26
#define D_ 27
#define E 28 //E3 Hi-Hat
#define F 29
#define F_ 30 
#define G 31 
#define G_ 32 //G#1 SNARE
#define A 33
#define A_ 34
#define B 35

// Macros for notes
#define THUMBNOTE   C
#define INDEXNOTE   A_
#define MIDDLENOTE  E
#define RINGNOTE    C
#define PINKYNOTE   C
// Octaves for every finger
const int octaves[5] = {2, 2, 2, 2, 2};

// Pin variables
const int thumbPin  = A0;
const int indexPin  = A1;
const int middlePin = A2;
const int ringPin   = A3;
const int pinkyPin  = A4;
const int flexPin   = A5;

// Channel
const int channel = 1;

// Sensitivity range for force sensors
const int MIN_SENSITIVITY = 20;
const int MAX_SENSITIVITY = 600;

// Velocity range to convert sensitivities
const int MIN_VELOCITY = 30;
const int MAX_VELOCITY = 127;

// Velocity minimum response
const float MIN_VAL_RESPONSE = 50.f;

// Debounce and note hold times
const float NOTE_HOLD_TIME = 50.f;
const float DEBOUNCE_TIME = 25.f;

// For debug, default should be 5 because 5 fingers.
const int NUM_FINGERS = 3;

// MidiNote struct to hold info
struct MidiNote {
  int velocity = 0;
  int note = 0;
  float startTime = 0.0f;
};

// Note and queues
MidiNote note1, note2, note3, note4, note5;

//Queue<MidiNote, 5> noteQueue;
MidiNote noteQueue[5] = {note1, note2, note3, note4, note5};

// Bunch of float arrays to hold info for every fingers
float tapValues[5] = {0.f};
float lastTapVals[5] = {0.f};
float lastTimeChecks[5] = {0.f};
float releaseBuffers[5] = {0.f};

// Create and bind the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE();

/// @brief The main function that checks for taps
void checkPresses();

/// @brief Flushes fingers that are currently being held or having debounce applied
void flushQueue();

/// @brief Returns the appropriate note according to the finger index and octave
/// @param index The finger index
/// @param octave The octave of the note
/// @return The converted note according to octave
int returnNote(int index, int octave);

/// @brief Queues up a note that should be played through MIDI according to finger index and velocity
/// @param index The finger index
/// @param velocity The velocity of the note or force
void queueNote(int index, int velocity);

/// @brief Converts a base note according to the given octave (E.G., Note A + Octave 2 = A2 in MIDI)
/// @param note The base note
/// @param octave The octave 
/// @return The MIDI number of the note according to octave
int convertNote(int note, int octave);

void setup()
{   
    // Setting pins to INPUT
    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
    // pinMode(A3, INPUT);
    // pinMode(A4, INPUT);
    // pinMode(A5, INPUT);
    Serial.begin(115200); //**  Baud Rate 31250 for MIDI class compliant jack | 115200 for Hairless MIDI
    // Serial.begin(9600); //**  Baud Rate 31250 for MIDI class compliant jack | 115200 for Hairless MIDI
}

void loop()
{ 
  tapValues[0] = constrain(map(analogRead(thumbPin), MIN_SENSITIVITY, MAX_SENSITIVITY, MIN_VELOCITY, MAX_VELOCITY), MIN_VELOCITY, MAX_VELOCITY);
  tapValues[1] = constrain(map(analogRead(indexPin), MIN_SENSITIVITY, MAX_SENSITIVITY, MIN_VELOCITY, MAX_VELOCITY), MIN_VELOCITY, MAX_VELOCITY);
  tapValues[2] = constrain(map(analogRead(middlePin), MIN_SENSITIVITY, MAX_SENSITIVITY, MIN_VELOCITY, MAX_VELOCITY), MIN_VELOCITY, MAX_VELOCITY);
  tapValues[3] = constrain(map(analogRead(ringPin), MIN_SENSITIVITY, MAX_SENSITIVITY, MIN_VELOCITY, MAX_VELOCITY), MIN_VELOCITY, MAX_VELOCITY);
  tapValues[4] = constrain(map(analogRead(pinkyPin), MIN_SENSITIVITY, MAX_SENSITIVITY, MIN_VELOCITY, MAX_VELOCITY), MIN_VELOCITY, MAX_VELOCITY);
  checkPresses();
  flushQueue();
}

void checkPresses() {
  for(int i = 0; i < NUM_FINGERS; i++) {
    if(releaseBuffers[i] == 0.0f && lastTapVals[i] == 0.0f && lastTimeChecks[i] == 0.0f) {

      if(tapValues[i] > MIN_VAL_RESPONSE && lastTimeChecks[i] == 0.0f) {
        lastTapVals[i] = tapValues[i];
        lastTimeChecks[i] = millis();
      } 
    } else if(lastTimeChecks[i] != 0.0f && lastTapVals[i] > MIN_VAL_RESPONSE){
      if(millis() - lastTimeChecks[i] < 2.f) {
        lastTapVals[i] = max(lastTapVals[i], tapValues[i]);
      } else {
        queueNote(i, lastTapVals[i]);
        lastTimeChecks[i] = 0.0f;
      }
    } 
  }
}

void queueNote(int index, int velocity) {
  MidiNote thisNote;
  thisNote.velocity = velocity;
  thisNote.note = returnNote(index, octaves[index]);
  thisNote.startTime = millis();
  noteQueue[index] = thisNote;
  MIDI.sendNoteOn(thisNote.note, thisNote.velocity, channel);
}

int returnNote(int index, int octave) {
  if(index == 0) 
    return convertNote(THUMBNOTE, octave);
  else if(index == 1) 
    return convertNote(INDEXNOTE, octave);
  else if(index == 2) 
    return convertNote(MIDDLENOTE, octave);
  else if(index == 3) 
    return convertNote(RINGNOTE, octave);
  else if(index == 4) 
    return convertNote(PINKYNOTE, octave);
  return -1;
}

void flushQueue() {
  for(int i = 0; i < NUM_FINGERS; i++) {
    if(noteQueue[i].startTime != 0.0f && millis() - noteQueue[i].startTime > NOTE_HOLD_TIME && tapValues[i] < MIN_VAL_RESPONSE) {
      MidiNote curNote = noteQueue[i];
      MIDI.sendNoteOff(curNote.note, curNote.velocity, channel);
      MidiNote newNote;
      noteQueue[i] = newNote;
      lastTapVals[i] = 0.0f;
    } else if(releaseBuffers[i] != 0.f && millis() - releaseBuffers[i] > DEBOUNCE_TIME) {
      releaseBuffers[i] = 0.0f;
    }
  }
}

int convertNote(int note, int octave) {
    int actualOctave = octave;
    if(actualOctave < 1) 
        actualOctave = 1;
  return note + ((octave - 1) * 12);
}