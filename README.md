# led_controller
Extended led controller for single LED.

# Description 
Simple LED controller based on system timer with perion 100 uS. 
Provide two modes of working - simple pattern with configuradable pattern, time on & off (in mS), bright for on stage (1-10). And extended mode, with controlled steps, light, length, and external functions for check of needed of proceed this step. 

# Using 
Include "led_con.h" in your hal init
Call led_con_init and transmit to it pin_struct for your LED
Rtuern value will be clb for timer interrupt 
Setup your timer for 100uS reload and interrupt mode. 

Create patterns list like in demo led_patterns.c/h or change in this file

Call function in your code: led_start_simple_stage(_led_miple_pattern * your_simple_pattern) for simple pattern variant, or led_start_extend_stage (_led_pattern * extend_pattern). If you need for disable LED indication you can use function led_stop();
