#include <RunningMedian.h>
#include <math.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

const int wait = 1000;  //milliseconds LEDs are on
const int numPts = 200; //number of data points taken within the wait time
const int interval = wait / numPts; //time between every data point based on the wait time and number of points
int values[numPts];     //array to contain all data points within the wait time
double rTop;
double rBot;
double rValues[5];     //small array to average out last 10 R Values calculated
double redavg[5];
int count = 0;          //count for R Values array

// Running average initialisation & Runnin median initialisation
const int numReadings = 9;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

int med_readings[numReadings];  // the readings for median sort
void bubbleSort();              // Bubble Sort function prototype
int median;



void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //Setup screen
  pinMode(5, OUTPUT);   //Red 650nm laser diode
  pinMode(3, OUTPUT);   //InfraRed 940nm LED
  pinMode(7, INPUT);    //Analog pin for photodiode readings
  display.display();    //Start up display
  delay(1000);
  display.clearDisplay();
  Serial.begin(9600);
  Serial.print("Working");
  while (! Serial);
}
void loop() {
  display.setTextSize(0);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("SpO2: ");  //Display SpO2 value on screen
  int minVal = 1023;  //minimum value taken
  int maxVal = 0;     //maximum value taken
  digitalWrite(5, 1); //Red on
  digitalWrite(3, 0); //IR off
  for (int j = 0; j < numPts; j++) {
    values[j] = analogRead(7);  //Read value of photodiode and assign to position j in values array
    
    // Store maxima/minima as they are read, process data for waveform after.
    if (values[j] < minVal) {   //Assign current reading to minimum value if current reading is smaller than previous minimum
      minVal = values[j];
    }
    else if (values[j] > maxVal) {  //Assign current reading to maximum value if current reading is larger than previous maximum
      maxVal = values[j];
    }

  if (minVal == 0) { //To avoid all null values
    minVal = 1;
  }
  
    // running average:
    total = total - readings[readIndex];      // subtract the last reading
    readings[readIndex] = values[j];          // Assign current reading
    total = total + readings[readIndex];      // Add reading to total
    readIndex = readIndex + 1;                // advance to the next position in the array.
    if (readIndex >= numReadings) {           // if we're at the end of the array...
      readIndex = 0;                          // ...wrap around to the beginning.
    }
    average = total / numReadings;            // calculate the average
    values[j]=average;
    Serial.println(average);
    delay(interval);;        // delay in between reads for stability


// Running Median Calculations taken from http://www.tigoe.com/pcomp/code/arduinowiring/42/

    // Assign current reading
//    med_readings[readIndex] = values[j];
//    Serial.print("New val");
//    Serial.println(med_readings[readIndex]);
//    // Bubble sort readings
//    bubbleSort();
//    //Get middle element
//    median = med_readings[numReadings / 2];
//    Serial.print("Median: ");
//    Serial.println(median);

    // Print everything

    //Edit out 0 values with average of their surrounding or preceeding values.
    if ((j>2) && (values[j] == 0) && (values[j+1]!=0)){
      values[j] = (values[j-1]+values[j+1])/2;
      //Serial.println(values[j]);
    }
    if ((j>2) && (values[j] == 0) && (values[j+1]== 0)){
      values[j] = (values[j-2]+values[j-1])/2;
//      //Serial.println("small ");
    }
    if ((values[j] > 400) && (values[j-10] < 400)){
      values[j] = values[j-10];
      }
    
    //Serial.println(values[j]);
    
  }
  
  double rTop = log((double)minVal / (double)maxVal); //Top portion of the R Value formula ln(Redmin/Redmax)

  minVal = 1023; //Resetting min and max
  maxVal = 0;
  digitalWrite(5, 0); //Red off
  digitalWrite(3, 1); //IR on
  for (int j = 0; j < numPts; j++) {
    values[j] = analogRead(7);
    
    // Store max & min
    if (values[j] < minVal) {
      minVal = values[j];
    }
    else if (values[j] > maxVal) {
      maxVal = values[j];
    }
  

  if (minVal == 0) { //To avoid all null values
    minVal = 1;
  }
  
    // running average:
    total = total - readings[readIndex];      // subtract the last reading
    readings[readIndex] = values[j];          // Assign current reading
    total = total + readings[readIndex];      // Add reading to total
    readIndex = readIndex + 1;                // advance to the next position in the array.
    if (readIndex >= numReadings) {           // if we're at the end of the array...
      readIndex = 0;                          // ...wrap around to the beginning.
    }
    average = total / numReadings;            // calculate the average
    values[j]=average;
    Serial.println(average);
    delay(interval);;        // delay in between reads for stability
    
     //Edit out 0 values with average of their surrounding or preceeding values.
    if ((j>2) && (values[j] == 0) && (values[j+1]!=0)){
      values[j] = (values[j-1]+values[j+1])/2;
    }
    if ((j>2) && (values[j] == 0) && (values[j+1]== 0)){
      values[j] = (values[j-2]+values[j-1])/2;
    }
        if ((values[j] > 400) && (values[j-10] < 400)){
      values[j] = values[j-10];
      }
  }



  double rBot = log((double)minVal / (double)maxVal); //Bottom portion of the R Value formula ln(IRmin/IRmax)
  
  double rValue = rTop / rBot; //R Value formula
  rValues[count] = rValue;     //Assign current R value to position count in array

   if (rValues[4] != NULL) { //if the array is full
    double rSum = 0;
    for (int i = 0; i < 5; i++) {
      rSum = rSum + rValues[i]; //Sum of all R Values in array for average
     }
    double rAvg = rSum / 5;    //Average of R Values
      
//    int check = 0;
//    for (int i = numPts; i > numPts - 10; i--) {
//      check = check + values[i];
//    }
//    check = check/9;
//    if (check > 300){     
//      display.print("---");
//      display.clearDisplay();
//      }

    display.print(100*rAvg);  //Display R Value average x100 and a percentage sign
    display.print("%");
    display.print(" HR: N/A");
    display.display();
    display.clearDisplay();
    display.drawFastHLine(1,30,50, WHITE);
//    Serial.print("SPO2 : ");
//    Serial.print(100*rAvg);
//    Serial.println("%");
    
  }
  else {                        //If array not full, display current R Value
      display.print(100*rValue);
      display.print("%");
      display.print(" HR: lo");
      display.display();
      display.clearDisplay();
//      Serial.print("SPO2 : ");
//      Serial.print(100*rValue);
//      Serial.print("%");

  }

  count++; //Increase count
  count = count % 5; //To make sure count goes from 0 to 9 and back to 0 (circular array)
}

void bubbleSort() {
  int out, in, swapper;
  for(out=0 ; out < numReadings; out++) {  // outer loop
    for(in=out; in<(numReadings-1); in++)  {  // inner loop
      if( med_readings[in] > med_readings[in+1] ) {   // out of order?
        // swap them:
        swapper = med_readings[in];
        med_readings[in] = med_readings[in+1];
        med_readings[in+1] = swapper;
      }
    }
  }
}
