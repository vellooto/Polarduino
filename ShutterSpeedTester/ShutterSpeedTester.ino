int photoDiode = A0;
int sensorValue;
float voltage;
unsigned long t1,t2;
boolean sensorState;
boolean sensorLastState;
unsigned long openTime;


void setup() {
  
  pinMode(photoDiode, INPUT);
  Serial.begin(9600);
}

void loop() {
    
  while (analogRead(photoDiode) > 50){}
  t1 = micros();

      
  while (analogRead(photoDiode) < 50){}
  t2 = micros();
  
  openTime = (t2 - t1);
  
  Serial.print("Shutter speed: ");
  Serial.print(openTime);
  Serial.print("us\n");
  
}
