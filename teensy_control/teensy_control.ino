#include <Arduino.h>
#include "i2c_driver.h"
#include "imx_rt1060_i2c_driver.h"
#include "PWM_valve.h"
#include "Valve.h"
// Blink the LED to make sure the Teensy hasn't hung
IntervalTimer blink_timer;
IntervalTimer pwm_timer;

volatile bool led_high = false;
void blink_isr();

const uint16_t slave_address = 0x002D;
I2CSlave& slave = Slave;
void after_receive(int size);

const int duty_unit = 200; //in micro second, suppose total period is 50Hz
const int num_pwm = 6;
const int num_val = 10;

// create two lists for pwm_valves, pwm_list is the original list, pwm_sort_list is after sorting
PWM_valve pwm_list[num_pwm] = {
  PWM_valve(2, duty_unit),
  PWM_valve(3, duty_unit),
  PWM_valve(4, duty_unit),
  PWM_valve(5, duty_unit),
  PWM_valve(6, duty_unit),
  PWM_valve(7, duty_unit)
};
PWM_valve pwm_sort_list[num_pwm];

// normal valve list
Valve valve_list[num_val] = {
    Valve(8),
    Valve(9),
    Valve(10),
    Valve(11),
    Valve(12),
    Valve(13),
    Valve(14),
    Valve(15),
    Valve(20),
    Valve(21)
    };

// Double receive buffers to hold data from master.
const size_t slave_rx_buffer_size = num_pwm;
uint8_t slave_rx_buffer[slave_rx_buffer_size] = {};
uint8_t slave_rx_buffer_2[slave_rx_buffer_size] = {};
volatile size_t slave_bytes_received = 0;

void log_message_received();


void setup() {
  // Turn the LED on
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, true);

  // Create a timer to blink the LED
  blink_timer.begin(blink_isr, 500000);

  //create pwm_valve list
  memcpy(pwm_sort_list, pwm_list, sizeof(PWM_valve) * num_pwm);
  pwm_timer.begin(PWM_valve_on_off, duty_unit * 100); //it is in micro second

  // Configure I2C Slave
  slave.after_receive(after_receive);
  slave.set_receive_buffer(slave_rx_buffer, slave_rx_buffer_size);

  // Enable the slave
  slave.listen(slave_address);

  // Enable the serial port for debugging
  Serial.begin(115200);
//  Serial.println("Started");



}

void loop() {
  if (slave_bytes_received) {
    // Handle the message.
    log_message_received();

    // Clear slave_bytes_received to signal that we're ready for another message
    slave_bytes_received = false;
    //decode the message
    decode_msg();
    
  }


  // We could receive multiple message while we're asleep.
  // This example is modelling an application where it's Ok
  // to drop messages.
  delayMicroseconds(50);
}

// Called by the I2C interrupt service routine.
// This method must be as fast as possible.
// Do not perform IO in it.
void after_receive(int size) {
  // This is the only time we can guarantee that the
  // receive buffer is not changing.
  // Copy the content so we can handle it in the main loop.
  if (!slave_bytes_received) {
    memcpy(slave_rx_buffer_2, slave_rx_buffer, size);
    slave_bytes_received = size;
  }
  // else ignore this message because the main loop hasn't
  // handled the previous one yet.


}

void log_message_received() {
  if (slave.has_error()) {
    if (slave.error() == I2CError::buffer_overflow) {
//      Serial.println("App: Buffer Overflow. (Master sent too many bytes.)");
    } else {
//      Serial.println("App: Unexpected error");
    }
  }
//  Serial.print("App: Slave received ");
//  Serial.print(slave_bytes_received);
//  Serial.print(" bytes: ");
  for(size_t i=0; i<slave_bytes_received; i++) {
//     Serial.print(slave_rx_buffer_2[i]);
//     Serial.print(", ");
  }
//  Serial.println();

}

void blink_isr() {
  led_high = !led_high;
  digitalWrite(LED_BUILTIN, led_high);
}
void decode_msg()
{
//  Serial.print("set duty:");
  for (int i = 0; i < num_pwm; i++) {
    pwm_list[i].setDuty((int)slave_rx_buffer_2[i]);
//    Serial.print((int)slave_rx_buffer_2[i]);
//    Serial.print(',');
  }
  for (int i = num_pwm; i < num_pwm + num_val;i++){
    if((bool)slave_rx_buffer_2[i]){
      valve_list[i].on();
    }
    else{
      valve_list[i].off();
    }
  }
//    Serial.print('\n');
  memcpy(pwm_sort_list, pwm_list, sizeof(PWM_valve) * num_pwm);
  heapSort(pwm_sort_list, num_pwm);
}
// reference from https://www.geeksforgeeks.org/heap-sort/
void heapify(PWM_valve arr[], int n, int i)
{
  int largest = i; // Initialize largest as root
  int l = 2 * i + 1; // left = 2*i + 1
  int r = 2 * i + 2; // right = 2*i + 2

  // If left child is larger than root
  if (l < n && arr[l] > arr[largest])
    largest = l;

  // If right child is larger than largest so far
  if (r < n && arr[r] > arr[largest])
    largest = r;

  // If largest is not root
  if (largest != i)
  {
    std::swap(arr[i], arr[largest]);

    // Recursively heapify the affected sub-tree
    heapify(arr, n, largest);
  }
}
void heapSort(PWM_valve arr[], int n)
{
  // Build heap (rearrange array)
  for (int i = n / 2 - 1; i >= 0; i--)
    heapify(arr, n, i);

  // One by one extract an element from heap
  for (int i = n - 1; i > 0; i--)
  {
    // Move current root to end
    std::swap(arr[0], arr[i]);

    // call max heapify on the reduced heap
    heapify(arr, i, 0);
  }
}
void PWM_valve_on_off()
{
  
  for (int i = 0; i < num_pwm; i++)
  {
    pwm_sort_list[i].on();
  }
  int pre_duty = 0;
  for (int i = 0; i < num_pwm; i++)
  {
    int cur_sleep_t = pwm_sort_list[i].cal_off_t(pre_duty);
    delayMicroseconds(duty_unit*cur_sleep_t);
    pwm_sort_list[i].off();
    Serial.print("val");
    Serial.print(pwm_sort_list[i].pin_id-2);
    Serial.print(':');
    Serial.print(cur_sleep_t*duty_unit);
    Serial.print('\t');
  }
  Serial.print('\n');
  
}
