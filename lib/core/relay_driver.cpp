#include "relay_driver.h"
#include "adapter.h"
#include <stdint.h>
#include <stdio.h>

uint8_t relay_toogle_count = 0;
bool is_relay_enabled = false;

char message_buffer[64] = {0};
unsigned long update_started_at_micros = 0;
unsigned long previous_relay_update_micros = 0;
volatile unsigned long current_relay_update_micros = 0;
volatile unsigned long current_transistor_update_micros = 0;
unsigned long total_relay_update_duration = 0;
unsigned long total_transistor_update_duration = 0;
unsigned long avarage_relay_update_duration = 0;
unsigned long avarage_transistor_update_duration = 0;

void turn_on_relay();
void turn_off_relay();
bool try_count_stats();
void on_relay_updated();
void on_transistor_updated();

void setup_relay_driver() {
  setup_pin(RELAY_DRIVER_PIN, pin_mode::OUTPUT_MODE);
  setup_pin(RELAY_INPUT_PIN, pin_mode::INPUT_PULLUP_MODE);
  setup_pin(TRANSISTOR_INPUT_PIN, pin_mode::INPUT_PULLUP_MODE);
  attach_interrupt(RELAY_INPUT_PIN, on_relay_updated,
                   voltage_state::CHANGE_STATE);
  attach_interrupt(TRANSISTOR_INPUT_PIN, on_transistor_updated,
                   voltage_state::CHANGE_STATE);
}

void toogle_relay() {

  if (try_count_stats()) {
    snprintf(message_buffer, sizeof(message_buffer),
             "Average transistor update time : "
             "%lu, relay update time: %lu",
             avarage_transistor_update_duration, avarage_relay_update_duration);
    log_message(message_buffer);
  }

  if (relay_toogle_count >= RELAY_TOGGLE_MAX_COUNT) {
    return;
  }
  if (get_micros_from_start() - update_started_at_micros <
      RELAY_TOGGLE_DELAY_MICROS) {
    return;
  }

  relay_toogle_count++;

  update_started_at_micros = get_micros_from_start();

  if (is_relay_enabled) {
    turn_off_relay();
  } else {
    turn_on_relay();
  }

  is_relay_enabled = !is_relay_enabled;
}

void turn_on_relay() {
  change_pin_state(RELAY_DRIVER_PIN, pin_state::HIGH_STATE);
}

void turn_off_relay() {
  change_pin_state(RELAY_DRIVER_PIN, pin_state::LOW_STATE);
}

void on_relay_updated() {
  current_relay_update_micros = get_micros_from_start();
}
void on_transistor_updated() {
  current_transistor_update_micros = get_micros_from_start();
}

bool try_count_stats() {
  if (current_relay_update_micros - previous_relay_update_micros <=
      DEBOUNCE_DELAY_MICROS) {
    return false;
  }

  total_transistor_update_duration +=
      current_transistor_update_micros - update_started_at_micros;
  total_relay_update_duration +=
      current_relay_update_micros - current_transistor_update_micros;
  avarage_transistor_update_duration =
      total_transistor_update_duration / relay_toogle_count;
  avarage_relay_update_duration =
      total_relay_update_duration / relay_toogle_count;

  previous_relay_update_micros = current_relay_update_micros;
  return true;
}