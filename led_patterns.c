#include "led_patterns.h" 

volatile bool ch_b1 = true;
volatile bool ch_b2 = true;

bool ch_b1_f(void){
	return ch_b1;
}

bool ch_b2_f(void){
	return ch_b2;
};




_led_simple_pattern demo_simple = {
	.bit_pattern = 0b10101011, 
	.size = 8,
	.time_on = 100,
	.time_off = 100,
	.bright = 1
};

_led_pat_stage demo_step[] = {
	{.light = 1, .length = _LED_SHORT_LIGHT}, 
	{.light = 8, .length = _LED_SHORT_PAUSE}
};

_led_pat_stage demo_step_2[] = {
	{.light = 4, .length = _LED_SHORT_LIGHT}, 
	{.light = 5, .length = _LED_SHORT_PAUSE},
	{.light = 0, .length = 1000},
	{.light = 10, .length = 10},
	{.light = 0, .length = 100},
	{.light = 10, .length = 50},
	{.light = 0, .length = 100},
};

_led_pattern_config demo_lpc[] = {
	{.step = demo_step, .patterns_count = sizeof(demo_step)/sizeof(demo_step[0]), .check_clb = ch_b1_f},
	{.step = demo_step_2, .patterns_count = sizeof(demo_step_2)/sizeof(demo_step_2[0]), .check_clb = ch_b2_f},
};

_led_pattern demo = {.steps = demo_lpc, .steps_count = 2, .repeats = 0};