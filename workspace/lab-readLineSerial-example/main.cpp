/*
 * Example for reading and evaluating user serial input (UART) to control an LED.
 */

#include "mbed.h"
#include <cstdio>

#define MSG_BUFFER 150

// Create a UnbufferedSerial object with a default baud rate.
static UnbufferedSerial pc(USBTX, USBRX);

char buff;
char input_msg[MSG_BUFFER] = "";
int idx = 0;

// Create a DigitalOutput object to toggle an LED whenever data is received.
static DigitalOut led(LED1);

/*
 * Check user input for LED control commands.
 */
void ledControl() {
  if (!strcmp(input_msg, "led on\r")) {
    led.write(1);
  } else if (!strcmp(input_msg, "led off\r")) {
    led.write(0);
  }
}

void readLineSerial() {
  if (pc.readable()) {
    pc.read(&buff, 1);  // read single char
    pc.write(&buff, 1); // echo the user input
    input_msg[idx] = buff; // add input char to buffer

    if (buff == '\r') { // input via mbed-studio serial monitor
      ledControl();     // check input message for led control cmds
      
      idx++;
      input_msg[idx] = '\n';        // add line feed character
      pc.write(&input_msg[idx], 1); // print new line      
        // pc.write(&input_msg, sizeof(input_msg));  // alternative: print whole input_msg

      memset(&input_msg, 0, sizeof(input_msg)); // clear array
      idx = 0;

    } else {
      idx++;
    }
  }
}

void on_rx_interrupt() {
  //   led = !led; //toggle LED on each keyboard input
  readLineSerial();
}

int main(void) {
  pc.baud(115200); //set baudrate to 115200
  led.write(1); //turn led on
  
  // Register a callback to process a Rx (receive) interrupt.
  pc.attach(&on_rx_interrupt, SerialBase::RxIrq);

  char msg[] = "Echoes back to the screen anything you type\n";
  pc.write(msg, sizeof(msg));

  while (1) {
    printf("Just printing some stuff to the console.\n");
    ThisThread::sleep_for(5s);
  }

}
