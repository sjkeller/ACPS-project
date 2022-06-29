#include "mbed.h"

#define CURRENT_FACTOR 8.43
#define DISABLE_SOLAR_PIN PB_5
#define DISCHARGE_PIN PB_7


// main() runs in its own thread in the OS
AnalogIn solar_voltage(A2);
AnalogIn cap_voltage(A0);

float solar_vol_val;
float cap_vol_val;
float soc;


int main()
{
    solar_voltage.set_reference_voltage(3.3);
    cap_voltage.set_reference_voltage(3.3);
     
    while (true) {
        solar_vol_val = solar_voltage.read_voltage() / CURRENT_FACTOR;
        cap_vol_val = cap_voltage.read_voltage();
        soc = (cap_voltage * cap_voltage - 0.25) / 7.29;
        printf("%f", solar_vol_val);
        ThisThread::sleep_for(1s);
    }
}

