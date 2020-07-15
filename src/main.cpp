#include <SPI.h>
#include <NTPClient.h>
#include <WiFiNINA.h>
#include <GyverTimer.h>
#include <GyverButton.h>
#include <TroykaMeteoSensor.h>
#include <TroykaTextLCD.h>
#include <TroykaLight.h>
#include <WiFiUdp.h>
#define PIN 3
#define PIN 2
#define PIN 1
#define PIN 0
TroykaTextLCD lcd(&Wire, 0x3E, 5);
// библиотека для работы с датчиком освещённости (Troyka-модуль)
int lcdbrithness;
// создаём объект для работы с датчиком освещённости и передаём ему номер пина выходного сигнала
TroykaLight sensorLight(A5);
// Переменная ступень для изменения шим управления насосом
    int fadeAmount;
// Последнее значение предыдущей переменной
    int fadeAmountLast;
// текущее значение ШИМ управления насосом
    int valueSHIM;
// Последнее значение ШИМ перед экстренной остановкой насоса
    int lastPower;
// показания датчика уровня жижкости
    int sensorValue;
// оптимальный показания датчика уровня жидкости
    int valueNorma = 70;
// критически высокий уровень датчика жидкости при котором происходит остановка ШИМ насоса
    int valueMax = 85;
// переменная для записи состояния ШИМ насоса
    boolean  stop = 0;
// переменная интервал между циклами
    int valueTimer = 500;
// яркость дисплея LCD
	int brightness = 130;
// Время с начала работы программы
    unsigned long time;
// заданное время работы насоса
    unsigned long timeSendWork = 60000000;
// начало работы шим насоса
    unsigned long timeWork;
// время заданное задержки изменения ШИМ насоса
    int timeSendPause = 8000;
// время момента изменения шим
    unsigned long timePause;
// создаём объект для работы с датчиком
      TroykaMeteoSensor meteoSensor;
    uint32_t sec;
    int timeHours;
    int timeMins;
    int timeSecs; 
GButton buttleft(0);
GButton buttdown(1);
GButton buttrith(2);
GButton buttup(3);
int  valueleft = 0;
int  valuedown = 0;
int  valuerith = 0;
int  valueup = 0;  
int  lcdprintnumber = 1;
int  lcdparam = 0;
int  lcdsbstrnumber = 0;
int  lcdwindow = 100;
int  oldlcdwindow  = 0;
int  btnincriment = 1;
uint32_t tmrSleep;
uint32_t sleeplcdwindow;
long  A = 95;
long  B = 10000;
long  C = 40;
long  D;
long  D1;
boolean lcdsubstring = 0;
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = "Yohanan";        // your network SSID (name)
char pass[] = "100200300";    // your network password (use for WPA, or use as key for WEP), length must be 8+
int keyIndex = 0;                 // your network key Index number (needed only for WEP)
int timezone =3;
int status = WL_IDLE_STATUS;
// A UDP instance to let us send and receive packets over UDP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 10800, 60000);
//unsigned long epoch;
WiFiServer server(80);
GTimer_ms myTimer0(100);
GTimer_ms myTimer1(500);
GTimer_ms myTimer2(1000);
GTimer_ms myTimer3(10000);
GTimer_ms myTimer4(3600000);
void startprocedure()
{ 
  //Запускаем запоминание времени
  time = millis();
  /*sec = millis() / 1000ul;
  timeHours = (sec / 3600ul);
  timeMins = (sec % 3600ul) / 60ul;
  timeSecs = (sec % 3600ul) % 60ul;*/
  
  // считывание данных с датчика освещённости
  sensorLight.read();

  // назначаем контакт выход ШИМ насоса
  pinMode(6, OUTPUT);

  // записывает текущее значение ШИМ насоса в переменную
  analogWrite(6, valueSHIM);

  // считывает в переменную значение датчика уровня жидкости
  sensorValue = analogRead(A0) / 4;

  //записывает переменную с датчика метео
  int stateSensor = meteoSensor.read(); 
}
void fadeselect() {
  // сравнивает абсолютное значение значение датчика уровня жидкости и установленного уровня
  // чем больше разница тем большее значение шага изменения ШИМ
  if (abs(sensorValue - valueNorma) > 1) {
    fadeAmount = 1;
  }
  if (abs(sensorValue - valueNorma) > 3) {
    fadeAmount = 2;
  }
  if (abs(sensorValue - valueNorma) > 5) {
    fadeAmount = 5;
  }
  if (abs(sensorValue - valueNorma) > 10) {
    fadeAmount = 10;
  }
  if (abs(sensorValue - valueNorma) > 50) {
    fadeAmount = 20;
  }
  if (abs(sensorValue - valueNorma) > 65) {
    fadeAmount = 50;
  }
  //if (abs(sensorValue - valueNorma) > 250) {
  // fadeAmount = 100;
  //}
}
void volumespirt()
 {
D = (((A * B )/C) - B);
if(A < 0) A = 0;
if(A > 100) A = 100;
if(C < 0) C = 0;
if(C > 100) C = 100;
//Serial.print("Крепость%: ");
//Serial.println(A);
//Serial.print("Объём спирта Мл: ");
//Serial.println(B);
//Serial.print("Объём общий Мл: ");
//Serial.println((A * B)/C);
//Serial.print("Объём разбавителя: ");
//Serial.println(D);
//Serial.print("Результирующая крепость (%): " );
//Serial.println(C);
  }
void selectpumpvalue() 
{
  // увеличивает или уменьшает значение ШИМ насоса
  if (sensorValue > valueNorma && (millis() - timePause) > timeSendPause % 3) {
    valueSHIM = valueSHIM - fadeAmount;
    timePause = millis();
  }
  if (sensorValue < valueNorma && (millis() - timePause) > timeSendPause) {
    valueSHIM = valueSHIM + fadeAmount;
    timePause = millis();
  }
  // ограничивает ШИМ насоса
  if (valueSHIM > 255) {
    valueSHIM = 255;
  }
  // не дает уходить в отрицательные значения
  if (valueSHIM < 0) {
    valueSHIM = 0;
  }
  // в cлучае достижения максимального уровня устанавливает ШИМ насоса в 0, перводит переменную stop
  // в значение 1, запоминает последнее ШИМ насоса
  if (sensorValue > valueMax && stop == 0) {
    //timePause = (millis() - timeSendPause);
    stop = 1;
    fadeAmountLast = fadeAmount;
    lastPower = valueSHIM;
    valueSHIM = 0;
  }
  if (sensorValue > valueMax) {
    valueSHIM = 0;
  }
  // в случае снижение уровня датчика жидкости, во время остановки ШИМ насоса, присваивает ШИМ
  // последнее значение отняв половину последнего шага, сбрасывает переменные
  if (sensorValue < (valueMax - 2) && stop == 1) {
    valueSHIM = lastPower -  fadeAmountLast % 2;
    //fadeAmountLast = 0;
    //lastPower = 0;
    stop = 0;
    timePause = (millis() - timeSendPause);
  }
  // ограничивает ШИМ насоса
  if (valueSHIM > 255) {
    valueSHIM = 255;
  }
  // не дает уходить в отрицательные значения
  if (valueSHIM < 0) {
  valueSHIM = 0;
  }
}
void lcdprint001() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:0.1 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint002() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:0.2 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint003() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:0.3 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint004() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:0.4 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint005() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:0.5 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint006() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:0.6 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint0() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("0");
  lcd.print("S");
  lcd.print(sensorValue);
  //lcd.setCursor(0, 5);
  lcd.print("/");
  lcd.print(valueNorma);
  //lcd.setCursor(0, 9);
  lcd.print(" P");
  lcd.print(valueSHIM);
  lcd.print("!");
  lcd.print(millis() / 60000);
  lcd.setCursor(0, 1);
  lcd.print("F");
  lcd.print(fadeAmount);
  //lcd.setCursor(1, 6);
  lcd.print(" t");
  lcd.print((timeSendPause - (millis() - timePause))/ 1000);
  lcd.print(" T");
  lcd.print(meteoSensor.getTemperatureC(), 1);
  lcd.print(" H");
  lcd.print(meteoSensor.getHumidity(), 0);
 }
void lcdprint101() 
{
	valueNorma = lcdparam;
	if (valueMax < valueNorma) valueNorma = valueMax;
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("PAGE:1.1 ");
	lcd.print("VALUE") ;
    lcd.setCursor(0, 1);
	lcd.print("NORM IS: ");
	lcd.print(valueNorma);
 }
void lcdprint102() 
{
	valueMax = lcdparam;
	if (valueMax < valueNorma) valueMax = valueNorma;
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("PAGE:1.2 ");
	lcd.print("VALUE");
    lcd.setCursor(0, 1);
	lcd.print("MAX IS: ");
	lcd.print(valueMax);
 }
void lcdprint103() 
{
  int WHOUR;
  WHOUR = lcdparam;
  timeSendWork = (WHOUR * 60 * 60 * 1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:1.3 ");
  lcd.print("WORK");
  lcd.setCursor(0, 1);
  lcd.print("TIME IS: ");
  lcd.print(WHOUR);
  lcd.print(" HOUR");
 }
void lcdprint104() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:1.4 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint105() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:1.5 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint106() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:1.6 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint1() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:1 ");
  lcd.print("POWER:");
  lcd.print(valueSHIM);
  lcd.setCursor(0, 1);
  lcd.print("SENSOR: ");
  lcd.print(sensorValue);
  lcd.print("/");
  lcd.print(valueNorma);
}
void lcdprint201() 
{
	timeSendPause = lcdparam;
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("PAGE:2.2 ");
	lcd.print("TIMER") ;
    lcd.setCursor(0, 1);
	lcd.print("PAUSE: ");
	lcd.print(timeSendPause);
 }
void lcdprint202() 
{
	timeSendPause = lcdparam;
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("PAGE:2.2 ");
	lcd.print("TIMER") ;
    lcd.setCursor(0, 1);
	lcd.print("PAUSE: ");
	lcd.print(timeSendPause);
 }
void lcdprint203() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:2.3 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint204() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:2.4 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint205() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:2.5 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint206() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:2.6 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint2() 
{
  lcd.clear();
  lcd.setCursor(0, 0);  
  lcd.print("PAGE: 2 ");
  lcd.print("STEP: ");
  lcd.print(fadeAmount);
  lcd.setCursor(0, 1);
  if (stop == 1) lcd.print("ALARM ");
  if (stop == 0) lcd.print("SHTIL ");
  lcd.print("TIME: ");
  lcd.print(timeSendPause);
}
void lcdprint301() 
{
	A = lcdparam;
	if(A>100) A =0;
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("PAGE:3.1 ");
	lcd.print("PROF%: ") ;
    lcd.setCursor(0, 1);
	lcd.print("      ");
	lcd.print(A);
	lcd.print("%");
 }
void lcdprint302() 
{
	A = lcdparam;
	if(A>100) A =0;
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("PAGE:3.2 ");
	lcd.print("PROF%: ") ;
    lcd.setCursor(0, 1);
	lcd.print("      ");
	lcd.print(A);
	lcd.print("%");
 }
void lcdprint303() 
{
	B = lcdparam;
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("PAGE:3.3 ");
	lcd.print("VOL ML:") ;
    lcd.setCursor(0, 1);
	lcd.print("      ");
	lcd.print(B);
	lcd.print(" ML");
 }
void lcdprint304() 
{
	C = lcdparam;
	if(C>100) C =0;
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("PAGE:3.4 ");
	lcd.print("PROF%: ") ;
    lcd.setCursor(0, 1);
	lcd.print("     ");
	lcd.print(C);
	lcd.print("%:");
}
void lcdprint305() 
{
	C = lcdparam;
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("P3.5 ");
	lcd.print("VOL RAZBAV:") ;
    lcd.setCursor(0, 1);
	lcd.print(D);
	lcd.print(" ML:");
 }
void lcdprint306() 
{
	C = lcdparam;
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("P:3.6 ");
	lcd.print("VOL ALL: ") ;
    lcd.setCursor(0, 1);
	//lcd.print(" ");
	lcd.print(A * B / C);
	lcd.print("ML");
}
void lcdprint3() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE: 3 ");
  lcd.print(timeClient.getFormattedTime());
  lcd.setCursor(0, 1);
  lcd.print("T: ");
  lcd.print(meteoSensor.getTemperatureC(), 1);
  lcd.print("*");
  lcd.print(" H: ");
  lcd.print(meteoSensor.getHumidity(), 0);
  lcd.print("%");
}
void lcdprint401() 
{
  valueMax = lcdparam;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:4.1 ");
  lcd.print("MAX ") ;
  lcd.setCursor(0, 1);
  lcd.print("VALUE IS: ");
  lcd.print(valueMax);
 }
void lcdprint402() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:4.2 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint403() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:4.3 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint404() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:4.4 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint405() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:4.5 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint406() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:4.6 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint4() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("4");
  lcd.print("SENSOR MAX: ");
  lcd.print(valueMax);
  lcd.setCursor(0, 1);
  lcd.print("SENSOR NORM: ");
  lcd.print(valueNorma);
}
void lcdprint501() 
{
  A = lcdparam;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE5.1 ");
  lcd.print("PROF%:");
  lcd.setCursor(0, 1);
  lcd.print(A);
  lcd.print("%");
 }
void lcdprint502() 
{
  C  = lcdparam;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE5.2 ");
  lcd.print("PROF%:");
  lcd.setCursor(0, 1);
  lcd.print(C);
  lcd.print("%");
 }
void lcdprint503() 
{
  D1 = lcdparam;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("P 5.3");
  lcd.print("VOLUME ALL:");
  lcd.setCursor(0, 1);
  lcd.print(D1);
  lcd.print("ML");
 }
void lcdprint504() 
{
  long SPIRT = ((C * D1)/A);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("P:5.4 ");
  lcd.print("SPIRT VOL:");
  lcd.setCursor(0, 1);
  lcd.print(SPIRT);
  lcd.print(" ML");
 }
void lcdprint505() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:5.5 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint506() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:5.6 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint5()
  {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE: 5 ");
  lcd.setCursor(0, 1);
  lcd.print("PAUSE:");
  lcd.print((timeSendPause - (millis() - timePause))/ 1000);
  lcd.print(" LSTEP:");
  lcd.print(fadeAmountLast);
 }
void lcdprint601() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:6.1 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint602() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:6.2 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint603() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:6.3 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint604() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:6.4 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint605() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint606() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:6.6 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint6() 
    {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("P:6 LIGTH:");
  lcd.print(sensorLight.getLightLux(),0);
  lcd.print("LUX");
  lcd.setCursor(0, 1);
  lcd.print(" LCD: ");
  lcd.print(lcdbrithness);
 } 
void lcdprint701() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:7.1 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint702() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:7.2 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint703() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:7.3 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint704() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:7.4 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint705() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:7.5 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint706() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE:7.6 ");
  lcd.setCursor(0, 1);
  lcd.print("RETURN PRESSLEFT");
 }
void lcdprint7() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PAGE7 ");
  lcd.print(timeClient.getFormattedTime());
  lcd.setCursor(0, 1);
  lcd.print("LAST POWER: ");
  lcd.print(lastPower);
}
void serialprint() 
  {
  Serial.print("STOP: ");
  Serial.println(stop);
  Serial.print("Last Power: ");
  Serial.println(lastPower);
  Serial.print("fadeAmountLast: ");
  Serial.println(fadeAmountLast);
  Serial.print("Sensor Value: ");
  Serial.println(sensorValue);
  Serial.print("Power Value: ");
  Serial.println(valueSHIM);
  Serial.print("Delta Value: ");
  Serial.println(abs(sensorValue - valueNorma));
  Serial.print("Fade Amount: ");
  Serial.println(fadeAmount);
  Serial.print("Temperature: ");
  Serial.print(meteoSensor.getTemperatureC());
  Serial.println(" C \t");
  Serial.print("Humidity:  ");
  Serial.println(meteoSensor.getHumidity());
  Serial.print("Light Lux: ");
  Serial.println(sensorLight.getLightLux());
  //выводим время с момента старта программы
  Serial.println(time);
  Serial.println(timePause);
}
void buttons()  
  {    
	buttleft.tick();  // обязательная функция отработки. Должна постоянно опрашиваться
	buttrith.tick();  // обязательная функция отработки. Должна постоянно опрашиваться
    buttup.tick();    // обязательная функция отработки. Должна постоянно опрашиваться  
    buttdown.tick();  // обязательная функция отработки. Должна постоянно опрашиваться
  
  int tmrhldbtn;
  if (buttup.isPress() || buttdown.isPress()) tmrhldbtn = millis();
  if ((millis() - tmrhldbtn) > 1500)
  {
  if (buttup.isHold() || buttdown.isHold()) btnincriment++;
  tmrhldbtn = millis();
  }
  if (buttup.isRelease() || buttdown.isRelease()) btnincriment = 1;
 
	if (lcdsubstring == 0)
		{
		if (buttleft.isClick()) lcdprintnumber--;
		if (buttleft.isStep())  lcdprintnumber--;  
  
		if (buttrith.isClick()) lcdprintnumber++; 
		if (buttrith.isStep())  lcdprintnumber++; 
		
		if (buttup.isStep() || buttup.isClick() || buttdown.isStep() || buttdown.isClick())
			{			
			//lcdparam = 0;
			lcdsubstring = 1; 
			lcdsbstrnumber = 1;
			}
		}
		
	if (lcdsubstring == 1)
	  {
		if (buttleft.isClick()) lcdsbstrnumber--;
		if (buttleft.isStep())  lcdsbstrnumber--; 
  
		if (buttrith.isClick()) lcdsbstrnumber++; 
		if (buttrith.isStep()) 	lcdsbstrnumber++; 	
	    
		if (buttup.isClick()) 	lcdparam++;
		if (buttdown.isClick())	lcdparam--;
		 
		if (buttdown.isStep())	(lcdparam = lcdparam - btnincriment);
		if (buttup.isStep())	(lcdparam = lcdparam + btnincriment); 
			
		
	}
  
  if (lcdprintnumber > 7) lcdprintnumber = 0;
  if (lcdprintnumber < 0) lcdprintnumber = 7;
  
  if (lcdsbstrnumber > 6) lcdsbstrnumber =0;
  if (lcdsbstrnumber < 0) lcdsbstrnumber =6;
  
  if (lcdparam > 100000) lcdparam =0;
  if (lcdparam < 0) lcdparam = 0;
  
  if (lcdsbstrnumber == 0) lcdsubstring = 0;
  
  lcdwindow = ((lcdprintnumber * 100) +	lcdsbstrnumber);
  //Serial.print("LCDWINDOW: ");
  //Serial.println(lcdwindow);
  if ((buttleft.isRelease()) || (buttrith.isRelease()) || (buttdown.isRelease()) || (buttup.isRelease())) tmrSleep = millis();
  if ((millis() - tmrSleep) > 60000) 
  {
	  sleeplcdwindow = lcdwindow;
	  lcdwindow = 300;
	  if ((buttleft.isRelease()) || (buttrith.isRelease()) || (buttdown.isRelease()) || (buttup.isRelease()))
	  {
		  tmrSleep = millis();
		  lcdwindow = sleeplcdwindow;
	  }
	}
  
 }
void lcdprint () 
{
	if (lcdwindow != oldlcdwindow)
	{
		switch (lcdwindow) 
		{
			case 0: 	lcdprint0();
				break;
				
			case 001: 	lcdprint001();
				break;
				
			case 100: 	lcdprint1();
				break;
				
			case 101:
				{
				lcdparam = valueNorma;
				lcdprint101();
				}
				break;
				
			case 102: 
				{
				lcdparam = valueMax;
				lcdprint102();
				}
				break;
				
			case 103: 	lcdprint103();
				break;
				
			case 104: 	lcdprint104();
				break;
				
			case 105: 	lcdprint105();
				break;
				
			case 106: 	lcdprint106();
				break;
				
			case 200: 	lcdprint2();
				break;
				
			case 201: 
				{
				lcdparam = timeSendPause;
				lcdprint201();
				}
				break;
				
			case 202: 
				{
				lcdparam = timeSendPause;
				lcdprint202();
				}
				break;
				
			case 203: 	lcdprint203();
				break;
				
			case 204: 	lcdprint204();
				break;
				
			case 205: 	lcdprint205();
				break;
				
			case 206: 	lcdprint206();
				break;
				
			case 300: 	lcdprint3();
				break;
				
			case 301: 	
				{
				lcdparam = A;
				lcdprint301();
				}
				break;
				
			case 302: 
				{
				lcdparam = A;
				lcdprint302;
				}
				break;
				
			case 303: 
				{
				lcdparam = B;
				lcdprint303();
				}
				break;
				
			case 304: 
				{
				lcdparam = C;
				lcdprint304();
				}
				break;
				
			case 305: 	lcdprint305();
				break;
				
			case 306: 	lcdprint306();
				break;
				
			case 400: 	lcdprint4();
				break;
				
			case 401:
			{			lcdprint401();
						lcdparam = valueMax;
			}
				break;
				
			case 402: 	lcdparam = valueMax;
			
				break;
			case 403: 	lcdprint403();
				break;
			case 404: 	lcdprint404();
				break;
			case 405: 	lcdprint405();
				break;
			case 406: 	lcdprint406();
				break;
			case 500: 	lcdprint5();
				break;
			case 501:    
			{
				lcdparam = A; 	
				lcdprint501();
			}
				break;
			case 502:
			{
				lcdparam = C;
				lcdprint502();
			}
				break;
			case 503:
			{
				lcdparam = 10000;
				lcdprint503();
			}
				break;
			case 504: 	lcdprint504();
				break;
			case 505: 	lcdprint505();
				break;
			case 506: 	lcdprint505();
				break;
			case 600: 	lcdprint6();
				break;
			case 601: 	lcdprint601();
				break;
			case 602: 	lcdprint602();
				break;
			case 603: 	lcdprint603();
				break;
			case 604: 	lcdprint604();
				break;
			case 605: 	lcdprint605();
				break;
			case 606: 	lcdprint606();
				break;
			case 700: 	lcdprint7();
				break;
			case 701: 	lcdprint701();
				break;
			case 702: 	lcdprint702();
				break;
			case 703: 	lcdprint703();
				break;
			case 704: 	lcdprint704();
				break;
			case 705: 	lcdprint705();
				break;
			case 706: 	lcdprint706();
				break;
		}
	oldlcdwindow = lcdwindow;
	}
	else 
	{
					switch (lcdwindow) 
		{
			case 0: 	lcdprint0();
				break;
			case 001: 	lcdprint001();
				break;
			case 100: 	lcdprint1();
				break;
			case 101: 	lcdprint101();
				break;
			case 102:   lcdprint102();
				break;
			case 103: 	lcdprint103();
				break;
			case 104: 	lcdprint104();
				break;
			case 105: 	lcdprint105();
				break;
			case 106: 	lcdprint106();
				break;
			case 200: 	lcdprint2();
				break;
			case 201: 	lcdprint201();
				break;
			case 202: 	lcdprint202();
				break;
			case 203: 	lcdprint203();
				break;
			case 204: 	lcdprint204();
				break;
			case 205: 	lcdprint205();
				break;
			case 206: 	lcdprint206();
				break;
			case 300: 	lcdprint3();
				break;
			case 301: 	lcdprint301();
				break;
			case 302: 	lcdprint302();
				break;
			case 303: 	lcdprint303();
				break;
			case 304: 	lcdprint304();
				break;
			case 305: 	lcdprint305();
				break;
			case 306: 	lcdprint306();
				break;
			case 400: 	lcdprint4();
				break;
			case 401: 	lcdprint401();
				break;
			case 402: 	lcdprint402();
				break;
			case 403: 	lcdprint403();
				break;
			case 404: 	lcdprint404();
				break;
			case 405: 	lcdprint405();
				break;
			case 406: 	lcdprint406();
				break;
			case 500: 	lcdprint5();
				break;
			case 501: 	lcdprint501();
				break;
			case 502: 	lcdprint502();
				break;
			case 503: 	lcdprint503();
				break;
			case 504: 	lcdprint504();
				break;
			case 505: 	lcdprint505();
				break;
			case 506: 	lcdprint506();
				break;
			case 600: 	lcdprint6();
				break;
			case 601: 	lcdprint601();
				break;
			case 602: 	lcdprint602();
				break;
			case 603: 	lcdprint603();
				break;
			case 604: 	lcdprint604();
				break;
			case 605: 	lcdprint605();
				break;
			case 606: 	lcdprint606();
				break;
			case 700: 	lcdprint7();
				break;
			case 701: 	lcdprint701();
				break;
			case 702: 	lcdprint702();
				break;
			case 703: 	lcdprint703();
				break;
			case 704: 	lcdprint704();
				break;
			case 705: 	lcdprint705();
				break;
			case 706: 	lcdprint706();
				break;
		}
	}
}
void ntptask()
    {
  timeClient.update();
  //Serial.print("ntptask: ");
  //Serial.println(timeClient.getFormattedTime());
  //Serial.println(timeClient.getEpochTime());
  //Serial.println("end!");
    }
void webtask()
{  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            int sensorReading = analogRead(analogChannel);
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("<br />");
          }
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}
void printWifiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
void setup() 
{
  Serial.begin(115200);
  buttleft.setDebounce(50);        // настройка антидребезга (по умолчанию 80 мс)
  buttleft.setTimeout(300);        // настройка таймаута на удержание (по умолчанию 500 мс)
  buttleft.setClickTimeout(600);   // настройка таймаута между кликами (по умолчанию 300 мс)
  buttleft.setType(HIGH_PULL);
  buttleft.setDirection(NORM_OPEN);
  buttdown.setDebounce(50);        // настройка антидребезга (по умолчанию 80 мс)
  buttdown.setTimeout(300);        // настройка таймаута на удержание (по умолчанию 500 мс)
  buttdown.setClickTimeout(600);   // настройка таймаута между кликами (по умолчанию 300 мс)
  buttdown.setType(HIGH_PULL);
  buttdown.setDirection(NORM_OPEN);
  buttrith.setDebounce(50);        // настройка антидребезга (по умолчанию 80 мс)
  buttrith.setTimeout(300);        // настройка таймаута на удержание (по умолчанию 500 мс)
  buttrith.setClickTimeout(600);   // настройка таймаута между кликами (по умолчанию 300 мс)
  buttrith.setType(HIGH_PULL);
  buttrith.setDirection(NORM_OPEN);
  buttup.setDebounce(50);        // настройка антидребезга (по умолчанию 80 мс)
  buttup.setTimeout(300);        // настройка таймаута на удержание (по умолчанию 500 мс)
  buttup.setClickTimeout(600);   // настройка таймаута между кликами (по умолчанию 300 мс)
  buttup.setType(HIGH_PULL);
  buttup.setDirection(NORM_OPEN);
   // устанавливаем количество столбцов и строк экрана
  lcd.begin(16, 2);
  // устанавливаем контрастность в диапазоне от 0 до 63
  // для Arduino c логическим напряжением 5 В используйте параметр 30
  // для Arduino c логическим напряжением 3,3 В используйте параметр 45
  lcd.setContrast(25);
  // устанавливаем яркость в диапазоне от 0 до 255
  //запускаем метеодатчик
  meteoSensor.begin();
    while (!Serial);

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION)
  {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    //delay(10000);
  }

  Serial.println("Connected to wifi");
//  printWifiStatus();
  Serial.println("Starting connection to server...");
  timeClient.begin();
  server.begin();
  ntptask();
 }
void lcdbrith()
	{
		int brightness = sensorLight.getLightLux();
		brightness = constrain(brightness,10,500);
		lcdbrithness = map(brightness, 10, 500, 15, 255);
		lcd.setBrightness(lcdbrithness);
		//Serial.print("lcdbrithness; ");
	    //Serial.println(lcdbrithness);
	}
void loop()
  {
    startprocedure();
    buttons();
	//webtask();
   
   if (myTimer0.isReady())
   {
   //Serial.println("Timer 0!");
    }

   if (myTimer1.isReady())
   {
   //Serial.println("Timer 1!");
   fadeselect();
   selectpumpvalue();
   volumespirt();
      }
     
  if (myTimer2.isReady())
  {
      //Serial.println("Timer 2!")
	lcdbrith(); 
    lcdprint();
	
  /*Serial.print("lcdwindow is: ");
  Serial.println(lcdwindow);
  Serial.print("oldlcdwindow is: ");
  Serial.println(oldlcdwindow);
  Serial.print("tmrSleep is: ");
  Serial.println(tmrSleep);
  Serial.print("Delta: ");
  Serial.println(millis() - tmrSleep);*/
	  
	/*Serial.print("lcdprintnumber: ");
	Serial.println(lcdprintnumber);
	Serial.print("lcdsubstring: ");
	Serial.println(lcdsubstring);
	Serial.print("lcdparam: ");
	Serial.println(lcdparam);
	Serial.print("lcdsbstrnumber: ");
	Serial.println(lcdsbstrnumber);
	Serial.print("lcdwindow: ");
	Serial.println(lcdwindow);*/
  }
       
  if (myTimer3.isReady())
  {
     //Serial.println("Timer 3!");
	 //volumespirt();
   } 
  if (myTimer4.isReady())
  {
     ntptask();
     //Serial.println("Timer 4!");
	 //volumespirt();
   }
          
  //lcdprint0();
  //lcdprint1();
  //lcdprint2();
  //lcdprint3();
  //lcdprint4();
  //serialprint();
  //testbutton();  
}
