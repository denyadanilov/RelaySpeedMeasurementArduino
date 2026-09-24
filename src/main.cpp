#include "adapter.h"
#include "relay_driver.h"

void setup() {
  initialize_logger();
  setup_relay_driver();
}

void loop() { toogle_relay(); }