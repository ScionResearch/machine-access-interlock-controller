#include "sys_init.h"

// ---------------------->  Forward declarations  <--------------------//

void enable_input_active_CB(void);
void interlocksHandler(void);
void enabledHandler(void);
void analogueHandler(void);
void checkSerial(void);
void enable(void);
void disable(void);
void setLEDrgb(uint32_t *LED_pins, uint32_t colour, bool invert);
void setLED(uint32_t *LED_pins, uint8_t colour);
void saveFlash(void);
void faultHandler(uint32_t _fault);
void setupPins(void);
void setupFlash(void);
void setupHWInterrupts(void);
void setupComms(void);
void termWriteMain(void);
void termWrite1(void);
void termWrite2(void);
void termWrite3(void);
void termWrite4(void);
void termWrite5(void);
void termWrite6(void);

//----------------------------->  Setup  <-----------------------------//

void setup(void) {
  setupPins();
  delay(1000);
  setupComms();
  setupFlash();
  setupHWInterrupts();

  if (debug) setLEDrgb(status_led_pin, MAGENTA, true);
  else setLEDrgb(status_led_pin, GREEN, true);
  medium_loop_TS_ms = millis();
  slow_loop_TS_ms = millis();
}

//--------------------------->  Main Loop  <---------------------------//

void loop(void) {
  // Fast loop -------->>> High priority functions
  interlocksHandler();
  if (enabled) enabledHandler();
  if (menu_page >= 0) checkSerial();  // Fast serial menu if terminal connected

  // Medium loop -------->> (100ms)
  if (millis() - medium_loop_TS_ms >= medium_loop_time_ms) {
    medium_loop_TS_ms += medium_loop_time_ms;
    analogueHandler();
    if (current_wind_down_timer_active && (millis() - current_wind_down_TS_ms >= current_wind_down_time_ms)) current_wind_down_timer_active = false;
    if (fault) {
      fault_led_state ^= 1;
      setLED(enable_led_pin, fault_led_state_colour[fault_led_state]);
    }
  }

  // Slow loop --------> (200ms)
  if (millis() - slow_loop_TS_ms >= slow_loop_time_ms) {
    slow_loop_TS_ms += slow_loop_time_ms;
    if (running) {
      run_led_state ^= 1;
      setLED(enable_led_pin, run_led_state_colour[run_led_state]);
    }
    if (menu_page < 0) checkSerial();
  }
}

//------------------------->  ISR Functions  <-------------------------//

void enable_input_active_CB(void) {
  if (millis() - enable_debounce_TS_ms < enable_debounce_time_ms) return;
  enable_debounce_TS_ms = millis();
  if (digitalRead(PIN_ENABLE_IN_DRAIN) && !enabled && interlocks_sealed) {
    enabled = true;
    enabled_state_changed = true;
    enable_timeout_TS_ms = millis();
  }
}

//------------------------->  Loop Functions  <------------------------//

void interlocksHandler(void) {
  bool interlocks_state_changed = false;
  bool interlocks_enabled = false;
  interlocks_sealed = true;
  for (int i = 0; i < 4; i++) {
    bool interlock_state = digitalRead(interlock_pin[i]);
    if (interlock_enabled[i]) {
      interlocks_enabled = true;
      if (!interlock_state) {
        interlocks_sealed = false;
        setLED(interlock_led_pin, RG_RED);
        disable();
      }
    }
    if (interlock_state ^ interlock_sealed[i]) {
      interlocks_state_changed = true;
      interlock_sealed[i] = interlock_state;
      if (debug) Serial.printf("Interlock %i changed state to %s\n", i + 1, interlock_state ? "Sealed" : "Unsealed");
    }
  }
  if ((interlocks_state_changed && interlocks_sealed) || !interlocks_enabled) setLED(interlock_led_pin, RG_GREEN);
}

void enabledHandler(void) {
  if (!running && !paused && (millis() - enable_timeout_TS_ms >= enable_timeout_time_ms)) {  // Pre-run timer timeout
    disable();
    if (debug) Serial.printf("Disabled due to pre-run %is timer timeout\n", enable_timeout_time_ms / 1000);
  } else if (paused && (millis() - pause_timeout_TS_ms >= pause_timeout_time_ms)) {  // Post-run timer timeout
    disable();
    if (debug) Serial.printf("Disabled due to post-run %is timer timeout\n", pause_timeout_time_ms / 1000);
  } else if (enabled_state_changed) {  // Enable only just triggered
    if (fault) {
      faultHandler(FAULT_SYS_OK);  // clear fault status on valid enable signal
      setLED(enable_led_pin, RG_BLACK);
      digitalWrite(PIN_OUTPUT_GATE_2, HIGH);
      enabled_state_changed = false;
      return;
    }
    if (interlocks_sealed) {
      enable();
      if (debug) Serial.printf("Enabled with %is timer\n", enable_timeout_time_ms / 1000);
    }
    enabled_state_changed = false;
  }
}

void analogueHandler(void) {
  // PSU supply voltage check
  uint32_t adc_raw_psu = analogRead(PIN_V_SUPPLY_FB);
  psu_volts = (float)adc_raw_psu * V_FB_MUL_V;
  if (psu_volts < LIMIT_MIN_PSU_VOLTS) faultHandler(FAULT_PSU_UV);
  else if (psu_volts > LIMIT_MAX_PSU_VOLTS) faultHandler(FAULT_PSU_OV);
  // CT analogue signal processing
  uint32_t adc_raw_CT = analogRead(PIN_CT_ADC_IN);
  if (adc_raw_CT <= LIMIT_CT_ADC_ZERO) adc_raw_CT = 0;
  else if (adc_raw_CT >= LIMIT_CT_ADC_DISCONNECT) {
    if (enabled) {
      if (!startup_high_current) {
        running = true;
        startup_high_current = true;
        startup_current_TS_ms = millis();
        digitalWrite(PIN_OUTPUT_GATE_1, LOW);
        if (debug) {
          Serial.printf("RUNNING with high startup current -> CT raw: %i, CT amps: ", adc_raw_CT);
          Serial.println(ct_amps);
        }
        return;
      } else if (millis() - startup_current_TS_ms >= startup_current_time_ms) {
        startup_high_current = false;
        if (debug) Serial.println("Startup current exceeded max time value");
        faultHandler(FAULT_OVERCURRENT);
        return;
      }
      return;
    }
    faultHandler(FAULT_CT_DISCONECT);
    ct_amps = 0;
    return;
  }
  ct_amps_no_cal = ((float)adc_raw_CT / 4096.0) * amps_per_volt;
  ct_amps = ct_amps_no_cal * current_multiplier;
  if (ct_amps > current_threshold) {
    if (!enabled && !fault) {                      // Running while not enabled - fault condition
      if (current_wind_down_timer_active) return;  // ignore fault condition during current drop off time
      faultHandler(FAULT_RUN_WHILE_DIS);
      if (debug) {
        Serial.printf("FAULT -> CT raw: %i, CT amps: ", adc_raw_CT);
        Serial.println(ct_amps);
      }
    } else if (paused && !fault) {  // Resume
      paused = false;
      running = true;
      digitalWrite(PIN_OUTPUT_GATE_1, LOW);
      if (debug) {
        Serial.printf("RESUMED -> CT raw: %i, CT amps: ", adc_raw_CT);
        Serial.println(ct_amps);
      }

    } else if (!running && !fault) {  // Start running
      running = true;
      digitalWrite(PIN_OUTPUT_GATE_1, LOW);
      if (debug) {
        Serial.printf("RUNNING -> CT raw: %i, CT amps: ", adc_raw_CT);
        Serial.println(ct_amps);
      }
    }
  } else {
    if (running) {  // Pause
      running = false;
      paused = true;
      pause_timeout_TS_ms = millis();
      digitalWrite(PIN_OUTPUT_GATE_1, HIGH);
      setLED(enable_led_pin, RG_GREEN);
      if (debug) {
        Serial.printf("PAUSED -> CT raw: %i, CT amps: ", adc_raw_CT);
        Serial.println(ct_amps);
      }
    }
  }
}

void checkSerial(void) {
  if (!Serial && menu_page >= 0) {
    menu_page = -1;
    return;
  } else if (!Serial) return;
  if ((menu_page == MENU_MAIN || menu_page == MENU_CT_CAL) && (millis() - current_update_TS_ms >= current_update_time_ms)) {
    current_update_TS_ms += current_update_time_ms;
    uint8_t r = menu_page ? 3 : 12;
    Serial.println(term_clr_line_from(r, 37) + String(ct_amps) + String("A"));
    if (menu_page == MENU_CT_CAL) Serial.print(term_cursor(5, 37 + term_buffer_ptr));
    if (menu_page == MENU_MAIN && fault_status_changed) {
      Serial.print(term_cond_colour_str(fault_msg[fault_ptr] + term_clr_line(), fault_ptr, ANSI_GREEN, ANSI_RED));
      fault_status_changed = false;
    }
  }
  if (Serial.available()) {
    char data = Serial.read();
    if (menu_page < 0) {
      current_update_TS_ms = millis();
      termWriteMain();
      return;
    } else if (data == 'b' || data == 'B') termWriteMain();
    else if ((data == 's' || data == 'S') && settings_changed) {
      saveFlash();
      Serial.print(term_cursor(14, 14) + term_colour_str("**Settings saved**", ANSI_GREEN));
    } else if (data == BACKSPACE && menu_page >= MENU_ENABLE_TIME && term_buffer_ptr > 0) {
      Serial.write(BACKSPACE);
      term_buffer_ptr--;
    } else if (data == ENTER && menu_page >= MENU_ENABLE_TIME && term_buffer_ptr > 0) {
      if (menu_page <= MENU_PAUSE_TIME) {
        String buf = String(term_buffer);
        buf.remove(term_buffer_ptr);
        uint32_t new_timer_value = buf.toInt();
        if (debug) Serial.println(String("\nGot integer value ") + String(new_timer_value));
        // Apply new enable timer value
        if (menu_page == MENU_ENABLE_TIME && new_timer_value <= LIMIT_MAX_ENABLE_TIME_S && new_timer_value >= LIMIT_MIN_ENABLE_TIME_S) enable_timeout_time_ms = new_timer_value * 1000;
        // Apply new pause timer value
        if (menu_page == MENU_PAUSE_TIME && new_timer_value <= LIMIT_MAX_PAUSE_TIME_S && new_timer_value >= LIMIT_MIN_PAUSE_TIME_S) pause_timeout_time_ms = new_timer_value * 1000;
      } else if (menu_page <= MENU_CT_CAL) {
        String buf = String(term_buffer);
        buf.remove(term_buffer_ptr);
        float new_float_value = String(buf).toFloat();
        if (debug) Serial.println(String("\nGot float value ") + String(new_float_value));
        // Apply new run current threshold
        if (menu_page == MENU_CURRENT_THRESH && new_float_value <= LIMIT_MAX_CURRENT_THRESH_A && new_float_value >= LIMIT_MIN_CURRENT_THRESH_A) current_threshold = new_float_value;
        // Apply new run CT ratio
        if (menu_page == MENU_CT_A_PER_V && new_float_value <= LIMIT_MAX_AMPS_PER_VOLT && new_float_value >= LIMIT_MIN_AMPS_PER_VOLT) amps_per_volt = new_float_value;
        // Apply new calibration
        if (menu_page == MENU_CT_CAL && new_float_value <= LIMIT_MAX_CAL_AMPS && new_float_value >= LIMIT_MIN_CAL_AMPS) current_multiplier = new_float_value / ct_amps_no_cal;
      }
      term_buffer_ptr = 0;
      term_menu[menu_page]();
      settings_changed = true;
    } else if (menu_page <= MENU_CT_CAL) {
      uint8_t data_int = data - '0';
      if (data != DECIMAL_POINT && (data_int < 0 || data_int > 9)) return;  // Invalid charactor
      if (menu_page == MENU_MAIN) {                                         // --> At main menu, changing to selection
        if (data_int > 6) return;
        term_buffer_ptr = 0;
        term_menu[data_int]();
      } else if (menu_page == MENU_INTERLOCKS) {  // --> At interlock settings, editing interlock enable state
        if (data_int > 0 && data_int < 5) {
          data_int--;
          interlock_enabled[data_int] ^= 1;
          settings_changed = true;
          termWrite1();
        } else return;
      } else {  // --> At pre or post timer settings, save digit to buffer and echo to terminal
        if (term_buffer_ptr >= 9) return;
        term_buffer[term_buffer_ptr] = data;
        term_buffer_ptr++;
        Serial.write(data);
      }
    }
  }
}

//----------------------->  Utility Functions  <-----------------------//

void enable(void) {
  setLED(enable_led_pin, RG_GREEN);
  digitalWrite(PIN_CONTACTOR_GATE, HIGH);
  if (current_wind_down_timer_active) current_wind_down_timer_active = false;
}

void disable(void) {
  digitalWrite(PIN_CONTACTOR_GATE, LOW);
  digitalWrite(PIN_OUTPUT_GATE_1, HIGH);
  digitalWrite(PIN_LED_ENABLE_G, LOW);
  if (!fault) digitalWrite(PIN_LED_ENABLE_R, LOW);
  if (enabled) {
    enabled = false;
    running = false;
    paused = false;
    current_wind_down_TS_ms = millis();
    current_wind_down_timer_active = true;
  }
}

void setLEDrgb(uint32_t *LED_pins, uint32_t colour, bool invert) {
  uint8_t r = (colour >> 16) & 0xFF;
  uint8_t g = (colour >> 8) & 0xFF;
  uint8_t b = colour & 0xFF;

  if (invert) {
    r = 0xFF - r;
    g = 0xFF - g;
    b = 0xFF - b;
  }

  analogWrite(LED_pins[0], r);
  analogWrite(LED_pins[1], g);
  analogWrite(LED_pins[2], b);
}

void setLED(uint32_t *LED_pins, uint8_t colour) {
  bool r = colour & 1;
  bool g = (colour >> 1) & 1;
  digitalWrite(LED_pins[0], r);
  digitalWrite(LED_pins[1], g);
}

void saveFlash(void) {
  flash_data data;
  data.initialised = true;
  for (int i = 0; i < 4; i++) data.interlock_enabled[i] = interlock_enabled[i];
  data.amps_per_volt = amps_per_volt;
  data.enable_timeout_time_ms = enable_timeout_time_ms;
  data.pause_timeout_time_ms = pause_timeout_time_ms;
  data.current_threshold = current_threshold;
  data.current_multiplier = current_multiplier;
  flash.write(data);
  settings_changed = false;
}

void faultHandler(uint32_t _fault) {
  fault_ptr = _fault;
  disable();
  if (_fault) {
    digitalWrite(PIN_OUTPUT_GATE_2, LOW);
    fault = true;
  } else fault = false;
  fault_status_changed = true;
  if (debug) {
    Serial.print("Fault -> ");
    Serial.println(fault_msg[fault_ptr]);
  }
}

//------------------------>  Setup Functions  <------------------------//

void setupPins(void) {
  pinMode(PIN_V_SUPPLY_FB, INPUT);
  pinMode(PIN_CT_ADC_IN, INPUT);
  pinMode(PIN_ENABLE_IN_DRAIN, INPUT_PULLUP);
  pinMode(PIN_INTERLOCK_IN_DRAIN_1, INPUT_PULLUP);
  pinMode(PIN_INTERLOCK_IN_DRAIN_2, INPUT_PULLUP);
  pinMode(PIN_INTERLOCK_IN_DRAIN_3, INPUT_PULLUP);
  pinMode(PIN_INTERLOCK_IN_DRAIN_4, INPUT_PULLUP);

  pinMode(PIN_CONTACTOR_GATE, OUTPUT);
  pinMode(PIN_OUTPUT_GATE_1, OUTPUT);
  pinMode(PIN_OUTPUT_GATE_2, OUTPUT);
  pinMode(PIN_LED_STATUS_R, OUTPUT);
  pinMode(PIN_LED_ENABLE_G, OUTPUT);
  pinMode(PIN_LED_ENABLE_R, OUTPUT);
  pinMode(PIN_LED_INTERLOCK_G, OUTPUT);
  pinMode(PIN_LED_INTERLOCK_R, OUTPUT);

  digitalWrite(PIN_CONTACTOR_GATE, LOW);
  digitalWrite(PIN_OUTPUT_GATE_1, HIGH);
  digitalWrite(PIN_OUTPUT_GATE_2, HIGH);
  digitalWrite(PIN_LED_STATUS_R, HIGH);
  digitalWrite(PIN_LED_ENABLE_G, LOW);
  digitalWrite(PIN_LED_ENABLE_R, LOW);
  digitalWrite(PIN_LED_INTERLOCK_G, LOW);
  digitalWrite(PIN_LED_INTERLOCK_R, LOW);

  analogReadResolution(12);
}

void setupFlash(void) {
  // Get data from flash and check whether data is valid
  flash_data data = flash.read();
  bool flash_initialised = true;
  if (data.initialised) {  // if inisitised flag is set, check data is within safe limits
    if (debug) Serial.println("Getting persistent config data from flash...");
    if (data.amps_per_volt > 1000) flash_initialised = false;
    else if (data.enable_timeout_time_ms > 3600000) flash_initialised = false;
    else if (data.pause_timeout_time_ms > 3600000) flash_initialised = false;
    else if (data.current_threshold < 0.1 || data.current_threshold > 1000) flash_initialised = false;
    if (debug) {
      Serial.printf("Got data:\nInitialised: %s\nAmps per Volt: %i\nEnable timeout: %i\nPause timeout: %i\nCurrent threshold: ", data.initialised ? "true" : "false", data.amps_per_volt, data.enable_timeout_time_ms, data.pause_timeout_time_ms);
      Serial.println(data.current_threshold);
    }
  } else flash_initialised = false;

  if (!flash_initialised) {
    if (debug) Serial.println("Flash not yet initialised, storing default values");
    saveFlash();
  } else {
    for (int i = 0; i < 4; i++) interlock_enabled[i] = data.interlock_enabled[i];
    amps_per_volt = data.amps_per_volt;
    enable_timeout_time_ms = data.enable_timeout_time_ms;
    pause_timeout_time_ms = data.pause_timeout_time_ms;
    current_threshold = data.current_threshold;
    current_multiplier = data.current_multiplier;
  }
  interlocksHandler();  // Get initial pin states and set interlock status now that interlock enable settings have been loaded
}

void setupHWInterrupts(void) {
  attachInterrupt(PIN_ENABLE_IN_DRAIN, enable_input_active_CB, RISING);
}

void setupComms(void) {
  Serial.begin(115200);
  if (debug && digitalRead(PIN_ENABLE_IN_DRAIN)) {
    uint32_t serial_retries = 100;
    while (!Serial) {
      delay(100);
      serial_retries--;
      if (serial_retries == 0) {
        debug = false;
        break;
      }
    }
    if (debug) {
      Serial.println("Starting Interlock Safety Controller in debug mode");
      Serial.println(String("Interlocks are currently ") + String(interlocks_sealed ? "sealed" : "unsealed"));
    }
  } else debug = false;
}

//-------------------->  Terminal Menu Functions  <--------------------//

// Write the main menu to terminal
void termWriteMain(void) {
  char col_37[10];
  term_col(37).toCharArray(col_37, 10);
  term_buffer_ptr = 0;
  Serial.printf("\fInterlock safety controller v%s\n\n", VERSION_FW);
  Serial.printf("Enter item number to edit\n\n");
  Serial.printf("1 - Configure interlocks\n");
  Serial.printf("2 - Enable/Pre-start timeout%s%is\n", col_37, enable_timeout_time_ms / 1000);
  Serial.printf("3 - Pause/Post-stop timeout%s%is\n", col_37, pause_timeout_time_ms / 1000);
  Serial.printf("4 - Run current threshold%s", col_37);
  Serial.print(current_threshold);
  Serial.println("A");
  Serial.printf("5 - CT Amps per Volt%s", col_37);
  Serial.print(amps_per_volt);
  Serial.println("A/1V");
  Serial.printf("6 - Current calibration multiplier%s", col_37);
  Serial.println(current_multiplier);
  Serial.printf("\nCurrent meausurement now:%s", col_37);
  Serial.print(ct_amps);
  Serial.println("A");
  Serial.print(term_cond_colour_str(fault_msg[fault_ptr], fault_ptr, ANSI_GREEN, ANSI_RED));
  menu_page = MENU_MAIN;
  Serial.flush();
}

// Write the interlock config page to terminal
void termWrite1(void) {
  char term_str[300];
  snprintf(term_str, 300, "\f1 - Configure interlocks\n\n"
                          "Enter interlock number to toggle enable/disable\n\n"
                          "Interlock 1: %s\n"
                          "Interlock 2: %s\n"
                          "Interlock 3: %s\n"
                          "Interlock 4: %s\n\n"
                          "Type 's' to save to flash, 'b' to go back\n",
           interlock_enabled[0] ? "Enabled" : "Disabled", interlock_enabled[1] ? "Enabled" : "Disabled", interlock_enabled[2] ? "Enabled" : "Disabled", interlock_enabled[3] ? "Enabled" : "Disabled");
  Serial.print(term_str);
  menu_page = MENU_INTERLOCKS;
  Serial.flush();
}

// Write the enable timer config page to terminal
void termWrite2(void) {
  String term_str = term_cls() + "2 - Enable/Pre-start timeout\n\n"
                                 "Current value configured:"
                    + term_col(37) + (String)(enable_timeout_time_ms / 1000) + "s\n\n"
                                                                               "Enter new value in seconds:\n\n"
                                                                               "Type 's' to save to flash, 'b' to go back";
  Serial.print(term_str);
  Serial.print(term_cursor(5, 37));
  menu_page = MENU_ENABLE_TIME;
}

// Write the pause timer config page to terminal
void termWrite3(void) {
  String term_str = term_cls() + "3 - Pause/Post-stop timeout\n\n"
                                 "Current value configured:"
                    + term_col(37) + (String)(pause_timeout_time_ms / 1000) + "s\n\n"
                                                                              "Enter new value in seconds:\n\n"
                                                                              "Type 's' to save to flash, 'b' to go back";
  Serial.print(term_str);
  Serial.print(term_cursor(5, 37));
  menu_page = MENU_PAUSE_TIME;
}

// Write the run current threshold config page to terminal
void termWrite4(void) {
  String term_str = term_cls() + "4 - Run current threshold\n\n"
                                 "Current value configured:"
                    + term_col(37) + (String)current_threshold + "A\n\n"
                                                                 "Enter new value in Amps:\n\n"
                                                                 "Type 's' to save to flash, 'b' to go back";
  Serial.print(term_str);
  Serial.print(term_cursor(5, 37));
  menu_page = MENU_CURRENT_THRESH;
}

// Write the CT ratio config page to terminal
void termWrite5(void) {
  String term_str = term_cls() + "5 - CT Amps per Volt\n\n"
                                 "Current value configured:"
                    + term_col(37) + (String)amps_per_volt + "A/1V\n\n"
                                                             "Enter new value in Amps:\n\n"
                                                             "Type 's' to save to flash, 'b' to go back";
  Serial.print(term_str);
  Serial.print(term_cursor(5, 37));
  menu_page = MENU_CT_A_PER_V;
}

// Write the current calibration config page to terminal
void termWrite6(void) {
  String term_str = term_cls() + "6 - Current calibration	multiplier\n\n"
                                 "Current measurement now:"
                    + term_col(37) + (String)ct_amps + "A\n\n"
                                                       "Enter actual value in Amps:\n\n"
                                                       "Type 's' to save to flash, 'b' to go back";
  Serial.print(term_str);
  Serial.print(term_cursor(5, 37));
  menu_page = MENU_CT_CAL;
}