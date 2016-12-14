/*
  Kühlschrank Turnoffer

*/
enum ApplicationStates{
  Uninitialized = 0,
  Idle = 1,
  SchrankAus = 2
};

ApplicationStates state;
const int schrankRelais = 2;
const int buttonInterrupt = 3;
//4 not used
const int statusLED = 5;
const int buttonLED = 6;
// 7 not used
// 8 not used
const int LED36min = 9;
const int LED16min = 10;
const int LED8min = 11;
const int timeDial = A7;


int ausCounter = 0;
volatile char globalButtonPressed = 0;
int timeDialValue = 0;


void ButtonCallback()
{
 Serial.println("Button released");
 globalButtonPressed = 1; 
 digitalWrite(buttonLED,HIGH);
}


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(schrankRelais, OUTPUT);
  pinMode(buttonInterrupt, INPUT);
  pinMode(statusLED, OUTPUT);
  pinMode(buttonLED, OUTPUT);
  pinMode(LED36min, OUTPUT);
  pinMode(LED16min, OUTPUT);
  pinMode(LED8min, OUTPUT);
  state=Idle;
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(buttonInterrupt), ButtonCallback, RISING);
  Serial.println("Initialisation Complete!");
}


void blinkState()
{
  for(int count = 0; count < state; count++)
  {
    /*for(unsigned char i = 0 ; i < 200; i+=2)
    {
      analogWrite(LED_BUILTIN, i);   // turn the LED on (HIGH is the voltage level)
      delay(1);// wait for a bit
    }*/
    digitalWrite(LED_BUILTIN, HIGH);    // turn the LED off by making the voltage LOW
    digitalWrite(statusLED, HIGH);
    delay(50);// wait for a bit 
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    digitalWrite(statusLED, LOW);
    delay(200);// wait for a bit
    
  }
  
}

void enterState(ApplicationStates requestedState)
{
  state = requestedState;
  if(requestedState == SchrankAus)
  {
    Serial.println("Turning Fridge off");
    
    ausCounter = timeDialValue;
    digitalWrite(buttonLED,LOW);
  }
  else{
    Serial.println("Idle");
    ausCounter = 0;
  }
  
}

void idleAction()
{
  digitalWrite(schrankRelais,HIGH);
}

void schrankAusAction()
{
  digitalWrite(schrankRelais,LOW);
  
  Serial.println(ausCounter);
  ausCounter--;
  if(0 == ausCounter)
  {
    enterState(Idle);
  }
  
}

void doStateAction()
{
  switch(state)
  {
    case Uninitialized:
      setup();
      break;
    case Idle:
      idleAction();
      break;
    case SchrankAus:
      schrankAusAction();
  }
}
void dimm8(int value)
{
  int dimmval = value * 32;
  if(dimmval > 255)
  {
    dimmval = 255;  
  }
 
  analogWrite(LED8min, dimmval); 
}

void TimeLEDS(int value)
{
  if(value < 8)
  {
    digitalWrite(LED36min, LOW);
    digitalWrite(LED16min, LOW);
    dimm8(value);
  }
  else if(value < 16) // > 8
  {
    digitalWrite(LED36min, LOW);
    analogWrite(LED16min, 0);
    digitalWrite(LED8min, HIGH); 
  }
  else if(value < 16 + 8) 
  {
    digitalWrite(LED36min, LOW);
    analogWrite(LED16min, 20);
    digitalWrite(LED8min, LOW); 
  }
  else if(value < 36)
  {
    digitalWrite(LED36min, LOW);
    analogWrite(LED16min, 20);
    digitalWrite(LED8min, HIGH); 
  }
  else if(value < 36 + 8) 
  {
    digitalWrite(LED36min, HIGH);
    analogWrite(LED16min, 0);
    digitalWrite(LED8min, LOW); 
  }
  else if(value < 36 + 16) // 
  {
    digitalWrite(LED36min, HIGH);
    analogWrite(LED16min, 0);
    digitalWrite(LED8min, HIGH); 
  }
  else if(value < 36 + 16 + 8 )
  {
    digitalWrite(LED36min, HIGH);
    analogWrite(LED16min, 20);
    digitalWrite(LED8min, LOW); 
  }
  else
  {
    digitalWrite(LED36min, HIGH);
    analogWrite(LED16min, 20);
    digitalWrite(LED8min, HIGH); 
  }
  
}

int readTimeDial()
{
  int raw10BitDialValue = analogRead(timeDial);
  int raw8bitDialValue = raw10BitDialValue >> 2;
  int dialInMinutes =  raw8bitDialValue *60 /250;
  return dialInMinutes + 2;
}
// the loop function runs over and over again forever
void loop() {

  timeDialValue = readTimeDial();
  if(ausCounter > timeDialValue)
  {
     ausCounter = timeDialValue;
  }
  TimeLEDS(ausCounter);
  if(state > SchrankAus) //out of range
  {
    state = Uninitialized;
  }

    
  if(globalButtonPressed)
  {
    enterState(SchrankAus);
    globalButtonPressed = 0;
  }

  doStateAction();
  

  blinkState();
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
