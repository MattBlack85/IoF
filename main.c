#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <wiringPi.h>

#define GPIO23     4
#define GPIO24     5

static int trigger = GPIO23;
static int echo = GPIO24;
struct timespec startTimeUsec;
struct timespec endTimeUsec;


void recordPulseLength (void) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startTimeUsec);
    while ( digitalRead(echo) == HIGH );
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endTimeUsec);;
}

float get_distance() {
  double sound_speed = 340.29;

  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);

  // Initialize the sensor's trigger pin to low. If we don't pause
  // after setting it to low, sometimes the sensor doesn't work right.
  digitalWrite(trigger, LOW);
  delay(500); // .5 seconds

  // Triggering the sensor for 10 microseconds will cause it to send out
  // 8 ultrasonic (40Khz) bursts and listen for the echos.
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);

  // The sensor will raise the echo pin high for the length of time that it took
  // the ultrasonic bursts to travel round trip.
  while ( digitalRead(echo) == LOW);
  recordPulseLength();

  long travelTimeUsec = endTimeUsec.tv_nsec - startTimeUsec.tv_nsec;
  double distanceMeters = ((travelTimeUsec/1000000000.0)*sound_speed)/2;

  return distanceMeters * 100;
}


int main()
{
  int x;
  float measure, sum, read_distance_array [10];


  // Initialise the GPIO
  wiringPiSetup();
  sleep(2);

  while(1) {
    // Wait 30 sec between one measurement and the other
    sleep(30);
    // Take 10 measures and find the average
    for (x = 0; x < 10; x++) {
      sum = 0;
      sleep(0.5);
      measure = get_distance();
      // Push the read to the array
      read_distance_array[x] = measure;
    }
    for (x = 0; x < 10; x++) {
      sum = sum + read_distance_array[x];
    }

    printf("Average distance: %.2f\n", sum / 10);
  }

  return 0;
}
