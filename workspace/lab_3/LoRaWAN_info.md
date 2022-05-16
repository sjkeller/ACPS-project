## Notes for Lab 3 (LoRaWAN) 

- **MQTT broker:** The IP address of the MQTT broker is (accessible via eduroam)

  ~~~
  broker_adress = "134.28.36.100" 
  ~~~

- **Build profile:** For some reason it is necessary to build the program using **`Develop` **build profile in Mbed Studio, otherwise the program will get stuck after the first transmission made.

- **New .mbedignore file:** When creating new programs, don't forget to copy the **.mbedignore** file provided in the Lorawan example. The update file enables / does not disable the LoRaWAN libraries.

- **Set Device EUI:** Before building and running the program make sure to modify the value of the `"lora.device-eui"` AND the `"lora.application-key"` in the **mbed_app.json** file so that the last hex value matches
  your board number, e.g., `0x02` if you have **board #2**, **board #12** -> `0x12` , etc  

- **UART baudrate:** Note, that the **baud rate** is changed to **`115200`** in the lorawan-example, see `mbed_app.json`file.

- **TX Intervals / Duty Cycling:** 

    > LoRaWAN v1.0.2 specifcation is exclusively duty cycle based. The Lorawan example program comes with duty cycle enabled by default. In other words, the Mbed OS LoRaWAN stack enforces duty cycle. The stack keeps track of transmissions on the channels in use and schedules transmissions on channels that become available in the shortest time possible. We recommend you keep duty cycle on for compliance with your country specific regulations.
    >
    > However, you can define a timer value in the application, which you can use to perform a periodic uplink when the duty cycle is turned off. Such a setup should be used only for testing or with a large enough timer value. For example:

    If you want to try, you can disable the duty cycle setting in the `mbed_app.json`:

    ```c++
    "target_overrides": {
        "*": {
            "lora.duty-cycle-on": false
        }
    }
    ```

    But please use large values for the **fixed transmission intervals**, e.g.,:

    ~~~c++
    #define TX_TIMER                        10s
    ~~~



## Known issues/bugs

- Shortly after first messages are sent, the following error is returned 

  ~~~
  Error in reception - Code = 9 
  ~~~

​		If this appears only once at the beginning. You can safely ignore this error.

- Program get stuck if not built with "develop" profile.

## Resources / Further Reading

- [**Mbed OS -** LoRaWAN usage (quick overview)](https://os.mbed.com/docs/mbed-os/v6.15/apis/lorawan-usage.html)
- [**Mbed OS -** LoRaWAN stack API documentation](https://os.mbed.com/docs/mbed-os/v6.15/apis/lorawan-apis.html)
- [**Mbed OS -** LoRaWAN network architecture](https://os.mbed.com/docs/mbed-os/v6.15/apis/lora-tech.html)
- [The Things Network](https://www.thethingsnetwork.org/docs/lorawan/) provides a comprehensive overview on the LoRaWAN technology.




### Sending data to the end device

Because LoRaWAN (in Class-A mode, which you're using here) is not continuously connected to the network, you **need to wait for a receive (RX) window** to occur to receive data. An RX window opens after a transmission. So you need to ***send* to the network before you can receive** a message. 

### **Restrictions on sending data (Duty Cycle Regulations)**

This is important if you develop your own LoRa-enabled systems in the future!

You cannot send data constantly because of spectrum regulations. Although the spectrum that LoRa uses is unlicensed, it is regulated. For example, in Europe, there are **duty cycle limitations of 1%** - meaning you can only send 1% of the time (e.g. 24 hours x 60 minuts * 1 % = 14.4 minutes per day, 36 seconds per hours, ...). In the US, there's dwell time, which requires you to wait at least 400 ms between transmissions. If you violate these regulations, your data transmission fails. How fast you are allowed to send data depends on the spread factor you use. With a higher spread factor, it takes longer to send a message - though the chance that a gateway receives it increases. However, you need to wait longer before you can send data again. During development, you can set the spread factor to SF7 (the lowest), so you can send every 6-7 seconds.

**See also:** 

- https://www.thethingsnetwork.org/docs/lorawan/duty-cycle/
- [Airtime Calculator](https://www.thethingsnetwork.org/airtime-calculator)
