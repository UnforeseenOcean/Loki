# Loki
Turn any motorized toy into musical moving thing

## Overview
This project turns any battery operated toys (though USB powered toys are recommended) into a musical puppet. Recommended as a project for intermediate Arduino and electronics tinkerers.

## How it works
It's divided into two parts -- one that controls the audio and one that controls the movement.

The audio control is made up of two components, one is shared between the two parts. One component is the Arduino Nano board, but you can use any 3.3V/5V board you'd like. The other is DFPlayer Mini, an MP3 player module.

Arduino sends signals to the DFPlayer Mini module to play the music, which is then fed into either 8002 amplifier module, or PAM8403 amplifier module. Only the left channel is used for this purpose.

Two resistors adjust the volume of the music, as raw output will burn out small speakers.

BUSY pin from the DFPlayer Mini is connected to the Arduino's analog pin (though digital pin works as well since we will be using `digitalRead()` anyway) to keep the music playing and stop after one song.

The next part is the movement control. It is made up of three components. A motor driver, an LM393 sensor module, and a simple transistor circuit.

The transistor circuit amplifies the audio signal further to activate the LM393 module's output. The circuit is placed between the "S" pin and "G" pin of the module, so external power for that circuit is not required.

LM393 module is then used to turn the weak signal from the transistor into a full on-or-off signal going between ground and VCC. It is connected to one pin of the motor driver.

The motor driver is a classic H-bridge motor controller, with one pin of the input tied to the ground
