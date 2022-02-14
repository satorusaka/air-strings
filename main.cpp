#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <array>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "bsp/board.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#include "tusb.h"
#include "usb_descriptors.h"

#include "ws2812.pio.h"
#include <PicoLed.hpp>

//--------------------------------------------------------------------+

#define LED_PIN1 10
#define LED_PIN2 21
#define LED_LENGTH 3
#define LED_BRIGHTNESS 80

#define NUM_INPUTS 6
uint8_t SENSOR[NUM_INPUTS] = {11, 20, 13, 19, 12, 18};
uint8_t KEYS[NUM_INPUTS] = {HID_KEY_1, HID_KEY_2, HID_KEY_3, HID_KEY_4, HID_KEY_5, HID_KEY_6};

#define FLAG_VALUE 123 // for multicore

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+
void hid_task(void);

/*------------- Core1 MAIN -------------*/
void core1_main()
{
  sleep_ms(500);
  auto ledStrip1 = PicoLed::addLeds<PicoLed::WS2812B>(pio0, 0, LED_PIN1, LED_LENGTH, PicoLed::FORMAT_GRB);
  ledStrip1.setBrightness(LED_BRIGHTNESS);

  // auto ledStrip2 = PicoLed::addLeds<PicoLed::WS2812B>(pio1, 0, LED_PIN2, LED_LENGTH, PicoLed::FORMAT_GRB);
  // ledStrip2.setBrightness(LED_BRIGHTNESS);

  // std::array strips{ledStrip1, ledStrip2};

  while (1)
  {
    ledStrip1.fill(PicoLed::RGB(0, 0, 255));
    ledStrip1.show();
    sleep_ms(500);
    ledStrip1.fill(PicoLed::RGB(255, 0, 0));
    ledStrip1.show();

    // for (size_t i = 0; i < strips.size(); i++)
    // {
    //   strips[i].fill(PicoLed::RGB(255, 255, 255));
    //   strips[i].show();
    // }
    sleep_ms(10);
  }
}

/*------------- MAIN -------------*/
int main(void)
{
  // core1 init
  multicore_launch_core1(core1_main);
  // uint32_t g = multicore_fifo_pop_blocking(); // Wait for it to start up

  // Initialize the gpio
  for (int i = 0; i < NUM_INPUTS; i++)
  {
    gpio_init(SENSOR[i]);
    gpio_set_dir(SENSOR[i], GPIO_IN);
    gpio_pull_up(SENSOR[i]);
  }

  // Initialize the board
  board_init();
  tusb_init();

  while (1)
  {
    tud_task(); // tinyusb device task
    hid_task();
  }

  return 0;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id)
{
  // skip if hid is not ready yet
  if (!tud_hid_ready())
    return;

  // use to avoid send multiple consecutive zero report for keyboard
  static bool has_keyboard_key = false;

  uint8_t keycode[NUM_INPUTS] = {0};

  int keypresscount = 0;
  for (int i = 0; i < NUM_INPUTS; i++)
  {
    if (gpio_get(SENSOR[i]) != 0)
    {
      keycode[keypresscount] = KEYS[i];
      keypresscount++;
    }
  }

  if (keypresscount != 0)
  {
    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
    has_keyboard_key = true;
    board_led_write(has_keyboard_key);
  }
  else
  {
    // send empty key report if previously has key pressed
    if (has_keyboard_key)
      tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
    has_keyboard_key = false;
    board_led_write(has_keyboard_key);
  }
}

// Every 3ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
  // Poll every 3ms
  const uint32_t interval_ms = 3;
  static uint32_t start_ms = 0;

  // check if time as passed
  if (board_millis() - start_ms < interval_ms)
    return; // not enough time
  start_ms += interval_ms;

  // Remote wakeup
  if (tud_suspended())
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }
  else
  {
    // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_hid_report(REPORT_ID_KEYBOARD);
  }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint8_t len)
{
  (void)instance;
  (void)len;
  send_hid_report(REPORT_ID_KEYBOARD);
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
  (void)instance;
}
