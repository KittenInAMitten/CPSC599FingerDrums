# CPSC599FingerDrums

## Description
CPSC 599.88 | WINTER 2024 | 

## Software Requirements and Resources

### MIDIUSB Alternative
- Due to **MIDIUSB** not working on an Arduino Uno, I managed to find a tutorial and another library called **MIDI Library** that worked with the Uno. I have linked the tutorial below along with all the software needed. All the steps to configure the Arduino are in Step 10. 
- **Link:** (Please look at step 10 for the Arduino UNO) https://www.instructables.com/DIY-USB-Midi-Controller-With-Arduino-a-Beginners-G

### MIDI Library
- **Link:** https://www.arduino.cc/reference/en/libraries/midi-library/

### Hairless MIDI to Serial
- This converts the Serial signals form the Arduino Uno to MIDI.
- **Link:** https://projectgus.github.io/hairless-midiserial/

### LoopMIDI - Virtual Loopback MIDI Cable
- This is used to create a virtual MIDI port for Hairless MIDI to Serial to use.
- **Link:** https://www.tobias-erichsen.de/software/loopmidi.html

### Virtual MIDI Synth
- This is used to play sounds using MIDI.
- **Link:** https://coolsoft.altervista.org/en/virtualmidisynth

### Virtual MIDI Piano Keyboard
- This is used as a virtual keyboard for the Arduino to use.
- **Link:** https://vmpk.sourceforge.io/

### ColomboGMGS2 Soundfont for Virtual MIDI Synth
- This was listed as a free soundfont in the Virtual MIDI Synth post above and its Bank 127 is used for the Drumkit sounds.
- **Link:** https://musical-artifacts.com/artifacts/1234


## Setup Instructions