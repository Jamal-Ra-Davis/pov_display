

void setup() {
  // put your setup code here, to run once:
  Serial1.begin(115200);
  SerialUSB.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  while(SerialUSB.available() > 0)
  {
    Serial1.write(SerialUSB.read());
  }
  while(Serial1.available() > 0)
  {
    SerialUSB.write(Serial1.read());
  }
  delay(5);
}
