#ifndef PAUSE_DETECTOR_HPP
#define PAUSE_DETECTOR_HPP

#include "msg_decoder.hpp"

class pause_detector : public rtos::task<>{
private:
    msg_decoder & decoder;
    hwlib::target::pin_in &signal;
    hwlib::target::pin_out &gnd;
    hwlib::target::pin_out &vdd;
    enum class states{idle, signal, pause};
    states state;

public:
    pause_detector(msg_decoder & decoder, hwlib::target::pin_in &signal, hwlib::target::pin_out &gnd, hwlib::target::pin_out &vdd):
        task("pause_detector"),
        decoder(decoder),
        signal(signal),
        gnd(gnd),
        vdd(vdd)
    {}

    void process(int pause){
        if(pause > 1500 && pause < 1700){
            decoder.send_pause(1600);
            state = states::signal;
        } else if(pause > 700 && pause < 900){
            decoder.send_pause(800);
            state = states::signal;
        } else if(pause > 2900 && pause < 3100){
            decoder.send_pause(3000);
            state = states::idle;
        } else {
            decoder.send_pause(pause);
            state = states::idle;
        }
    }

    void main() override {
        gnd.write(0);
        vdd.write(1);
        gnd.flush();
        vdd.flush();
        auto pause_start = 0;
        auto pause_end = 0;

        state = states::idle;
        for(;;){
            auto reading = signal.read();
            switch(state){
                case states::idle:
                    if(reading == 0){
                        state = states::signal;
                    }
                    break;
                
                case states::signal:
                    if(reading == 1){
                        pause_start = hwlib::now_us();
                        state = states::pause;
                    }
                    break;
                
                case states::pause:
                    if(reading == 0){
                        pause_end = hwlib::now_us();
                        process(pause_end - pause_start);
                    }
                    break;
            }
        }
    }
};

#endif
