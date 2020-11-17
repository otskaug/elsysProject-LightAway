#include "TFMiniPlus.h" // from https://github.com/senegalo/tfmini-plus by Karim Mansour
#include <Servo.h>  
TFMiniPlus tfmini;
Servo servo;
int angle = 0;  // current angle written to the servo
int numbPos = 0; // number of position data gathered during one  "for(angle = 0; angle < 181; angle+=2)" type loop.
uint16_t dist; // distance value in cm from the sensor
uint16_t strength; // value of the strength of the signal, higher value indicates that the meassurement have less inaccuracy.
double temp; // temperature value from sensor, not used in this project
float pos_cm; // distance value from sensor to meassured position point, only x-axis distance taken

double dist_array[91]; // array of distance values from stationary objects - if tree in the way it should not trigger sensor.
double pos_array[91]; //array of all gathered position during one sweep of 180 degrees.
double voltageArray[4]; // array with the voltage values that shall be passed to the light sources after each 180 degree sweep.
int lightsourcePosArray[4] = {34, 14, -14, -34}; // array of relative position in cm between the indvidual light sources and the sensor.

int distLim = 25; //limit for how far away the person can be from the light source before it does not light up.
int distFaktor = 10.2; // the factor we multiply the distance with as long as the distance is lower than distLim.
int numbLight = 4; // the number of individual light sources in the setup
int minimumDist = 5; // The minimum distance that the sensor can meassure accuratly 

void setup() {

  servo.attach(8); // attach servo to pin 8 - makes PWM pin 9 and 10 unavailable, therefore we can have a maximum of 4 PWM pins in use while having a servo attached to the arduino
  servo.write(angle); // writes an angle (in degrees) to the servo

  Serial.begin(115200);  // initialize serial
  tfmini.begin(&Serial);  // Pass the Serial class to the tfmini
 
   for(int i = 0; i < 181; i+=2){
    int k = (i)/2;
    dist_array[k] = sqrt(20*20+(cos(radians(i))*100)*(cos(radians(i))*100)); // the values for expected distance measured by the sensor for each angle.
    //Serial.println("Angle: " + String(i) + ", dist_value: " + String(dist_array[k]));
  } 
}

void loop() {

  for(angle = 0; angle < 181; angle+=2){ // One "Swipe". Had to increase the angle by two at a time due to problems with memory storage space for dynamic variables

   servo.write(angle); // writes an angle (in degrees) to the servo
  // read the data frame sent by the mini, if valid dataframe do calculations
  if (tfmini.readData()) {
    // Distance "default in CM"
    dist = tfmini.getDistance();

    if(dist > minimumDist && dist < dist_array[numbPos]){ //The sensor cannot accurately meassure distances less than 5cm, also check if the distance value for a given angle is less than expected distance for than angle (say if an object is in place and always meassured at a specific angle, we can pass only distance values that are less than the expected dist value for that angle).
      pos_cm = dist * cos(radians(angle)); // calculate the distance (in cm) on the path from the sensor to the object using trigonometry
      pos_array[numbPos] = pos_cm; // add the meassured distance to the array containing all gathered position data for one "swipe"
      numbPos++; //add one to the numbPos variable to indicate that we have one more position point for the current "swipe"
      //Serial.println("Angle from plus: " + String(angle) + "; " + "pos_cm: " + String(pos_cm));
    }
    
    if(angle == 180){ // when one sweep is done do calculations to find voltage for each light source
      Serial.println("inside if plus"); //debugging
      Serial.println("numbPos: " + (String)numbPos); //debugging, should always be 91 - can be seen that will be lower if one shakes the sensor a lot while it gatheres data

      double shortestDistLP[] = {99999.9, 99999.9, 99999.9, 99999.9}; // An array storing the currently calculated shortest distance between any given "light point" and position data from sensor

      // Loop to find the shortest distance between any meassured position and the closest light source
      for(int i = 0; i < numbLight; i++){
        for(int k = 0; k < numbPos; k++){
          if(abs(pos_array[k] - lightsourcePosArray[i]) < shortestDistLP[i]){ // checks if there are any position data from the swipe that gives a shorter distance to a given light source than the currently stored value
            shortestDistLP[i] = abs(pos_array[k] - lightsourcePosArray[i]); // sets the new shortest distance, takes the absolute value as cosine function used to calculate the distance return negative values for angles between 90 and 180.
          }
        }
      } 

    // Loop to set voltage output on the PWM pins based on the shortest distance to the light source associated to the given pin
    for(int i = 0; i < numbLight; i++){
      if(shortestDistLP[i] > distLim){ // If shortestDistLP equals 99999.9 no position data was found to be closer to the light source, and therefore it should not light up
        voltageArray[i] = 0;
      } else {
        voltageArray[i] = 255 - shortestDistLP[i]*distFaktor; // the function to decide the voltage output given a distance value. Now set to a linear equation with volt as y value and distance as x value. AnalogWrite for PWM pins take in value from 0 to 255, therefore if shortest distance equals 0 we want the volt output to be 255. 
                                                             //The gradient (a in y=ax + b) is set to 12.75 so that the voltage value should be set to 0 if the distance is more than 20cm.
      } 
    }

    for(int i = 0; i < 4; i++){
     Serial.println("VoltageArray[" + String(i) + "]: " + String(voltageArray[i]) + ", ShortestDistLP[" + String(i) + "]: " + String(shortestDistLP[i])); //debug print
    }

    // write the calcutaled voltage values for all the light sources to their respective PWM pin
    analogWrite(3, voltageArray[0]);
    analogWrite(5, voltageArray[1]);
    analogWrite(6, voltageArray[2]);
    analogWrite(11,voltageArray[3]);

    numbPos = 0; // reset the variable that keeps track of the amount of stored position data for one swipe at the end of the current swipe

    //Serial.println("End of if-statement plus"); // debug print 
    servo.write(180);     
   }
  } // if tfmini statement 
 } // angle plus for loop end


for(angle = 180; angle > -1; angle-=2){
  servo.write(angle);
  delay(10);
  
}
servo.write(0);
/*
   // this code is equal to the loop above that goes from 0 to 180 degrees, only difference is that this loop goes from 180 down to 0 degrees. Therefore, look at the loop above for comments 
   
   for(angle = 160; angle > 19; angle-=2){

    servo.write(angle); // writes an angle (in degrees) to the servo

    delay(5);
  if (tfmini.readData()) {

    dist = tfmini.getDistance();

    if(dist > 1 && dist < dist_array[numbPos]){      

      pos_cm = dist * cos(radians(angle));
      pos_array[numbPos] = pos_cm;

      //Serial.println("Angle from minus: " + String(angle) + "; " + "pos_cm: " + String(pos_cm));


      numbPos++;
    }

    if(angle == 20){
      Serial.println("inside if minus");
      Serial.println("numbPos: " + (String)numbPos);
      
      double shortestDistLP[] = {99999.9, 99999.9, 99999.9, 99999.9};
      for(int i = 0; i < 4; i++){
        for(int k = 0; k < numbPos; k++){
          if(abs(pos_array[k] - lightsourcePosArray[i]) < shortestDistLP[i]){
            shortestDistLP[i] = abs(pos_array[k] - lightsourcePosArray[i]);
          }
        }
      } 
      
    for(int i = 0; i < 4; i++){
      if(shortestDistLP[i] > 30){
        voltageArray[i] = 0;
      } else {
        voltageArray[i] = 255 - shortestDistLP[i]*8.5;
      } 
    }

    for(int i = 0; i < 4; i++){
     Serial.println("VoltageArray[" + String(i) + "]: " + String(voltageArray[i]) + ", ShortestDistLP[" + String(i) + "]: " + String(shortestDistLP[i]));
    }

    analogWrite(3, voltageArray[0]);
    analogWrite(5, voltageArray[1]);
    analogWrite(6, voltageArray[2]);
    analogWrite(11,voltageArray[3]);
      
    numbPos = 0;

    //Serial.println("End of if-statement minus");

    servo.write(20);
   }
  }
 }
 */
} // void loop end
