/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"
#include "bsp/board_api.h"
#include "tusb.h"
#include "mpu6050.h"
#include "usb_descriptors.h"
#include "ssd1306.h"

// I2C Defines
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15

// MPU6050 defines
#define MPU6050_ADDRESS 0x68 // 7bit i2c address

// General read/write functions for I2C
void setPin(char address, char reg, char value) {
    char data[2] = {reg, value};
    i2c_write_blocking(I2C_PORT, address, data, 2, false);
}

char readPin(char address, char reg) {
    char value;
    i2c_write_blocking(I2C_PORT, address, &reg, 1, true);  // true to keep host control of bus
    i2c_read_blocking(I2C_PORT, address, &value, 1, false);  // false - finished with bus
    return value;
}

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;
static float hid_accel_x = 0.0f;
static float hid_accel_y = 0.0f;
static int mode_toggle = 0;

void led_blinking_task(void);
void hid_task(void);

/*------------- MAIN -------------*/
int main(void)
{
  board_init();

  // Initialise the Wi-Fi chip
  if (cyw43_arch_init()) {
      printf("Wi-Fi init failed\n");
      return -1;
  }
  
  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400*1000);
  
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);
  
  // Set up the OLED display
  ssd1306_setup();
  ssd1306_clear();
  ssd1306_update();

  // Check communication with the mpu6050
  char IMU_init =readPin(MPU6050_ADDRESS, WHO_AM_I);
  if (IMU_init == 0x68) {
      char message[26];
      sprintf(message, "MPU6050 recognized");
      ssd1306_drawString(0, 0, message);
      ssd1306_update();
  } else {
      char message[26];
      sprintf(message, "MPU6050 not recognized");
      ssd1306_drawString(0, 0, message);
      sprintf(message, "WHO_AM_I: 0x%X", IMU_init);
      ssd1306_drawString(0, 10, message);
      ssd1306_update();
      while (true) {
          cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
          sleep_ms(100);
      }
  }
  sleep_ms(2000);
  ssd1306_clear();
  ssd1306_update();
  
  // Set up the mpu6050
  mpu6050_setup();
  
  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);

  if (board_init_after_tusb) {
    board_init_after_tusb();
  }

  // Set up gpio for mode toggle button, pull up
  gpio_init(16);
  gpio_set_dir(16, GPIO_IN);
  gpio_pull_up(16);
  gpio_init(17);
  gpio_set_dir(17, GPIO_OUT);
  gpio_put(17, 0);


  while (1)
  {
    
    tud_task(); // tinyusb device task
    led_blinking_task();
    // ssd1306_clear();
    
    // Burst read for all data
    int16_t accel_x, accel_y, accel_z, temp, gyro_x, gyro_y, gyro_z;
    mpu6050_readAll(&accel_x, &accel_y, &accel_z, &temp, &gyro_x, &gyro_y, &gyro_z);

    // convert to real units
    float accel_x_f = accel_x * 0.000061;
    float accel_y_f = accel_y * 0.000061;
    float accel_z_f = accel_z * 0.000061;

    float temp_f = temp / 340.0 + 36.53;

    float gyro_x_f = gyro_x * 0.007630;
    float gyro_y_f = gyro_y * 0.007630;
    float gyro_z_f = gyro_z * 0.007630;

    hid_accel_x = accel_x_f;
    hid_accel_y = accel_y_f;

    // // Display section
    // // Line for x axis acceleration
    // int8_t length_x = (int8_t)(accel_x_f * 64); // 128 pixels for the +/- 2g range
    // if (length_x >= 0) {
    //     ssd1306_drawLine_h(64, 16, length_x, 1);
    // } else {
    //     ssd1306_drawLine_h(64 + length_x, 16, -length_x, 1);
    // }
    // // Line for y axis acceleration
    // int8_t length_y = (int8_t)(accel_y_f * -16); // 32 pixels for the +/- 2g range
    // if (length_y >= 0) {
    //     ssd1306_drawLine_v(64, 16, length_y, 1);
    // } else {
    //     ssd1306_drawLine_v(64, 16 + length_y, -length_y, 1);
    // }
    // ssd1306_update();
    if (gpio_get(16) == 0) {
        mode_toggle = 1 - mode_toggle; // toggle between 0 and 1
        gpio_put(17, mode_toggle);
        sleep_ms(300); // debounce delay
    }

    hid_task();
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, uint32_t btn)
{
  // skip if hid is not ready yet
  if ( !tud_hid_ready() ) return;

  switch(report_id)
  {
    case REPORT_ID_KEYBOARD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_keyboard_key = false;

      if ( btn )
      {
        uint8_t keycode[6] = { 0 };
        keycode[0] = HID_KEY_A;

        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
        has_keyboard_key = true;
      }else
      {
        // send empty key report if previously has key pressed
        if (has_keyboard_key) tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        has_keyboard_key = false;
      }
    }
    break;

    case REPORT_ID_MOUSE:
    {
      if (mode_toggle == 0) { // IMU mouse mode
        int8_t delta_x = (int8_t)(hid_accel_x * -64);
        int8_t delta_y = (int8_t)(hid_accel_y * 64);
        if (delta_x > 127) delta_x = 127;
        if (delta_x < -127) delta_x = -127;
        if (delta_y > 127) delta_y = 127;
        if (delta_y < -127) delta_y = -127;

        tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta_x, delta_y, 0, 0);
      } else { // drawing a square
        static uint8_t pos = 0;
        uint8_t delta_x = 0;
        uint8_t delta_y = 0;

        if (pos < 25) {
          delta_x = 5;
        } else if (pos < 50) {
          delta_y = 5;
        } else if (pos < 75) {
          delta_x = -5;
        } else if (pos < 100) {
          delta_y = -5;
        }
        tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta_x, delta_y, 0, 0);
        pos = (pos + 1) % 100;
        
      }
    }
    break;

    case REPORT_ID_CONSUMER_CONTROL:
    {
      // use to avoid send multiple consecutive zero report
      static bool has_consumer_key = false;

      if ( btn )
      {
        // volume down
        uint16_t volume_down = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &volume_down, 2);
        has_consumer_key = true;
      }else
      {
        // send empty key report (release key) if previously has key pressed
        uint16_t empty_key = 0;
        if (has_consumer_key) tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &empty_key, 2);
        has_consumer_key = false;
      }
    }
    break;

    case REPORT_ID_GAMEPAD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_gamepad_key = false;

      hid_gamepad_report_t report =
      {
        .x   = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0,
        .hat = 0, .buttons = 0
      };

      if ( btn )
      {
        report.hat = GAMEPAD_HAT_UP;
        report.buttons = GAMEPAD_BUTTON_A;
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

        has_gamepad_key = true;
      }else
      {
        report.hat = GAMEPAD_HAT_CENTERED;
        report.buttons = 0;
        if (has_gamepad_key) tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
        has_gamepad_key = false;
      }
    }
    break;

    default: break;
  }
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
  // Poll every 10ms
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  uint32_t const btn = board_button_read();

  // Remote wakeup
  if ( tud_suspended() && btn )
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }else
  {
    // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_hid_report(REPORT_ID_MOUSE, btn);
  }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) len;

  uint8_t next_report_id = report[0] + 1u;

  if (next_report_id < REPORT_ID_COUNT)
  {
    send_hid_report(next_report_id, board_button_read());
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD)
    {
      // bufsize should be (at least) 1
      if ( bufsize < 1 ) return;

      uint8_t const kbd_leds = buffer[0];

      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock On: disable blink, turn led on
        blink_interval_ms = 0;
        board_led_write(true);
      }else
      {
        // Caplocks Off: back to normal blink
        board_led_write(false);
        blink_interval_ms = BLINK_MOUNTED;
      }
    }
  }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // blink is disabled
  if (!blink_interval_ms) return;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state);
  led_state = 1 - led_state; // toggle
}
