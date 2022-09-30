# Loki
Turn any motorized toy into musical moving thing

## Overview
This project turns any battery operated toys (though USB powered toys are recommended) into a musical puppet. Recommended as a project for intermediate Arduino and electronics tinkerers.

**ERRATA FOR THE SCHEMATIC: C1 (1000uF) should be replaced with a 100uF electrolytic capacitor.** If the circuit doesn't respond correctly, remove this capacitor.

**ERRATA FOR THE SCHEMATIC: 1K resistors should be added between DFPlayer Mini and Arduino.**

## How it works
![IMG1657434887](https://user-images.githubusercontent.com/11834016/178134115-42bc4238-87ee-466f-a4fe-d91a786a03e4.png)

It's divided into two parts -- one that controls the audio and one that controls the movement.

The audio control is made up of two components, one is shared between the two parts. One component is the Arduino Nano board, but you can use any 3.3V/5V board you'd like. The other is DFPlayer Mini, an MP3 player module.

Arduino sends signals to the DFPlayer Mini module to play the music, which is then fed into either 8002 amplifier module, or PAM8403 amplifier module. Only the left channel is used for this purpose.

Two resistors adjust the volume of the music, as raw output will burn out small speakers.

BUSY pin from the DFPlayer Mini is connected to the Arduino's analog pin (though digital pin works as well since we will be using `digitalRead()` anyway) to keep the music playing and stop after one song.

The next part is the movement control. It is made up of three components. A motor driver, an LM393 sensor module, and a simple transistor circuit.

The transistor circuit amplifies the audio signal further to activate the LM393 module's output. The circuit is placed between the "S" pin and "G" pin of the module, so external power for that circuit is not required.

LM393 module is then used to turn the weak signal from the transistor into a full on-or-off signal going between ground and VCC. It is connected to one pin of the motor driver.

The motor driver is a classic H-bridge motor controller, with one pin of the input tied to the ground to enable braking and prevent interference from triggering the motor. (LL or HH brakes the motor) You can use pretty much any module here.

# Build instructions

The motor doesn't need any more than one 104 capacitor placed between the positive and negative pins in most cases, but if needed, refer to the screenshot below to connect interference suppression capacitors. If your toy already has a circuit attached to the motor such as the second picture, do not remove them. This is required to prevent the motor noise from interfering with amplifiers, power circuits and the Arduino itself.

![IMG1657432344](https://user-images.githubusercontent.com/11834016/178133110-47a58dfc-38bf-42aa-9c00-d1ff843b90f8.png)

![IMG1657432536](https://user-images.githubusercontent.com/11834016/178133189-7fb017cd-a53e-421e-bac7-bc10b3c16d89.png)

This diagram, while crude, should help you set it up.

![IMG1657432654](https://user-images.githubusercontent.com/11834016/178202643-3cc788c1-09ec-45d3-9fe9-f1d576d7166a.png)

Depending on your power source, you might need to make this simple circuit to add a little bit of load to the circuit. I know there are more intelligent way to handle this (such as a circuit that connects 150mA load every few seconds) but this dumb circuit can also act as a way to add lighting and it's very compact, so it can be stuffed into the toy. This circuit will also act as a buffer if the motor pulls too much current while it spins up.

![IMG1657433279](https://user-images.githubusercontent.com/11834016/178133483-023f9771-fa96-4a59-8bd8-634060db2aea.png)

If you cannot get past the initialization step, you need to increase the current capability of the power source. Brown-out detection on Arduino will keep resetting the CPU if the power rail is unstable.

**NOTE: If your Arduino does not reset properly on power-up state, extend the reset switch and place it somewhere on your toy. It will be needed to play the music.**

# Generating music files

You can use any DAW to set up a beat detection (I used Fruity Peak Controller, Maximus, and Parametric EQ 2) for easier conversion.

Or if you have some time on your hand, you can place a tone where you want.

Set the DAW to output only the tone to the right channel and music only to the left channel.

Set the tone volume to the loudest possible volume. This ensures a proper turn-on and turn-off signal. You won't hear it while it's inside the unit, so it's okay if it clips/distorts.

Save the files in this syntax:
```
XXXXFilename.mp3
```
Where XXXX is a 4-digit index number, from 0001. It must be sequential, else the code will fail.

### Good example
```
0001Welcome.mp3
0002Loki.mp3
0003UniverseOnFire.mp3
0004Lowenherz.mp3
0005ConstellationOfTears.mp3
0006HypaHypa.mp3
...
```
### Bad example
```
001Welcome.mp3
0003Loki.mp3
4Lowenherz.mp3
HypaHypa.mp3
...
```

For more info on how the signal should be structured, refer to the 0001Demo.mp3 inside this repository.

# Motor reversing circuit
Some toys (such as dancing cactus) may need the motor to reverse its direction every so often to prevent damages.

Build the circuit as follows, and attach it to the input of the H-bridge motor driver.

![IMG1657520515](https://user-images.githubusercontent.com/11834016/178201157-39ab81a8-5b2a-4cf6-b999-2ce706ecf5dc.png)

D13 is the D13 pin (or its equivalent) on the MCU board, and SIG is the output from LM393 module. Connect it and use the new version of the code.

Then adjust the timing of the blinks on the Arduino accordingly.

## How it works

When both inputs are LOW or only the D13 is HIGH, the circuit will not allow the signal to pass through.

When only the SIG pin is HIGH, it passes the signal through.

But when both SIG and D13 pin is high, it will change the signal from 1-0 to 0-1.

![IMG1657515188](https://user-images.githubusercontent.com/11834016/178490419-64f1d649-e6bf-4ade-9df2-eb4bcfaf3451.png)
