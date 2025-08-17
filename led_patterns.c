#include "led_patterns.h" 

//Demo variables for extended check
volatile bool ch_b1 = true;
volatile bool ch_b2 = true;

//Functions for control output for indication steps
bool ch_b1_f(void){
	return ch_b1;
}

bool ch_b2_f(void){
	return ch_b2;
};

void reset_demo_to_simple(void){
	led_start_simple_stage(&bolid_normal); 
}

void reset_simple_to_demo(void){
	led_start_extend_stage(&demo);
}

//Extended patterns zone
//Steps: array of bright and time of indication
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

//Stages: array of stages indication with link to control functions (if function return FALSE - step will be skipped
_led_pattern_config demo_lpc[] = {
	{.step = demo_step, .patterns_count = sizeof(demo_step)/sizeof(demo_step[0]), .check_clb = ch_b1_f},
	{.step = demo_step_2, .patterns_count = sizeof(demo_step_2)/sizeof(demo_step_2[0]), .check_clb = ch_b2_f},
};

//Pattern: If repeats = 0 - indication will be always, steps count - how steps in stages, step - link to steps array
//Link to this varible used for start execution 
_led_pattern demo = {.steps = demo_lpc, .steps_count = 2, .repeats = 3, .clb = reset_demo_to_simple};


//Simple patterns zone
//Link to this variable used for start execution
_led_simple_pattern demo_simple = {
	.bit_pattern = 0b10101011, 
	.size = 8,
	.time_on = 100,
	.time_off = 100,
	.bright = 1,
	.repeat = 0, //Indepened
	.clb = 0,
};

_led_simple_pattern bolid_normal = {
	.bit_pattern = 0b10,
	.size = 2,
	.time_on = 100,
	.time_off = 4000,
	.bright = 5,
	.repeat = 2, //Infinity
	.clb = reset_simple_to_demo,
};

_led_simple_pattern bolid_broadcast = {
	.bit_pattern = 0b10,
	.size = 2,
	.time_on = 100,
	.time_off = 4000,
	.bright = 5,
	.repeat = 0,
	.clb = 0,
};