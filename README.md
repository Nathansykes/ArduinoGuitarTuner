# Arduino Guitar Tuner
Uses an Arduino Uno R3 with microphone and servo motor to detect frequency and tune the guitar to the correct pitch

[Link to repo](https://github.com/Nathansykes/ArduinoGuitarTuner)

## Instructions for use
#### Uploading
Open the sketch `AutomaticGuitarTuner.ino` in Arduino IDE

Upload it to the Arduino Uno
#### Running
The program will run continuously

Use serial to set the selected string (1-6) or set the desired frequency by entering a number above 6

Place the motor on the corresponding tuning peg

Play the string and the motor will turn in the direction to bring the frequency closer to the target pitch

## Parts List
- Arduino Uno R3 - [Amazon](https://www.amazon.co.uk/dp/B008GRTSV6)
- Microphone Sound Sensor - [Amazon](https://www.amazon.co.uk/dp/B07VPWMVR8)
- Continuous Rotation Servo Motor - [Amazon](https://www.amazon.co.uk/dp/B092VN3MTX)
- Breadboard and Jumper wires

## Circuit Diagram
![Circuit Diagram](https://raw.githubusercontent.com/Nathansykes/ArduinoGuitarTuner/master/CircuitDiagram.png)

## Known Issues
- Frequency can give inaccurate readings if not enough amplitude
- Motor does not provide enough torque to tune to E Standard
- Noise from the motor can intefere with readings from the microphone

## Third Party Libraries
This makes uses of the following libraries: 
#### arduinoFFT 
- [GitHub](https://github.com/kosme/arduinoFFT)  
- [License](https://raw.githubusercontent.com/kosme/arduinoFFT/master/LICENSE)
