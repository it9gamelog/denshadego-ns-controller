/*
 * Pop'n Arcade Controller
 * https://github.com/it9gamelog/popncontroller
 */

#pragma once

#include <stdint.h>
#include <Arduino.h>
#include <PluggableUSB.h>
#include <HID.h>

typedef struct
{
  InterfaceDescriptor inf;
  HIDDescDescriptor   desc;
  EndpointDescriptor  in;
  EndpointDescriptor  out;
} NintendoSwitchDescriptor;

typedef enum {
  SWITCH_Y       = 0x01,
  SWITCH_B       = 0x02,
  SWITCH_A       = 0x04,
  SWITCH_X       = 0x08,
  SWITCH_L       = 0x10,
  SWITCH_R       = 0x20,
  SWITCH_ZL      = 0x40,
  SWITCH_ZR      = 0x80,
  SWITCH_MINUS   = 0x100,
  SWITCH_PLUS    = 0x200,
  SWITCH_LCLICK  = 0x400,
  SWITCH_RCLICK  = 0x800,
  SWITCH_HOME    = 0x1000,
  SWITCH_CAPTURE = 0x2000,
} NintendoSwitch_Buttons_t;

typedef enum {
  HAT_TOP           = 0,
  HAT_TOP_RIGHT     = 1,
  HAT_RIGHT         = 2,
  HAT_BOTTOM_RIGHT  = 3,
  HAT_BOTTOM        = 4,
  HAT_BOTTOM_LEFT   = 5,
  HAT_LEFT          = 6,
  HAT_TOP_LEFT      = 7,
  HAT_CENTER        = 8,
} NintendoSwitch_Hat_t;

#define STICK_CENTER 128

typedef struct __attribute__ ((packed)) {
  uint16_t buttons; // 16 buttons; see JoystickButtons_t for bit mapping
  uint8_t  hat;     // HAT switch; one nibble w/ unused nibble
  uint8_t  lx;      // Left  Stick X
  uint8_t  ly;      // Left  Stick Y
  uint8_t  rx;      // Right Stick X
  uint8_t  ry;      // Right Stick Y
  uint8_t  vendor;
} NintendoSwitch_Report_t;

class NintendoSwitchController_ : public PluggableUSBModule
{
  public:
    NintendoSwitchController_(void);
    void press(uint16_t buttons);
    void release(uint16_t buttons);
    void setHat(uint8_t hat);
    void setLx(uint8_t pos);
    void setLy(uint8_t pos);
    void setRx(uint8_t pos);
    void setRy(uint8_t pos);
    void setVendor(uint8_t value);
    void send(void);
    void recv(void);
    
  protected:
    uint8_t epType[2];
    uint8_t protocol;
    uint8_t idle;

    NintendoSwitch_Report_t report, last_report;
    
    struct __attribute__ ((packed)) {
      uint16_t buttons; // 16 buttons; see JoystickButtons_t for bit mapping
      uint8_t  hat;     // HAT switch; one nibble w/ unused nibble
      uint8_t  lx;      // Left  Stick X
      uint8_t  ly;      // Left  Stick Y
      uint8_t  rx;      // Right Stick X
      uint8_t  ry;      // Right Stick Y
    } incoming;
    
    int getInterface(uint8_t* interfaceCount);
    int getDescriptor(USBSetup& setup);
    bool setup(USBSetup& setup);
};

extern NintendoSwitchController_ NintendoSwitchController;
