#include <time.h>
#include <wiringPi.h>
#include "pump.h"

static int PUMP_PIN = GPIO20;


void setup_pump(void)
{
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, HIGH);
}

void refill_water(void)
{
  /* Turn on the pump and refill for 5 sec */
  digitalWrite(PUMP_PIN, LOW);
  sleep(5);
  digitalWrite(PUMP_PIN, HIGH);
}
