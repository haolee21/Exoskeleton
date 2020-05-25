#include <Arduino.h>
#include "i2c_driver.h"
#include "imx_rt1060_i2c_driver.h"
// Blink the LED to make sure the Teensy hasn't hung
IntervalTimer blink_timer;
volatile bool led_high = false;
void blink_isr();

const uint16_t slave_address = 0x002D;
I2CSlave& slave = Slave;
void after_receive(int size);

// Double receive buffers to hold data from master.
const size_t slave_rx_buffer_size = 4;
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

  // Configure I2C Slave
  slave.after_receive(after_receive);
  slave.set_receive_buffer(slave_rx_buffer, slave_rx_buffer_size);

  // Enable the slave
  slave.listen(slave_address);

  // Enable the serial port for debugging
  Serial.begin(9600);
  Serial.println("Started");
}

void loop() {
    if (slave_bytes_received) {
        // Handle the message.
        log_message_received();

        // Clear slave_bytes_received to signal that we're ready for another message
        slave_bytes_received = 0;
    }

    // We could receive multiple message while we're asleep.
    // This example is modelling an application where it's Ok
    // to drop messages.
    delay(1);
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
            Serial.println("App: Buffer Overflow. (Master sent too many bytes.)");
        } else {
            Serial.println("App: Unexpected error");
        }
    }
    Serial.print("App: Slave received ");
    Serial.print(slave_bytes_received);
    Serial.print(" bytes: ");
    for(size_t i=0; i<slave_bytes_received; i++) {
        Serial.print(slave_rx_buffer_2[i]);
        Serial.print(", ");
    }
    Serial.println();
}

void blink_isr() {
    led_high = !led_high;
    digitalWrite(LED_BUILTIN, led_high);
}
