#include <Servo.h>  //Servo library
/*
  ESTRATÈGIA DE NAVEGACIÓ
  Navega recte durant un nombre random de segons 
  Gira un angle random
*/

// BOTÓ
unsigned int buttonPin = 5;

// SENSORS LINEA
unsigned int lineSensorPin[3] = {A1,A2,A3}; // {L,C,R}
unsigned int lineSensorThreshold[3] = {200,200,200};
unsigned int lineSensorVal[3] = {};
boolean lineSensorDet[3] = {};

// SENSORS DISTÀNCIA
unsigned int distSensorPin[2] = {A4,A5}; // {L,R}
unsigned int distSensorVal[3] = {};

unsigned int cruiseTimer;

int whenToTurn;
int angleToTurn;

int margin[3] = {200,200,200};

// SERVOS
Servo servoL; 
Servo servoR;   

extern volatile unsigned long timer0_millis; //pel reset de millis

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(13,OUTPUT); // LED
  servoL.attach(2);
  servoR.attach(3);
  servoR.write(89);
  servoL.write(89);
  setCruisingParameters(1);  // defineix l'angle que girarà i quan (com més gran el multiplier més tarda en girar)
}

void loop() {
  checkButton();    // si el botó està pulsat --> calibra
  readLine();       // llegeix els sensors de línea
  checkLine();      // què fer en cada cas quan troba la línea
  randomCruising(); // estratègia base: avança durant un temps random i gira uns graus random
  //printSensors();

  if (millis()>15000){ // reset de millis perquè no peti i tal
    setMillis(183);
  }
}

void setMillis(unsigned long new_millis){ // reset de millis
  uint8_t oldSREG = SREG;
  cli();
  timer0_millis = new_millis;
  SREG = oldSREG;
}

void turn(int angle){ //gira un angle determinat entre -180 i 180
  if (angle < 0) angle+= 360;
  else if(angle > 360) angle = angle%360;
  if (angle > 180){ 
    angle = 360 - angle;
    //Serial.println(angle);
    long sec = (long) 400*angle/180; //calcula los segundos de 
    //Serial.println(sec);
    servoR.write(180);
    servoL.write(180);
    delay(sec);
  }
  else {
    long sec = (long) 400*angle/180;
    servoR.write(0);
    servoL.write(0);
    delay(sec);
  }
}

void goForward(){ //potser va endarrere, si és el cas canviar 0 per 180
  servoR.write(0);
  servoL.write(180);
  Serial.println("forward");
}

void goBackwards(){
  servoR.write(180);
  servoL.write(0);
  Serial.println("backwards");
}

void setCruisingParameters(int multiplier){ // defineix l'angle que girarà i quan (com més gran el multiplier més tarda en girar)
  whenToTurn = random (3,10)*100*multiplier; // defineix el temps que tardarà en girar
  cruiseTimer = millis();
  angleToTurn = random (-180,180); //13.416 és l'arrel quadrada de 180 
  //--> està fet així perquè siguin més probables angles petits
}

void randomCruising() { // avança durant un temps random i gira uns graus random
  if (cruiseTimer + whenToTurn < millis()){
    turn(angleToTurn);
    setCruisingParameters(1);
  } else {
    //que vagi endavant recte
    goForward();
  }
}

void checkLine() { // què fer en cada cas quan troba la línea
  // sensors: {L,C,R}
  if (lineSensorDet[1]){
    Serial.println("middle sensor");
    goBackwards();
    delay(100);
    turn(random (-180,180));
    goForward();
    //go back + turn
  }
  if (lineSensorDet[0]){
    Serial.println("left sensor");
    //turn R (almost 180º)
    turn(180);
    setCruisingParameters(1);
  } else if (lineSensorDet[2]){
    Serial.println("right sensor");
    //turn L (almost 180º)
    turn(-180);
    setCruisingParameters(1);
  }
}

void checkButton() { // si el botó està pulsat calibra
  boolean buttonState = digitalRead(buttonPin);
  if (buttonState == LOW) {
    digitalWrite(13,HIGH); // encèn LED
    Serial.println("calibrating");
    calibrate(30); // redefineix el llindar
    digitalWrite(13,LOW); // apaga el LED
  }
}

void calibrate (int N){ // redefineix el valor llindar  a partir de les mesures
  // suma els valors dels sensors durant N iteracions
  int sum[3] = {0,0,0};
  for (int i=0;i<N;i++){
    readLine();
    for (int i=0;i<3;i++){
      sum[i] += lineSensorVal[i];
    }
  }
  // fa la mitjana i defineix el llindar 
  for (int i=0;i<3;i++){
    lineSensorThreshold[i] = sum[i]/N - margin[i];
  }
}

void readLine(){ // llegeix els sensors de línea defineix la variable booleana lineSensorDet
  for (int i=0;i<3;i++){
    lineSensorVal[i] = analogRead(lineSensorPin[i]); // legeix els sensors de línea
    lineSensorDet[i] = lineSensorVal[i]<lineSensorThreshold[i]; // boolean línea o no
  }

}

void readDist (){
  for (int i=0;i<2;i++){
    distSensorVal[i] = analogRead(distSensorPin[i]);
  }
}

void printSensors(){
  for (int i=0;i<3;i++){
    int s = analogRead(lineSensorPin[i]); // legeix els sensors de línea
    Serial.print(s);
    Serial.print(" ");
  }
  Serial.println();

}
