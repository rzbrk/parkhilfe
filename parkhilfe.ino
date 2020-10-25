// Define the I/O ports the HC-SR04 ultrasonic distance sensor is
// connected to
#define TRIGPIN 9
#define ECHOPIN 10

// Define the I/O pins the traffic ligth and the signal are connected to
#define REDPIN 5
#define YELLOWPIN 4
#define GREENPIN 3
#define SIGPIN 2

// Define temperature to calculate the speed from sound
#define TEMP 20 // degrees celcius

// The ultrasonic sensor measurements are averaged using an array. Here,
// the length of this array and the default value when it is initialized
// can be defined
#define ARR_LEN 30
#define DUR_DEFAULT 20000.0 // microseconds

// Define the final delay in the infinite loop
#define DELAY 20 // milliseconds

// Define the signal length when traffic light turns red
#define STOPSIG 3000 // milliseconds

// Define the tresholds for changing the traffic light
// From green to yellow
#define TRSHLD_YELLOW 100 // cm
// From yellow to red
#define TRSHLD_RED 10 // cm

// Enable/disable debugging output over serial console (9600 baud)
#define DEBUG_ENABLE true

///////////////////////////////////////////////////////////////////////////////

// Define some variables
float cs, duration, duration2, interval, distance;
float dur_array[ARR_LEN];
String color;
bool signal = false;
unsigned long start_time;

void setup() {
  if (DEBUG_ENABLE) {
    // Activate serial port for debugging
    Serial.begin(9600);
  }

  // Setting up I/O ports for ultrasonic sensor
  pinMode(TRIGPIN, OUTPUT);
  pinMode(ECHOPIN, INPUT);

  // Setting up I/O ports for traffic light and set it to green
  pinMode(REDPIN, OUTPUT);
  pinMode(YELLOWPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(SIGPIN, OUTPUT);
  digitalWrite(REDPIN, LOW);
  digitalWrite(YELLOWPIN, LOW);
  digitalWrite(GREENPIN, HIGH);
  color = "green";

  // Write starting values to array dur_array
  for (int i = 0; i < ARR_LEN; i++) {
    dur_array[i] = DUR_DEFAULT;
  }

  start_time = micros();

  // Calculate the sound of speed to calculate the distance between sensor
  // and obstacle from the travel time of the sound waves later in the code.
  // The speed of sound cs in dry air is depending on temperature TEMP:
  cs = ( 0.03313 + 0.0000606 * TEMP ); // cm/µs
}

// This function shifts all elements in the array dur_array to the right,
// dropping the far-right element and inserting value d to the first,
// left-most position in the array
void arr_add_elem(float d) {
  for (int i = ARR_LEN - 2; i >= 0; i--) {
    dur_array[i+1] = dur_array[i];
  }
  dur_array[0] = d;
}

// This function returns the average of all values in array dur_array
float arr_average() {
  float a = 0;
  for (int i = 0; i < ARR_LEN; i++) {
    a = a + dur_array[i];
  }
  a = a / ARR_LEN;
  return a;
}

// Output the current value of boolean variable signal to I/O pin SIGPIN
void beep() {
  digitalWrite(SIGPIN, signal);
}

// Depending on the value of global variable color set the lights of the
// traffic light
void tl_setlights() {
  if (DEBUG_ENABLE) {
    Serial.print("Switching traffic light to ");
    Serial.println(color);
  }
  if (color == "red") {
    digitalWrite(REDPIN, HIGH);
    digitalWrite(YELLOWPIN, LOW);
    digitalWrite(GREENPIN, LOW);
  }
  if (color == "yellow") {
    digitalWrite(REDPIN, LOW);
    digitalWrite(YELLOWPIN, HIGH);
    digitalWrite(GREENPIN, LOW);
  }
  if (color == "green") {
    digitalWrite(REDPIN, LOW);
    digitalWrite(YELLOWPIN, LOW);
    digitalWrite(GREENPIN, HIGH);
  }
}

///////////////////////////////////////////////////////////////////////////////

// Main loop
void loop() {
  // Trigger ultrasonic sensor and perform measurement
  digitalWrite(TRIGPIN, LOW); 
  delayMicroseconds(2); 
  digitalWrite(TRIGPIN, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(TRIGPIN, LOW);

  // Travel durations of sound waves are in microseconds. First, add the
  // latest measurement to the array dur_array
  arr_add_elem(pulseIn(ECHOPIN, HIGH));
  // Retrieve the average of all values in array
  duration = arr_average();

  // Calculate the one-way distance from sensor to obstacle
  distance = (duration * cs) / 2.0;

  // Following comes the switching logic for the traffic light depending on
  // the measured distance
  //
  //     red             yellow                    green
  // +--------+---------------------------+-------------------------> distance
  // |        |                           |
  // 0        TRSHLD_YELLOW               TRSHLD_RED
  //
  if ((distance > TRSHLD_YELLOW) && (color != "green")) {
    color = "green";
    tl_setlights();
  }
  if ((distance < TRSHLD_YELLOW) && (distance >= TRSHLD_RED)
    && (color != "yellow")) {
    color = "yellow";
    tl_setlights();
 }
  if ((distance < TRSHLD_RED) && (color != "red")) {
    color = "red";
    tl_setlights();
    // Produce signal for 3 seconds
    signal = true;
    beep();
    delay(STOPSIG);
    signal = false;
    beep();
  }

  // When the traffic lights shows yellow produce a pulsed signal. The pulse
  // length (interval) varies with the current duration/distance
  if (color == "yellow") {
    unsigned long current_time = micros();
    // The interval varies between 10 milliseconds (distance = TRSHLD_RED) and
    // 250 milliseconds (distance = TRSHLD_YELLOW)
    interval = 10000 + 240000
      * (( distance ) / ( TRSHLD_YELLOW - TRSHLD_RED ));
    if (current_time - start_time > interval) {
      start_time = micros();
      signal = ! signal;
    }
  } else {
    signal = false;
  }

  beep();

  // Debug output
  if (DEBUG_ENABLE) {
    if (color == "red") {
      Serial.print("R");
    } else {
      Serial.print("r");
    }
    if (color == "yellow") {
      Serial.print("Y");
    } else {
      Serial.print("y");
    }
    if (color == "green") {
      Serial.print("G");
    } else {
      Serial.print("g");
    }
    Serial.print(" ");
    if (signal) {
      Serial.print("S");
    } else {
      Serial.print("s");
    }
    Serial.print(", ");
    Serial.print(duration);
    Serial.print(" µs, ");
    Serial.print(distance);
    Serial.println(" cm, ");
  }

  delay(DELAY);
}
