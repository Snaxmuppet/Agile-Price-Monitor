#include <arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <LiquidCrystal_I2C.h>
#include <NodeRedTime.h>

// ------------------------------------------------------------------
// Defines

#define LCDCOLUMNS 20
#define LCDROWS 4

#define GREEN 19
#define YELLOW 18
#define RED 17

#define strRED "RED"
#define strYELLOW "YELLOW"
#define strGREEN "GREEN"

#define SCRSELECTBUTTON 10

#define BLANKLCDLINE "                    "

// const char* ssid = "SnaxNet-IoT";
// const char* password = "6yorkroad-iot";

#define MAXPAGE 4

#define MSG_BUFFER_SIZE (50)

// ---------------------------------------------------------------
// Global Variables

char colour[11] = "colour";
int currentPage = 0;
int nextPage = 1;

char displayTime[6] = "00:00";

// a "tm" structure is used to decompose a seconds
// value into its component parts (year, month, day, etc)
tm timeinfo;

// a variable to hold the seconds value
time_t epochTime;

// set LCD address, number of columns and rows
LiquidCrystal_I2C lcd(0x3F, LCDCOLUMNS, LCDROWS);

// Node Red Time
NodeRedTime nodeRedTime("http://192.168.1.45:1880/time/");
const char *TZ_INFO = "GMT0BST,M3.5.0/1,M10.5.0";

WiFiUDP udp;
WiFiClient espClient;
PubSubClient client(espClient);

const char *SSID = "SnaxNet";
const char *PASSWORD = "snaxmuppet4481";
const char *MQTTSERVER = "192.168.1.45";

unsigned long lastMsg = 0;

char msg[MSG_BUFFER_SIZE];
int value = 0;
char lcdLine[20];

char displayPrice[] = "00.00";
float price = atof((char *)displayPrice);

char sHour[3] = "hh";
char sMin[3] = "mm";

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
  Serial.print("Price received = ");
  Serial.println(price);
}

void initLCD()
{
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void printToLCD(char *text, int row, int column, bool clear)
{
  if (clear)
    lcd.clear();

  lcd.setCursor(column, row);
  lcd.print(text);
}

void setup_wifi()
{
  delay(100);
  digitalWrite(RED, HIGH);

  // We start by connecting to a WiFi network

  Serial.print("Connecting to ");
  Serial.println(SSID);

  strcpy(lcdLine, (char *)SSID);
  printToLCD("Wifi...", 0, 0, 1);
  printToLCD((char *)SSID, 1, 0, 0);

  delay(1000);

  WiFi.disconnect();
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(RED, LOW);
    delay(500);
    Serial.print(".");
    digitalWrite(RED, HIGH);
    delay(500);
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  printToLCD("Good", 2, 0, 0);

  delay(2000);
}

void reconnect()
{
  digitalWrite(YELLOW, HIGH);
  printToLCD("MQTT...", 0, 0, 1);
  delay(500);

  // Loop until we're reconnected
  while (!client.connected())
  {
    digitalWrite(YELLOW, LOW);
    delay(500);
    Serial.println("Attempting MQTT connection...");

    printToLCD("MQTT...", 0, 0, 1);
    delay(1000);

    // Create a random client ID
    String clientId = "ESP32AgilePriceMonitor";

    //  Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      printToLCD("Good", 1, 0, 0);
      delay(1000);

      Serial.print("Subscribing to: ");
      Serial.println(clientId);
      delay(1000);

      client.subscribe("agile/currentPeriodPrice");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      digitalWrite(YELLOW, HIGH);
      delay(500);
    }
  }
}

bool showPage(int reqPage)
{
  if (reqPage == 1)
  {

    printToLCD("Running", 0, 0, 1);
    printToLCD((char *)displayTime, 0, 9, 0);
    printToLCD("1", 0, 19, 0);
    printToLCD(BLANKLCDLINE, 3, 0, 0);
    printToLCD(displayPrice, 3, 0, 0);
    printToLCD(colour, 3, 6, 0);
  }

  currentPage = reqPage;
  return 1;
}

void setLEDColour()
{
  if (price >= 25)
  {
    digitalWrite(RED, HIGH);
    strcpy(colour, "RED");
  }
  else if (price >= 20)
  {
    digitalWrite(YELLOW, HIGH);
    strcpy(colour, "YELLOW");
  }
  else if (price >= 10)
  {
    digitalWrite(GREEN, HIGH);
    strcpy(colour, "GREEN");
  }
  else if (price >= 0)
  {
    digitalWrite(GREEN, HIGH);
    digitalWrite(YELLOW, HIGH);
    strcpy(colour, "GREEN/YELLOW");
  }
  else
  {
    ledsON();
    strcpy(colour, "GRN/YEL/RED");
  }
}

void initLEDS(int count, int gap)
{

  while (count > 0)
  {
    ledsON();
    delay(gap);
    ledsOFF();
    delay(gap);
    --count;
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{

  payload[length] = '\0';

  price = atof((char *)payload);
  dtostrf(price, 4, 2, displayPrice);

  Serial.print("Converted price: ");
  Serial.println(price);

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  Serial.print(displayPrice);

  Serial.println();

  initLEDS(60, 20);
  setLEDColour();
  showPage(1);
}

void getTime()
{
  Serial.println("getting time...");

  // fetch wallclock time as a seconds value
  bool success = nodeRedTime.serverTime(&epochTime);

  // could time be obtained?
  if (success)
  {

    // if you want your local time, use localtime_r()
    if (localtime_r(&epochTime, &timeinfo))
    {

      // display in human-readable form
      Serial.printf("local time: %s", asctime(&timeinfo));
    }

    // // if you want UTC aka GMT, use gmtime_r()
    // if (gmtime_r(&epochTime, &timeinfo)) {

    //   // display in human-readable form
    //   Serial.printf("UTC: %s", asctime(&timeinfo));
    // }

    strftime(sHour, 3, "%H", &timeinfo);
    strftime(sMin, 3, "%M", &timeinfo);

    strcpy(displayTime, sHour);
    strcat(displayTime, ":");
    strcat(displayTime, sMin);

    Serial.print("displayTime: ");
    Serial.println(displayTime);

    printToLCD((char *)displayTime, 0, 9, 0);
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

void setup()
{
  pinMode(YELLOW, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  // pinMode(SCRSELECTBUTTON, INPUT);

  Serial.begin(115200);

  delay(1000);

  // inform time.h of the rules for local time conversion
  setenv("TZ", TZ_INFO, 1);

  initLCD();

  initLEDS(5, 100);

  Serial.println("Start setup... ");

  strcpy(lcdLine, "Start...");
  printToLCD(lcdLine, 0, 0, 1);

  // printToLCD((char *)ntpClient.getUnixTime(),2,0,0);
  delay(3000);

  setup_wifi();

  client.setServer(MQTTSERVER, 1883);
  client.setCallback(callback);
  client.setKeepAlive(90);

  if (!client.connected())
  {
    reconnect();
  }

  Serial.println("End setup... ");

  ledsON();
  delay(1000);
  ledsOFF();

  digitalWrite(BUILTIN_LED, HIGH);
}

void loop()
{
  getTime();

  Serial.print("connected status: ");
  Serial.println(client.connected());
  delay(2000);

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  if (nextPage != currentPage)
    showPage(nextPage);

  delay(10000);
}
