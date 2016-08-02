/*
  Use ESP8266 microcontroller to run FONA 3G/GPS board
 */

#include <ESP8266WiFi.h>
 
extern "C" {
  #include "user_interface.h"
}

// WiFi Variables
const char WiFiAPPSK[]="ECE420";
WiFiServer server(80);

// Timer for FONA calls
os_timer_t fonaTimer;

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

String latitude;
String North_South;
String longitude;
String East_West;
String Date;
String UTC_Time;
String altitude;
String Speed;
String course;
float Converted_Latitude;
float Converted_Longitude;
char snd[255];

// Get connected devices
//struct station_info *stat_info;

/*
unsigned char softap_stations_cnt;
struct station_info *stat_info;
struct ip_addr *IPaddress;
uint32 uintaddress;*/

void setup() {
  // Initialize Serial:
  Serial.begin(115200);
  
  // Reserve 200 bytes for the inputString:
  inputString.reserve(200);

  // Initiate timer for FONA commands
  os_timer_setfn(&fonaTimer, timerCallback, NULL);
  os_timer_arm(&fonaTimer, 5000, true);

  // Get WiFi Station info
  //stat_info = wifi_softap_get_station_info();

  // 15 Sec wait to change pins from Arduino to FONA
  // ( Development only )
  //delay(15000);

  
  //fonaConfig();

  // WiFi Setup
  initHardware();
  setupWiFi();
  server.begin();
  
  
  //gpsLooper();
}

void loop() {
  // print the string when a newline arrives:
  serialEvent();

  WiFiClient client = server.available();
  if(!client)
  {
    return;
  }

  // Read the first line of the request
  String req = client.readStringUntil('/r');
  client.flush();

  //soft_stations_cnt = wifi_softap_get_station_num();
  //stat_info = wifi_softap_get_station_info();
  

  // Prepare the HTTP response
  String s = "HTTP:/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  
  s += "ESP Welcome Screen!\n";
 
  s += "</html>\n";

  //stat_info = STAILQ_NEXT(stat_info, next);
  
  // Send the response to the client
  client.print(s);
  delay(1);
  
}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  int ended=0;
  int count=0;
  while (!ended) {
    // get the new byte:
    if( Serial.available() )
    {
     inputString = Serial.readString();
     Serial.println(inputString);
     ParseSerialData();
    } else {
      ended=1;
      stringComplete = 1;
    }
  }
}
/*
 Parse Serial Data into usable chunks
 */
void ParseSerialData()
{
  int firstIndex = inputString.indexOf(':');
  int commaIndex  = inputString.indexOf(',',firstIndex);
  int secondComma = inputString.indexOf(',',commaIndex+1);
  int thirdComma = inputString.indexOf(',',secondComma+1);
  int fourthComma = inputString.indexOf(',',thirdComma+1);
  int fifthComma = inputString.indexOf(',',fourthComma+1);
  int sixthComma = inputString.indexOf(',', fifthComma+1);
  int seventhComma = inputString.indexOf(',',sixthComma+1);
  int eighthComma = inputString.indexOf(',', seventhComma+1);
  int ninthComma = inputString.indexOf(',', eighthComma+1);

  latitude = inputString.substring(firstIndex+1,commaIndex);
  North_South = inputString.substring(commaIndex+1, secondComma);
  longitude = inputString.substring(secondComma+1, thirdComma);
  East_West = inputString.substring(thirdComma+1, fourthComma);
  Date = inputString.substring(fourthComma+1, fifthComma);
  UTC_Time = inputString.substring(fifthComma+1, sixthComma);
  altitude = inputString.substring(sixthComma +1, seventhComma);
  Speed = inputString.substring(seventhComma+1, eighthComma);
  course = inputString.substring(eighthComma+1, ninthComma);

  convertGPS();

  Serial.print("Latitude = ");Serial.println(Converted_Latitude,6);
  Serial.print("North_South = ");Serial.println(North_South);
  Serial.print("Longitude = ");Serial.println(Converted_Longitude,6);
  Serial.print("East_West= ");Serial.println(East_West);
  Serial.print("Date = ");Serial.println(Date);
  Serial.print("UTC Time = ");Serial.println(UTC_Time);
  Serial.print("Altitude = ");Serial.println(altitude);
  Serial.print("Speed = ");Serial.println(Speed);
  Serial.print("Course = ");Serial.println(course);
}

/*
 * Function that converts the GPS data from Decimal minutes into Decimal Degrees
 */
void convertGPS(){
  String latitudeDecimal = latitude.substring(0,2);
  String latitudeDegrees = latitude.substring(2);

  float latDecimal = latitudeDecimal.toFloat();
  float latDegrees = latitudeDegrees.toFloat();

  latDegrees = latDegrees/60.0000;

  Converted_Latitude = latDecimal + latDegrees;
  
  String long_pos_or_neg = longitude.substring(0,1);
  String longitudeDecimal = longitude.substring(1,3);
  String longitudeDegrees = longitude.substring(3);

  float longDecimal = longitudeDecimal.toFloat();
  float longDegrees = longitudeDegrees.toFloat();
  if(long_pos_or_neg == "0"){
    longDecimal *= -1.0;
  }

  Serial.println(longDecimal, 6);
  Serial.println(longDegrees, 6);

  longDegrees = longDegrees/60.0000;
  Serial.println(longDegrees, 6);

  Converted_Longitude = longDecimal - longDegrees;
  
}
/*
 Writes snd buffer to serial
 */
void serialWrite() {
  Serial.printf("%s",snd);
}

void fonaConfig() {
  
  // Turn GPS on
  char str[] = "AT+CGPS=1,2\r\n";
  strncpy(snd, str, sizeof(str) );
  serialWrite();

  delay(1000);

  // Is GPS on?
  char str2[] = "AT+CGPS?\r\n";
  strncpy(snd, str2, sizeof(str2));
  serialWrite();

}

void queryGPS() {
  char str[] = "AT+CGPSINFO\r\n";
  strncpy(snd, str, sizeof(str));
  serialWrite();
}

void gpsLooper() {
  while(1) {
    queryGPS();
    delay(3000);
  }
}

void setupWiFi()
{
  WiFi.mode(WIFI_AP);

  String WiFiName = "Shuttle Tracker";
  char APName[WiFiName.length()+1];
  memset(APName, 0, WiFiName.length()+1);

  for( int i=0; i<WiFiName.length(); i++ )
  {
    APName[i] = WiFiName.charAt(i);
  }

  WiFi.softAP(APName, WiFiAPPSK);
}

void initHardware()
{
  
}

void timerCallback(void *pArg)
{
  queryGPS();
}


