//Define DIT's and DAH's

const unsigned long DIT = 300;
const unsigned long DAH = 900;


void morse_s() {
  for (int i = 0; i < 3; i++) { //3 short pulses
    digitalWrite(LED_BUILTIN, HIGH);
    delay(DIT);
    digitalWrite(LED_BUILTIN, LOW);
    delay(DIT);
  }
  
}

void morse_o() {
  for (int i = 0; i < 3; i++) { //3 long pulses
    digitalWrite(LED_BUILTIN, HIGH);  
    delay(DAH);
    digitalWrite(LED_BUILTIN, LOW);
    delay(DIT);
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  morse_s();
  delay(DIT*2);
  morse_o();
  delay(DIT*2);
  morse_s();
  delay(DIT*7);
}
