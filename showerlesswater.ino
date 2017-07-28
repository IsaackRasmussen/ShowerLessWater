/********************************************************************/
// First we include the libraries
#include <OneWire.h>
#include <DallasTemperature.h>
#include "rgb_lcd.h"

/********************************************************************/
// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 3

#define
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
/********************************************************************/

rgb_lcd lcd;

int waterSwitch = 0;  // 0 when water is flowing
int waterPin = 2;

void setup(void)
{
  // start serial port
  Serial.begin(9600);
  // Start up the library
  sensors.begin();

  pinMode(waterPin, INPUT_PULLUP);

  lcd.begin(16, 2);
  // lcd.setRGB(255,0,0);
  lcd.display();
  lcd.clear();
}

int stepCounter = 0;
void loop(void)
{
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  /********************************************************************/
  sensors.requestTemperatures(); // Send the command to get temperature readings
  /********************************************************************/

  // read water switch status
  waterSwitch = digitalRead(waterPin);

  lcd.setCursor(0, 0);
  lcd.print("Cel: ");
  lcd.setCursor(5, 0);
  lcd.print(sensors.getTempCByIndex(0));
  lcd.setCursor(11, 0);
  lcd.print(sensors.getTempCByIndex(1));

  lcd.setCursor(0, 1);
  if (waterSwitch > 0)
  {
    lcd.print("Water is off");
  } else
  {
    lcd.print("Water is on ");
  }

  if (stepCounter > 3)
  {
    if (waterSwitch == 1)
    { // Waterflow: Off
      // Show welcome message

    } else
    { // Waterflow: On
      // Start shower timer
    }

    Serial.print("Temperature is: ");
    Serial.println(sensors.getTempCByIndex(0)); // Why "byIndex"?
    // You can have more than one DS18B20 on the same bus.
    // 0 refers to the first IC on the wire
    Serial.print("Water flow: ");
    Serial.println(waterSwitch);

    stepCounter = 0;
  }

  stepCounter++;
  delay(200);
}
