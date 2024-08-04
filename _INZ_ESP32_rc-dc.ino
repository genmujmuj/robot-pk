#include <Wire.h>
#include <HMC5883L_Simple.h>

#define CH1 A0
#define CH2 A1
#define CH3 A2
#define CH4 A3

#define ECHO1 7
#define ECHO2 8
#define TRIG_PIN 9

#define LED_PIN 2

// left/right motor 1 PWM
#define M1F 5
#define M1B 6

// left/right motor 2 PWM
#define M2F 10
#define M2B 11

HMC5883L_Simple Compass;

const int trm = 15;  //trim value for the chValues

int ch1Value;
int ch2Value;
int ch3Value;

int M1V=0;
int M2V=0;

int prevM1V = 0;
int prevM2V = 0;

int dis_1; //ECHO1
int dis_2; //ECHO2

int F = 0;

/*Function for reading the PPM values from FS-iA6B*/
int readChannel(int channelInput){
  int minch = -100;
  int maxch = 100;
  int ch = pulseIn(channelInput, HIGH, 30000);
  if (ch < 100) return 0;
  return map(ch, 1000, 2000, minch, maxch);
}

void cleanRead(){
 ch1Value = readChannel(CH1);  // Reading speed control
 ch2Value = readChannel(CH2);  // Reading turning control
 ch3Value = readChannel(CH3);  // Reading auto switch
 if (abs(ch1Value) < trm) {ch1Value = 0;}  // Trim the values
 if (abs(ch2Value) < trm) {ch2Value = 0;}
 if (abs(ch3Value) < trm) {ch3Value = 0;}
}

/*function for driving the PWM for motor drivers, takes chXValue and converts it to PWM signals*/
void driveMotors(int M1speed,int M2speed){
  M1speed = constrain(M1speed, -100, 100);
  M2speed = constrain(M2speed, -100, 100);

// if motor changed direction, wait for 50 ms with both PWM pins at 0
  if ((M1speed > 0 && prevM1V < 0) || (M1speed < 0 && prevM1V > 0)) {
    analogWrite(M1F, 0);
    analogWrite(M1B, 0);
    delay(50);
  }
  if ((M2speed > 0 && prevM2V < 0) || (M2speed < 0 && prevM2V > 0)) {
    analogWrite(M2F, 0);
    analogWrite(M2B, 0);
    delay(50);
  }

  if (M1speed > 0){
    analogWrite(M1F,map(abs(M1speed),0,100,0,255));
    analogWrite(M1B,0);
    }
  else if (M1speed < 0){
    analogWrite(M1F,0);
    analogWrite(M1B,map(abs(M1speed),0,100,0,255));
    }
  else if (M1speed == 0){
    analogWrite(M1F,0);
    analogWrite(M1B,0);
    }
    
  if (M2speed > 0){
    analogWrite(M2F,map(abs(M2speed),0,100,0,255));
    analogWrite(M2B,0);
    }
  else if (M2speed < 0){
    analogWrite(M2F,0);
    analogWrite(M2B,map(abs(M2speed),0,100,0,255));
    }
  else if (M2speed == 0){
    analogWrite(M2F,0);
    analogWrite(M2B,0);
    }
    
  prevM1V = M1speed;
  prevM2V = M2speed;
  
  delay(50);
}

int readDistance(int echo) {
  long duration;
  int distance;
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(echo, HIGH);
  distance = duration * 0.034 / 2;

  return distance;
}

void turn(int dir){
    float deg_1;
    float deg_2;
    float deg_target;
    float deg_corected;
    float correction = 0;
    int v = 50;
    driveMotors(0,0);
    deg_1 = Compass.GetHeadingDegrees();
    deg_target = deg_1 + (45*dir);
    Serial.print("deg_1 = ");
    Serial.print(deg_1);
    Serial.print(" ;deg_target =");
    Serial.println(deg_target);
    if (deg_target < 0 || deg_target >= 360) {
      deg_target = deg_target - (360*dir);
      correction = 360 * dir;
      }
    else {correction = 0;}
    Serial.print("correction: ");
    Serial.println(correction);
    while (deg_corected <= deg_target && dir > 0){
      Serial.print("TURNING RIGHT deg_corected = ");
      deg_2 = Compass.GetHeadingDegrees();
      deg_corected = deg_2 + correction;
      Serial.println(deg_corected);
      driveMotors(v*dir,v*dir*(-1));
      delay(50);
    }
    while (deg_2 >= deg_target && dir < 0){
      Serial.print("TURNING LEFT deg_2 = ");
      deg_2 = Compass.GetHeadingDegrees();
      Serial.println(deg_2);
      driveMotors(v*dir,v*dir*(-1));
      delay(50);
      }
    driveMotors(0,0);
    Serial.println("finished");
  }

void setup() {
  Wire.begin();
  Compass.SetDeclination(23, 35, 'E');
  Compass.SetSamplingMode(COMPASS_SINGLE);
  Compass.SetScale(COMPASS_SCALE_130);
  Compass.SetOrientation(COMPASS_HORIZONTAL_X_NORTH);
  
  pinMode(CH1, INPUT);
  pinMode(CH2, INPUT);
  pinMode(CH3, INPUT);

  pinMode(ECHO1, INPUT);
  pinMode(ECHO2, INPUT);

  pinMode(M1F, OUTPUT);
  pinMode(M1B, OUTPUT);
  pinMode(M2F, OUTPUT);
  pinMode(M2B, OUTPUT);
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  Serial.begin(9600);
}

void loop() {
  
 cleanRead(); 
 
 while (ch3Value < 0){
  /* Drive with a radio control */
  cleanRead();

//  if(ch3Value == 0){
//    digitalWrite(LED_PIN,1);
//    }
//  else {
//    digitalWrite(LED_PIN,0);
//    }
    
  if (ch2Value == 0){
    M1V = ch1Value;
    M2V = ch1Value;
  }
  else {
    M1V = ch1Value + ch2Value;
    M2V = ch1Value - ch2Value;
   }
  driveMotors(M1V,M2V);
  Serial.print(M1V);
  Serial.print(" - - ");
  Serial.println(M2V);
 }
 while (ch3Value == 0){
  cleanRead();
  float heading = Compass.GetHeadingDegrees();
   
  Serial.print("Heading: \t");
  Serial.println( heading );   
  delay(1000);
  } 
 while (ch3Value > 0){   /* Drive with auto mode */
  cleanRead();
//  dis_1 = readDistance(ECHO1);
//  dis_2 = readDistance(ECHO2);
  
  if (ch2Value > 50){
    delay(100);
    turn(1);
    }
  else{
    driveMotors(0,0);
    }
//  cleanRead();
//  dis_1 = readDistance(ECHO1);      // 1 = front, 2 = left
//  dis_2 = readDistance(ECHO2);
//  
//  if (dis_2){
//    Serial.print("F");
//    Serial.println(F);
//    delay(500);
//    Serial.println("R90");
//    turn(1);
//    F = 0;
//    }
//  else {
//    driveMotors(60,60);
//    F=F+1;
//    delay(250);
//    }
//  Serial.println(dis_1);
//  Serial.println(dis_2);
  }

}
