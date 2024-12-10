#include <Wire.h>
#include <array>
#include <map>
#include "rgb_lcd.h"
#include "ADS1115.h"
#ifdef SOFTWAREWIRE
    #include <SoftwareWire.h>
    SoftwareWire myWire(3, 2);
    ADS1115<SoftwareWire> ads(myWire);//IIC
#else
    #include <Wire.h>
    ADS1115<TwoWire> ads(Wire);//IIC
#endif

using namespace std;

// Code Exception
#define MORSE_CODE_OVERSIZED 1

// Table de correspondance pour le morse
#define EMPTY 0
#define DOT 1
#define DASH 2

const std::map<array<char, 4>, char> morseTable = 
{
    {{DOT, DASH, EMPTY, EMPTY},   'A'},
    {{DASH, DOT, DOT, DOT},       'B'},
    {{DASH, DOT, DASH, DOT},      'C'},
    {{DASH, DOT, DOT, EMPTY},     'D'},
    {{DOT, EMPTY, EMPTY, EMPTY},  'E'},
    {{DOT, DOT, DASH, DOT},       'F'},
    {{DASH, DASH, DOT, EMPTY},    'G'},
    {{DOT, DOT, DOT, DOT},        'H'},
    {{DOT, DOT, EMPTY, EMPTY},    'I'},
    {{DOT, DASH, DASH, DASH},     'J'},
    {{DASH, DOT, DASH, EMPTY},    'K'},
    {{DOT, DASH, DOT, DOT},       'L'},
    {{DASH, DASH, EMPTY, EMPTY},  'M'},
    {{DASH, DOT, EMPTY, EMPTY},   'N'},
    {{DASH, DASH, DASH, EMPTY},   'O'},
    {{DOT, DASH, DASH, DOT},      'P'},
    {{DASH, DASH, DOT, DASH},     'Q'},
    {{DOT, DASH, DOT, EMPTY},     'R'},
    {{DOT, DOT, DOT, EMPTY},      'S'},
    {{DASH, EMPTY, EMPTY, EMPTY}, 'T'},
    {{DOT, DOT, DASH, EMPTY},     'U'},
    {{DOT, DOT, DOT, DASH},       'V'},
    {{DOT, DASH, DASH, EMPTY},    'W'},
    {{DASH, DOT, DOT, DASH},      'X'},
    {{DASH, DOT, DASH, DASH},     'Y'},
    {{DASH, DASH, DOT, DOT},      'Z'}
};

class Sensor
{
  protected:
  // Attribute
  uint8_t pin;

  public:
  Sensor(const uint8_t& pin):pin(pin)
  {
    pinMode(pin,INPUT);
  }
  
  bool operator==(const Sensor& sensor2) 
  {
        return this->pin == sensor2.pin;
  }
};

class Actuator
{
  protected:
  // Attribute
  uint8_t pin;

  public:
  // Constructor
  Actuator(const uint8_t& pin):pin(pin)
  {
    pinMode(pin,OUTPUT);
  }
};

class LightSensor
{
  private:
    int16_t lightLevel;
    const channel_t channel;

  public:
  LightSensor(const channel_t& c):channel(c){}

  int16_t getLightLevel()
  {
    return lightLevel = ads.getConversionResults(channel);
  }
};

class RotaryAngleSensor : public Sensor
{
  private:
    // Attribute
    float angle;
    const float adcRef;
    const float Vcc;
    const int fullAngle;
    
  public:
    // Constructor
    RotaryAngleSensor(const uint8_t& pin, const float& adcRef,const float& Vcc, const int& fullAngle):
    Sensor(pin),
    adcRef(adcRef),
    Vcc(Vcc),
    fullAngle(fullAngle)
    {}

    float getAngle()
    {
      return angle = (((float)analogRead(pin)*adcRef/1023)*fullAngle)/Vcc;
    }
};

class Led : public Actuator
{
  private:
    bool ledStatus;

  public:
    // Constructor
    Led(const uint8_t& pin):Actuator(pin){}

    // Assessor
    void setLedStatus(bool state)
    {
      ledStatus = state;
      digitalWrite(pin, ledStatus);
    }

    bool getLedStatus()
    {
      ledStatus = digitalRead(pin);
      return ledStatus;
    }

    // Method
    void swapLedStatus()
    {
      setLedStatus(!getLedStatus());
    }
};

class TouchSensor : public Sensor
{
  private:
    // Attribute
    bool touchStatus;
    bool timerIsSet;
    unsigned long previousTime;
    unsigned long currentTime;
    String previewLetterCode;
    String previewMorse;
    String morseText;

  public:
    // Constructor
    TouchSensor(const uint8_t& pin):Sensor(pin){}

    // Assessor
    bool getTouchStatus()
    {
      touchStatus = digitalRead(pin);
      return touchStatus;
    }

    String getPreviewMorse()
    {
      return previewMorse;
    }

    String getMorseText()
    {
      return morseText;
    }

    void clearMorseText()
    {
      morseText = "\0";
    }

    // Method
    char readMorseChar()
    {
      char code = 0;
      const unsigned int letterEnd = 200;
      const unsigned int borderDotDash = 1500;
      const unsigned int intervalReset = 3500;
      unsigned long TimePushed;
      String previewMorseChar = "\0";
      // static String previewLetterCode = "\0";

      if(getTouchStatus()==true)
      {
        if(!timerIsSet)
        {
          previousTime = millis();
          timerIsSet = true;
        }
        currentTime = millis();
        TimePushed = currentTime-previousTime;
        
        if( (TimePushed>=letterEnd) && (TimePushed<borderDotDash) )
        {
          previewMorseChar = "o";
        }
        else if((TimePushed<intervalReset) && (TimePushed>=borderDotDash))
        {
          previewMorseChar = "-";
        }
        else if(TimePushed>=intervalReset)
        {
          previewMorseChar = "R";
        }
      }
      else
      {
        if(timerIsSet)
        {
          TimePushed = currentTime-previousTime;
          if(TimePushed<letterEnd)
          {
            // Time < 0.15s : Dot
            code = 3;
            previewLetterCode = "\0";
            Serial.println("Space");
          }
          else if(TimePushed<borderDotDash)
          {
            // Time < 0.65s : Dot
            code = 1;
            previewLetterCode += "o";
            Serial.println("Dot");
          }
          else if((TimePushed<intervalReset) && (TimePushed>=borderDotDash))
          {
            // Time >= 0.65s and < 2.5s : Dash
            code = 2;
            previewLetterCode += "-";
            Serial.println("Dash");
          }
          else
          {
            // Time >= 2.5s : Reset
            code = 4;
            previewLetterCode = "\0";
            Serial.println("Reset");
          }
          timerIsSet = false;
          previewMorseChar = "\0";
        }
      }
      previewMorse = previewLetterCode + previewMorseChar; 
      // 0 nothing
      // 1  dot
      // 2  dash
      // 3  letter end
      // 4  reset

      return code;
    }

    // Throw si idx > 3
    void decodeMorse()
    {
      char code = readMorseChar();
      static array<char, 4> letterCode = {EMPTY,EMPTY,EMPTY,EMPTY} ;
      static unsigned int idx = 0;
      std::map<std::array<char, 4>, char>::const_iterator it;

      switch(code)
      {
        // Nothing 
        case(0):
        break;

        // Dot or Dash
        case(1):
        case(2):
          if(idx>3)
          {
            idx = 0;
            previewLetterCode = "\0";
            // Erreur si + de 4 symboles morse entrés pour une lettre 
            throw MORSE_CODE_OVERSIZED;
          }
          else
          {
            letterCode[idx] = code;
            idx++;           
          }
        break;

        // Space detected -> Fill the string with the chosen caractere
        case(3):
          it = morseTable.find(letterCode);
          if(it != morseTable.end())
          {
            morseText += it->second;
          }
          Serial.println(morseText);
          fill(letterCode.begin(),letterCode.end(), EMPTY);
          idx = 0;
        break;

        // Reset
        case(4):
          // morseText = "\0";
          morseText = morseText.substring(0,(morseText.length()-1)); // Delete last caractere of the string
          idx = 0;
          fill(letterCode.begin(),letterCode.end(), EMPTY);
        break;
      }
    }

};

class Button : public Sensor
{
   private:
    // Attribute
    bool status;
    bool previousStatus;
    bool wasPushedAlreadyExe;
    unsigned long previousTime;
    unsigned long currentTime;

    public:
    // Constructor
    Button(const uint8_t& pin):Sensor(pin){}

    // Assessor
    bool getStatus()
    {
      status = digitalRead(pin);
      return status;
    }

    // Method
    bool wasPushed()
    {
      const unsigned char interval = 100;
      bool res = false;

      if(!wasPushedAlreadyExe || previousStatus == LOW)
      {
        previousTime = millis();
        previousStatus = getStatus();
        wasPushedAlreadyExe = true;
      }
      
      currentTime = millis();

      if(currentTime-previousTime > interval)
      {
       previousTime = millis(); 
       wasPushedAlreadyExe = false;
       res = (previousStatus != getStatus());
      }

      return res;
    }
};

class ScreenLcd : public rgb_lcd
{
  private:
  const unsigned int interval; // 0.7sec read time
  bool timerIsSet;
  unsigned long previousTime;
  unsigned long currentTime;
  unsigned long idxStrDisplay;
  String lastText;
  String lastTextFirstLine;
  String lastTextSecondLine;

  public:
  ScreenLcd(const uint8_t& c, const uint8_t& r):interval(700)
  {
    begin(c, r, 0, Wire);
    leftToRight();
  }
  
  void displayFirstLine(String text)
  {
    if(lastTextFirstLine != text)
    {
      lastTextFirstLine = text;
      setCursor(0, 0);
      print("                "); // Clear the line
      setCursor(0, 0);
      print(text);
    }
  }

  void displaySecondLine(String text)
  {
    if(lastTextSecondLine != text)
    {
      lastTextSecondLine = text;
      setCursor(0, 1);
      print("                "); // Clear the line
      setCursor(0, 1);
      print(text);
    }
  }


  void display(String text)
  {
    if(text.length()<=16)
    {
      if(lastText != text)
      {
        lastText = text;
        clear();
        setCursor(0, 0);
        print(text.substring(0, 16));
        // Serial.println(text.substring(0, 16));
      }
    }
    else if(text.length()>32)
    {
      if(lastText != text)
      {
        lastText = text;
        idxStrDisplay = 0;
        timerIsSet = false;
        //Serial.println("New string");
      }
      if(!timerIsSet)
      {
        previousTime = millis();
        timerIsSet = true;
      }
      
      currentTime = millis();

      if(currentTime-previousTime > interval)
      {
        previousTime = millis(); 
        timerIsSet = false;
        clear();
        setCursor(0, 0);
        print(text.substring(idxStrDisplay, idxStrDisplay+16));
        //Serial.println(text.substring(idxStrDisplay, idxStrDisplay+16));
        setCursor(0, 1);
        print(text.substring(idxStrDisplay+16, idxStrDisplay+32));
        //Serial.println(text.substring(idxStrDisplay+16, idxStrDisplay+32));
        if((idxStrDisplay+32)<text.length())
        {
          idxStrDisplay++;
        }
        else
        {
          idxStrDisplay = 0;
        }
      }
    }
    else
    {
      if(lastText != text)
      {
        lastText = text;
        clear();
        setCursor(0, 0);
        print(text.substring(0, 16));
        Serial.println(text.substring(0, 16));
        setCursor(0, 1);
        print(text.substring(16, 32));
        Serial.println(text.substring(16, 32));
      }
    }
  }
};

class Game
{
  // Attribute
  protected:
  static TouchSensor touchSensor;
  static Led led;
  static Button button;
  static ScreenLcd lcd;
  static RotaryAngleSensor angleSensor;
  static LightSensor lightSensor;
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
TouchSensor Game::touchSensor(D6);
Led Game::led(D7);
Button Game::button(D8);
ScreenLcd Game::lcd(16, 2);
RotaryAngleSensor Game::angleSensor(A0, 3.3, 3.3, 300);
LightSensor Game::lightSensor(channel2);

class Adventure : public Game
{
  private:
  String playerName;
  String morsePreview;

  public:
  void playAdventure()
  {
    int angle;
    static int lastAngle = 0;

    switch(getStep())
    {
      case(0):
        lcd.display("Bonjour, appuyez sur le bouton !");
        if(button.wasPushed())
        {
          Serial.println("En appuyant vous passez les dialogues.");
          nextStep();
        }
      break;

      case(1):
        lcd.display("En appuyant vous passez les dialogues   ");
        if(button.wasPushed())
        {
          Serial.println("Vous voila dans une aventure interactive.");
          nextStep();
        }
      break;

      case(2):
        lcd.display("Vous voila dans une aventure interactive   ");
        if(button.wasPushed())
        {
          Serial.println("Passez et entrez votre nom avec l'os a morse.");
          nextStep();
        }
      break;

      case(3):
        lcd.display("Passez et entrez votre nom avec l'os a morse   ");
        if(button.wasPushed())
        {
          lcd.clear();
          nextStep();
        }
      break;

      case(4):
        touchSensor.decodeMorse();
        lcd.displayFirstLine(touchSensor.getMorseText());
        lcd.displaySecondLine(touchSensor.getPreviewMorse());

        if(button.wasPushed())
        {
          playerName = touchSensor.getMorseText();
          Serial.println(playerName + " vous venez de decouvrir un coffre !");
          nextStep();
        }
      break;

      case(5):
        lcd.display(playerName + " vous venez de decouvrir un coffre !   ");
        if(button.wasPushed())
        {
          Serial.println("Il est verrouille mais un papier griffonne l'accompagne...");
          nextStep();
        }
      break;

      case(6):
        lcd.display("Il est verrouille mais un papier griffonne l'accompagne...");
        if(button.wasPushed())
        {
          Serial.println("Je brille sans bruler, ne rouille jamais, du lit de l'eau je suis parfois extrait.");
          nextStep();
        }
      break;

      case(7):
        lcd.display("Je brille sans bruler, ne rouille jamais, du lit de l'eau je suis parfois extrait   ");
        // Clear le buffer d'affiche
        if(button.wasPushed())
        {
          touchSensor.clearMorseText();
          lcd.clear();
          nextStep();
        }
      break;

      case(8):
         touchSensor.decodeMorse();
         lcd.displayFirstLine(touchSensor.getMorseText());
         lcd.displaySecondLine(touchSensor.getPreviewMorse());
         if(touchSensor.getMorseText()=="OR")
         {
          Serial.println("Le coffre s'ouvre ! Il contient une carte menant vers un tresor !");
          nextStep();
         }
      break;

      case(9):
          lcd.display("Le coffre s'ouvre ! Il contient une carte menant vers un tresor !   ");
          if(button.wasPushed())
          {
           Serial.println("Vous embarquez sur votre " + playerName + "'s Revenge.");
           nextStep();
          }
      break;

      case(10):
          lcd.display("Vous embarquez sur votre " + playerName + "'s Revenge.");
          if(button.wasPushed())
          {
           Serial.println("La carte indique que l'ile se trouve au SUD-EST.");
           nextStep();
          }
      break;

      case(11): 
          lcd.display("La carte indique que l'ile se trouve au SUD-EST");
          if(button.wasPushed())
          {
            Serial.println("Passez et mettez le bon cap avec le gouvernail.");
            nextStep();
          }
      break;

      case(12):
          lcd.display("Passez et mettez le bon cap avec le gouvernail");
          if(button.wasPushed())
          {
           nextStep();
          }
      break;

      case(13): 
          lcd.displayFirstLine("Cap :");
          angle = (int)angleSensor.getAngle();
          if((angle > (lastAngle+1)) || (angle < (lastAngle-1)))
          {
            lastAngle = angle;
            lcd.displaySecondLine(String(angle) + " degres");
          }
          if((angle >= 130) && (angle <= 140) && button.wasPushed())
          {
           Serial.println("Apres 1 semaine de navigation vous arrivez sur l'ile.");
           nextStep();
          }
      break;

      case(14): 
          lcd.display("Apres 1 semaine de navigation vous arrivez sur l'ile");
          if(button.wasPushed())
          {
            Serial.println("Vous trouvez une grotte cachee.");
            nextStep();
          }
      break;

      case(15): 
          lcd.display("Vous trouvez une grotte cachee");
          if(button.wasPushed())
          {
            Serial.println("Allumez votre torche pour decouvrir la grotte.");
            nextStep();
          }
      break;

      case(16): 
          lcd.display("Allumez votre torche pour decouvrir la grotte");
          Serial.println(String(lightSensor.getLightLevel()));
          if(lightSensor.getLightLevel() >= 25000)
          {
           Serial.println("Waouh, quelle magnifique groovy torche !");
           led.setLedStatus(HIGH);
           nextStep();
          }
      break;

      case(17): 
          lcd.display("Waouh, quelle magnifique groovy torche !");
          if(button.wasPushed())
          {
           Serial.println("Vous avancez et trouvez au fond de la grotte...");
           nextStep();
          }
      break;

      case(18): 
          lcd.display("Vous avancez et trouvez au fond de la grotte...");
          if(button.wasPushed())
          {
           Serial.println("Un oeuf en OR !");
           nextStep();
          }
      break;

      case(19): 
          lcd.display("Un oeuf en OR !");
          if(button.wasPushed())
          {
           Serial.println("Aventure finie"); 
           nextStep();
          }
      break;

      case(20): 
          lcd.display("Aventure finie");
          if(button.wasPushed())
          {
           nextStep();
          }
      break;

      default:
        setStep(0);
      break;
    }
  }
};

Adventure adventure;

void setup() 
{
 Serial.begin(115200);
 Serial.println("Bonjour");

  while(!ads.begin(0x48))
  {
        Serial.println("ads1115 init failed!!!");
        delay(1000);
  }
  ads.setOperateMode(ADS1115_OS_SINGLE);
  ads.setOperateStaus(ADS1115_MODE_SINGLE);
  ads.setPGAGain(ADS1115_PGA_4_096);          // 1x gain   +/- 4.096V  1 bit =  0.125mV
  ads.setSampleRate(ADS1115_DR_8);            //8 SPS
}

void loop() 
{
  try
  {
    adventure.playAdventure();
  }
  catch(int errorCode)
  {
    switch(errorCode)
    {
      case MORSE_CODE_OVERSIZED:
        Serial.println("Vous ne pouvez pas entrer plus de 4 symboles pour coder une lettre en morse");
      break;

      default:
      break;
    }
  } 
}
