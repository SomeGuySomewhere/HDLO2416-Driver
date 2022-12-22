/*
  HDLO 2416 driver Version 0.5
  20-12-22
  Kevin van Dijk
  9008402
  TCR Schiedam

  To do list:

  Make it possible to write a string to one display
  Make support for multiple displays
  Make support to scroll characters across 1 or more displays
  Make the code quicker


  Already done list:

  Write a single character to a single (selectable) segment
  Write a single number to a (selectable) segment
  Write a bigger number to one display, and it will automatically devide it over all segment (Max 9999 as of version 0.5)



  Problems with the code:

  There arent any that i know of yet. But there is the fact that this display is not meant to be driven by a MicroController, but by an 8-bit CPU.
  How so? because the HDLO had Data pins wich would connect to the Data bus of a cpu like a Zilog z80 or a MOS 6502, and would've used logic gates to select the segments and individual displays.
  Add the fact that an ATMega 328pu based Arduino board (Uno, Micro, pro-mini, etc) don't have any complete ports available so we cant use any direct port manipulation.
  This makes it so we have to convert everything we want to write to the display to paralell binary, spread across Digital output pins D2..D8. Could have used a shift register (wich was the plan)
  but if there is no need for more components, its a added benefit to not use those!


 Connection scheme
 Arduino pin    HDLO pin name   HDLO Pin
  D2            D0              11
  D3            D1              12
  D4            D2              13
  D5            D3              14
  D6            D6              16
  D7            D5              17
  D8            D4              18

  D9            BL              18
  D10           WR^             6
  D11           CU^             5
  D12           CLR^            3

  A0            CUE             4
  A1            A0              8
  A2            A1              7


  Refrences:

  HDLO 2416 Datasheet

  https://arnowelzel.de/en/led-display-dl-2416-hdlo-2416
  (Could've copied his code, but thats not fun!)

*/



//============HDLO pin definitions=================
const int numPins = 7;                              // Number of output pins
const int outputPins[numPins] = { 2, 3, 4, 5, 6, 7, 8 };  // Array of pin numbers

//using constants saves some RAM
const int BL = 9;
const int WR = 10;
const int CU = 11;
const int CLR = 12;

const int CUE = A0;

const int segsel1 = A1;
const int segsel2 = A2;
//===================================================
const int potPin = A4;

void setup() {
  // Set each pin as an output
  for (int i = 0; i < numPins; i++) {   //Quick setup for the Data pins
    pinMode(outputPins[i], OUTPUT);
  }

  pinMode(BL, OUTPUT);
  pinMode(WR, OUTPUT);
  pinMode(CU, OUTPUT);
  pinMode(CLR, OUTPUT);
  pinMode(CUE, OUTPUT);

  pinMode(segsel1, OUTPUT);
  pinMode(segsel2, OUTPUT);

  Serial.begin(9600);

  displayInit();    //init display is not needed, but its good practice to clear it
}
void loop() {

  for (int i = 0; i < 9999; i++) {
    writeNumber(i);
    delay(50);
//    displayShow();
  }
  /*
  int sensVal = analogRead(potPin);   //used to test the response of the display with this code, its pretty good!
  int mappedVal = map(sensVal, 0, 1024, 0, 100);
  writeNumber(mappedVal);
  delay(100);
  */
}

void displayInit() {
  digitalWrite(WR, HIGH);
  digitalWrite(CLR, HIGH);
  digitalWrite(BL, HIGH);
  digitalWrite(CU, HIGH);
  digitalWrite(CUE, LOW);
  digitalWrite(WR, LOW);
  delay(1);
  digitalWrite(WR, HIGH);
}

void displayShow() {        //Writes the data on the D0..D6 pins to the display by quickly pulsing the WR
  digitalWrite(WR, LOW);
  digitalWrite(WR, HIGH);
}


void writeBinarychar(int value) {     //Example: writeBinary("A"); will translate into ASCII 65 wich the display can understand. DOES NOT WRITE A STRING, JUST ONE CHARACTER!
  for (int i = 2; i < 9; i++) {
    // Set the pin to high or low depending on the value of the current bit
    digitalWrite(i, value & (1 << (i - 2)));
    int valin = value & (1 << (i - 2));
    //=====Just some debugging========
    Serial.print("D ");
    Serial.print(i - 2);
    Serial.print(" = ");
    if (valin >= 1) {
      Serial.println("1");
    } else {
      Serial.println("0");
    }
    //================================
  }
  displayShow();
}

void writeNumber(int value) {               //Writes a 1-4 digit number to a display
  int first = ((value / 1000) % 10) + 48;   //These lines disect the 1-4 digit number into individual digits
  int second = ((value / 100) % 10) + 48;   // We need to add 48 to get in the HDLO row of numbers. without this we will get the first row of characters described in the datasheet
  int third = ((value / 10) % 10) + 48;     //
  int fourth = (value % 10) + 48;           //
  /*
  Serial.println("==========");
  Serial.print("First = "); Serial.println(first - 48);
  Serial.print("Second = "); Serial.println(second - 48);
  Serial.print("Third = "); Serial.println(third - 48);
  Serial.print("Fourth = "); Serial.println(fourth - 48);
  Serial.println("==========");
  */
  for (int b = 1; b < 5; b++) {
    for (int i = 2; i < 9; i++) {
      //Serial.print("B = "); Serial.println(b);
      if (b == 4) {
        digitalWrite(i, fourth & (1 << (i - 2)));                     //Write the pin with the ANDed data of the digit.
        //Serial.print("Fourth2 = "); Serial.println(fourth - 48);
        segSelect(1);                                                 //select a segment
      }
      if (b == 3) {
        digitalWrite(i, third & (1 << (i - 2)));
        //Serial.print("Third2 = "); Serial.println(third - 48);
        segSelect(2);
      }
      if (b == 2) {
        digitalWrite(i, second & (1 << (i - 2)));
        //Serial.print("Second2 = "); Serial.println(second - 48);
        segSelect(3);
      }
      if (b == 1) {
        digitalWrite(i, first & (1 << (i - 2)));
        //Serial.print("First2 = "); Serial.println(first - 48);
        segSelect(4);
      }
    }
    displayShow();
  }
}
void writeString(char string[]) {   //not yet implemented, will write a string in the future
  int len = strlen(string);
  //  Serial.println(len);
}


void segSelect(int segment) {     //selects the wanted segment
  if (segment > 4) {              //since one display does not have more than 4 segments, we make 4 the max
    segment = 4;
  }
  switch (segment) {              //quick and easy switch to select the segments
    case 1:
      digitalWrite(segsel1, LOW); //segment 1 (Right)
      digitalWrite(segsel2, LOW);
      //delay(500);
      break;

    case 2:
      digitalWrite(segsel1, HIGH);  //segment 2
      digitalWrite(segsel2, LOW);
      //delay(500);
      break;

    case 3:
      digitalWrite(segsel1, LOW);   //segement 3
      digitalWrite(segsel2, HIGH);
      //delay(500);
      break;

    case 4:
      digitalWrite(segsel1, HIGH);  //segment 4 (left)
      digitalWrite(segsel2, HIGH);
      //delay(500);
      break;
  }
}
