/*        Picoinjector v1.0, a picoliter injection system.
This Arduino file is designed to be uploaded on an Arduino Nano for the Picoinjector project 
Source : https://github.com/…
 2021.01.05                                                                             */
////////////////////////////////////////////////////////////////////////////////////////////////
//                         KEY PARAMETERS TO SET BY USERS
////////////////////////////////////////////////////////////////////////////////////////////////

  const int TIMEUNIT              = 25;     // Base Time Unit in milliseconds.
  const int POT_VALUE_LOW         = 0;      // Lowest value available from the potentiometer.
  const int POT_VALUE_HIGH        = 1023;   // Highest value available from the potentiometer. Can make this higher by reducing resistor value in series with potentiometer, but go below 125 Ohms and current will exceed 40 mA max on Analog pin.
  const int SENSITIVITY           = 20;     // Number of allowed settings of potentiometer is (Sensitivity) (1 based…) make sure that potentiometer range = (High-low)/sensitivity isn't smaller than potentiometer noise.
  const int POT_NOISE             = 5;      // Amplitude of fluctuations in the potentiometer value when it is static. Measured value. some power supplies are noisier than others.
  const int STABILITY_TIME        = 500;    // Number of milliseconds for which stability must be achieved when adjusting potentiometer to accept new value.
  const int MONITOR_DELAY         = 100;    // Delay between loops for analog monitoring cycles - allows user time to adjust potentiometer a significant amount between loops.
  const int PAUSE_BEFORE_BLINK    = 500;    // Number of milliseconds to pause before starting blinks.
  const int BLINK_ON_DURATION     = 100;    // This combo seems to work nicely.
  const int BLINK_OFF_DURATION    = 250;    
  const int NUM_PROGRAMMED_PULSES = 28;     // number of pulses to implement in hard coded sequence
  int PROGRAMMED_SEQUENCE[NUM_PROGRAMMED_PULSES] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000}; // sequence of pulses in ms
  int Python_Array [28] = {0};              // Empty array to process Python Data
  const int PROGRAMMED_PULSE_DELAY= 1000;    // Delay between pulses.
  const int MONITOR_FOOTSWITCH_PERIOD = 500; // Seems long enough for a double click. Can get a triple in there too.
  const int FOOTBUTTONDELAY = 1000;          // Liberally dispersed throughout the code to give time for people to release the button.
  char  serialData;
  int SwithcValue;
  
////////////////////////////////////////////////////////////////////////////////////////////////
//                               GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////////////////////// 
  
  // Set up the pins
  const int D_FootButtonPin   = 2;      // Set the foot pedal to digital pin 2
  const int D_SolenoidPin     = 5;      // Set the solenoid valve to digital pin 5
  const int D_LEDPin          = 10;     // Set proxy LED to digital pin 10
  const int A_PotPin          = 0;      // Set the potentiometer to analog pin A0
  const int D_SwitchPin       = 8;      // Set the switch to digital pin 10

  // Declare global variables - These are designed to be adjusted simultaneously using the SetPulseDuration function .
 
  long PulseDuration          = 0;
  long SerialInput            = 0;
  int PotSetting              = 0;
  int PotValue                = 0;      // Potentiometer value reading
  int numButtonClicks         = 0;

////////////////////////////////////////////////////////////////////////////////////////////////   
//                                        MODULES
//////////////////////////////////////////////////////////////////////////////////////////////// 

// include the library code,also can use OLED screens:
#include <LiquidCrystal.h>

// Creates an LCD object. Parameters: (rs, enable, d4, d5, d6, d7)
LiquidCrystal lcd (12, 11, 7, 6, 4, 3);

void setup() {

  //initialiase serial port for output messages
  Serial.begin (9600);

  //initialiase the pins
  pinMode(A_PotPin , INPUT);
  pinMode(D_SwitchPin , INPUT);
  pinMode(D_FootButtonPin , INPUT); 
  pinMode(D_LEDPin , OUTPUT);
  pinMode(D_SolenoidPin , OUTPUT);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16 ,2);

  // set initial conditions on pins.
  digitalWrite(D_SolenoidPin, LOW);
  digitalWrite(D_LEDPin, LOW);

  // monitor the potentiometer to make sure it is stable, includes lighting the LED to show aliveness…
  MonitorAnalog();
  SetPluseDuration(); // set the initial pulse duration and report new values over serial link.
  LEDBlinks(); // blink the LED to indicate the potentiometer setting.
}

// function reads the potentiometer and populates the three global variables PotValue, PotSetting and PulseDuration
// with mutually consistent	values. These global values are not intended to be set using any other function.
void SetPluseDuration() {
  PotValue = analogRead(A_PotPin);

  // compute the Potentiometer Setting as an integer between 1 and Sensitivity
  PotSetting = int(float(SENSITIVITY - 1) * float(PotValue - (POT_VALUE_LOW + 20)) / float((POT_VALUE_HIGH - 20) - (POT_VALUE_LOW + 20))) + 1;

  // compute pulse duration in milliseconds
  PulseDuration = PotSetting * TIMEUNIT;
  Serial.println("Computing Pulse Duration in Milliseconds.");
  Serial.println("Time Unit");
  Serial.println(TIMEUNIT);
  Serial.println("PotValue");
  Serial.println(PotValue);
  Serial.println("PotSetting");
  Serial.println("PulseDuration");
  Serial.println(PulseDuration);
}

// function flashes the LED PotSetting number of times
void LEDBlinks() {
  int i=0;

  // Switch LED off for PAUSE_BEFORE_BLINK milliseconds prior to the blinks starting.
  digitalWrite(D_LEDPin, LOW);
  delay(PAUSE_BEFORE_BLINK);

  for (i=0; i<PotSetting; i++) {
    digitalWrite(D_LEDPin, HIGH);
    delay(BLINK_ON_DURATION);
    digitalWrite(D_LEDPin, LOW);
    delay(BLINK_OFF_DURATION);
  }
}

// monitors the foot switch and counts the pulses
void MonitorFootSwitch() {
  unsigned long StartTime = 0;
  int previousValue = 1; //assume we get here with the first switch still down.
  int currentValue = 1;
  int changesDetected = 1; // the very first pulse to get us here from the main loop.

  // get the start time
  StartTime = millis();

  while ((abs(millis() - StartTime)) < MONITOR_FOOTSWITCH_PERIOD) {
    previousValue = currentValue;
    currentValue = digitalRead(D_FootButtonPin);
    if (previousValue != currentValue) {
      changesDetected = changesDetected + 1;
    }
  }
  
  if (currentValue==1) {
    changesDetected += 1; //assume that if it was high when we left the loop it will go low - means we always get an even number of changes.
  }
  numButtonClicks = floor((float)changesDetected / 2.0); 
}

// continuously monitors the value on the Analog pin until PotValue is stable (i.e. no changes bigger than POT_NOISE) for STABILITY_TIME milliseconds.
void MonitorAnalog() {
  int	PreviousPotValue = 0;
  int	CurrentPotValue = 0;
  int KeepLooping = 1;
  unsigned long StartTime = 0;
  unsigned long CurrentTime = 0;

  // Set LED Pin high, so user knows that system has detected potentiometer change.
  digitalWrite(D_LEDPin, HIGH);
  // get the start time
  StartTime = millis();

  // start	looping
  while (KeepLooping) {
    // Store the PotValue from the previous	loop.
    PreviousPotValue = CurrentPotValue;
    // LCD shows current Pot value
    lcd.clear() ;
    lcd.setCursor(0, 0) ;
    lcd.print("Calibrating");
    // read in the new PotValue.
    CurrentPotValue = analogRead(A_PotPin);
    Serial.println(CurrentPotValue);
    // if	the current PotValue has changed from the previous PotValue by more than POT_NOISE, then restart timer.
    if(abs(PreviousPotValue	- CurrentPotValue) > POT_NOISE) {
      StartTime = millis();
    }
    // get the current time
    CurrentTime = millis();
    // Check for an overflow condition for the timer. The timer	hits an overflow every 50 days.
    // If the system is left active for that long it is conceivable that the user adjusts the potentiometer
    // at precisely the moment the overflow occurs thus potentially getting locked into a 50 day wait.
    // If CurrentTime is lower than the StartTime then we must have hit the overflow condition.
    // Solution	is simply to restart the timer.
    // The user will just have to wait an additional STABILITY_TIME milliseconds for the stability condition to emerge.
    // Better than waiting an additional 50 days for the loop exit conditions to emerge.
    if(CurrentTime <= StartTime) {
      StartTime = millis();
    }

    // if	the time elapsed since the StartTime exceeds STABILITY_TIME then stop looping
    if(( CurrentTime - StartTime ) > STABILITY_TIME) {
      KeepLooping = 0;
    }

    delay(MONITOR_DELAY); // put in	delay to ensure that user has time to change the potentiometer a significant amount between loops.
  }

  // switch	off LED.
  digitalWrite(D_LEDPin, LOW);

}

void rapidFlash(int numPulses, int delayOn, int delayOff) {
  int	currentPulse = 0;
  for(currentPulse = 0; currentPulse < numPulses; currentPulse +=1) {
    digitalWrite(D_LEDPin, HIGH); // light the LED
    delay(delayOn); // wait	for	the pulse	duration
    digitalWrite(D_LEDPin, LOW);
    delay(delayOff); // wait for the pulse duration
  }
}

void loop() {
  int	pulsed = 0; // Set to zero at the beginning	of each	loop.
  int PotValueNew = 0; // initialise the new pot value
  int	currentPulse = 0; // counter for number of pulses
  unsigned long startTime = 0; // timer
  int SwitchValue = 0;
  digitalWrite(D_SolenoidPin, LOW); //ensure output pins are set to low at the start of each loop.
  digitalWrite(D_LEDPin, LOW);
  // Read a new potentiometer value.
  PotValueNew = analogRead(A_PotPin);
  SwitchValue = digitalRead(D_SwitchPin);

  if(Serial.available() > 0) { // read python GUI for Pulse duration
    for(int	i = 0; i < 28; i++) {
      Python_Array[i] = Serial.parseInt(); // fills entire python array with values in serial buffer
      if(Python_Array[i] == 0) { // if next value is already zero all other values will be zero and thus skip next step
      for(int j = i; j < 28; j++) {
        Python_Array[j] = 0; // fill rest	of array with zeros
      }
      break;
      }
    }

    if(Python_Array[0] = 1) {
      PulseDuration = Python_Array[1]; // first value of new python array determines what needs to be done
    }
    if(Python_Array[0] = 2) { // if value is two python sequance gets coppied to programmed sequence
      memcpy(PROGRAMMED_SEQUENCE, Python_Array + 1, sizeof(PROGRAMMED_SEQUENCE));
    }
  }

  // if the potentiometer value has been changed by more than POT_NOISE from the currently set value then
  // jump to a potentiometer monitoring loop which exits when the pot is stable for STABILITY_TIME milliseconds
  if(abs(PotValueNew - PotValue) > POT_NOISE) {
    MonitorAnalog(); // continuously reads the analog until it is stable for more than STABILITY_TIME
    SetPluseDuration(); // Read the latest value and set the new pulse duration.
    lcd.clear();
    lcd.print(PulseDuration);
    lcd.setCursor(4, 0);
    lcd.print("ms");
    Serial.println(0); // place	holder to	indicate given information
    Serial.println(PulseDuration);
    Serial.println(0); // place	holder
    LEDBlinks(); // blink the LED the appropriate number of times with the latest values
  }

  // check for button	presses
  numButtonClicks = 0; // assume no button clicks have happened/reset after last loop
  numButtonClicks = digitalRead(D_FootButtonPin); // read the button pin
  if(numButtonClicks == 1) {
    MonitorFootSwitch(); // as soon as button	is pressed we monitor it for a some time to see	if it	is pressed repeatedly
    // function	sets the global variable numButtonClicks
  }
  if(SwitchValue==HIGH) {// dispay	if switch	is in sequance mode
    lcd.clear();
    lcd.print("Sequence Mode On");
  }
  // do behaviour based on global variable numButtonClicks. 1 is a single pulse
  if(numButtonClicks==1 && SwitchValue==LOW) {
    digitalWrite(D_SolenoidPin, HIGH);
    digitalWrite(D_LEDPin, HIGH);
    Serial.println(POT_NOISE); // will change to different information such as Volume
    Serial.println(PulseDuration);
    Serial.println(D_LEDPin); // will change to different information such as total time
    lcd.setCursor(0, 1); // light	the LED
    lcd.print("Single Puff");
    delay(PulseDuration); // wait	for	the pulse	duration
    digitalWrite(D_SolenoidPin, LOW); // close the solenoid and switch off the LED
    digitalWrite(D_LEDPin, LOW);
  }

  // switch to perform the pulse sequence. Click to interrupt
  if(numButtonClicks==1 && SwitchValue==HIGH) {
    numButtonClicks = 0;
    delay(FOOTBUTTONDELAY);
    for(currentPulse = 0; currentPulse < NUM_PROGRAMMED_PULSES; currentPulse +=1) {
      lcd.clear();
      lcd.setCursor(10, 0);
      lcd.print(currentPulse+1); // display	pulse number
      lcd.setCursor(0, 0);
      lcd.print("Sequence #");
      lcd.setCursor(0, 1);
      lcd.print(PROGRAMMED_SEQUENCE[currentPulse]); // display pulse value
      lcd.setCursor(5, 1);
      lcd.print("ms");
      delay(750); // have time between the next value is displayed and the puff is exicuted
      Serial.println(currentPulse+1); // will	change to	different information such as Volume
      Serial.println(PROGRAMMED_SEQUENCE[currentPulse]);
      Serial.println(D_LEDPin);
      digitalWrite(D_SolenoidPin, HIGH);
      digitalWrite(D_LEDPin, HIGH); // light the LED
      delay(PROGRAMMED_SEQUENCE[currentPulse]); // wait for the pulse duration of the current time
      digitalWrite(D_SolenoidPin, LOW) ; // close the solenoid and switch off the LED
      digitalWrite(D_LEDPin, LOW);

      startTime = millis();
      while((numButtonClicks == 0)&&((millis() - startTime) < PROGRAMMED_PULSE_DELAY)) {
        numButtonClicks = digitalRead(D_FootButtonPin);
      }

      if(numButtonClicks > 0) {
        rapidFlash(5, 100, 100);
        lcd.clear();
        lcd.print("Sequence Broken");
        break; //break sequance by pushing button again, also	display	sequance is	broken
      }
      if (PROGRAMMED_SEQUENCE[currentPulse+1]==0) {//sequance ends if next puff value is 0, nessassary due to how python array is formed   
        lcd.clear();
        lcd.print("Sequence Ended");
        break;
      }
    }
    numButtonClicks = 0;
    delay(FOOTBUTTONDELAY);
  }

  if(numButtonClicks > 2) { // an error	alert
    rapidFlash(5, 100, 100);
    delay(FOOTBUTTONDELAY);
    lcd.clear();
    lcd.print("Error: too many");
    lcd.setCursor(0, 1);
    lcd.print("clicks");
    Serial.println(404); //send number to	indicate error occured
    Serial.println(PulseDuration);
    Serial.println(0);
    delay(2000);
  }
  delay(150);
  lcd.clear();
  lcd.print(PulseDuration);
  lcd.setCursor(4, 0);
  lcd.print("ms");
}




