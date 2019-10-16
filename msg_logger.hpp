#ifndef MSG_LOGGER_HPP
#define MSG_LOGGER_HPP

#include "hwlib.hpp"
#include "/home/user/rtos/rtos.hpp"

class msg_logger : public rtos::task<>{
private:
    hwlib::terminal_from & display;
    struct input{int player; int data;};
    rtos::channel<input, 2> messages;
    rtos::timer timed_out;

public:
    msg_logger(hwlib::terminal_from & display):
        task("msg_logger"),
        display(display),
        messages(this, "messages"),
        timed_out(this, "timed_out timer")
    {}

    void log(input message){
        messages.write(message);
    }

    void process(input message){
        if(message.player == 0){
            hwlib::cout << "Order: " << message.data << "\n" << hwlib::flush;
        } else {
            hwlib::cout << "Player " << message.player << " has hit you with a " << message.data << "\n" << hwlib::flush;
        }
    }

    void main() override{
        for(;;){
            input message = messages.read();

            process(message);
            timed_out.set(5'000'000);
            wait(timed_out + messages);
        }
    }
};

#endif
