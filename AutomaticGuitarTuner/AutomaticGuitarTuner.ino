#include <arduinoFFT.h>
#include <Servo.h>
#include <string.h>

#pragma region Notes
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#pragma endregion

#define SAMPLES 128
#define SAMPLING_FREQUENCY 2048

arduinoFFT FFT = arduinoFFT();

double vReal[SAMPLES];
double vImag[SAMPLES];

unsigned int SamplingPeriod;
unsigned long Microseconds;
double PeakFrequency;
byte Divider = 1;

int StringNotes[] = {0, NOTE_E4, NOTE_B3, NOTE_G3, NOTE_D3, NOTE_A2, NOTE_E2};
int TargetNote = NOTE_E2;
int StringNumber = 0;

//this is the acceptable variance from the target note in hz
double Variance = 2.5;

int Speed = 0;

//continuous servo
Servo servo;
#define ServoZero 90

void setup()
{
  Serial.begin(9600);
  SamplingPeriod = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
  servo.attach(9); 
}

void loop()
{
  SetStringFromSerial();
  if(StringNumber == 0)
    return;
  SampleAudio();
  TuneString();
}

void TuneString()
{
  if (PeakFrequency > TargetNote + Variance)
  {
    Serial.println("Too high");
    Speed = 100;
  }
  else if (PeakFrequency < TargetNote - Variance)
  {
    Serial.println("Too low");
    Speed = 75;
  }
  else
  {
    Serial.println("In tune");
    Speed = 90;
  }
  servo.write(Speed);
  delay(10);
}

void SetStringFromSerial()
{
  delay(10);
  if (Serial.available() > 0)
  {
    int str = (int)Serial.read();
    Serial.print("String: ");
    Serial.println(str);
    if (str > 0 && str <= 6)
    {
      StringNumber = str;
      TargetNote = StringNotes[str];
    }
    else
    {
      Serial.println("Invalid string number");
      StringNumber = 0;
    }
  }
}

bool NoteWithin20Percent(double frequency)
{
  if (frequency > TargetNote * 1.2 || frequency < TargetNote * 0.2)
    return false;
  else
    return true;
}

double CleanFrequency(double frequency)
{
  //were need to check anything outside 20% of the note for resonant frequencies and subdivide it
  if (!NoteWithin20Percent(frequency))
  {
    //check if it is a resonant frequency
    if (!NoteWithin20Percent(frequency / 2))
      return frequency / 2;
    else if (!NoteWithin20Percent(frequency / 3))
      return frequency / 3;
  }
  return frequency;
}

double getMean(double arr[], int size);
double getStdDev(double arr[], int size);

const int passes = 10;
void SampleAudio()
{
  //read the audio 10 times and get the average whilst also excluding outliers
  double cleanFrequencies[passes];
  for(int i = 0; i < sizeof(cleanFrequencies); i++)
  {
    delay(2);
    for (int i = 0; i < SAMPLES; i++)
    {
      Microseconds = micros();
      vReal[i] = analogRead(A0);
      vImag[i] = 0;
      while (micros() < Microseconds + SamplingPeriod) { }
    }

    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

    double readFrequency = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
    cleanFrequencies[i] = CleanFrequency(readFrequency);
  }
  
  double mean = getMean(cleanFrequencies, sizeof(cleanFrequencies));
  double stdDev = getStdDev(cleanFrequencies, sizeof(cleanFrequencies));
  double threshold = 3;
  int validFrequenciesCount = 0;
  for(int i = 0; i < sizeof(cleanFrequencies); i++)
  {
    double z = (cleanFrequencies[i] - mean) / stdDev;
    if(abs(z) > threshold)
      validFrequenciesCount++;
  }
  
  double finalFrequencies[validFrequenciesCount];
  int index = 0;
  for(int i = 0; i < sizeof(cleanFrequencies); i++)
  {
    double z = (cleanFrequencies[i] - mean) / stdDev;
    if(abs(z) > threshold)
      finalFrequencies[index++] = cleanFrequencies[i];
  }

  //now get the average of the final frequencies
  double averageFrequency = getMean(finalFrequencies, sizeof(finalFrequencies));

  PeakFrequency = CleanFrequency(averageFrequency);

  //get the amplitde
  double amplitude = vReal[0]; //this might work
  Serial.print("amplitude = ");
  Serial.println(amplitude);


  Serial.print(PeakFrequency);
  Serial.println("hz");
}


//math functions

double getMean(double arr[], int size) {
    double sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum / size;
}

double getStdDev(double arr[], int size) {
    double mean = getMean(arr, size);
    double sum = 0;
    for (int i = 0; i < size; i++) {
        sum += pow(arr[i] - mean, 2);
    }
    return sqrt(sum / (size - 1));
}