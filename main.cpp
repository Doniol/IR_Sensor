#include "pulse_detector.hpp"

int main(){
    hwlib::wait_ms(500);

    auto scl = hwlib::target::pin_oc(hwlib::target::pins::scl);
    auto sda = hwlib::target::pin_oc(hwlib::target::pins::sda);
    auto i2c_bus = hwlib::i2c_bus_bit_banged_scl_sda(scl, sda);
    auto oled = hwlib::glcd_oled(i2c_bus, 0x3c);
    auto font = hwlib::font_default_8x8();
    auto display = hwlib::terminal_from(oled, font);

    auto tsop_signal = hwlib::target::pin_in(hwlib::target::pins::d8);
    auto tsop_gnd    = hwlib::target::pin_out(hwlib::target::pins::d9);
    auto tsop_vdd    = hwlib::target::pin_out(hwlib::target::pins::d10);

    auto logger = msg_logger(display);
    auto decoder = msg_decoder(logger);
    auto detector = pulse_detector(decoder, tsop_signal, tsop_gnd, tsop_vdd);

    rtos::run();
}