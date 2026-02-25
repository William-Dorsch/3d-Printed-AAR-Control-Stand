// potentiometer analog pin definitions
const int autoBrake = A0;
const int indyBrake = A1;
const int throttle = A3;
const int reverser = A5;
const int dyno = A2;
const int hdlt_front = A4;

// button digital pin definitons
const int horn = 5;
const int bell = 3;
const int alerter = 4;
const int sand = 6;
const int bail_off = 2;
const int gaugeLight = 9;
const int stepLight = 7;
const int ditchLight = 7;
const int engineRun = 12;
const int fuelPump = 11;
const int genField = 10;
const int reverserForward = 13; // microswitch
const int reverserReverse = 14;

// indicator light defintions (unassigned currently, you will need to change these)
const int SAND = 0;
const int WHEEL_SLIP = 0;
const int BRAKE_WARN = 0;
const int PCS_OPEN = 0;

// calibration values
// throttle
int notchCenters[9] = {720, 680, 642, 603, 561, 520, 482, 442, 403};

// dynamic brake
const int dynoMin = 405;
const int dynoMax = 725;

// automatic brake
const int releaseMinimumThreshold = 290;
const int fullService= 520;
const int fullServiceHandleOffThreshold= 600;
const int handleOffEmergencyThreshold= 660;

// independent brake
const int indyMin = 440;
const int indyMax = 750;

// below this point are variables used within code and should not be modified
int sandState = 0;
int bellStateUDP = 0;
int hornStateUDP = 0;
int sandStateUDP = 0;
int automaticBrakePositionUDP = 0;
int sandToggleLogic = 0;
int autoValue = 0;
int mappedValue = 0;
int throttlePositionUDP = 0;
int throttlePosition = 0;  // Potentiometer value
int throttleNotch = 0;     // Calculated throttle notch (0-8)
int independentBrakePositionUDP = 0;
int independentBrakeBailUDP = 0;
int indyValue = 0;
int indyMappedValue = 0;
int dynoMappedValue = 0;
int reverserStateUDP = 0;
int dynamicBrakePositionUDP = 0;
int alerterPressedUDP = 0;
int genFieldUDP = 0;
int conFuelPumpUDP = 0;
int engRunUDP = 0;
int gaugeLightUDP = 0;
int stepLightUDP = 0;
int ditchLightUDP = 0;

// Track previous throttle position to determine direction
int previousThrottlePosition = 0;
bool increasingThrottle = true;

unsigned long autoHighStartTime = 0;
bool autoHighTimerRunning = false;
const unsigned long holdDuration = 500; // 500 ms
int autoMappedValue = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(bell, INPUT_PULLUP);
  pinMode(horn, INPUT_PULLUP);
  pinMode(sand, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(alerter, INPUT_PULLUP);
  pinMode(bail_off, INPUT_PULLUP);
  pinMode(autoBrake, INPUT);
  pinMode(indyBrake, INPUT);
  pinMode(throttle, INPUT);
  pinMode(reverser, INPUT);
  pinMode(dyno, INPUT);
  pinMode(hdlt_front, INPUT);
  pinMode(gaugeLight, INPUT_PULLUP);
  pinMode(stepLight, INPUT_PULLUP);
  pinMode(ditchLight, INPUT_PULLUP);
  pinMode(genField, INPUT_PULLUP);
  pinMode(fuelPump, INPUT_PULLUP);
  pinMode(engineRun, INPUT_PULLUP);
  pinMode(reverserForward, INPUT_PULLUP);
  pinMode(reverserReverse, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  //Serial.println(digitalRead(11));
  autoValue = analogRead(autoBrake);  // Read the potentiometer value (0 to 1023)
  // Serial.println(analogRead(throttle));
  //Serial.print("brake position: ");
  //Serial.println(autoValue);
   //Map the potentiometer value to the range 0-255, with 360 -> 0 and 585 -> 255
  //mappedValue = map(autoValue, 405, 650, 0, 255);
  //Serial.println(analogRead(A3));
   //Constrain the output to stay within the 0-255 range
  //automaticBrakePositionUDP = constrain(mappedValue, 0, 255);
  int autoMappedValue;
  /*
  if (autoValue >= 650) {
    autoMappedValue = 255;
  } else if (autoValue >= 590 && autoValue <= 650) {
    autoMappedValue = 254;
  } else if (autoValue >= 290 && autoValue <= 510) {
    autoMappedValue = map(autoValue, 290, 510, 0, 253);
  } else  if (autoValue > 510 && autoValue < 590) {
    autoMappedValue = 253;
   } else {
     autoMappedValue = 0; // Below 405
  }
  automaticBrakePositionUDP = constrain(autoMappedValue, 0, 255);
  */
  // In your loop() or appropriate function
  if (autoValue >= handleOffEmergencyThreshold) {
    if (!autoHighTimerRunning) {
      autoHighStartTime = millis();
      autoHighTimerRunning = true;
    }

    if (millis() - autoHighStartTime >= holdDuration) {
      autoMappedValue = 255;
    } else {
      autoMappedValue = 254; 
    }
  } else {
    autoHighTimerRunning = false; // reset timer if condition breaks

    if (autoValue >= fullServiceHandleOffThreshold && autoValue < handleOffEmergencyThreshold) {
      autoMappedValue = 254;
    } else if (autoValue > fullService && autoValue < fullServiceHandleOffThreshold) {
      autoMappedValue = 253;
    } else if (autoValue >= releaseMinimumThreshold && autoValue <= fullService) {
      autoMappedValue = map(autoValue, 290, 520, 0, 253);
    } else {
      autoMappedValue = 0;
    }
  }

  automaticBrakePositionUDP = constrain(autoMappedValue, 0, 255);
    //Serial.println(automaticBrakePositionUDP);
  indyValue = analogRead(indyBrake);
  indyMappedValue = map(indyValue, indyMin, indyMax, 0, 255);
  //Serial.println(indyValue);
  independentBrakePositionUDP = constrain(indyMappedValue, 0, 255);
  
  if (bail_off == LOW){
    independentBrakeBailUDP = 1;
  } else {
    independentBrakeBailUDP = 0;
  }
  
  // Read potentiometer value
    throttlePosition = analogRead(throttle);  // Replace A0 with your throttle pin
    //Serial.print("throttle position: ");
    //Serial.println(throttlePosition);

    // Determine which notch the throttle is in
    int minDifference = abs(throttlePosition - notchCenters[0]);
    throttleNotch = 0;

    for (int i = 1; i < 9; i++) {
        int difference = abs(throttlePosition - notchCenters[i]);
        if (difference < minDifference) {
            minDifference = difference;
            throttleNotch = i;
        }
    }

    // Set throttlePositionUDP based on notch
    switch (throttleNotch) {
        case 0:
            throttlePositionUDP = 152;
            throttlePositionUDP = 0;
            break;
        case 1:
            throttlePositionUDP = 128;
            throttlePositionUDP = 1;
            break;
        case 2:
            throttlePositionUDP = 129;
            throttlePositionUDP = 2;
            break;
        case 3:
            throttlePositionUDP = 132;
            throttlePositionUDP = 3;
            break;
        case 4:
            throttlePositionUDP = 133;
            throttlePositionUDP = 4;
            break;
        case 5:
            throttlePositionUDP = 142;
            throttlePositionUDP = 5;
            break;
        case 6:
            throttlePositionUDP = 143;
            throttlePositionUDP = 6;
            break;
        case 7:
            throttlePositionUDP = 134;
            throttlePositionUDP = 7;
            break;
        case 8:
            throttlePositionUDP = 135;
            throttlePositionUDP = 8;
            break;
    }
    /*
    // Print to serial for debugging
    Serial.print("Throttle Position: ");
    Serial.print(throttlePosition);
    Serial.print(" | Notch: ");
    Serial.print(throttleNotch);
    Serial.print(" | UDP Value: ");
    Serial.println(throttlePositionUDP);
    */
    
  
  int dynoValue = analogRead(dyno);  // Read the potentiometer value (0 to 1023)
  //Serial.print("dynamic brake value: ");
  //Serial.println(dynoValue);
   //Map the potentiometer value to the range 0-255, with 360 -> 0 and 585 -> 255
  dynoMappedValue = map(dynoValue, dynoMin, dynoMax, 0, 255);
  
   //Constrain the output to stay within the 0-255 range
  dynamicBrakePositionUDP = constrain(dynoMappedValue, 0, 255);
  
  // archived potentiometer-based reverser code
/*
  int reverserPotentiometer = analogRead(reverser);\
  //Serial.println(reverserPotentiometer);
  if (reverserPotentiometer > 750){
    reverserStateUDP = 255;
  } else if (reverserPotentiometer < 625){
    reverserStateUDP = 0;
  } else {
    reverserStateUDP = 128;
  }*/

if (digitalRead(reverserForward) == LOW){
  reverserStateUDP = 255;
} else if (digitalRead(reverserReverse) == LOW){
  reverserStateUDP = 0;
} else {
  reverserStateUDP = 128;
}

  int hornState = digitalRead(horn);
  if (hornState == LOW) {
    hornStateUDP = 1;
  } else {
    hornStateUDP = 0;
  }
  
  int bellState = digitalRead(bell);
  if (bellState == LOW) {
    bellStateUDP = 1;
  } else {
    bellStateUDP = 0;
  }

  if (digitalRead(sand) == LOW){
    sandStateUDP = 1;
  } else {
    sandStateUDP = 0;
  } 
   
  if (digitalRead(bail_off) == LOW){
    independentBrakeBailUDP = 1;
  } else {
    independentBrakeBailUDP = 0;
  }

  if (digitalRead(alerter) == LOW){
    alerterPressedUDP = 1;
  } else {
    alerterPressedUDP = 0;
  }

  genFieldUDP = digitalRead(genField);
  engRunUDP = digitalRead(engineRun);
  conFuelPumpUDP = digitalRead(fuelPump);
  gaugeLightUDP = digitalRead(gaugeLight);
  stepLightUDP = digitalRead(stepLight);
  ditchLightUDP = digitalRead(ditchLight);
  
  Serial.print("RS:");
  Serial.print(reverserStateUDP);
  Serial.print(",TH:");
  Serial.print(throttlePositionUDP);
  Serial.print(",DB:");
  Serial.print(dynamicBrakePositionUDP);
  Serial.print(",IB:");
  Serial.print(independentBrakePositionUDP);
  Serial.print(",AB:");
  Serial.print(automaticBrakePositionUDP);
  Serial.print(",IL:");
  Serial.print(independentBrakeBailUDP);
  Serial.print(",HR:");
  Serial.print(hornStateUDP);
  Serial.print(",BL:");
  Serial.print(bellStateUDP);
  /*
  Serial.println(",HLF:");
  Serial.print(frontHeadlightUDP);
  Serial.print(",HLR:");
  Serial.print(rearHeadlightUDP);
  */
  Serial.print(",AL:");
  Serial.print(alerterPressedUDP);
  
  Serial.print(",SD:");
  Serial.print(sandStateUDP);
  
  Serial.print(",GF:");
  Serial.print(!genFieldUDP);
  Serial.print(",FP:");
  Serial.print(!conFuelPumpUDP);
  Serial.print(",ER:");
  Serial.print(!engRunUDP);
  Serial.print(",GL:");
  Serial.print(!gaugeLightUDP);
  Serial.print(",SL:");
  Serial.print(!stepLightUDP);
  Serial.print(",DL:");
  Serial.println(!ditchLightUDP);
  /*
  Serial.print(isolSwUDP);
  Serial.print(",STR:");
  Serial.print(startSwUDP);
  Serial.print(",STP:");
  Serial.print(startSwUDP);
  Serial.print(",HBA:");
  Serial.print(handbrakeApplyUDP);
  Serial.print(",HBR:");
  Serial.print(handbrakeReleaseUDP);
  Serial.print(",EOTE:");
  Serial.print(eotEmergencyUDP);
  Serial.print(",WW:");
  Serial.print(frontWiperUDP);
  Serial.print(",GL:");
  Serial.print(autoBrakeCutoutUDP);
  Serial.println();

  Serial.flush();
  */

  delay(50);
}
