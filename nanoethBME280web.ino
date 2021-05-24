/****************************************************************************/
/*                                                                          */
/****************************************************************************/


#define P(x) (__FlashStringHelper*)(x)    // F-Makro für Variablen

const char cHtmlButton[] PROGMEM = { "<input type=\"button\" style=\"font-size:20px\" value=\"Zur&uuml;ck\" onClick=\"history.back();\">" };

const char cHtmlHeader[] PROGMEM = { "<html><title>Wetterstation ZB04</title>\n<center>\n<h2>Wetterstation ZB04</h2>\n<br><br>\n" };

const char cFile[] PROGMEM = {__FILE__};
const char cDate[] PROGMEM = {__DATE__};
const char cTime[] PROGMEM = {__TIME__};


#include <UIPEthernet.h> // Used for Ethernet

#define BME280



#ifdef BME280
#include "cactus_io_BME280_I2C.h"
BME280_I2C bme(0x76); // I2C using address 0x76
#endif

#define	VERSION	"0.01"

/*
in EEPROM
- Adresse
- Namen
- IP
- TOPIC
- offset
*/


// **** ETHERNET SETTING ****
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x78, 0xEE  };                                       
IPAddress ip(192, 168, 178, 205);                        
EthernetServer server(80);

/****************************************************************************/
/*                                                                          */ 
/****************************************************************************/
void setup() {
#ifdef SERIALDEBUG
  Serial.begin(9600);
#endif

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();

#ifdef SERIALDEBUG
  Serial.print("IP:");
  Serial.println(Ethernet.localIP());
#endif


#ifdef BME280
  if (!bme.begin()) {
    while (1);
  }

  bme.setTempCal(0.5);// Temp was reading high so subtract x degree
#endif

}


/****************************************************************************/
/*                                                                          */ 
/****************************************************************************/
void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();

  if (client) {  
#ifdef SERIALDEBUG
    Serial.println("+");
#endif

    // an http request ends with a blank line
    boolean currentLineIsBlank = true;

    bme.readSensor();

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
#ifdef SERIALDEBUG
        Serial.print(c);
#endif
        if (c == '\n' && currentLineIsBlank) {
          client.print("<!--");
          client.println(P(cFile));
          client.println(VERSION);
          client.println(P(cDate));
          client.println(P(cTime));
          client.println("\n-->");

          client.println(P(cHtmlHeader));

          client.print("Luftdruck: ");
          client.print(bme.getPressure_HP());
          client.println(" hPa");
          client.println("<br><br>");
          client.print("Luftfeuchte: ");
          client.println(bme.getHumidity());
          client.println("<br><br>");
          client.print("Temperatur: ");
          client.print(bme.getTemperature_C());
          client.println(" &deg;C <br><br>");
          client.println(P(cHtmlButton));

          client.print("<!--");
          /* das wäre schön im eeprom zu haben */
          //client.print("MQTT:ZB04/Dach/Wetterstation/Temperatur ");
          client.print("\nMQTT:Temperatur ");
          client.print(bme.getTemperature_C());
          //client.print("MQTT:ZB04/Dach/Wetterstation/Humidity ");
          client.print("\nMQTT:Humidity ");
          client.print(bme.getHumidity());
          //client.print("MQTT:ZB04/Dach/Wetterstation/Airpressure ");
          client.print("\nMQTT:Airpressure ");
          client.print(bme.getPressure_HP());
          client.println("\n-->");

          client.println("</body>");
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
    delay(10);

    // close the connection:
    client.stop();
#ifdef SERIALDEBUG
    Serial.println("-\n");
#endif
  }


/*
#ifdef BME280
  bme.readSensor();

  bme.getPressure_HP();
  bme.getHumidity();
  bme.getTemperature_C();
#endif
*/

} /* loop */
