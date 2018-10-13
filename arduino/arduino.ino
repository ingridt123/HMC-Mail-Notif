//#include <ESP8266WiFi.h>
//#include "Gsender.h"
#include<Wire.h>

#define Addr 0x13

//#pragma region Globals
//const char* ssid = "Claremont-ETC";             // WIFI network name
//const char* password = "abcdeabcde";            // WIFI network password
//
//WiFiServer server(80);
//
//uint8_t connection_state = 0;                    // Connected to WIFI or not
//uint16_t reconnect_interval = 10000;             // If not connected wait time to try again
//#pragma endregion Globals

// Variable to store the HTTP request
//String header;

// Auxiliar variables to store the current output state
// Assign output variables to GPIO pins
int esp8266 = 16;     // GPIO 16 = D0

// Variables for sending email
//Gsender *gsender = Gsender::Instance(); // Getting pointer to class instance
//String email = ""
//String subject = "Mail time";
//String msg = "You've got mail!";

// Upper and lower ranges for luminance
int detected = 15;
int empty = 30;


//// Connect to WiFi
//uint8_t WiFiConnect(const char* nSSID = nullptr, const char* nPassword = nullptr)
//{
//    static uint16_t attempt = 0;
//    Serial.print("Connecting to ");
//    if(nSSID) {
//        WiFi.begin(nSSID, nPassword);  
//        Serial.println(nSSID);
//    } else {
//        WiFi.begin(ssid, password);
//        Serial.println(ssid);
//    }
//
//    uint8_t i = 0;
//    while(WiFi.status()!= WL_CONNECTED && i++ < 50)
//    {
//        delay(200);
//        Serial.print(".");
//    }
//    ++attempt;
//    Serial.println("");
//    if(i == 51) {
//        Serial.print("Connection: TIMEOUT on attempt: ");
//        Serial.println(attempt);
//        if(attempt % 2 == 0)
//            Serial.println("Check if access point available or SSID and Password\r\n");
//        return false;
//    }
//    Serial.println("Connection: ESTABLISHED");
//    Serial.print("Got IP address: ");
//    Serial.println(WiFi.localIP());
//    server.begin();
//    return true;
//}
//
//void Awaits()
//{
//    uint32_t ts = millis();
//    while(!connection_state)
//    {
//        delay(50);
//        if(millis() > (ts + reconnect_interval) && !connection_state){
//            connection_state = WiFiConnect();
//            ts = millis();
//        }
//    }
//}


void setup()
{
  // Initialise I2C communication as MASTER 
  Wire.begin();
  // Initialise Serial Communication, set baud rate = 9600
  Serial.begin(9600);

  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select command register
  Wire.write(0x80);
  // Enable ALS and proximity measurement, LP oscillator
  Wire.write(0xFF);
  // Stop I2C Transmission
  Wire.endTransmission();

  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select proximity rate register
  Wire.write(0x82);
  // Set 1.95 proximity measurements/sec
  Wire.write(0x00);
  // Stop I2C Transmission
  Wire.endTransmission();

  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select ambient light rate register
  Wire.write(0x84);
  // Continuos conversion mode, Ambient light rate 2 samples/s
  Wire.write(0x9D);
  // Stop I2C Transmission
  Wire.endTransmission();
  delay(300);
  
  Serial.begin(115200);

  // Set up pins
  pinMode(esp8266, OUTPUT);
  
//  connection_state = WiFiConnect();
//  if(!connection_state)       // if not connected to WIFI
//  Awaits();                   // constantly trying to connect

}


void loop(){
  // check luminance every 5 minutes
  unsigned int data[4];
  for(int i = 0; i < 4; i++)
  { 
    // Start I2C Transmission
    Wire.beginTransmission(Addr);
    // Select data register
    Wire.write((133+i));
    // Stop I2C Transmission
    Wire.endTransmission();

    // Request 4 byte of data from the device
    Wire.requestFrom(Addr, 1);

    // luminance msb, luminance lsb, proximity msb, proximity lsb
    if(Wire.available() == 1)
    {
      data[i] = Wire.read();
    }
    delay(300);
  }

  // Convert the data
  float luminance = ((data[0] * 256) + data[1]);
  float proximity = ((data[2] * 256) + data[3]);

  // Output data to serial monitor
  Serial.print("Ambient Light luminance :");
  Serial.println(luminance);
  Serial.print("Proximity of the device :");
  Serial.println(proximity);
  delay(1000);

  // Counts number of times mail is detected continuously
  int count = 0;

  // Check luminance
  if (luminance < detected) {
    count++;
  }
  else if (luminance >= empty) {
    count = 0;
  }

  // if mail detected 3 times, send email
  if (count == 3) {
    // write to ESP8266 to send email
    digitalWrite(esp8266, HIGH);
    delay(1000);
    digitalWrite(esp8266, LOW);
  }

  delay(300000);

//  if(gsender->Subject(subject)->Send(email, msg)) {
//      Serial.println("Message send.");
//  } else {
//      Serial.print("Error sending message: ");
//      Serial.println(gsender->getError());
//  }

//  WiFiClient client = server.available();   // Listen for incoming clients
//
//  if (client) {                             // If a new client connects,
//    Serial.println("New Client.");          // print a message out in the serial port
//    String currentLine = "";                // make a String to hold incoming data from the client
//    while (client.connected()) {            // loop while the client's connected
//      if (client.available()) {             // if there's bytes to read from the client,
//        char c = client.read();             // read a byte, then
//        Serial.write(c);                    // print it out the serial monitor
//        header += c;
//        if (c == '\n') {                    // if the byte is a newline character
//          // if the current line is blank, you got two newline characters in a row.
//          // that's the end of the client HTTP request, so send a response:
//          if (currentLine.length() == 0) {
//            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
//            // and a content-type so the client knows what's coming, then a blank line:
//            client.println("HTTP/1.1 200 OK");
//            client.println("Content-type:text/html");
//            client.println("Connection: close");
//            client.println();
//            
//            // turns the GPIOs on and off
//            if (header.indexOf("GET /1/out") >= 0) {
//              Serial.println("LED1 off");
//              roomate1State = "out";
//              if(gsender->Subject(subject)->Send("amit.harlev@gmail.com", "Setup test")) {
//                  Serial.println("Message send.");
//              } else {
//                  Serial.print("Error sending message: ");
//                  Serial.println(gsender->getError());
//              }
//              digitalWrite(led1, HIGH);
//            } else if (header.indexOf("GET /1/in") >= 0) {
//              Serial.println("LED1 on");
//              roomate1State = "in";
//              digitalWrite(led1, LOW);
//            }
//            
//            // Display the HTML web page
//            client.println("<!DOCTYPE html><html>");
//            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
//            client.println("<link rel=\"icon\" href=\"data:,\">");
//            // CSS to style the on/off buttons 
//            // Feel free to change the background-color and font-size attributes to fit your preferences
//            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
//            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
//            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
//            client.println(".button2 {background-color: #77878A;}</style></head>");
//            
//            // Web Page Heading
//            client.println("<body><h1>ESP8266 Web Server</h1>");
//            
//            // Display current state, and ON/OFF buttons for GPIO 5  
//            client.println("<p> 'Your Name' is " + roomate1State + "</p>");
//            // If the output5State is off, it displays the ON button       
//            if (roomate1State =="out") {
//              client.println("<p><a href=\"/1/in\"><button class=\"button\">IN</button></a></p>");
//            } else {
//              client.println("<p><a href=\"/1/out\"><button class=\"button button2\">OUT</button></a></p>");
//            } 
//            client.println("</body></html>");
//            
//            // The HTTP response ends with another blank line
//            client.println();
//            // Break out of the while loop
//            break;
//          } else { // if you got a newline, then clear currentLine
//            currentLine = "";
//          }
//        } else if (c != '\r') {  // if you got anything else but a carriage return character,
//          currentLine += c;      // add it to the end of the currentLine
//        }
//      }
//    }
//    // Clear the header variable
//    header = "";
//    // Close the connection
//    client.stop();
//    Serial.println("Client disconnected.");
//    Serial.println("");
//  }
//
//  int buttonValue = digitalRead(button1);
//   if (buttonValue == LOW){
//      pressed = true;
//   }
//   if (buttonValue == HIGH && pressed){
//      pressed = false;
//      if (roomate1State == "in"){
//        digitalWrite(led1,LOW);
//        roomate1State = "out";
//      } else {
//        digitalWrite(led1,HIGH);
//        roomate1State = "in";
//      }
//   }
}
