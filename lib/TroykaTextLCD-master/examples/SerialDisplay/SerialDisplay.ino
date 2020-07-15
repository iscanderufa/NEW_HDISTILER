// библиотека для работы с дисплеем
#include <TroykaTextLCD.h>
// по умолчанию в параметры конструктора дисплея зашиты параметры:
// выбор шины I²C (&Wire), I²C-адрес и пин подсветки
// TroykaTextLCD lcd(&Wire, 0x3E, 7);

// создаем объект для работы с дисплеем
TroykaTextLCD lcd;

void setup() {
  // открываем монитор Serial-порта
  Serial.begin(9600);
  // устанавливаем количество столбцов и строк экрана
  lcd.begin(16, 2);
  // устанавливаем контрастность в диапазоне от 0 до 63
  // для Arduino c логическим напряжением 5 В используйте параметр 30
  // для Arduino c логическим напряжением 3,3 В используйте параметр 45
  lcd.setContrast(30);
  // устанавливаем яркость в диапазоне от 0 до 255
  lcd.setBrightness(255);
}

void loop() {
  // если пришли данные из Serial-порта
  if (Serial.available()) {
    // очищаем экран
    lcd.clear();
    // ожидаем все данные
    while (Serial.available() > 0) {
      // выводим все пришедшие данные на экран
      lcd.write(Serial.read());
    }
  }
}
