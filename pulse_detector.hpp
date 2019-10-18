#ifndef PULSE_DETECTOR_HPP
#define PULSE_DETECTOR_HPP

#include "msg_decoder.hpp"

class pulse_detector : public rtos::task<>{
private:
    msg_decoder & decoder;
    hwlib::target::pin_in &signal;
    hwlib::target::pin_out &gnd;
    hwlib::target::pin_out &vdd;
    enum class states{signal, pause};
    states state;

public:
    pulse_detector(msg_decoder & decoder, hwlib::target::pin_in &signal, hwlib::target::pin_out &gnd, hwlib::target::pin_out &vdd):
        task("pulse_detector"),
        decoder(decoder),
        signal(signal),
        gnd(gnd),
        vdd(vdd)
    {}

    void process(int pulse){
        if(pulse > 9000 && pulse < 21000){
            decoder.send_pulse(1600);
            // hwlib::cout << 1;
        } else if(pulse > 2000 && pulse < 9000){
            decoder.send_pulse(800);
            // hwlib::cout << 0;
        }
    }

    void main() override {
        gnd.write(0);
        vdd.write(1);
        gnd.flush();
        vdd.flush();
        auto pulse_start = 0;
        auto pulse_end = 0;

        state = states::signal;
        for(;;){
            auto reading = signal.read();        
            switch(state){
                case states::signal:
                    if(reading == 0){
                        pulse_start = hwlib::now_us();
                        state = states::pause;
                    }
                    break;
                
                case states::pause:
                    if(reading == 1){
                        pulse_end = hwlib::now_us();
                        // hwlib::cout << pulse_end - pulse_start << "\n";
                        state = states::signal;
                        process(pulse_end - pulse_start);
                    }
                    break;
            }
        }
    }
};

#endif
