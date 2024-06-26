#include <arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <LiquidCrystal_I2C.h>
#include <OneButton.h>
#include <NTPClient.h>
#include <Wire.h>
#include <ESP32Time.h>
#include <ArduinoJson.h>

// ------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------
// Debugging
//
#define DEBUG 0

#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define wait(x) delay(x)
#else
#define debug(x)
#define debugln(x)
#define wait(x)
#endif

//-------------------------------------------------------------------

#define LCDCOLUMNS 20
#define LCDROWS 4

#define GREEN 19
#define YELLOW 18
#define RED 17

#define strRED "RED           "
#define strYELLOW "YELLOW        "
#define strGREEN "GREEN         "
#define strGreenYellow "GREEN/YELLOW  "
#define strGreenYellowRed "GRN/YEL/RED   "

#define scrButton 15
#define pressed HIGH

#define BLANKLCDLINE "                    "

#define MAXPAGE 2

#define MSG_BUFFER_SIZE (50)

// ---------------------------------------------------------------
// Global Variables

char colour[15] = "- No  Colour -";
int currentPage = 0;
int nextPage = 1;

// set LCD address, number of columns and rows
LiquidCrystal_I2C lcd(0x3F, LCDCOLUMNS, LCDROWS);

// // set LCD address, number of columns and rows
// LiquidCrystal_I2C lcd(0x3F, LCDCOLUMNS, LCDROWS);

WiFiUDP udp;
WiFiClient wifiClient;
PubSubClient MQTTClient(wifiClient);

OneButton button(scrButton, true, true);

// const char *SSID = "SnaxNet";
// const char *PASSWORD = "snaxmuppet4481";

char SSID[] = "SnaxNet-IoT";
char PASSWORD[] = "6yorkroad-iot";
IPAddress local_ip;
char ipAddress[] = "000.000.000.000";

char MQTTSERVER[] = "192.168.1.45";
char clientId[] = "ESP32AgilePriceMonitor1";
char subscribecurrentPeriodPrice[] = "agile/currentPeriodPrice";
char subscribeperiodAvgMinPrices[] = "agile/periodAvgMinPrices";
char subscribeperiodAvgStrtTimes[] = "agile/periodAvgStrtTimes";
char subscribeAgile[] = "agile/#";

unsigned long lastMsg = 0;

char msg[MSG_BUFFER_SIZE];
int value = 0;
char lcdLine[21];

char displayPrice[] = "00.00";
float price = atof((char *)displayPrice);

char TZ_INFO[] = "GMT0GMT,M3.5.0/1,M10.5.0";
NTPClient timeClient(udp, "pool.ntp.org", 0);
ESP32Time rtc(0);
char displayTime[9] = "00:00:00";

// ------------------------------------------------------------------------------
//

void ledsOFF()
{
  digitalWrite(RED, LOW);
  digitalWrite(YELLOW, LOW);
  digitalWrite(GREEN, LOW);
}

void ledsON()
{
  digitalWrite(RED, HIGH);
  digitalWrite(YELLOW, HIGH);
  digitalWrite(GREEN, HIGH);
}

void priceReceived(float price)
{
  debug("Price received = ");
  debugln(price);
}

void initLCD()
{
  lcd.init();
  lcd.backlight();
  lcd.clear();

  Wire.setClock(10000);
}

void printToLCD(char text[], int row, int column, bool clear)
{
  if (clear)
    lcd.clear();

  // debug("Sending to LCD : >");
  // debug(text);
  // debugln("<");

  lcd.setCursor(column, row);
  lcd.print(text);
}

void getLocalIP(IPAddress IP)
{
  uint32_t ip_addr = (uint32_t)IP;
  IPAddress _ip;
  _ip = ip_addr;

  snprintf(ipAddress, 16, "%d.%d.%d.%d", _ip[0], _ip[1], _ip[2], _ip[3]);
}

void setup_wifi()
{
  wait(100);
  digitalWrite(RED, HIGH);

  // We start by connecting to a WiFi network

  debug("Connecting to ");
  debugln(SSID);

  strcpy(lcdLine, (char *)SSID);
  printToLCD((char *)"Wifi...", 0, 0, 1);
  printToLCD(lcdLine, 1, 0, 0);
  delay(2000);

  WiFi.disconnect();
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(RED, LOW);
    wait(500);
    debug(".");
    digitalWrite(RED, HIGH);
    wait(500);
  }

  debugln("");
  debugln("WiFi connected");
  debug("IP address: ");
  debugln(WiFi.localIP());

  local_ip = WiFi.localIP();
  getLocalIP(local_ip);

  printToLCD((char *)"Good: IP Address: ", 2, 0, 0);
  printToLCD(ipAddress, 3, 0, 0);
  delay(3000);
}

void showPage(int reqPage)
{
  switch (reqPage)
  {
  case 1:
    printToLCD((char *)"Running", 0, 0, 1);
    printToLCD((char *)"1", 0, 19, 0);
    printToLCD((char *)BLANKLCDLINE, 3, 0, 0);
    printToLCD(displayPrice, 3, 0, 0);
    printToLCD(colour, 3, 6, 0);
    break;

  case 2:

    printToLCD((char *)"Page 2   ", 0, 0, 1);
    printToLCD((char *)"2", 0, 19, 0);
    printToLCD((char *)BLANKLCDLINE, 3, 0, 0);
    printToLCD((char *)"Love You!", 1, 5, 0);
    printToLCD(displayPrice, 3, 0, 0);
    printToLCD(colour, 3, 6, 0);
    break;
  }

  currentPage = reqPage;
}

void reconnect()
{
  digitalWrite(YELLOW, HIGH);
  printToLCD((char *)"Connecting to MQTT...", 0, 0, 1);
  wait(500);
  int row = 0;

  // Loop until we're reconnected
  while (!MQTTClient.connected())
  {
    digitalWrite(YELLOW, LOW);
    wait(500);

    printToLCD((char *)".", 1, row, 0);
    wait(500);

    //  Attempt to connect
    MQTTClient.connect(clientId);

    digitalWrite(YELLOW, HIGH);

    row++;
  }

  // Must be connected
  debugln("connected");
  printToLCD((char *)"Good", 1, 0, 0);
  wait(1000);

  if (!MQTTClient.subscribe(subscribeAgile))
  {
    Serial.println("subscribeAgile Not Subscribed");
  }

  delay(100);

  digitalWrite(YELLOW, HIGH);
  wait(500);

  showPage(currentPage);
}

void setLEDColour()
{
  if (price >= 25)
  {
    digitalWrite(RED, HIGH);
    strcpy(colour, strRED);
  }
  else if (price >= 20)
  {
    digitalWrite(YELLOW, HIGH);
    strcpy(colour, strYELLOW);
  }
  else if (price >= 10)
  {
    digitalWrite(GREEN, HIGH);
    strcpy(colour, strGREEN);
  }
  else if (price >= 0)
  {
    digitalWrite(GREEN, HIGH);
    digitalWrite(YELLOW, HIGH);
    strcpy(colour, strGreenYellow);
  }
  else
  {
    ledsON();
    strcpy(colour, strGreenYellowRed);
  }
}

void initLEDS(int count, int gap)
{

  while (count > 0)
  {
    ledsON();
    wait(gap);
    ledsOFF();
    wait(gap);
    --count;
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{

  payload[length] = '\0';

  debug("Message arrived [");
  debug(topic);
  debugln("] ");
  wait(1000);

  // check topic for the subscribed topic that has changed...

  // agile/currentPeriodPrice
  if (strcmp(topic, subscribecurrentPeriodPrice) == 0)
  {
    price = atof((char *)payload);
    dtostrf(price, 4, 2, displayPrice);

    debug("Converted price: ");
    debugln(price);
    debug(displayPrice);
    debugln();
    wait(1000);

    initLEDS(60, 20);
    setLEDColour();

    printToLCD(displayPrice, 3, 0, 0);
    printToLCD(colour, 3, 6, 0);
  }

  // agile/periodAvgMinPrices
  // if (strcmp(topic, subscribeperiodAvgMinPrices) == 0)
  // {
  //   printToLCD((char *)"Rec. Avg Min Prices", 2, 0, 0);
  //   delay(1000);
  //   printToLCD((char *)BLANKLCDLINE, 2, 0, 0);
  // }

  // agile/periodAvgStartTimes
  if (strcmp(topic, subscribeperiodAvgStrtTimes) == 0)
  {
    printToLCD((char *)"Rec.Avg Start Times", 2, 0, 0);
    delay(1000);
    printToLCD((char *)BLANKLCDLINE, 2, 0, 0);
  }
}

void selectNextPage(int reqPage)
{
  if (reqPage == 0)
  {
    if (currentPage == MAXPAGE)
      reqPage = 1;
    else
      reqPage = currentPage + 1;
  }
  nextPage = reqPage;
}

void setup_MQTT()
{
  digitalWrite(YELLOW, HIGH);

  MQTTClient.disconnect();

  MQTTClient.setKeepAlive(90);
  MQTTClient.setServer(MQTTSERVER, 1883);
  MQTTClient.setCallback(callback);

  if (!MQTTClient.connected())
  {
    reconnect();
  }
  digitalWrite(YELLOW, HIGH);
}

void singleClick()
{
  debugln("Single Click");
  wait(2000);

  selectNextPage(0);
}

void setup_button()
{
  button.attachClick(singleClick);
  // button.attachDoubleClick(doubleClick);
  // button.attachLongPressStop(longClick);
}

void setup_timeClient()
{
  debugln("Getting NTP time...");
  wait(1000);

  timeClient.begin();
  wait(100);

  while (!timeClient.update())
  {
    timeClient.forceUpdate();
  }

  unsigned long epochTime = timeClient.getEpochTime();
  rtc.setTime(epochTime);

  debug("RTC Time: ");
  debugln(rtc.getTime());

  delay(3000);
}

void getRTCTime()
{
  int Hr = rtc.getHour(true);
  int Min = rtc.getMinute();
  int Sec = rtc.getSecond();

  snprintf(displayTime, 16, "%02i:%02i:%02i", Hr, Min, Sec);
  printToLCD((char *)displayTime, 0, 9, 0);
}

void setup()
{

  pinMode(YELLOW, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  delay(1000);

  initLCD();

  initLEDS(5, 100);

  debugln("Start setup... ");

  strcpy(lcdLine, "Start...");
  printToLCD(lcdLine, 0, 0, 1);

  setup_wifi();
  setup_MQTT();
  setup_button();
  setup_timeClient();

  debugln("End setup... ");

  ledsON();
  wait(1000);
  ledsOFF();

  digitalWrite(GREEN, HIGH);
  digitalWrite(BUILTIN_LED, LOW);
}

void loop()
{
  // debug("MQTT connected status: ");
  // debugln(MQTTClient.connected());

  getRTCTime();

  while (!MQTTClient.loop())
  {
    reconnect();
  }

  button.tick();

  if (nextPage != currentPage)
  {
    showPage(nextPage);
  }
}
