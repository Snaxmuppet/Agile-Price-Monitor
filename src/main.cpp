#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <LiquidCrystal_I2C.h>
#include <NodeRedTime.h>

// set the LCD number of columns and rows
const int lcdColumns = 20;
const int lcdRows = 4;

const int GREEN = 14;
const int YELLOW = 12;
const int RED = 13;

const char *strRED = "RED";
const char *strYELLOW = "YELLOW";
const char *strGREEN = "GREEN";
char *colour = "colour";

char *blankLCDLine = "                    ";

// const char* ssid = "SnaxNet-IoT";
// const char* password = "6yorkroad-iot";
const char *ssid = "SnaxNet";
const char *password = "snaxmuppet4481";
const char *mqtt_server = "192.168.1.45";

const int maxPage = 4;
int currentPage = 0;
int nextPage = 1;

char displayTime[] = "no time";

// a "tm" structure is used to decompose a seconds
// value into its component parts (year, month, day, etc)
tm timeinfo;

// a variable to hold the seconds value
time_t epochTime;

// set LCD address, number of columns and rows
LiquidCrystal_I2C lcd(0x3F, lcdColumns, lcdRows);

WiFiUDP udp;

NodeRedTime nodeRedTime("http://192.168.1.45:1880/time/");
const char *TZ_INFO = "GMT0BST,M3.5.0/1,M10.5.0";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
char *lcdLine;

char displayPrice[] = "00.00";
float price = atof((char *)displayPrice);

void setup()
{
  pinMode(YELLOW, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BUILTIN_LED, OUTPUT); // Initialize the BUILTIN_LED pin as an output

  Serial.begin(115200);
  delay(1000);

  // inform time.h of the rules for local time conversion
  setenv("TZ", TZ_INFO, 1);

  initLCD();

  initLEDS(5, 100);

  Serial.println("Start setup... ");

  lcdLine = "Start...";
  printToLCD(lcdLine, 0, 0, 1);

  // printToLCD((char *)ntpClient.getUnixTime(),2,0,0);
  delay(3000);

  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

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

  // delay(1000);  //wait for 1 sec

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  if (nextPage != currentPage)
    showPage(nextPage);

  // unsigned long now = millis();
  // if (now - lastMsg > (1 * 60 * 1000)) {
  //   lastMsg = now;
  //   ++value;
  //   snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
  //   Serial.print("Publish message: ");
  //   Serial.println(msg);
  // client.publish("outTopic", msg);
  //}
}

void setup_wifi()
{
  digitalWrite(RED, HIGH);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  strcpy(lcdLine, ssid);
  printToLCD("Wifi...", 0, 0, 1);
  printToLCD(lcdLine, 1, 0, 0);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

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
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  printToLCD("Good", 2, 0, 0);
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
    Serial.print("Attempting MQTT connection...");

    printToLCD("MQTT...", 0, 0, 1);
    delay(1000);

    // Create a random client ID
    String clientId = "ESP8266Client";
    // clientId += String(random(0xffff), HEX);
    //  Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      printToLCD("Good", 1, 0, 0);
      delay(1000);

      // Once connected, publish an announcement...
      // client.publish("outTopic", "hello world");
      // ... and resubscribe
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

  // Switch on appropriate LED
  //
  // RED >= 25
  // YELLOW >= 15 AND < 25
  // GREEN < 15
  //
  initLEDS(60, 20);
  setLEDColour();
  showPage(1);
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
  }
}

void selectNextPage(int reqPage)
{
  if (reqPage == 0)
  {
    if (currentPage == maxPage)
      reqPage = 1;
    else
      reqPage = currentPage + 1;
  }
  nextPage = reqPage;
}

bool showPage(int reqPage)
{
  int intHour = 0;
  int intMin = 0;
  int intSec = 0;

  char displayTime[6] = "00:00";

  intHour = timeinfo.tm_hour;
  intMin = timeinfo.tm_min;

  Serial.println(intHour);
  Serial.println(intMin);

  strcpy(displayTime, (char *)intHour);

  Serial.println("displayTime");
  Serial.println((char *)displayTime);

  if (reqPage == 1)
  {

    printToLCD("Running", 0, 0, 1);
    printToLCD((char *)displayTime, 0, 9, 0);
    printToLCD("1", 0, 19, 0);
    printToLCD(blankLCDLine, 3, 0, 0);
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
