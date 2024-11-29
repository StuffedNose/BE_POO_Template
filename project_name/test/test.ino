#include <Wire.h>
#include "rgb_lcd.h"

class Game
{
  // Attribute
  protected:
  static TouchSensor touchSensor(D6);
  static LedActuator led(D7);
  static Button button(D8);
  static rgb_lcd lcd;
  int step;

  public:
  // Constructor
  Game():step(0){}

  // Assessor
  int getStep()
  {
    return step;
  }

  void setStep(int stepNumber)
  {
    step = stepNumber;
  }
  
  // Methods
  void nextStep()
  {
    step++;
  }
};



class Tutorial : public Game
{
  public:
  void playTutorial()
  {
    switch(getStep())
    {
      case(0):
        // Initialization
        lcd.begin(16, 2);
        lcd.print("Bonjour, appuyez sur le bouton!");
        nextStep();
      break;

      case(1):
        
      break;

      default:
        setStep(0);
      break;
    }
  }
};

class Actuator
{
  protected:
  // Attribute
  uint8_t pin;

  public:
  // Constructor
  Actuator(const uint8_t& pin)
  {
    this->pin = pin;
    pinMode(this->pin,OUTPUT);
  }
};

class LedActuator : public Actuator
{
  private:
    bool ledStatus;

  public:
  // Constructor
  LedActuator(const uint8_t& pin):Actuator(pin){}

  // Assessor
  void setLedStatus(bool state)
  {
    digitalWrite(pin, state);
  }

  bool getLedStatus()
  {
    return ledStatus;
  }

  // Method
  void swapLedStatus()
  {
    setLedStatus(!getLedStatus());
  }
};

class Sensor
{
  protected:
  // Attribute
  uint8_t pin;

  public:
  // Constructor
  Sensor(const uint8_t& pin)
  {
    this->pin = pin;
    pinMode(this->pin,INPUT);
  }
};

class TouchSensor : public Sensor
{
  private:
    // Attribute
    bool touchStatus;

  public:
    // Constructor
    TouchSensor(const uint8_t& pin):Sensor(pin){}

    // Assessor
    bool getTouchStatus()
    {
      touchStatus = digitalRead(pin);
      return touchStatus;
    }
};

class Button : public Sensor
{
   private:
    // Attribute
    bool status;

    public:
    // Constructor
    Button(const uint8_t& pin):Sensor(pin){}

    // Assessor
    bool getStatus()
    {
      return status;
    }

    // Method
    bool readStatus()
    {
      status = digitalRead(pin);
      return status;
    }

    bool wasPushed()
    {
      return (this.getStatus() == 1) && (this.readStatus() == 0));
    }
};

void tutorial(rgb_lcd& lcd, Bouton & bouton)
{

  switch(tutorial_step)
  {
    case(0):
      //lcd.print("Bonjour, appuyez sur le bouton!");
      last_state = bouton.getStatut();
      if((last_state == 1) && (bouton.getStatut() == 0))
      {
        tutorial_step++;
        lcd.clear();
      }
    break;
    
    case(1):
      lcd.print("En appuyant vous passez le dialogue");
      last_state = bouton.getStatut();
      if((last_state == 1) && (bouton.getStatut() == 0))
      {
        tutorial_step++;
        lcd.clear();
      }
    break;

    default:
      tutorial_step = 0;
    break;
  }
}



void setup() 
{
  lcd.begin(16, 2);
  lcd.print("Bravo, Romain!");
  //lcd.autoscroll();
}

void loop() 
{
  //bool condition_ok = false;
  //Code de l'aventure interactive
  tutorial(lcd, bouton);
 /* lcd.clear();
  lcd.print("Bonjour, appuyez sur le bouton!");
  while(condition_ok==false)
  {
    condition_ok = bouton.getStatut();
    delay(0);
  }
  delay(2000);
  lcd.clear();
  lcd.print("En appuyant vous passer le dialogue");
  while(bouton.getStatut()==0)
  {
    delay(0);
  }
  delay(2000);
  lcd.clear();
  lcd.print("Bienvenue.");
  while(bouton.getStatut()==0)
  {
    delay(0);
  }
  delay(2000);
  lcd.clear();
  lcd.print("Vous voila dans une aventure interactive.");
  while(bouton.getStatut()==0)
  {delay(0);}
  delay(2000);
  lcd.clear();
  lcd.print("Pour commencer rentrer votre nom...");
  while(bouton.getStatut()==0)
  {delay(0);}
  delay(2000);
  lcd.clear();
  lcd.print("Pour cela aidez vous du fichier...");
  while(bouton.getStatut()==0)
  {delay(0);}
  delay(2000);
  lcd.clear();
  lcd.print("Morse dans le dossier actuel...");
  while(bouton.getStatut()==0)
  {delay(0);}
  delay(2000);
  lcd.clear();
  lcd.print("et du piezo pour rentrer le nom.");
  while(bouton.getStatut()==0)
  {
    delay(0);
  }
  delay(2000);



//code de test

  if(touchSensor.getTouchStatus())
  {
    lcd.display();
    led.setLedStatus(HIGH);
  }
  else
  {
    lcd.noDisplay();
    led.setLedStatus(LOW);
  }
  */
}
