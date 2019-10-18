#ifndef MSG_DECODER_HPP
#define MSG_DECODER_HPP

#include "msg_logger.hpp"

class msg_decoder : public rtos::task<>{
private:
    rtos::channel<int, 32> pulses;
    enum class states{idle, message_in_progress};
    states state;
    int count;
    int player [5];
    int data [5];
    int control [5];
    int player2 [5];
    int data2 [5];
    int control2 [5];
    msg_logger & logger;

public:
    msg_decoder(msg_logger & logger):
        task("msg_decoder"),
        pulses(this, "pauses"),
        count(1),
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
        return ans1 && ans2;
    }

    void send(){
        int converted_player = 0;
        for(unsigned int i = 4; i >= 0; i--){
            if(player[i] == 1){
                converted_player += 2^(4 - i);
            }
        }

        int converted_data = 0;
        for(unsigned int i = 4; i >= 0; i--){
            if(data[i] == 1){
                converted_data += 2^(4 - i);
            }
        }
        
        logger.log({converted_player, converted_data});
    }

    void main() override{
        state = states::idle;
        for(;;){
            int pulse = pulses.read();
            // hwlib::cout << pulse << "\n";
            // hwlib::cout << " " << pulse << " " << decode(pulse) << "\n";
            // hwlib::cout << decode(pulse);

            switch(state){
                case states::idle:
                    if(pulse == 800){
                        state = states::message_in_progress;
                    }
                    break;
                
                case states::message_in_progress:
                    // hwlib::cout << pulse << " " << decode(pulse) << "\n";
                    if(count == 0 || count == 16){
                        //ignore
                    } else if(count < 16){
                        if(count < 6){
                            player[count - 1] = decode(pulse);
                        } else if(count < 11){
                            data[count - 6] = decode(pulse);
                        } else if(count < 16){
                            control[count - 11] = decode(pulse);
                        }
                    } else if(count < 32){
                        if(count < 22){
                            player2[count - 17] = decode(pulse);
                        } else if(count < 27){
                            data2[count - 22] = decode(pulse);
                        } else if(count < 32){
                            control2[count - 27] = decode(pulse);
                        }
                    }
                    count++;
            }


            // switch(state){
            //     case states::idle:
            //         if(pulse == 800){
            //             state = states::message_in_progress;
            //         }
            //         break;

            //     case states::message_in_progress:
            //         if(pulse == 1600 || pulse == 800){
            //             if(count < 5){
            //                 player[count] = decode(pulse);
            //             } else if(count < 10){
            //                 data[count - 5] = decode(pulse);
            //             } else if(count < 15){
            //                 control[count - 10] = decode(pulse);
            //             } else if(count < 20){
            //                 player2[count - 15] = decode(pulse);
            //             } else if(count < 25){
            //                 data2[count - 20] = decode(pulse);
            //             } else if(count < 30){
            //                 control2[count - 25] = decode(pulse);
            //             }
            //             count++;

            //             if(count == 14){
            //                 hwlib::cout << " ";
            //             }
            //             hwlib::cout << decode(pulse);
            //         } else if(pulse != 3000){
            //             state = states::idle;
            //             count = 1;
            //         }

            //         if(count == 32 && check()){
            //             send();
            //             state = states::idle;
            //             count = 1;
            //         }
            //         break;
            // }
        }
    }
};

#endif
