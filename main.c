#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <wiringPi.h>

#define GPIO23      4;
#define GPIO24      5;
#define GPIO20      28;
static int TRIGGER_PIN    = GPIO23;
static int ECHO_PIN       = GPIO24;
static int PUMP_PIN       = GPIO20;
static double SOUND_SPEED = 340.29;
static int MAX_READS      = 30;
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

void refill_water()
{
  /* Turn on the pump and refill for 3 sec */
  digitalWrite(PUMP_PIN, LOW);
  sleep(3);
  digitalWrite(PUMP_PIN, HIGH);
}

int setup_pump()
{
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, HIGH);

return 0;
}


int main()
{
  int x;
  double average_distance, measure, sum, read_distance_array [30];

  wiringPiSetup();
  setup_pump();
  sleep(2);

  while(1) {
    // Wait 30 sec between one measurement and the other
    sleep(30);
    // Take 30 measures and find the average
    for (x = 0; x < MAX_READS; x++) {
      sum = 0;
      usleep(100000);
      measure = get_distance();
      // Push the read to the array
      read_distance_array[x] = measure;
    }
    for (x = 0; x < MAX_READS; x++) {
      sum = sum + read_distance_array[x];
    }
    average_distance = sum / 30;
    printf("Average distance: %.2f\n", average_distance);
    if (average_distance > 4.00) {
      printf("Refilling the tank\n");
      refill_water();
    }
  }

  return 0;
}
