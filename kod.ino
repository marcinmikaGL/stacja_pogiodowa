#include <SuplaDevice.h>

// Add include to DS sensor
#include <supla/sensor/DS18B20.h>
#include <supla/sensor/wind.h>
#include <supla/sensor/rain.h>

  uint8_t mac[6] = {0x00, 0x01, 0x02, 0x0C, 0x05, 0x05};

  #include <supla/network/esp_wifi.h>
  Supla::ESPWifi wifi("szklarnia", "aaAA123456!!");


float diameter = 2.75;
float mph; //Utworzenie zmiennej mile/godzinę
float kmh; //Utowrzenie zmiennej km/h

//Kierunek wiatru
int winddir = 0; //Utworzenie zmiennej wnddir (kierunek wiatru)
const byte WDIR = A0; // Przypisanie portu A0 jako portu odczytującego kierunek wiatru (jako stała)


// Odczyt obrotów (RPM)
volatile int half_revolutions = 0; //Utworzenie zmiennej połowa pełnego obrotu (half revolutions)
int rpm = 0; //Utworzenie zmiennej RPM (obroty)
unsigned long lastmillis = 0; //Utworzenie zmiennej long lastmilis

void rpm_fan() { //funkcja rpm_fan
  unsigned long static last_event = 0;
  if (millis() - last_event < 5) {   //debouncing
    return;
  }
  half_revolutions++; //zwiększ wartość zmiennej half_revolutions
}

int get_wind_direction() //Funkcja odczytująca kierunek wiatru
{
  unsigned int adc; //Utworzenie zmiennej adc

  adc = analogRead(WDIR); // Odczyt aktualnej wartości ADC

  //Jeżeli wartośc będzie się zawierała w podanym zakresie, zostanie zwrócony kierunek wiatru
  if ((adc > 760) && (adc < 799)) return (0);
  if ((adc > 450) && (adc < 470)) return (45);
  if ((adc > 85) && (adc < 100)) return (90);
  if ((adc > 170) && (adc < 199)) return (135);
  if ((adc > 270) && (adc < 299)) return (180);
  if ((adc > 610) && (adc < 640)) return (225);
  if ((adc > 930) && (adc < 960)) return (270);
  if ((adc > 870) && (adc < 899)) return (315);
  return (-1); //Jeśli jest jakiś błąd zostanie zwrócona wartość -1
}

void setup() {

  Serial.begin(115200);
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), rpm_fan, FALLING);

  // Replace the falowing GUID with value that you can retrieve from https://www.supla.org/arduino/get-guid
  char GUID[SUPLA_GUID_SIZE] = {0xAE,0xDD,0x6E,0xE4,0x39,0x73,0x7E,0x8F,0xEC,0x91,0xC5,0x9E,0x32,0xCE,0x6D,0xA2};

  // Replace the following AUTHKEY with value that you can retrieve from: https://www.supla.org/arduino/get-authkey
  char AUTHKEY[SUPLA_AUTHKEY_SIZE] = {0xAB,0xF9,0x9A,0xE7,0x0E,0xBC,0x60,0xE0,0x89,0xA0,0x39,0x2F,0x1A,0x24,0xEE,0x7E};


   if (millis() - lastmillis >= 1000) {
    //Aktualizuj co sekundę, będzie to równoznaczne z odczytem częstotliwości (Hz)

    lastmillis = millis();          // Aktualizacja lastmillis

    noInterrupts();                   // W trakcie kalkulacji wyłącz obsługę przerwań
    rpm = half_revolutions * 30;
    half_revolutions = 0;           // Restart licznika obrotów
    interrupts() ; //Przywróć przerwania

    mph = diameter / 12 * 3.14 * rpm * 60 / 5280;//Odczyt prędkości wiatru w milach/godzinę
    mph = mph * 3.5; // Kalibracja błędu odczytu, wartość należy dobrać we własnym zakresie
    kmh = mph * 1.609;// Zamiana mil/godzinę na km/h
    winddir = get_wind_direction(); //Odczyt kierunku wiatru

    Serial.print("\t KMH=\t"); //Przesłanie odczytanych danych do portu szeregowego
    Serial.println(kmh);
    Serial.println();
    Serial.print("winddir=\t");
    Serial.println(winddir);

  }


  DeviceAddress ds1addr = {0x28, 0xB0, 0x04, 0x04, 0x00, 0x00, 0x00, 0x08};
  DeviceAddress ds2addr = {0x28, 0xE1, 0x06, 0x04, 0x00, 0x00, 0x00, 0x92};
  DeviceAddress ds3addr = {0x28, 0x8C, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x24};
  DeviceAddress ds4addr = {0x28, 0x6D, 0x1C, 0x04, 0x00, 0x00, 0x00, 0xC4};  
  
  
  new Supla::Sensor::Wind();
  new Supla::Sensor::DS18B20(26, ds2addr);
  new Supla::Sensor::DS18B20(26, ds3addr);
  new Supla::Sensor::DS18B20(26, ds4addr);

  SuplaDevice.begin(GUID,              // Global Unique Identifier 
                    "svr79.supla.org",  // SUPLA server address
                    "marcinmika@o2.pl", 
                    AUTHKEY
);
    
}

void loop() {
  SuplaDevice.iterate();
}
