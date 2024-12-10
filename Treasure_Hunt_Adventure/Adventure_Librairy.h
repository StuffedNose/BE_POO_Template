/*********************************************************************
 * @file  Adventure_Librairy.h
 * @author Vincent Reis & Romain Villard
 * @brief Fichier declaration et definition de Adventure_Librairy
 *********************************************************************/
#ifndef ADVENTURE_LIBRAIRY_H_
#define ADVENTURE_LIBRAIRY_H_

#include <Wire.h>
#include <array>
#include <map>

// Librairies telechargees sur seeed Studio
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


// Code Exception
#define MORSE_CODE_OVERSIZED 1

// Code Morse
#define EMPTY 0
#define DOT 1
#define DASH 2

using namespace std;

// Map pour decoder le morse
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

// Classe de base pour les capteurs
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

// Classe de base pour les actionneurs
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

// Capteur de luminosite (connecte au module ADS1115)
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


// Capteur d'angle rotatif
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

// LED
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

// Capteur de touche avec des fonctionnalites code Morse
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

    // Method
    void clearMorseText()
    {
      morseText = "\0";
    }

    char readMorseChar()
    {
      char code = 0;
      const unsigned int letterEnd = 200;
      const unsigned int borderDotDash = 1500;
      const unsigned int intervalReset = 3500;
      unsigned long TimePushed;
      String previewMorseChar = "\0";

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
            // Time < 0.2s : Letter Validation
            code = 3;
            previewLetterCode = "\0";
            Serial.println("Space");
          }
          else if(TimePushed<borderDotDash)
          {
            // Time >= 0.2s and < 1.5s : Dot
            code = 1;
            previewLetterCode += "o";
            Serial.println("Dot");
          }
          else if((TimePushed<intervalReset) && (TimePushed>=borderDotDash))
          {
            // Time >= 1.5s and < 3.5s : Dash
            code = 2;
            previewLetterCode += "-";
            Serial.println("Dash");
          }
          else
          {
            // Time >= 3.5s : Reset
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

    // EXCEPTION si idx > 3 <=> evite un buffer overflow si plus de 4 caracteres morse sont rentres
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
            fill(letterCode.begin(),letterCode.end(), EMPTY);
            // Erreur si plus de 4 symboles morse entrés pour une lettre 
            throw MORSE_CODE_OVERSIZED;
          }
          else
          {
            letterCode[idx] = code;
            idx++;           
          }
        break;

        // Espace/entre detecte -> Remplir la chaine avec le caractere morse correspondant au code ecrit
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
          morseText = morseText.substring(0,(morseText.length()-1));              // Supprimer le dernier charactere de la string
          idx = 0;
          fill(letterCode.begin(),letterCode.end(), EMPTY);
        break;
      }
    }

};


// Bouton
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

// Ecran LCD
class ScreenLcd : public rgb_lcd
{
  private:
  const unsigned int interval;                                  // Decalage d'un caractere toutes les 0.7s
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
    if(text.length()<=16)                   // Si le texte tient sur une ligne
    {
      if(lastText != text)
      {
        lastText = text;
        clear();
        setCursor(0, 0);
        print(text.substring(0, 16));
      }
    }
    else if(text.length()>32)              // Si le texte ne tient pas sur l'ecran sans balayage
    {
      if(lastText != text)
      {
        lastText = text;
        idxStrDisplay = 0;
        timerIsSet = false;
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
        setCursor(0, 1);
        print(text.substring(idxStrDisplay+16, idxStrDisplay+32));
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
    else                                      // Si le texte tient sur deux lignes
    {
      if(lastText != text)
      {
        lastText = text;
        clear();
        setCursor(0, 0);
        print(text.substring(0, 16));
        setCursor(0, 1);
        print(text.substring(16, 32));
      }
    }
  }
};


// Jeu de base qui dispose des capteurs et actionneurs
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


// Aventure sous forme de MAE (herite de Game)
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
    static bool isFirst = true;

    switch(getStep())
    {
      case(0):
        if(isFirst)
        {
          Serial.println("Bonjour, appuyez sur le bouton !");
          isFirst = false;
        }
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
        try
        {
          touchSensor.decodeMorse();
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
          try
          {
            touchSensor.decodeMorse();
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
            Serial.println("Allumez votre torche avec le briquet pour decouvrir la grotte.");
            nextStep();
          }
      break;

      case(16): 
          lcd.display("Allumez votre torche avec le briquet pour decouvrir la grotte");
          Serial.println(String(lightSensor.getLightLevel()));
          if(lightSensor.getLightLevel() >= 27000)
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
           Serial.println("Felicitation ! L'aventure est finie."); 
           nextStep();
          }
      break;

      case(20): 
          lcd.display("Felicitation ! L'aventure est finie.");
      break;

      default:
        setStep(0);
      break;
    }
  }
};

#endif