#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wiringPi.h>
#include "pump.h"
#include "mail.h"

#define GPIO23      4;
#define GPIO24      5;
static int TRIGGER_PIN           = GPIO23;
static int ECHO_PIN              = GPIO24;
static double SOUND_SPEED        = 340.29;
static double WATER_LEVEL_CRITIC = 0.20;
static int MAX_READS             = 30;
static int CALIBRATION_PROBES    = 100;
static double ALERT_LEVEL;
static int MEASURE_INTERVAL      = 100000; // 100ms
struct timespec start_time;
struct timespec end_time;



void record_pulse_length (void) {
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
  while ( digitalRead(ECHO_PIN) == HIGH );
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);;
}

double get_distance() {
  double travel_time = 0.0;
  double distance    = 0.0;

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  /* Initialize the sensor's trigger pin to low. If we don't pause
     after setting it to low, sometimes the sensor doesn't work right. */
  digitalWrite(TRIGGER_PIN, LOW);
  delay(500); // .5 seconds

  /* Triggering the sensor for 10 microseconds will cause it to send out
     8 ultrasonic (40Khz) bursts and listen for the echos. */
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  /* The sensor will raise the echo pin high for the length of time that it took
     the ultrasonic bursts to travel round trip.*/
  while ( digitalRead(ECHO_PIN) == LOW);
  record_pulse_length();

  travel_time = end_time.tv_nsec - start_time.tv_nsec;
  distance = ((travel_time/1000000000.0) * SOUND_SPEED)/2;

  return distance * 100;
}

void calibrate_distance(void) {
  int x;
  double result, sum;
  sum = 0;

  for (x = 0; x < CALIBRATION_PROBES; x++) {
    result = get_distance();
    if (result < 0) {
      /* sometimes we read negatives values, ignore those */
      CALIBRATION_PROBES = CALIBRATION_PROBES - 1;
      continue;
    }
    sum = sum + result;
    usleep(250000);
  }
  ALERT_LEVEL = (sum / CALIBRATION_PROBES) + WATER_LEVEL_CRITIC;
}


int main()
{
  int x, alarm_counter;
  double average_distance, measure, sum, read_distance_array [30];

  printf("Setting up the GPIO...\n");
  wiringPiSetup();
  printf("GPIO setup!\n");
  printf("Setting up the pump...\n");
  setup_pump();
  printf("Pump setup!\n");
  sleep(2);

  printf("Calibrating system...\n");
  calibrate_distance();
  printf("Calibration over, alert level set to %.2fcm\n", ALERT_LEVEL);
  // Init the alarm counter
  alarm_counter = 0;
  while(1) {
    // Init the array
    memset(read_distance_array, 0, sizeof read_distance_array);
    // Wait 30 sec between one measurement and the other
    sleep(20);
    // Take 30 measures and find the average
    for (x = 0; x < MAX_READS; x++) {
      sum = 0;
      usleep(MEASURE_INTERVAL);
      measure = get_distance();
      // Push the read to the array
      read_distance_array[x] = measure;
    }
    for (x = 0; x < MAX_READS; x++) {
      sum = sum + read_distance_array[x];
    }
    average_distance = sum / MAX_READS;
    printf("Average distance: %.2fcm\n", average_distance);
    if (average_distance > ALERT_LEVEL) {
      alarm_counter = alarm_counter + 1;
    } else {
      alarm_counter = 0;
    }

    if (alarm_counter == 3) {
      printf("Refilling the tank\n");
      refill_water();
      send_mail("Tank refilled");
      alarm_counter = 0;
    }
  }

  return 0;
}
