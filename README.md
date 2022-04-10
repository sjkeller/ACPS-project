## Getting Started with STM32 LoRa Board

### Prerequisites

- Git installed, see [git_tutorial.md](./how_to/git_tutorial.md)
- Signed up in
  - GitLab 
  - GitHub 
- SSH Key Setup (recommend but not mandatory), see [generate_ssh_key.md](./how_to/generate_ssh_key.md)
- Cloned your group repository from GitLab

### Setup

- Download and install [Mbed Studio Version 1.4.3](https://os.mbed.com/studio/)   
- **Notes:**
  - When starting Mbed Studio for the first time, you are asked to to sign up (create a free account) on mbed.com to use the IDE.
  - `Mbed Studio V1.4.3` comes with `mbed-os 6.13.0`, but we will use the latest version, which is `mbed-os 6.15.1`. It's recommended to set up a **shared instance of Mbed OS `v6.15.1`** to save hard disk space, see the [instructions below](#shared-instance-of-mbed-os). You may skip this step and do it later, if you prefer to run the hello world program first.

### Hello World 

- Start Mbed Studio
- Connect your device 
  - Wait a moment until Mbed Studio has auto-detected the platform
  - If it does not detect the device automatically, select **Target:**  `disco_l072cz_lrwan1`	
- File -> Open Workspace -> Select the folder `workspace` in your repository
- File -> New Program -> `empty MBED OS program`
  - Give the program a name, e.g.,  `example-blinky`
- We have prepared a **`.mbedignore`** file that will save you some compilation time. Copy it into your program folder. 
- Here is the code for the blinky example. Copy and replace the content of `main.cpp` file:

~~~c++
#include "mbed.h"

// Blinking rate in milliseconds
#define BLINKING_RATE 1000ms 

// main() runs in its own thread in the OS
int main()
{
    // Initialize the digital pin LED1 as an output
    DigitalOut led(LED1);

    while (true) {        
        led = !led;
        ThisThread::sleep_for(BLINKING_RATE);
    }
}
~~~

- Run the program (build and upload to board)
- Expected Output: LED1 should Blink at 1 Hz




## Shared instance of Mbed OS

- Setting up a shared instance of Mbed OS that is used by across multiple programs will save space on your hard disk.

- **Step 1:**  Clone the Mbed OS repository from GitHub (using SSH):	

        git clone git@github.com:ARMmbed/mbed-os.git
        cd mbed-os
        git fetch --tags
        git checkout mbed-os-6.15.1 -b my-mbed-os-6.15.1
  
	- If you prefer using HTTPS URL instead of SSH then the clone command is:
	
	    	git clone https://github.com/ARMmbed/mbed-os
	
- **Step 2:** Whenever you create a new program you need to link/specify the location of the Mbed OS repository on your hard disk. Of course it is also possible to link/unlink the `mbed-os`library at any time afterwards.

- See also: [shared-instance-of-mbed-os](https://os.mbed.com/docs/mbed-studio/current/create-import/index.html#using-a-shared-instance-of-mbed-os-across-multiple-programs)

**Notes**

- If you don't use shared instances of Mbed OS, then by default Mbed Studio will create an instance of Mbed OS of the version, which Mbed Studio is shipped with. 
- see[ latest release](https://os.mbed.com/mbed-os/releases/)



## Technical references

- [**Development board:** B-L072Z-LRWAN1](http://www.st.com/content/st_com/en/products/evaluation-tools/product-evaluation-tools/mcu-eval-tools/stm32-mcu-eval-tools/stm32-mcu-discovery-kits/b-l072z-lrwan1.html)
  - *B-L072Z-LRWAN1 = STM32L0 Discovery kit LoRa, Sigfox, low-power wireless*  
- [**Microcontroller**: STM32L072CZ](http://www.st.com/content/st_com/en/products/microcontrollers/stm32-32-bit-arm-cortex-mcus/stm32l0-series/stm32l0x2/stm32l072cz.html)
- [Board datasheet](https://www.st.com/resource/en/user_manual/dm00329995.pdf)
- [Microcontroller datasheet](https://www.st.com/resource/en/datasheet/stm32l072cz.pdf)
- [**Board pinout**](https://os.mbed.com/platforms/ST-Discovery-LRWAN1/#board-pinout)

### MBed OS Documentation 

- [**Docs:** os.mbed.com/docs](https://os.mbed.com/docs/)
  - make sure you have selected the documentation for v6.15  
- [**API**](https://os.mbed.com/docs/mbed-os/v6.15/apis/index.html)

  

