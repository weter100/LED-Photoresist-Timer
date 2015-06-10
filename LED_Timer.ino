#include <LiquidCrystal.h>
#include <Bounce.h>

int PinPres1 = 5; // кнопки
int PinPres2 = 6;
int PinRelay = 13; // реле
int PinCLK = 2; // энкодер
int PinDT = 3;
int PinSW = 4;
long Preset0 = 0;  // начальное значение (сек.)
long Preset1 = 20;  // предустановка времени №1 (сек.)
long Preset2 = 140;  // предустановка времени №2 (сек.)
int sec_step = 5; // шаг энкодера (сек.)

unsigned long currentTime; // текущее время
unsigned long loopTime; // время окончания
unsigned long LimitTime = 0; // таймер
volatile boolean turn;
volatile boolean up;
boolean running = false; // флаг запуска отсчета
int awake1, awake2, awake3;

LiquidCrystal lcd(12, 11, 10, 9, 8, 7); // экран
Bounce bouncer1 = Bounce(PinPres1, 5); // дребезг кнопок
Bounce bouncer2 = Bounce(PinPres2, 5); 
Bounce bouncer3 = Bounce(PinSW, 5); 

void setup() {

  Preset0 = Preset0*1000;
  Preset1 = Preset1*1000;
  Preset2 = Preset2*1000;
  sec_step = sec_step*1000;
  LimitTime = Preset0;
  pinMode(PinCLK, INPUT); // входы
  pinMode(PinDT, INPUT);  
  pinMode(PinSW, INPUT);
  pinMode(PinPres1, INPUT);
  pinMode(PinPres2, INPUT);
  pinMode(PinRelay, OUTPUT);
  attachInterrupt (0, isr, CHANGE); // прерывание энкодера
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.print("  UV LED TIMER  ");
  lcd.setCursor(0,1);
  lcd.print("    ver. 1.0    ");
  delay(1000);
  lcd.clear();
  currentTime = millis();
}

void loop() {

  delay(200);

  bouncer1.update(); // дребезг кнопок
  bouncer2.update();
  bouncer3.update();
 
  // выбран режим отсчета
  /* ========================== */
  if (running == true) {

    // отсчет времени
    currentTime = millis();
    loopTime = currentTime + LimitTime; // когда закончить
    while(currentTime < loopTime) { // сравниваем текущее время с временем окончания

      digitalWrite(PinRelay, HIGH); // включаем

      // вывод на экран
      lcd.setCursor(0,0);
      lcd.print("   Working...   "); 
      lcd.setCursor(4, 2);  
      if (LimitTime/60/60/1000 < 10) { lcd.print("0"); }
      lcd.print(LimitTime/60/60/1000);
      lcd.print(":");
      if (LimitTime/60/1000%60 < 10) { lcd.print("0"); }
      lcd.print((LimitTime/60/1000)%60);
      lcd.print(":");
      if (LimitTime/1000%60 < 10) { lcd.print("0"); }
      lcd.print(LimitTime/1000%60);
      lcd.display();

      // кнопка
      bouncer3.update();
      if (bouncer3.read() != awake3) {
 
        awake3 = bouncer3.read();
        if (awake3 && running == true) {
          running = false;
          digitalWrite(PinRelay, HIGH); // включаем
          LimitTime = Preset0;
          break;
        }
      }

      LimitTime = LimitTime - (millis() - currentTime); // уменьшаем таймер
      currentTime = millis(); // получаем новое время
    }

    // окончание работы таймера 
    digitalWrite(PinRelay, LOW); // отключаем устройство

    lcd.setCursor(0, 0);
    lcd.print("             ");
    lcd.setCursor(0, 0);
    lcd.print("     FINISH     ");
    running = false;
    delay (3000);
    //lcd.begin(16, 2);

  // выбран режим клавиатуры
  /* ========================== */
  } else {

    digitalWrite(PinRelay, LOW); // отключаем устройство  
    
    if (LimitTime > 86400000) LimitTime = Preset0; // отбрасываем некорректные значения
    
    // вывод на экран
    lcd.setCursor(0,0);
    lcd.print("   Set timer:   "); 
    lcd.setCursor(4, 2);  
    if (LimitTime/60/60/1000 < 10) { lcd.print("0"); }
    lcd.print(LimitTime/60/60/1000);
    lcd.print(":");
    if (LimitTime/60/1000%60 < 10) { lcd.print("0"); }
    lcd.print((LimitTime/60/1000)%60);
    lcd.print(":");
    if (LimitTime/1000%60 < 10) { lcd.print("0"); }
    lcd.print(LimitTime/1000%60);
    lcd.display();

    // пресет №1
    if (bouncer1.read() != awake1) {
      awake1 = bouncer1.read();
      if (awake1) {
        lcd.setCursor(0,1);
        lcd.print("    preset 1    ");
        LimitTime = Preset1;
        delay(1000);
      }
    }

    // пресет №2
    if (bouncer2.read() != awake2) {
      awake2 = bouncer2.read();
      if (awake2) {
        lcd.setCursor(0,1);
        lcd.print("    preset 2    ");
        LimitTime = Preset2;
        delay(1000);
      }
    }

    // энкодер
    static long enc_pos=0;
    if (digitalRead(PinSW) && enc_pos != 0) {
      enc_pos=0;
    }
    if (turn) {
      if (up) {
        enc_pos--;
      } else {
        enc_pos++;
      }
      turn = false;
      LimitTime += enc_pos*sec_step;
    }

    // кнопка
    if (bouncer3.read() != awake3) {
      awake3 = bouncer3.read();
      if (awake3 && LimitTime != 0) {
        running = true;
      }
    }
  }
}

// прерывание энкодера
void isr ()  {
  up = (digitalRead(PinCLK) == digitalRead(PinDT));
  turn = true;
}
