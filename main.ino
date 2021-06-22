#include <DallasTemperature.h>
#include <OneWire.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>

const char* FIREBASE_HOST = “database_linkxxxxx";            //database link
const char* AUTH = “your_authentication_keyxxxx";     	 //database authentication key

const char* ssid = “wifi_ssid";     			 //wifi name
const char* password = “wifi_password";  		 //wifi password
char* state;

const char* IFTTT_HOST = "maker.ifttt.com";   		 //ifttt website
const char* event = "Water_quality_breach_call";    	 //ifttt applet name
const char* iftttkey = “ifttt_keyxxx";        		 //ifttt key


#define temperature  2           		           //DS18B20 on arduino pin2 corresponds to D4 on physical board
#define turb 5
#define buzzer 0
#define ph_sensor A0
float volt, ntu, tempp, value, ph, avgValue;       
          
OneWire oneWire(temperature);
DallasTemperature sensors(&oneWire);                        //passing oneWire reference to Dallas Temperature.

void setup() {
  
  Serial.begin(115200);    		                    //setting the baud rate
  sensors.begin();
  
  pinMode(A0,INPUT);    		                    //declaring the sensors as input
  pinMode(5,INPUT);       
  pinMode(0,OUTPUT);    		                    //buzzer as output
  digitalWrite(0,LOW);
  
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);   		          //connecting to the wifi network
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Firebase.begin(FIREBASE_HOST,AUTH);  		          //starting the database
  Serial.println("firebase started");
}
void loop(){
  
  sensors.requestTemperatures();                            // Sending the command to get temperatures  
  tempp=sensors.getTempCByIndex(0);   		          // getting temperature in Celsius 
          

  // Calculating turbidity
  for(int i=0;i<1000;i++)
  {
    volt += ((float)digitalRead(turb));
  }
  volt /= 1000;

  if(volt<2.5)
    ntu = 3000;  
  else
    ntu = -1120.4*volt*volt+5742.3*volt-4352.9;
  


// Calculating the pH values: 
for(int i=0;i<10;i++)                               //Taking 10 samples from the sensor 
  { 
    buf[i] = analogRead(ph_sensor); 
    delay(10); 
  } 
  for(int i=0;i<9;i++)                              //sorting the array in ascending order 
  { 
    for(int j=i+1;j<10;j++) 
    { 
     if(buf[i] > buf[j]) 
      { 
        temp = buf[i]; 
        buf[i] = buf[j]; 
        buf[j] = temp; 
      } 
    }  
} 
  avgValue = 0; 
  for(int i=2;i<8;i++)                                                //taking the average value of 6 centre samples
  avgValue += buf[i]; 
  float ph = ((float)avgValue)/1023)*5;                               //converting to volt 
  ph = 3.5*ph;                                                        //convert the volt into pH value 
  delay(800); 
  
Firebase.setFloat("temp",tempp);    	                              // sending the temp values to database
  if (Firebase.failed()) {      	                              //handling the error
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
  }
  Firebase.setFloat("turb",ntu);    	                              // sending the temp values to database
  if (Firebase.failed()) {      	                              //handling the error
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
  }
  Firebase.setFloat("ph",ph);   	                              // sending the temp values to database
  if (Firebase.failed()) {      	                              //handling the error
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
}

  delay(1000);
  WiFiClient client;

  Serial.println("Connecting to IFTTT... ");  
  if (client.connect(IFTTT_HOST, 80))                                 //connecting to IFTTT 
    Serial.println("connected");  
  else
    Serial.println("Connetion to IFTTT failed.");
  
 //checking all parameters
if(ntu<=1 && tempp>=15 && tempp<=35 && ph>=6.5 && ph<=8.5)
  {
    digitalWrite(0,LOW);   
    state="GOOD! CAN BE CONSUMED"; 
  }
  else{
    digitalWrite(0,HIGH);   
    state="BAD! CANNOT BE CONSUMED";
    Serial.println("Calling the user");
  
    //sending a voice message
  String toSend = "GET /trigger/";
    toSend += event;
    toSend += "/with/key/";
    toSend += iftttkey;
    toSend += "?value1=";
    toSend += ntu;
    toSend += "&value2=";
    toSend += tempp;
    toSend += "&value3=";
    toSend += ph;
    toSend += " HTTP/1.1\r\n";
    toSend += "Host: ";
    toSend += IFTTT_HOST;
    toSend += "\r\n";
    toSend += "Connection: close\r\n\r\n";
    client.print(toSend); 
    delay(5000);
    client.flush();
    client.stop();  
    delay(500);
   }
}
