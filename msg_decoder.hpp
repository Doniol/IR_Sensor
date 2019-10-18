#ifndef MSG_DECODER_HPP
#define MSG_DECODER_HPP

#include "msg_logger.hpp"

class msg_decoder : public rtos::task<>{
private:
    rtos::channel<int, 32> pulses;
    enum class states{receiving, decoding};
    states state;
    int count;
    int player [5];
    int data [5];
    int control [5];
    int player2 [5];
    int data2 [5];
    int control2 [5];
    msg_logger & logger;
    int received_pulses [32];

public:
    msg_decoder(msg_logger & logger):
        task("msg_decoder"),
        pulses(this, "pauses"),
        count(0),
        logger(logger)
    {}

    void send_pulse(int sent_pulse){
        pulses.write(sent_pulse);
    }

    int decode(int pause){
        if(pause == 800){
            return 0;
        }
        return 1;
    }

    bool check(){
        bool ans1 = true;
        bool ans2 = true;

        for(unsigned int i = 0; i < 5; i++){
            if(((control[i] == 1) && (player[i] == data[i])) || ((control[i] == 0) && (player[i] != data[i]))){
                ans1 = false;
            }
        }

        for(unsigned int i = 0; i < 5; i++){
            if((control[i] != control2[i]) || (data[i] != data2[i]) || (player[i] != player2[i])){
                ans2 = false;
            }
        }
        // if(ans1 && ans2){
        //     hwlib::cout << "Check returns True\n";
        // } else {
        //     hwlib::cout << "Check returns False\n";
        // }
        return ans1 && ans2;
    }

    void send(){
        int p = 0;
        for (int i = 0; i < 5; i++){
            if(player[4 - i]){
                p |= 1 << i;
            }
        }
        int8_t converted_player = p;

        int d = 0;
        for (int i = 0; i < 5; i++){
            if(data[4 - i]){
                d |= 1 << i;
            }
        }
        int8_t converted_data = d;
        
        // hwlib::cout << converted_player << " " << converted_data << "\n";
        logger.log({converted_player, converted_data});
    }

    void main() override{
        state = states::receiving;
        int pulse = 0;
        for(;;){
            // hwlib::cout << pulse << "\n";
            // hwlib::cout << " " << pulse << " " << decode(pulse) << "\n";
            // hwlib::cout << decode(pulse);

            switch(state){
                case states::receiving:
                    pulse = pulses.read();
                    if(pulse == 1600 || pulse == 800){
                        received_pulses[count] = pulse;
                        count++;
                    }
                    if(count == 32){
                        state = states::decoding;
                    }
                    break;
                
                case states::decoding:
                    // for(unsigned int i = 0; i < 32; i++){
                    //     hwlib::cout << received_pulses[i] << " ";
                    //     if(i == 15){
                    //         hwlib::cout << "\n";
                    //     }
                    // }

                    for(unsigned int i = 1; i < 32; i++){
                        if(i < 6){
                            player[i - 1] = decode(received_pulses[i]);
                        } else if(i < 11){
                            data[i - 6] = decode(received_pulses[i]);
                        } else if(i < 16){
                            control[i - 11] = decode(received_pulses[i]);
                        } else if(i < 22){
                            player2[i - 17] = decode(received_pulses[i]);
                        } else if(i < 27){
                            data2[i - 22] = decode(received_pulses[i]);
                        } else if(i < 32){
                            control2[i - 27] = decode(received_pulses[i]);
                        }
                    }

                    // for(unsigned int i = 0; i < 5; i++){
                    //     hwlib::cout << player[i];
                    // }
                    // hwlib::cout << " ";
                    // for(unsigned int i = 0; i < 5; i++){
                    //     hwlib::cout << data[i];
                    // }
                    // hwlib::cout << " ";
                    // for(unsigned int i = 0; i < 5; i++){
                    //     hwlib::cout << control[i];
                    // }
                    // hwlib::cout << "\n";
                    // for(unsigned int i = 0; i < 5; i++){
                    //     hwlib::cout << player2[i];
                    // }
                    // hwlib::cout << " ";
                    // for(unsigned int i = 0; i < 5; i++){
                    //     hwlib::cout << data2[i];
                    // }
                    // hwlib::cout << " ";
                    // for(unsigned int i = 0; i < 5; i++){
                    //     hwlib::cout << control2[i];
                    // }

                    if(check()){
                        send();
                    }                    

                    count = 0;
                    state = states::receiving;
                    break;
            }
        }
    }
};

#endif
