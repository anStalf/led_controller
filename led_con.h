#pragma once 

#include "main.h"

//Defaults
#define LED_IDLE_BRIGHT 0

//Visual defines (variables in milliseconds)
#define _LED_SHORT_LIGHT 100
#define _LED_LONG_LIGHT 300
#define _LED_SHORT_PAUSE 100
#define _LED_LONG_PAUSE 200

//Internal defines
#define _LED_CON_MAX_BRIGHT 10
#define _LED_US_COUNTS 9

typedef enum {
	_lc_idle,
	_lc_short_on,
	_lc_short_off,
	_lc_long_on,
	_lc_long_off,
	_lc_simple,
	_lc_control,
	_lc_skip,
} __led_stages; 

typedef struct {
	uint8_t light;
	uint16_t length;
} _led_pat_stage;

typedef struct {
	_led_pat_stage * step;
	uint8_t patterns_count;
	bool (* check_clb)(void);
} _led_pattern_config;

typedef struct {
	_led_pattern_config * steps;
	uint8_t steps_count; 
	uint8_t repeats; //0 - infinity
} _led_pattern;

typedef struct{
	_led_pat_stage step;
	bool control_enable; 
	__led_stages (*control_next_stage)(void);
}_led_lc_control;

typedef struct{
	uint32_t bit_pattern; 
	uint8_t size; 
	uint16_t time_on;
	uint16_t time_off;
	uint8_t bright;
	uint8_t repeat; // 0 - infinity repeats;
} _led_simple_pattern;

//API functions
void led_con_init(pin_control * pin);
void led_go(void);
void led_start_extend_stage(_led_pattern * extend_pattern);
void led_start_simple_stage(_led_simple_pattern * simple_pattern);
void led_stop(void);
