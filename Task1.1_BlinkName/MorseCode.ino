
// Pin definitions.
const int buttonPin = 2;

// Morse code unit definitions.
const unsigned long unit = 100;
const unsigned long DIT = unit;
const unsigned long DAH = unit * 3;
const unsigned long space = unit * 7;

// Define variables used in interrupt.
volatile bool buttonPressed = false;
volatile int currIdx = 0;

// Hardcoded name to be transmitted.
char n[] = "jonathan";

void setup() {
  Serial.begin(9600);
  
  
  pinMode(LED_BUILTIN, OUTPUT);
  // Init as INPUT_PULLDOWN so button will read 1 when pressed, 0 when unpressed.
  pinMode(buttonPin, INPUT_PULLDOWN);

  // Attach interrupt for button to appropriate pin.
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPress, CHANGE);

}

void loop() {
  if(buttonPressed)
  {
    morseOut(n[currIdx]);
    
    if(currIdx > strlen(n))
    {
      // If word is finished transmitted then wait for the space between words
      // Then reset buttonPressed flag and the current index.
      delay(space);
      buttonPressed = false;
      currIdx = 0;
    }
    else
    {
      // We have no finished transmitting word, so we increment to next index.
      currIdx++;
    }
  }
}

void buttonPress()
{
  // When button is pressed, reset current index to start and deluminate LED.
  buttonPressed = true;
  currIdx = 0;
  digitalWrite(LED_BUILTIN, LOW);
}

void morseOut(char letter)
{
  switch(letter)
  {
    case 'a': dit(); dah(); break;
    case 'h': dit(); dit(); dit(); dit(); break;
    case 'j': dit(); dah(); dah(); dah(); break;
    case 'n': dah(); dit(); break;
    case 'o': dah(); dah(); dah();
    case 't': dah();
    default: break;
  }
  delay(DAH);
}


void dit()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(DIT);
  digitalWrite(LED_BUILTIN, LOW);
  delay(DIT);
}

void dah()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(DAH);
  digitalWrite(LED_BUILTIN, LOW);
  delay(DIT);
}