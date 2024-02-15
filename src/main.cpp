/*
  CPSC 599.88 | A1: Finger Drums | Dr. Lora Oehlberg
  Winter 2024 | UCID: 30089672 | C.S.
*/

#include <MIDI.h> // The MIDI library

// 1.15 1.1 | 3.65 7.5 | 0.4

// A bunch of notes to MIDI macros 1.15 
#define C 24  // C2 Kick            // C3 Hi-Mid Tom
#define C_ 25 // C#2 Side Stick     // C#3 Crash Cymbal1 
#define D 26  // D2 Snare           // D3 High Tom
#define D_ 27 // D#2 Hand Clap      // D#3 Ride Symbal1
#define E 28  // E2 E-Snare         // E3 Hi-Hat
#define F 29  // F2 Low Floor Tom
#define F_ 30 // F#2 Closed Hi-hat
#define G 31  // G2 High Floor Tom  // G3 Splash Cymbal
#define G_ 32 // G#2 Pedal Hi-Hat
#define A 33  // A2 Low Tom         // A3 Crash Cymbal2
#define A_ 34 // A#2 Open Hi-hat
#define B 35  // B1 Acoustic Kick   // B2 Low-Mid Tom   // B3 Ride Cymbal2

// Macros for notes
#define THUMBNOTE   C
#define INDEXNOTE   A_
#define MIDDLENOTE  E
#define RINGNOTE    D
#define PINKYNOTE   D_
// Octaves for every finger
const int octaves[5] = {1, 2, 2, 2, 3};

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
const int MIN_SENSITIVITY = 25;
const int MAX_SENSITIVITY = 500;

// Velocity range to convert sensitivities
const int MIN_VELOCITY = 35;
const int MAX_VELOCITY = 127;

// Velocity minimum response
const float MIN_VAL_RESPONSE = (float)MIN_VELOCITY + 5.0f;

// Debounce and note hold times
const float NOTE_HOLD_TIME = 50.f;
const float DEBOUNCE_TIME = 25.f;

// For debug, default should be 5 because 5 fingers.
const int NUM_FINGERS = 5;

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

float bender = 0.0f;

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
    pinMode(A3, INPUT);
    pinMode(A4, INPUT);
    pinMode(A5, INPUT);
    pinMode(A6, INPUT);
    Serial.begin(115200); //**  Baud Rate 31250 for MIDI class compliant jack | 115200 for Hairless MIDI
    // Serial.begin(9600); //**  Baud Rate 31250 for MIDI class compliant jack | 115200 for Hairless MIDI
}

void loop()
{ 
  // Flex sensor alters pitch bender
  bender = (float)(constrain(map(analogRead(A5), 300, 500, 0, 200), 0, 200));
  if(bender < 100) bender = 0.0f;
  else bender = 1.0f;
  //More Control
  //bender = (float)((float)(constrain(map(analogRead(A5), 300, 500, 0, 200), 0, 200))/200.f);




  // Reading pin values
  tapValues[0] = constrain(map(analogRead(thumbPin), MIN_SENSITIVITY, MAX_SENSITIVITY, MIN_VELOCITY, MAX_VELOCITY), MIN_VELOCITY, MAX_VELOCITY);
  tapValues[1] = constrain(map(analogRead(indexPin), MIN_SENSITIVITY, MAX_SENSITIVITY, MIN_VELOCITY, MAX_VELOCITY), MIN_VELOCITY, MAX_VELOCITY);
  tapValues[2] = constrain(map(analogRead(middlePin), MIN_SENSITIVITY, MAX_SENSITIVITY, MIN_VELOCITY, MAX_VELOCITY), MIN_VELOCITY, MAX_VELOCITY);
  tapValues[3] = constrain(map(analogRead(ringPin), MIN_SENSITIVITY, MAX_SENSITIVITY, MIN_VELOCITY, MAX_VELOCITY), MIN_VELOCITY, MAX_VELOCITY);
  tapValues[4] = constrain(map(analogRead(pinkyPin), MIN_SENSITIVITY, MAX_SENSITIVITY, MIN_VELOCITY, MAX_VELOCITY), MIN_VELOCITY, MAX_VELOCITY);

  // Call functions
  checkPresses();
  flushQueue();
}

void checkPresses() {
  // For every finger
  for(int i = 0; i < NUM_FINGERS; i++) {
    // Check if the finger is at a ready state
    if(releaseBuffers[i] == 0.0f && lastTapVals[i] == 0.0f && lastTimeChecks[i] == 0.0f) {
      // Check if the tap value is at a minimum value and that it wasn't already registered
      if(tapValues[i] > MIN_VAL_RESPONSE && lastTimeChecks[i] == 0.0f) {
        // Register the tap and set lastTimeChecks to current time
        lastTapVals[i] = tapValues[i];
        lastTimeChecks[i] = millis();
      } 
    // Check if lastTimeChecks has been activated and value was registered
    } else if(lastTimeChecks[i] != 0.0f && lastTapVals[i] > MIN_VAL_RESPONSE){
      // If ms buffer time has not passed, check if a new larger value was read
      if(millis() - lastTimeChecks[i] < 4.f) {
        lastTapVals[i] = max(lastTapVals[i], tapValues[i]);
      // Otherwise just use whatever the latest value was saved
      } else {
        MIDI.sendPitchBend(bender, channel);
        queueNote(i, lastTapVals[i]);
        lastTimeChecks[i] = 0.0f;
      }
    } 
  }
}

void queueNote(int index, int velocity) {
  // Simply prep a note for sending and saving
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
    // If the note time has expired, tell MIDI to no longer play this note
    if(noteQueue[i].startTime != 0.0f && millis() - noteQueue[i].startTime > NOTE_HOLD_TIME && tapValues[i] < MIN_VAL_RESPONSE) {
      MidiNote curNote = noteQueue[i];
      MIDI.sendNoteOff(curNote.note, curNote.velocity, channel);
      MidiNote newNote;
      noteQueue[i] = newNote;
      lastTapVals[i] = 0.0f;
    // Debounce timer
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