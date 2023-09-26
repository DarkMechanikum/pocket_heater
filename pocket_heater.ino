#include <Arduino.h>
#define clock_pin 0
#define latch_pin 1
#define data_pin 2
#define temp_pin 3
#define button_pin 4
#define heater_pin 5
#define coefficient_A 298.15
#define coefficient_B 4036
#define starting_power_level 2
//#define heater_r 100

byte temp_set[4] = {30, 35, 45, 55};

enum class Button_state
{
    TRIGGERED,
    UNTRIGGERED,
    ASKED
};

class Sensors
{
private:

    const static long int coefficient = 1125300L;

public:

    [[nodiscard]] static long read_voltage()
    {
        ADMUX = _BV(MUX3) | _BV(MUX2);
        delay(2);
        ADCSRA |= _BV(ADSC);
        while (bit_is_set(ADCSRA, ADSC));
        uint8_t low  = ADCL;
        uint8_t high = ADCH;
        long result = (high << 8) | low;
        result = coefficient / result;
        return result;
    }

    [[nodiscard]] static float read_temp()
    {
        float tmpPinVoltage = analogRead(temp_pin);
        float tmpR = ((tmpPinVoltage)/(1023 - tmpPinVoltage))*100;
        return 1/coefficient_A + (1.0/coefficient_B)*log(tmpR/100.0); // NOLINT (loss of precision is appropriate)
    }

    [[nodiscard]] static byte voltage_to_charge_level(long voltage)
    {
        // 2800, 3150, 3500, 3850, 4200
        short int val = 2800;
        for(byte i = 0; i < 5; i++)
        {
            if(voltage <= val)
            {
                return i;
            }
            val += 350;
        }
        return 4;
    }
    static Button_state read_button()
    {
        static bool tick_flag = false;
        tick_flag = !tick_flag;
        static bool trigger_flag = false;
        if(!tick_flag)
        {
            trigger_flag = digitalRead(button_pin);
            return Button_state::ASKED;
        }
        if(digitalRead(button_pin) && trigger_flag)
        {
            trigger_flag = false;
            return Button_state::TRIGGERED;
        }
        else
        {
            trigger_flag = false;
            return Button_state::UNTRIGGERED;
        }
    }

};

class Display
{
private:

    byte current_display = 0;

    void set_display() const
    {
        digitalWrite(latch_pin, LOW);
        shiftOut(data_pin, clock_pin, LSBFIRST, current_display);
        digitalWrite(latch_pin, HIGH);
    }

    void clear_display()
    {
        current_display = 0;
        set_display();
    }

public:

    void set_pixel(byte value_1, bool is_on)
    {
        byte cur_byte = 1<<value_1;
        current_display += is_on? cur_byte : -cur_byte;
        set_display();
    }

    void power_on_animation()
    {
        clear_display();
        for(byte i = 0; i < 4; i++)
        {
            set_pixel(i, true);
            set_pixel(i+4, true);
            delay(100);
        }
        clear_display();
        for(byte i = 0; i < Sensors::voltage_to_charge_level(Sensors::read_voltage())-1; i++)
        {
            set_pixel(i, true);
        }
        for(byte i = 0; i < starting_power_level; i++)
        {
            set_pixel(i, true);
        }
    }

    [[maybe_unused]] void power_off_animation()
    {
        clear_display();
        for(byte i = 0; i < 4; i++)
        {
            set_pixel(i, true);
            set_pixel(i+4, true);
        }
        for(byte i = 3; i > -1; i++)
        {
            set_pixel(i, false);
            set_pixel(i+4, false);
            delay(100);
        }
    }

    void charging_animation(byte power_level)
    {
        static byte tick = 0;
        for(byte i = 0; i < power_level; i++)
        {
            set_pixel(i, true);
        }
        tick %= 5;
        if(tick == 4) clear_display();
        else set_pixel(tick, true);
        tick++;
    }
    void show_charge_and_power(byte power_level)
    {
        clear_display();
        byte charge = Sensors::voltage_to_charge_level(Sensors::read_voltage());
        for(byte i = 0; i < charge && i < power_level; i++)
        {
            if(i < charge) set_pixel(i, true);
            if(i < power_level) set_pixel(i+4, true);
        }
    }
};

class Heater
{
private:
    byte temp = 0;
public:
    void set_power_level(byte new_power_level = starting_power_level)
    {
        temp = temp_set[new_power_level];
    }

    void run_heater() const
    {
        if(Sensors::read_temp() < temp)
        {
            digitalWrite(heater_pin, HIGH);
            return;
        }
        digitalWrite(heater_pin, LOW);
    }
};

void setup() {
    pinMode(latch_pin, OUTPUT);
    pinMode(data_pin, OUTPUT);
    pinMode(clock_pin, OUTPUT);
    pinMode(heater_pin, OUTPUT);

    pinMode(temp_pin, INPUT);
    pinMode(button_pin, INPUT);

    Display display;
    display.power_on_animation();
}

void loop() {

    static Display display;
    static Heater heater;
    static byte power_level = starting_power_level;
    heater.set_power_level(power_level);
    heater.run_heater();
    Sensors::read_button();
    long cur_voltage = Sensors::read_voltage();
    if(cur_voltage > 4700)
    {
        display.charging_animation(power_level);
    }
    else
    {
        display.show_charge_and_power(power_level);
    }
}