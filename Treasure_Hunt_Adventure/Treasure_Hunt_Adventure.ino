/*********************************************************************
 * @file  Treasure_Hunt_Adventure.ino
 * @author Vincent Reis & Romain Villard
 * @brief Fichier Main
 *********************************************************************/
#include "Adventure_Librairy.h"

Adventure adventure;

void setup() 
{
  // Initialisation communication serie
  Serial.begin(115200);
  Serial.println("Bonjour");

  // Initialisation ADS1115
  while(!ads.begin(0x48))
  {
        Serial.println("ads1115 init failed!!!");
        delay(1000);
  }
  ads.setOperateMode(ADS1115_OS_SINGLE);
  ads.setOperateStaus(ADS1115_MODE_SINGLE);
  ads.setPGAGain(ADS1115_PGA_4_096);              // gain x1
  ads.setSampleRate(ADS1115_DR_8);                // 8 SPS
}

void loop() 
{
  adventure.playAdventure();
}