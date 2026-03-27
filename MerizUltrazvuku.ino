#include <Wire.h>
#include <LiquidCrystal_I2C.h> //knihovna na display
#include <Keypad.h> //knihovna na klávesnici

// LCD na I2C adrese 0x27, velikost 20 sloupců × 4 řádky
LiquidCrystal_I2C lcd(0x27, 20, 4);  


// pin pro vyslání ultrazvukového pulzu
const int trigPin = 9;  

// pin pro příjem odraženého signálu
const int echoPin = 8;  

// rozměr klávesnice (2×2 matice)
const byte ROWS = 2;  
const byte COLS = 2;  

// mapa znaků klávesnice (co vrací jednotlivé klávesy)
char keys[ROWS][COLS] = {
  {'1','2'},
  {'3','4'}
};

byte rowPins[ROWS] = {2, 3};  
// Arduino piny připojené na řádky klávesnice

byte colPins[COLS] = {4, 5};  
// Arduino piny připojené na sloupce klávesnice

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
// vytvoření objektu klávesnice: mapování znaků, zapojení pinů, velikost matice

unsigned long lastMeasure = 0;

// buffer pro průměr
int last10[10];          // pole posledních 10 měření
byte index10 = 0;        // aktuální pozice v bufferu
bool filled = false;     // jestli už je buffer plný

// uložené rozměry
int delka = 0;
int sirka = 0;
int vyska = 0;

int avg = 0;             // vypočtený průměr
bool showCalc = false;   // přepínač obrazovek

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.clear();

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {

  // ----- MĚŘENÍ -----
  if (!showCalc && millis() - lastMeasure >= 100) {  // měření každých 100 ms
    lastMeasure = millis();

    // vyslání ultrazvukového pulzu
    digitalWrite(trigPin, LOW);
    delayMicroseconds(3);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 25000);   // čekání na echo
    int distance = duration * 0.01715;               // převod času na cm

    // buffer
    if (duration != 0) {  // ignorujeme neplatná měření
      last10[index10] = distance;
      index10++;

      if (index10 >= 10) {
        
        index10 = 0;
        filled = true;
      }

      long sum = 0;
      int count = filled ? 10 : index10;  // kolik hodnot počítat
      for (int i = 0; i < count; i++) sum += last10[i];

      avg = sum / count;  // výpočet průměru
    }

    showMeasureScreen();
  }

  // ----- KLÁVESNICE -----
  char key = keypad.getKey(); //rozpozná že uživatel zmáčknul tlačítko
  if (key) {
    showCalc = false;  // návrat na měření při stisku libovolné klávesy

    if (key == '1') delka = avg;  // uloží aktuální průměr
    if (key == '2') sirka = avg;
    if (key == '3') vyska = avg;

    if (key == '4') {
      showCalc = true;
      showCalcScreen();
    }
  }
}

void showMeasureScreen() {
  lcd.setCursor(0, 0); //nultý řádek na nulté pozici
  lcd.print("Vzdalenost:       "); //vypíše
  lcd.setCursor(13, 0);
  lcd.print(avg);
  lcd.print(" cm");

  lcd.setCursor(0, 1);
  lcd.print("Delka:            ");
  lcd.setCursor(13, 1);
  lcd.print(delka);
  lcd.print(" cm");

  lcd.setCursor(0, 2);
  lcd.print("Sirka:            ");
  lcd.setCursor(13, 2);
  lcd.print(sirka);
  lcd.print(" cm");

  lcd.setCursor(0, 3);
  lcd.print("Vyska:            ");
  lcd.setCursor(13, 3);
  lcd.print(vyska);
  lcd.print(" cm");
}

void showCalcScreen() {
  lcd.clear();

  // ----- PLOCHA -----
  String plocha = String((long)delka * sirka);  // přetypování kvůli přetečení
  int length = String(plocha).length();         // délka čísla

  lcd.setCursor(7, 0);
  lcd.print("Plocha");

  // převod jednotek podle velikosti čísla
  if(length <= 3){
    plocha = plocha + " cm2";
  } 
  else if(length <= 5){
    plocha = plocha.substring(0, length-3) + "." + plocha.substring(length-3) + " dm2";
  } 
  else if(length <= 7){
    plocha = plocha.substring(0, length-5) + "." + plocha.substring(length-5) + " m2";
  }

  lcd.setCursor(0, 1);
  lcd.print("                    ");
  lcd.setCursor(0, 1);
  lcd.print(plocha);

  // ----- OBJEM -----
  String objem = String((long)delka * sirka * vyska);
  length = String(objem).length();

  lcd.setCursor(7, 2);
  lcd.print("Objem");

  // převod jednotek podle velikosti čísla
  if(length <= 3){
    objem = objem + " cm3";
  } 
  else if(length <= 6){
    objem = objem.substring(0, length-3) + "." + objem.substring(length-3) + " dm3";
  } 
  else if(length <= 8){
    objem = objem.substring(0, length-5) + "." + objem.substring(length-5) + " m3";
  }

  lcd.setCursor(0, 3);
  lcd.print("                    ");
  lcd.setCursor(0, 3);
  lcd.print(objem);
}
