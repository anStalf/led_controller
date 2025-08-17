#include "led_con.h"
#include "led_patterns.h"

pin_control * _led_pin;

//Pattern config zone

//Internal functions header
void _led_timer_init(void);
void led_con_comm(void);
__led_stages _led_get_next_stage(void);
__led_stages _led_get_next_stage_extend(void);
void _led_start_simple_stage(_led_simple_pattern * simple_pattern);
void _led_start_extend_stage(_led_pattern * extend_pattern);

void _led_on(void);
void _led_off(void);
void _led_tgl(void);

void _led_tim_tick(void);
void _led_ms_tick(void);
void _led_set_bright(uint8_t light);

//Internal variables, don't use no one of this directly!
static uint8_t __led_light = 0; //Light bright 
static uint8_t __led_curr_pos_light = 0;
static uint8_t __led_for_us_count = 0; 
static uint16_t __led_current_count_ms = 0;
static __led_stages __led_stage_now = _lc_idle; 
static __led_stages __new_stage = _lc_skip;
static _led_lc_control _fn_control;
static _led_pat_stage _simple_step;
static uint8_t LGS_pos = 0;
static uint8_t LGS_pos_step = 0;
static _led_simple_pattern * _current_pattern; 
static uint8_t _stage_pos = 0;
static uint8_t _stages_repeats = 0;
static uint8_t _extended_repeats = 0;


//API functions
void led_con_init(pin_control * pin){
	GPIO_output_init(pin);
	_led_pin = pin; 
	_led_timer_init (); //100uS
	TIM4_clb_reg(_led_tim_tick);
	_led_off();
	reg_common_task (led_con_comm);
	_led_set_bright (0);
}


//Start simple pattern
void led_start_simple_stage(_led_simple_pattern * simple_pattern){
	_current_pattern = simple_pattern;
	_stage_pos = 0;
	_stages_repeats = 0;
	__new_stage = _led_get_next_stage ();
	__led_stage_now = _lc_change; 
}

//Start extended pattern
void led_start_extend_stage(_led_pattern * extend_pattern){
	LGS_pos = 0;
	LGS_pos_step = 0;
	_extended_repeats = 0;
	_fn_control.control_enable = true;
	_fn_control.control_next_stage = _led_get_next_stage_extend;
	_fn_control.step = extend_pattern->steps[0].step[0];
	_fn_control.pattern_link = extend_pattern;
	__new_stage = _lc_control;
	__led_stage_now = _lc_change;
}


void led_go(void){
	//_led_start_simple_stage(&demo_simple);
	led_start_extend_stage(&demo);
}

void led_stop(void){
	_fn_control.control_next_stage = 0;
	_fn_control.control_enable = false;
	__led_stage_now = _lc_idle; 
	
}

//Internal functions

//Stage lighting processing (common fucntion)
void led_con_comm(void){
	switch (__led_stage_now){
		case _lc_idle:
			__led_current_count_ms = 0;
			_led_set_bright(LED_IDLE_BRIGHT);
			break; 
		case _lc_simple:
			_led_set_bright(_simple_step.light);
			if (__led_current_count_ms >= _simple_step.length){
				__led_current_count_ms = 0;
				__led_stage_now = _led_get_next_stage();
			}
			break;
		case _lc_control:
			if (!_fn_control.control_enable) break;
			_led_set_bright(_fn_control.step.light);
			if (__led_current_count_ms >= _fn_control.step.length){
				__led_current_count_ms = 0;
				if (_fn_control.control_next_stage == 0){
					__led_stage_now = _led_get_next_stage();
				}else{
					__led_stage_now = _fn_control.control_next_stage();
				}
			}
			break;
		case _lc_change:
			if (__new_stage != _lc_skip){
				__led_stage_now = __new_stage;
				__new_stage = _lc_skip;
				break;
			}
		case _lc_skip:
			_led_set_bright(0);
			if (_fn_control.control_next_stage == 0){
					__led_stage_now = _led_get_next_stage();
				}else{
					__led_stage_now = _fn_control.control_next_stage();
				}
			break;
		default:
			__led_stage_now = _lc_idle; 
			break;
	}
}



//Processing for the simple pattern
__led_stages _led_get_next_stage(void){	
	if (_current_pattern != 0){
		_stage_pos ++;
		if (_stage_pos >= _current_pattern->size){
			_stage_pos = 0;
			if (_current_pattern->repeat > 0){
				if (_stages_repeats < _current_pattern->repeat){
					_stages_repeats ++;
				}else{
					if (_current_pattern->clb != 0){
						_current_pattern->clb ();
						return _lc_change;
					}
					_current_pattern = 0;
					return _lc_idle;
				}
			}
		}
		bool pos_enable = false;
		pos_enable = _current_pattern->bit_pattern & (1 << _stage_pos);
		if (pos_enable){
			//ON zone
			_simple_step.light = _current_pattern->bright;
			_simple_step.length = _current_pattern->time_on;
		}else{
			//OFF zone
			_simple_step.light = 0;
			_simple_step.length = _current_pattern->time_off;
		}
		return _lc_simple;
	}
	
	return _lc_idle;
}

//Processing for extended pattern
__led_stages _led_get_next_stage_extend(void){

	LGS_pos_step++;
	if (LGS_pos_step >= _fn_control.pattern_link->steps[LGS_pos].patterns_count){
		LGS_pos_step = 0;
		LGS_pos++;
		if (LGS_pos >= _fn_control.pattern_link->steps_count){
			LGS_pos = 0;
			if (_fn_control.pattern_link->repeats != 0){
				_extended_repeats++;
				if (_extended_repeats > _fn_control.pattern_link->repeats){
					if (_fn_control.pattern_link->clb != 0){
						_fn_control.pattern_link->clb();
						return _lc_change;
					}
					return _lc_idle;
				}
			}
			/* Move to check repeat's counter
			if (_fn_control.pattern_link->clb != 0) {
				_fn_control.pattern_link->clb();
				return _lc_change;
			}
			*/
		}
		
		if (_fn_control.pattern_link->steps[LGS_pos].check_clb != 0){
			if (!_fn_control.pattern_link->steps[LGS_pos].check_clb()){
				_fn_control.step.length = 0;
				_fn_control.step.light = 0;
				_fn_control.control_next_stage = _led_get_next_stage_extend;
				//LGS_pos_step = demo.steps->patterns_count;
				return _lc_skip;
			}
		}
	}
 
	_fn_control.step = demo.steps[LGS_pos].step[LGS_pos_step];
	
	return _lc_control;
}


//LED tick execute every 100 uS
void _led_tim_tick(void){
	__led_curr_pos_light ++;
	if (__led_curr_pos_light >= _LED_CON_MAX_BRIGHT) __led_curr_pos_light = 0;
	if (__led_curr_pos_light < __led_light){
		_led_on();
	}else{
		_led_off();
	}
	__led_for_us_count ++;
	if (__led_for_us_count > _LED_US_COUNTS) {
		__led_for_us_count = 0;
		_led_ms_tick ();
	}
}

//Updated every 1 ms
void _led_ms_tick(void){
	__led_current_count_ms++;
}
//Set new bright value for current step
void _led_set_bright(uint8_t light){
	if (light > _LED_CON_MAX_BRIGHT) light = _LED_CON_MAX_BRIGHT;
	if (__led_light == light) return;
	__led_light = light;
}

//HW configuration
void _led_timer_init(void){
	  RCC_ClocksType clocks;
    TIM_TimeBaseInitType TIM_TimeBaseStructure;
    NVIC_InitType NVIC_InitStructure;

    // Peripherial clock enable
    RCC_APB1_Peripheral_Clock_Enable(RCC_APB1_PERIPH_TIM4);

    RCC_Clocks_Frequencies_Value_Get(&clocks);
    uint32_t timer_clock = clocks.Pclk1Freq;

    // Setup scaller for generation interrupt every 10 ms
    uint16_t prescaler = (uint16_t)((timer_clock / 100000UL) - 1);
    uint16_t period = 20 - 1;
		//uint16_t period = 100 - 1;

    // Initializing timer config structure
    TIM_Base_Struct_Initialize(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.Period    = period;
    TIM_TimeBaseStructure.Prescaler = prescaler;
    TIM_TimeBaseStructure.ClkDiv    = TIM_CLK_DIV1;
    TIM_TimeBaseStructure.CntMode   = TIM_CNT_MODE_UP;

    TIM_Base_Initialize(TIM4, &TIM_TimeBaseStructure);

    // Setup scaler load
    TIM_Base_Reload_Mode_Set(TIM4, TIM_PSC_RELOAD_MODE_IMMEDIATE);

    // Enable timer
    TIM_Interrupt_Enable(TIM4, TIM_INT_UPDATE);

    // Setup NVIC interrupt control structure
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Initializes(&NVIC_InitStructure);

    // Timer enable & start
    TIM_On(TIM4);	
}

void _led_on(void){
	out_on(_led_pin);
	//out_off(led);
} 

void _led_off(void){
	out_off(_led_pin);
	//out_on(led);
}

void _led_tgl(void){
	out_tgl(_led_pin); 
}