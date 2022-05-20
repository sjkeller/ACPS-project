## Reading serial input (UART) 

Depending on the **platform** (Windows/Linux/Mac) and the used **terminal** (serial monitor) line breaks are handled differently.

Typically, in the Windows environment a line is terminated with the two characters `\r\n`. The `\r` character represents a carriage return, and `\n` represents a newline. In Linux, only the `\n` character is used to terminate a line.

If you try to use the **serial terminal in Mbed Studio**, you will notice a couple of things:  

- (1) your input is not echoed to the terminal by default, 
- (2) when echoing the input and hitting the ENTER button the cursor jumps to the beginning of the line without clearing the previous input (or starting a new line), 
- (3) each character input is transmitted right away, which makes it difficult to evaluate inputs, e.g., commands to control an LED. 

The provided code example shows how these issues can be fixed.

### Alternative Terminal

Alternative to the Mbed Studio Terminal you can try other terminals as well, e.g., the Arduino Serial Monitor, which may be more convenient to use. For the example to work with the Arduino terminal, you must set the newline character to carriage return (CR).![arduino-terminal](\images\arduino-terminal.PNG)