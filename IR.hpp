#ifndef IR_HPP
#define IR_HPP

#include "hwlib.hpp"
#include "/home/user/rtos/rtos.hpp"



class msg_logger : public rtos::task<>{
private:
    hwlib::terminal_from & display;
    struct input{int address; int command;};
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

    void draw(input message){
        if(message.address != 0 && message.command != 0){
            display << "Address: " << message.address << "\nCommand: " << message.command 
            << hwlib::flush;
        } else {
            display << "" << hwlib::flush;
        }
    }

    void main() override{
        for(;;){
            input message = messages.read();

            draw(message);
            timed_out.set(5'000'000);
            wait(timed_out + messages);
            draw({0, 0});
        }
    }
};



class msg_decoder : public rtos::task<>{
private:
    rtos::channel<int, 32> pauses;
    enum class states{idle, message_in_progress};
    states state;
    int count;
    int address [8];
    int address_invert [8];
    int command [8];
    int command_invert [8];
    msg_logger & logger;

public:
    msg_decoder(msg_logger & logger):
        task("msg_decoder"),
        pauses(this, "pauses"),
        count(0),
        logger(logger)
    {}

    void send_pause(int sent_pause){
        pauses.write(sent_pause);
    }

    int decode(int pause){
        if(pause == 1690){
            return 1;
        }
        return 0;
    }

    bool check(int original[8], int inverted[8]){
        for(unsigned int i = 0; i < 8; i++){
            if(original[i] == inverted[i]){
                return false;
            }
        }
        return true;
    }

    void send(){
        int convertedAddress = 0;
        if(address[0] == 1){
            convertedAddress += 1;
        }
        for(unsigned int i = 1; i <= 7; i++){
            if(address[i] == 1){
                convertedAddress += 2^i;
            }
        }
        
        int convertedCommand = 0;
        if(command[0] == 1){
            convertedCommand += 1;
        }
        for(unsigned int i = 1; i <= 7; i++){
            if(command[i] == 1){
                convertedCommand += 2^i;
            }
        }

        logger.log({convertedAddress, convertedCommand});
    }

    void main() override{
        state = states::idle;
        for(;;){
            int pause = pauses.read();
            switch(state){
                case states::idle:
                    if(pause == 4500){
                        state = states::message_in_progress;
                    }
                    break;

                case states::message_in_progress:
                    if(pause == 1690 || pause == 560){
                        if(count < 8){
                            address[count] = decode(pause);
                        } else if(count < 16){
                            address_invert[count - 7] = decode(pause);
                        } else if(count < 24){
                            command[count - 15] = decode(pause);
                        } else if(count < 32){
                            command_invert[count - 23] = decode(pause);
                        }
                        count++;
                    } else {
                        state = states::idle;
                        count = 0;
                    }

                    if(count == 32 && check(command, command_invert) && check(address, address_invert)){
                        send();
                        state = states::idle;
                        count = 0;
                    }
                    break;
            }
        }
    }
};

#endif
