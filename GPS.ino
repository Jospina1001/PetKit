#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include <WiFi.h>
#include <WifiLocation.h>
#include <HTTPClient.h>

#define SCREEN_WIDTH 128 // OLED width,  in pixels
#define SCREEN_HEIGHT 64 // OLED height, in pixels
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

WiFiClient client; 

const char* googleApiKey = "AIzaSyDc5hVHFemfD5Du3HgD5qfS79jD3HF9Ppg";
const char* ssid = "Redmi";
const char* passwd = "68031001";

WifiLocation location (googleApiKey);
//WiFiMulti wifiMulti;

const uint32_t TiempoEsperaWifi = 5000;
//Colocamos el servidor:
WiFiServer servidor(80);

//Variables Locales

float latitud = 0;
float longitud = 0;
float temperatura = 28;
float humedad = 52;
float altitud = 2480;

//-------------------Para la parte del TIMER------------------------

//Declaramos variable de contador:
volatile int interruptCounter;

//Colocamos un contador para registrar las interrupciones:
int totalInterruptCounter;

//Apuntamos una variable hw_timer_t: 
hw_timer_t * timer = NULL;
//Con una variable porMUX_TYPE sincronizamos el bucle principal y la ISR.
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;


void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
 
}


void Inicio(){
  oled.clearDisplay(); // clear display
  oled.setTextSize(2);         // set text size
  oled.setTextColor(WHITE);    // set text color

  oled.setCursor(10, 20);       // set position to display
  oled.println("Iniciando"); // set text 
  oled.display();

  delay(1000);
  
}

void Bienvenida(){
  oled.clearDisplay(); // clear display
  oled.setTextSize(2);         // set text size
  oled.setTextColor(WHITE);    // set text color

  oled.setCursor(10, 20);       // set position to display
  oled.println("PETKIT v1"); // set text
  oled.display();
  
}

void setClock () {
    configTime (0, 0, "pool.ntp.org", "time.nist.gov");

    Serial.print ("Waiting for NTP time sync: ");
    time_t now = time (nullptr);
    while (now < 8 * 3600 * 2) {
        delay (500);
        Serial.print (".");
        now = time (nullptr);
    }
    struct tm timeinfo;
    gmtime_r (&now, &timeinfo);
    Serial.print ("\n");
    Serial.print ("Current time: ");
    Serial.print (asctime (&timeinfo));


    oled.setTextSize(1);         // set text size
    oled.setTextColor(WHITE);    // set text color

    oled.setCursor(0, 40);       // set position to display
    oled.println(asctime (&timeinfo)); // set text
    oled.display();

    Serial.println(WiFi.localIP());
    servidor.begin(); // iniciamos el servidor
}

void envioDatos(){

  if(WiFi.status()==WL_CONNECTED){
    HTTPClient http;
    //Colocamos en una cadena los datos que vamos a enviar:
    String datos_a_enviar = "longitud=" + String(longitud) + "&latitud=" + String(latitud) + "&temperatura=" + String(temperatura) +
    "&humedad=" + String(humedad) + "&altitud=" + String(altitud);

    //Colocamos comentario para verificar que se envíe el mensaje:
    Serial.println("Los datos a enviar son:" + datos_a_enviar+"/n");

    //Colocamos la dirección donde ejecutaremos el POST:
    http.begin("https://petkitsmart.000webhostapp.com/ESP32Post.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int codigo_respuesta = http.POST(datos_a_enviar);

    if(codigo_respuesta > 0){

      Serial.println("Código HTTP: " + String(codigo_respuesta));
      if(codigo_respuesta == 200){
        String cuerpo_respuesta = http.getString();
        Serial.println("El servidor respondió: ");
        Serial.println(cuerpo_respuesta);
      }
    }else{
      Serial.print("Error enviado POST, código: ");
      Serial.println(codigo_respuesta);
    }
    http.end();
  }else{
    Serial.println("Error de la conexión WIFI");
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("failed to start SSD1306 OLED"));
    while (1);
  }
  Inicio();
  

  WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, passwd);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(ssid);
        // wait 5 seconds for connection:
        Serial.print("Status = ");
        Serial.println(WiFi.status());
        delay(500);
   }
   Serial.println ("Connected");
   Bienvenida();
   setClock ();

   
}

void loop() {
  // put your main code here, to run repeatedly:
  location_t loc = location.getGeoFromWiFi();
  Serial.println("Location request data");
  Serial.println(location.getSurroundingWiFiJson()+"\n");

  longitud=loc.lon;
  latitud=loc.lat;
   
  Serial.println ("Location: " + String (latitud, 7) + "," + String (longitud, 7));
   //Serial.println("Longitude: " + String(loc.lon, 7));
  Serial.println ("Accuracy: " + String (loc.accuracy));
  Serial.println ("Result: " + location.wlStatusStr (location.getStatus ()));
   
  oled.clearDisplay(); // clear display
  oled.setTextSize(1);         // set text size
  oled.setTextColor(WHITE);    // set text color
    
  oled.setCursor(0, 0);       // set position to display
  oled.println("PETKIT v1"); // set text 
  
  oled.setCursor(0, 10);       // set position to display
  oled.println("Location: " + String (loc.lat, 7) + "," + String (loc.lon, 7)); 
  
  oled.setCursor(0, 35);       // set position to display
  oled.println("Accuracy: " + String (loc.accuracy));
  
  oled.setCursor(0, 50);       // set position to display
  oled.println("Result: " + location.wlStatusStr (location.getStatus ())); // set text 
  oled.display();

  envioDatos();

}
