/********************************************************************/
// First we include the libraries
#include <OneWire.h>
#include <DallasTemperature.h>
#include "rgb_lcd.h"

#include <CurieTime.h>

/********************************************************************/
// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 3

#define WATER_TEMP_SENSOR 0
#define ENV_TEMP_SENSOR 1

#define PIN_LED_ALERT     5
#define PIN_WATER_SWITCH  2
#define PIN_BUZZER        4
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
/********************************************************************/

rgb_lcd lcd;

// Only print when needed
int stepCounter = 0;

int lastWaterSwitch = 1;
int waterSwitch = 0;  // 0 when water is flowing
int waterTimerStart = -1;
int waterTimerEnd = -1; // When the shower has ended
int lastShowerDuration = -1;
int secsElapsed = -1;
int minsElapsed = -1;
int showerEndGracePeriod = 60; // Duration before shower is considered concluded

// Alerts
int alertIntervalLength = 20;
int lastAlertInterval = 0;

void setup(void)
{
  // start serial port
  // Serial.begin(9600);
  // Start up the library
  sensors.begin();

  pinMode(PIN_WATER_SWITCH, INPUT_PULLUP);
  pinMode(PIN_LED_ALERT, OUTPUT);


  lcd.begin(16, 2);
  lcd.setRGB(255, 255, 255);
  lcd.display();
  lcd.clear();
}

void loop(void)
{
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  /********************************************************************/
  sensors.requestTemperatures(); // Send the command to get temperature readings
  /********************************************************************/

  // read water switch status
  waterSwitch = digitalRead(PIN_WATER_SWITCH);
  // If switch changed, reset display
  if (waterSwitch != lastWaterSwitch)
  {
    lcd.setRGB(255, 255, 255);
    lcd.clear();
    lastWaterSwitch = waterSwitch;

    // Check if waterflow has started and start timer
    if (waterSwitch == 0)
    {
      // Check if last shower was concluded
      if (waterTimerEnd > 0 && lastShowerDuration > 0)
      {
        waterTimerStart = now() - lastShowerDuration;
      } else
      {
        waterTimerStart = now();
      }
    } else
    {
      // Store duration
      lastShowerDuration = now() - waterTimerStart;
      // Reset waterflow timer
      waterTimerStart = -1;
      waterTimerEnd = now();
    }
  }

  // Waterflow: Off
  // Show welcome message
  lcd.setCursor(0, 0);
  if (waterSwitch > 0)
  {
    // Show summary
    if (waterTimerEnd > 0)
    {
      // Only show for 60secs
      int graceSecsLeft = showerEndGracePeriod - (now() - waterTimerEnd);
      if ((now() - waterTimerEnd) < showerEndGracePeriod)
      {
        minsElapsed = lastShowerDuration / 60;
        lcd.print("Length: ");
        lcd.setCursor(9, 0);
        lcd.print("m");
        lcd.setCursor(10, 0);
        lcd.print(minsElapsed);
        lcd.setCursor(13, 0);
        lcd.print("s");
        lcd.setCursor(14, 0);
        lcd.print(lastShowerDuration - (60 * minsElapsed));

        lcd.setCursor(1, 1);
        lcd.print("Finished?");
        lcd.setCursor(11, 1);
        if (graceSecsLeft < 10)
        {
          lcd.print("0");
          lcd.setCursor(12, 1);
          lcd.print(graceSecsLeft);
        }
        else
        {
          lcd.print(graceSecsLeft);
        }

      } else
      {
        waterTimerEnd = -1;
        lcd.clear();
      }
    } else
    {
      lcd.print("Hi! ");
      lcd.setCursor(10, 0);
      lcd.print(hour());
      lcd.setCursor(12, 0);
      lcd.print(":");
      lcd.setCursor(13, 0);
      if (minute() < 10)
      {
        lcd.print("0");
        lcd.setCursor(14, 0);
        lcd.print(minute());
      } else
      {
        lcd.print(minute());
      }
      // Write room temperature
      lcd.setCursor(1, 1);
      lcd.print("Bathroom:");
      lcd.setCursor(11, 1);
      lcd.print(sensors.getTempCByIndex(ENV_TEMP_SENSOR));
    }
  }
  else
  { // Waterflow: On
    // Start shower timer
    lcd.print("Enjoy! C: ");
    lcd.setCursor(11, 0);
    lcd.print(sensors.getTempCByIndex(WATER_TEMP_SENSOR));
    // Show timer
    int secsElapsed = now() - waterTimerStart;
    int minsElapsed = secsElapsed / 60;

    lcd.setCursor(1, 1);
    lcd.print("Timer:");
    lcd.setCursor(9, 1);
    lcd.print("m");
    lcd.setCursor(10, 1);
    lcd.print(minsElapsed);
    lcd.setCursor(13, 1);
    lcd.print("s");
    lcd.setCursor(14, 1);
    lcd.print(secsElapsed - (60 * minsElapsed));

    // Alert every interval as a reminder, Buzz & Light
    Serial.println(secsElapsed);
    if ((secsElapsed - lastAlertInterval) > alertIntervalLength)
    {
      Serial.println("Alert");
      lastAlertInterval = secsElapsed;

      tone(PIN_BUZZER, 1000); // Send 1KHz sound signal...
      digitalWrite(PIN_LED_ALERT, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(300);                       // wait for a second
      noTone(PIN_BUZZER);     // Stop sound...
      digitalWrite(PIN_LED_ALERT, LOW);    // turn the LED off by making the voltage LOW
    }

    // Warn on display for length of shower
    if (secsElapsed > 20 && secsElapsed < 40)
    {
      lcd.setRGB(255, 165, 0);
    } else if (secsElapsed > 40)
    {
      lcd.setRGB(255, 0, 0);
    }
    else
    {
      lcd.setRGB(255, 255, 255);
    }
  }


  if (stepCounter > 3)
  {
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
