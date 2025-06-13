#pragma once

#include <Arduino.h>

//-------------------------------------------------------------------//
//                                                                   //
// Interlock safety controller version 1.0                           //
// April 2024 - Scion Research - J Peake                             //
// Board specific defines                                            //
//                                                                   //
//-------------------------------------------------------------------//

#include <sys/types.h>
#include <stdint.h>
#include "FlashStorage.h"
#include "terminal.h"

//--------------------------> Version info <--------------------------//

#define VERSION_HW  "1.0"
#define VERSION_FW  "1.0"

//--------------------> Hardware pin definitions <--------------------//

#define PIN_V_SUPPLY_FB 2
#define PIN_CT_ADC_IN 3
#define PIN_ENABLE_IN_DRAIN 4
#define PIN_INTERLOCK_IN_DRAIN_1 5
#define PIN_INTERLOCK_IN_DRAIN_2 6
#define PIN_INTERLOCK_IN_DRAIN_3 7
#define PIN_INTERLOCK_IN_DRAIN_4 8
#define PIN_CONTACTOR_GATE 9
#define PIN_OUTPUT_GATE_1 10
#define PIN_OUTPUT_GATE_2 11
#define PIN_LED_STATUS_R 15
#define PIN_LED_STATUS_G 16
#define PIN_LED_STATUS_B 17
#define PIN_LED_ENABLE_R 18
#define PIN_LED_ENABLE_G 19
#define PIN_LED_INTERLOCK_R 22
#define PIN_LED_INTERLOCK_G 23

//-----------------------------> Limits <-----------------------------//

#define LIMIT_MAX_ENABLE_TIME_S     1800
#define LIMIT_MIN_ENABLE_TIME_S     1
#define LIMIT_MAX_PAUSE_TIME_S      1800
#define LIMIT_MIN_PAUSE_TIME_S      1
#define LIMIT_MAX_CURRENT_THRESH_A  100.0
#define LIMIT_MIN_CURRENT_THRESH_A  0.1
#define LIMIT_MAX_AMPS_PER_VOLT     1000.0
#define LIMIT_MIN_AMPS_PER_VOLT     1.0
#define LIMIT_MAX_CAL_AMPS          100.0
#define LIMIT_MIN_CAL_AMPS          0.1
#define LIMIT_MAX_PSU_VOLTS         30.0
#define LIMIT_MIN_PSU_VOLTS         18.0
#define LIMIT_CT_ADC_ZERO           40
#define LIMIT_CT_ADC_DISCONNECT     4000

//---------------------------> Constants <----------------------------//

#define V_FB_MUL_mV     8.8623046875
#define V_FB_MUL_V      0.0088623046875	

//--------------------> Terminal menu enumeration <-------------------//

#define MENU_MAIN           0
#define MENU_INTERLOCKS     1
#define MENU_ENABLE_TIME    2
#define MENU_PAUSE_TIME     3
#define MENU_CURRENT_THRESH 4
#define MENU_CT_A_PER_V     5
#define MENU_CT_CAL         6

#define FAULT_SYS_OK        0
#define FAULT_PSU_UV        1
#define FAULT_PSU_OV        2
#define FAULT_RUN_WHILE_DIS 3
#define FAULT_CT_DISCONECT  4
#define FAULT_OVERCURRENT   5

//--------------------> 1bit colour definitions <---------------------//

#define RG_BLACK     0x0
#define RG_RED       0x1
#define RG_GREEN     0x2
#define RG_AMBER     0x3

//--------------------> 24bit colour definitions <--------------------//

#define RED       0xFF0000
#define GREEN     0x00FF00
#define BLUE      0x0000FF
#define YELLOW    0x55FF00
#define AMBER     0xFFFF00
#define CYAN      0x00FFFF
#define MAGENTA   0xFF0055
#define WHITE     0xFFFFFF
#define BLACK     0x0

//--------------------> Conventional definitions <--------------------//

#define SEALED    1
#define UNSEALED  0

//----------------------------> Structs <-----------------------------//

struct flash_data {
  bool initialised = false;
  bool interlock_enabled[4] = {true};
  uint32_t enable_timeout_time_ms = 30000;
  uint32_t pause_timeout_time_ms = 30000;
  uint32_t amps_per_volt = 20;
  float current_threshold = 2.0;
  float current_multiplier = 1.0;
};

typedef void (* GenericFP)(void);

//----------------------------> Objects <-----------------------------//

FlashStorage(flash, flash_data);

//------------------------> Global variables <------------------------//

// Boolean flags

bool debug = false;
bool enabled = false;
bool enabled_state_changed = false;
bool interlocks_sealed = false;
bool interlock_sealed[4] = {false};
bool interlock_enabled[4] = {true, true, true, true};
bool running = false;
bool paused = false;
bool fault = false;
bool run_led_state = false;
bool fault_led_state = false;
bool current_wind_down_timer_active = false;
bool settings_changed = false;
bool fault_status_changed = false;
bool startup_high_current = false;

// Timer values

uint32_t enable_timeout_time_ms = 30000;
uint32_t pause_timeout_time_ms = 30000;
uint32_t medium_loop_time_ms = 100;
uint32_t slow_loop_time_ms = 200;
uint32_t current_wind_down_time_ms = 2000;
uint32_t current_update_time_ms = 500;
uint32_t enable_debounce_time_ms = 1200;
uint32_t startup_current_time_ms = 5000;

// Timer timestamps

uint32_t enable_timeout_TS_ms = 0;
uint32_t pause_timeout_TS_ms = 0;
uint32_t medium_loop_TS_ms = 0;
uint32_t slow_loop_TS_ms = 0;
uint32_t current_wind_down_TS_ms = 0;
uint32_t current_update_TS_ms = 0;
uint32_t enable_debounce_TS_ms = 0;
uint32_t startup_current_TS_ms = 0;

// Analogue variables

float psu_volts = 0;
float ct_amps = 0;
float ct_amps_no_cal = 0;
float current_threshold = 2;
float amps_per_volt = 20;
float current_multiplier = 1.0;

// Terminal menu variables

char term_buffer[10];
uint32_t term_buffer_ptr = 0;
int32_t menu_page = -1;

void termWriteMain();
void termWrite1();
void termWrite2();
void termWrite3();
void termWrite4();
void termWrite5();
void termWrite6();
GenericFP term_menu[7] = {&termWriteMain, &termWrite1, &termWrite2, &termWrite3, &termWrite4, &termWrite5, &termWrite6};

// Fault status
uint32_t fault_ptr = 0;
String fault_msg[6] = { "System OK",
                        "Power supply undervolt",
                        "Power supply overvolt",
                        "Running while diabled",
                        "CT disconnected",
                        "Overcurrent" };

// Other globals

uint32_t interlock_pin[4] = {PIN_INTERLOCK_IN_DRAIN_1, PIN_INTERLOCK_IN_DRAIN_2, PIN_INTERLOCK_IN_DRAIN_3, PIN_INTERLOCK_IN_DRAIN_4};
uint32_t enable_led_pin[2] = {PIN_LED_ENABLE_R, PIN_LED_ENABLE_G};
uint32_t interlock_led_pin[2] = {PIN_LED_INTERLOCK_R, PIN_LED_INTERLOCK_G};
uint32_t status_led_pin[3] = {PIN_LED_STATUS_R, PIN_LED_STATUS_G, PIN_LED_STATUS_B};

uint32_t run_led_state_colour[2] = {RG_GREEN, RG_AMBER};
uint32_t fault_led_state_colour[2] = {RG_BLACK, RG_RED};

flash_data flash_data_array;