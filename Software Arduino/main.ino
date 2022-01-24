#include "Adafruit_GFX.h"    // Core graphics library
#include "MCUFRIEND_kbv.h"   // Hardware-specific library
#include <SoftwareSerial.h>
#include "Arduino.h"
#include "DFPlayer_Mini_Mp3.h" // Library for the sound module

MCUFRIEND_kbv tft;
SoftwareSerial espSerial(15, 14);
SoftwareSerial dfPlayer(39, 41); // RX, TX


#define DFPLAYER_PIN_TX 41
#define DFPLAYER_PIN_RX 39
#define BLACK   0x0000
#define RED     0xF800
#define GREEN   0x07E0
#define WHITE   0xFFFF
#define GREY    0x8410
#include "images.h"
#include "Fonts/FreeSans9pt7b.h"
#include "Fonts/FreeSans12pt7b.h"
#include "Fonts/FreeSerif12pt7b.h"
#include "FreeDefaultFonts.h"

int cigaretteCount = 0;
int cigaretteCountDuesseldorf = 3;
int goal = 5; // This is the first goal people have to reach
int LaserSensor = 29;
long lastCigaretteInserted = 0;
bool cigaretteHasBeenInserted = false;
bool factHasBeenPlayed = false;
bool ADSHasBeenPlayed = false;

void showmsgXY(int x, int y, int sz, const char *msg)
{
  int16_t x1, y1;
  uint16_t wid, ht;
  tft.setCursor(x, y);
  tft.setTextColor(BLACK);
  tft.setTextSize(sz);
  tft.print(msg);
  delay(1000);
}

void setup()
{
  Serial.begin(9600);
  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.invertDisplay(false);
  tft.setRotation(1);
  espSerial.begin(115200);
  delay(20000);
  pinMode(LaserSensor, INPUT);
  dfPlayer.begin(9600);
  mp3_set_serial (dfPlayer); //set SoftwareSerial for DFPlayer-mini mp3 module
  mp3_set_volume (100);
  tft.fillScreen(BLACK);
  Serial.println("Setup ready");
}

void loop(void)
{
  if ((millis() - lastCigaretteInserted) % 1800000 < 2000) // Plays a wake-up-sound every 30 minutes
  {
    playWakeUpSound();
  }

  if (digitalRead(LaserSensor) == 1) {
    Serial.println("Cigarette Detected");
    lastCigaretteInserted = millis(); // Saves the time when the last cigarette has been inserted
    cigaretteCount++;
    espSerial.write(1);
    cigaretteHasBeenInserted = true;
    playSound(); // Play the Sound
    playGame(); // Play the Game after the Sound. Delay()-function is used in playSound()
    factHasBeenPlayed = false;
    ADSHasBeenPlayed = false;
  }

  if (cigaretteHasBeenInserted == true)
  {
    // Using the if statement instead of delay() to ensure cigarettes can be inserted during that time
    if ((millis() - 27000) > lastCigaretteInserted) // Play advertisment for 5 seconds. Then go into sleep mode
    {
      cigaretteHasBeenInserted = false;
      tft.fillScreen(BLACK);
    }
    else if ((millis() - 22000 ) > lastCigaretteInserted && ADSHasBeenPlayed == false) // Play advertisment after showing the fact for 7 seconds
    {
      playADS();
      ADSHasBeenPlayed = true;
    }
    else if ((millis() - 15000) > lastCigaretteInserted && factHasBeenPlayed == false) // Play Fact after showing the game for 7 seconds
    {
      playFact();
      factHasBeenPlayed = true;
    }
  }
}

void playSound()
{
  tft.fillScreen(WHITE);
  int chooseSound = random(1, 6);
  Serial.print("Sound ");
  Serial.print(chooseSound);
  Serial.println(" starts playing");
  mp3_play(chooseSound);
  switch (chooseSound) { // Delay depending on the length of the sound
    case 1:
      delay(6000);
      break;
    case 2:
      delay(3000);
      break;
    case 3:
      delay(3000);
      break;
    case 4:
      delay(6000);
      break;
    case 5:
      delay(8000);
      break;
    default:
      delay(8000);
      break;
  }
  Serial.println("Sound has been played");
}

void playGame()
{
  // Prepare the screen for text to be displayed
  drawGreenRectangle();
  tft.setTextColor(BLACK);

  if (cigaretteCount == 1 || cigaretteCount % 100 == 0) // Every 100th bud there is a party mode. Or the first bud.
  {
    tft.setCursor(16, 80);
    tft.setTextSize(4);
    tft.print("Das war der ");
    tft.print(cigaretteCount);
    tft.print(".");
    tft.setCursor(16, 120);
    tft.print("Stummel!");
    tft.print("\n\n     PARTY MODE");
    mp3_play(7); // Play party music
    delay(8000);
  }
  else
  {
    int chooseGame = random(1, 4); // Choose between three games
    switch (chooseGame) {
      case 1: // Game 1: Shows how many cigarette butts have been inserted in total
        tft.setCursor(16, 120);
        tft.setTextSize(4);
        tft.print("Bisher entsorgte");
        tft.setCursor(16, 160);
        tft.print("Stummel: ");
        tft.print(cigaretteCount);
        break;
      case 2:
        // Prepare size and starting point of the text
        tft.setTextSize(4);
        tft.setCursor(16, 80);
        while (cigaretteCount > goal) // When goal is already reached
        {
          goal = goal * 1.5; // Increase the goal
          Serial.print("The new goal is now ");
          Serial.println(goal);
          break;
        }
        if (cigaretteCount == goal) // When the goal just got reached
        {
          // Show message on screen
          tft.print("Ziel: ");
          tft.print(goal);
          tft.setCursor(16, 120);
          tft.print("Aktuell: ");
          tft.print(cigaretteCount);
          tft.setCursor(16, 200);
          tft.print("Ziel erreicht!!!");
          mp3_play(6); // Play clapping sound
          delay(7000);
        }
        else // Goal has not been reached yet
        {
          tft.print("Gemeinsam ");
          tft.print(goal);
          tft.setCursor(16, 120);
          tft.print("Stummel sammeln!");
          tft.setCursor(16, 200);
          tft.print("Bisher: ");
          tft.print(cigaretteCount);
        }
        break;
      case 3: // Shows a leaderboard of different cities. In this case: Compare Duesseldorf and Cologne
        tft.setCursor(16, 100);
        tft.setTextSize(4);
        if (cigaretteCount > cigaretteCountDuesseldorf) // Cologne is winning
        {
          tft.print("1. Koeln: ");
          tft.print(cigaretteCount);
          tft.setCursor(16, 140);
          tft.print("2. Duesseldorf: ");
          tft.print(cigaretteCountDuesseldorf);
          tft.setCursor(16, 200);
          tft.print("Ihr gewinnt!!!");
        }
        else if (cigaretteCountDuesseldorf > cigaretteCount) // Duesseldorf is winning
        {
          tft.print("1. Duesseldorf: ");
          tft.print(cigaretteCountDuesseldorf);

          tft.setCursor(16, 140);
          tft.print("2. Koeln: ");
          tft.print(cigaretteCount);
          tft.setCursor(16, 200);
          tft.print("Aufholen!");
        }
        else // Both cities have the same cigarette count
        {
          tft.print("1. Koeln: ");
          tft.print(cigaretteCount);
          tft.setCursor(16, 140);
          tft.print("1. Duesseldorf: ");
          tft.print(cigaretteCountDuesseldorf);
          tft.setCursor(16, 200);
          tft.print("Schnell, sammel");
          tft.setCursor(16, 240);
          tft.print("einen Stummel!");
        }
        break;
      default:
        // statements
        break;
    }
  }
}

void playFact()
{
  // Prepare the screen for text to be displayed
  tft.fillScreen(WHITE);
  drawGreenRectangle();
  tft.setTextColor(BLACK);
  tft.setCursor(16, 60);
  int chooseFact = random(1, 13); // Choose between twelve facts
  switch (chooseFact) {
    case 1:
      tft.print("Pro");
      tft.setCursor(16, 100);
      tft.print("Zigarettenstummel");
      tft.setCursor(16, 140);
      tft.print("gelangen 2 bis 6 mg");
      tft.setCursor(16, 180);
      tft.print("Nikotin ins");
      tft.setCursor(16, 220);
      tft.print("Regenwasser.");
      break;
    case 2:
      tft.print("Eine Kippe");
      tft.setCursor(16, 100);
      tft.print("vergiftet bis zu 60");
      tft.setCursor(16, 140);
      tft.print("Liter Grundwasser.");
      break;
    case 3:
      tft.print("Zigarettenstummel");
      tft.setCursor(16, 100);
      tft.print("toeten Fische in");
      tft.setCursor(16, 140);
      tft.print("Seen und Fluessen.");
      break;
    case 4:
      tft.print("Zigarettenstummel");
      tft.setCursor(16, 100);
      tft.print("brauchen in");
      tft.setCursor(16, 140);
      tft.print("Salzwasser 100 bis");
      tft.setCursor(16, 180);
      tft.print("400 Jahre, um sich");
      tft.setCursor(16, 220);
      tft.print("zu zersetzen.");
      break;
    case 5:
      tft.print("Zigarettenstummel");
      tft.setCursor(16, 100);
      tft.print("sind 10 bis 15");
      tft.setCursor(16, 140);
      tft.print("Jahre in der freien");
      tft.setCursor(16, 180);
      tft.print("Natur giftig.");
      break;
    case 6:
      tft.print("Zigarettenstummel");
      tft.setCursor(16, 100);
      tft.print("sind mit 30 bis 40");
      tft.setCursor(16, 140);
      tft.print("Prozent der");
      tft.setCursor(16, 180);
      tft.print("haeufigste Muell an");
      tft.setCursor(16, 220);
      tft.print("Straenden und in");
      tft.setCursor(16, 260);
      tft.print("Staedten.");
      break;
    case 7:
      tft.print("In");
      tft.setCursor(16, 100);
      tft.print("Zigarettenstummeln");
      tft.setCursor(16, 140);
      tft.print("sind bis zu 7000");
      tft.setCursor(16, 180);
      tft.print("Schadstoffe");
      tft.setCursor(16, 220);
      tft.print("enthalten.");
      break;
    case 8:
      tft.print("Zwei Drittel der");
      tft.setCursor(16, 100);
      tft.print("Zigaretten in");
      tft.setCursor(16, 140);
      tft.print("Deutschland landen");
      tft.setCursor(16, 180);
      tft.print("auf dem Boden.");
      break;
    case 9:
      tft.setCursor(16, 20);
      tft.print("Zigarettenfilter");
      tft.setCursor(16, 60);
      tft.print("sind aus");
      tft.setCursor(16, 100);
      tft.print("Kunststoff. Sie");
      tft.setCursor(16, 140);
      tft.print("werden nicht");
      tft.setCursor(16, 180);
      tft.print("abgebaut, sondern");
      tft.setCursor(16, 220);
      tft.print("enden als");
      tft.setCursor(16, 260);
      tft.print("Mikroplastik.");
      break;
    case 10:
      tft.print("580 Mrd. Kippen");
      tft.setCursor(16, 100);
      tft.print("landen EU-weit");
      tft.setCursor(16, 140);
      tft.print("jaehrlich in der");
      tft.setCursor(16, 180);
      tft.print("Umwelt.");
      break;
    case 11:
      tft.print("12.000.000.000!");
      tft.setCursor(16, 100);
      tft.print("Das ist die Zahl");
      tft.setCursor(16, 140);
      tft.print("der");
      tft.setCursor(16, 180);
      tft.print("Zigarettenstummel,");
      tft.setCursor(16, 220);
      tft.print("die jeden Tag in");
      tft.setCursor(16, 260);
      tft.print("der Umwelt landen.");
      break;
    case 12:
      tft.print("In Koeln werden");
      tft.setCursor(16, 100);
      tft.print("taeglich 260 kg");
      tft.setCursor(16, 140);
      tft.print("Zigarettenstummel");
      tft.setCursor(16, 180);
      tft.print("weggeworfen.");
      break;
    default:
      break;
  }
}

void playADS()
{
  // Prepare the screen for text to be displayed
  tft.fillScreen(WHITE);
  drawGreenRectangle();
  tft.setTextColor(BLACK);
  tft.setCursor(40, 60);

  // Display ADS
  tft.print("Hole jetzt deinen");
  tft.setCursor(160, 100);
  tft.print("eigenen");
  tft.setCursor(80, 140);
  tft.print("Taschenascher!");
  tft.setCursor(50, 210);
  tft.print("www.tobacycle.de");
}

void playWakeUpSound()
{
  int soundNumber = random(8, 10);
  tft.drawBitmap(0, 0, augen_rechts, 480, 320, 0xffff);
  mp3_play(soundNumber);
  delay(9000);
}

void drawGreenRectangle() // Draws the green frame around the screen
{
  tft.drawRect(0, 0, 480, 320, GREEN);
  tft.drawRect(1, 1, 478, 318, GREEN);
  tft.drawRect(2, 2, 476, 316, GREEN);
  tft.drawRect(3, 3, 474, 314, GREEN);
  tft.drawRect(4, 4, 472, 312, GREEN);
  tft.drawRect(5, 5, 470, 310, GREEN);
  tft.drawRect(6, 6, 468, 308, GREEN);
  tft.drawRect(7, 7, 466, 306, GREEN);
  tft.drawRect(8, 8, 464, 304, GREEN);
  tft.drawRect(9, 9, 462, 302, GREEN);
  tft.drawRect(10, 10, 460, 300, GREEN);
}
