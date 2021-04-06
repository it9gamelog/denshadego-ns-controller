/*
 * Pop'n Arcade Controller
 * https://github.com/it9gamelog/popncontroller
 */

#include "NintendoSwitchController.h"


/*
 * VID: 0F0D, PID: 0092 is the IDs for the Pokken Controller made by HORI Co.
 * We must sent this VID, PID or else Nintendo Switch would not recognize.
 * 
 * The HID descriptor doesn't matter to NS (but matters to a sensible OS)
 * 
 * NS will always expected 1 input and 1 output endpoint.
 * 
 * NS also expects the payload in the fixed format, regardless what the HID says.
 * I am keeping the HID descriptor  unchanged (from the internet) for debugging, 
 * or using the hardware on normal PC.
 * 
 * We have found no use for the output (it mirrors to the input according to the internet), 
 *    but the EP must be defined. The buffer needed not to be cleared though. * 
 */
 
const DeviceDescriptor USB_DeviceDescriptorIAD PROGMEM =
  D_DEVICE(0xEF,0x02,0x01,64,0x0F0D,0x0092,0x100,IMANUFACTURER,IPRODUCT,ISERIAL,1);

static const uint8_t _hidReportDescriptor[] PROGMEM = {
  0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
  0x09, 0x05,        // Usage (Game Pad)
  0xA1, 0x01,        // Collection (Application)
  
  // Buttons
  0x15, 0x00,        //   Logical Minimum (0)
  0x25, 0x01,        //   Logical Maximum (1)
  0x35, 0x00,        //   Physical Minimum (0)
  0x45, 0x01,        //   Physical Maximum (1)
  0x75, 0x01,        //   Report Size (1)
  0x95, 0x10,        //   Report Count (16)
  0x05, 0x09,        //   Usage Page (Button)
  0x19, 0x01,        //   Usage Minimum (0x01)
  0x29, 0x10,        //   Usage Maximum (0x10)
  0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

  // HAT (4-bit)
  0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
  0x25, 0x07,        //   Logical Maximum (7)
  0x46, 0x3B, 0x01,  //   Physical Maximum (315)
  0x75, 0x04,        //   Report Size (4)
  0x95, 0x01,        //   Report Count (1)
  0x65, 0x14,        //   Unit (System: English Rotation, Length: Centimeter)
  0x09, 0x39,        //   Usage (Hat switch)
  0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
  // Placeholder (4-bit)
  0x65, 0x00,        //   Unit (None)
  0x95, 0x01,        //   Report Count (1)
  0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  // Stick
  0x26, 0xFF, 0x00,  //   Logical Maximum (255)
  0x46, 0xFF, 0x00,  //   Physical Maximum (255)
  0x09, 0x30,        //   Usage (X)
  0x09, 0x31,        //   Usage (Y)
  0x09, 0x32,        //   Usage (Z)
  0x09, 0x35,        //   Usage (Rz)
  0x75, 0x08,        //   Report Size (8)
  0x95, 0x04,        //   Report Count (4)
  0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  // Vendor
  0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
  0x09, 0x20,        //   Usage (0x20)
  0x95, 0x01,        //   Report Count (1)
  0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  // Output
  0x0A, 0x21, 0x26,  //   Usage (0x2621)
  0x95, 0x08,        //   Report Count (8)
  0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
  0xC0,              // End Collection  
  // 85 bytes
};

NintendoSwitchController_::NintendoSwitchController_(void) : PluggableUSBModule(2, 1, epType)
{
  epType[0] = EP_TYPE_INTERRUPT_IN;
  epType[1] = EP_TYPE_INTERRUPT_OUT;
  PluggableUSB().plug(this);
  
  report.buttons = 0;
  report.hat = HAT_CENTER;
  report.lx = STICK_CENTER;
  report.ly = STICK_CENTER;
  report.rx = STICK_CENTER;
  report.ry = STICK_CENTER;
  memcpy(&last_report, &report, sizeof(NintendoSwitch_Report_t));
}

int NintendoSwitchController_::getInterface(uint8_t* interfaceCount)
{  
  *interfaceCount += 1;
  
  NintendoSwitchDescriptor desc = {
    D_INTERFACE(pluggedInterface, 2, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
    D_HIDREPORT(sizeof(_hidReportDescriptor)),
    
    D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01),
    // Keep the OUT endpoint even if we aren't using. NS is expecting it.
    D_ENDPOINT(USB_ENDPOINT_OUT(pluggedEndpoint+1), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
  };
  return USB_SendControl(0, &desc, sizeof(desc));
}

int NintendoSwitchController_::getDescriptor(USBSetup& setup)
{
  // Sending our own Device Descriptior (PID and VID) by overriding Arduiro Core behavior
  if (setup.wValueH == USB_DEVICE_DESCRIPTOR_TYPE) {
    USB_SendControl(TRANSFER_PGM,&USB_DeviceDescriptorIAD,sizeof(USB_DeviceDescriptorIAD));
    return 1;
  }
  
  // Check if this is a HID Class Descriptor request
  if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) {
    return 0;
  }
  if (setup.wValueH != HID_REPORT_DESCRIPTOR_TYPE) {
    return 0;
  }

  // In a HID Class Descriptor wIndex cointains the interface number
  if (setup.wIndex != pluggedInterface) {
    return 0;
  }

  // Reset the protocol on reenumeration. Normally the host should not assume the state of the protocol
  // due to the USB specs, but Windows and Linux just assumes its in report mode.
  protocol = HID_REPORT_PROTOCOL;

  return USB_SendControl(TRANSFER_PGM, _hidReportDescriptor, sizeof(_hidReportDescriptor));
}

bool NintendoSwitchController_::setup(USBSetup& setup)
{
  if (pluggedInterface != setup.wIndex) {
    return false;
  }

  uint8_t request = setup.bRequest;
  uint8_t requestType = setup.bmRequestType;

  if (requestType == REQUEST_DEVICETOHOST_CLASS_INTERFACE)
  {
    if (request == HID_GET_REPORT) {
      // TODO: HID_GetReport();
      return true;
    }
    if (request == HID_GET_PROTOCOL) {
      // TODO: Send8(protocol);
      return true;
    }
  }

  if (requestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE)
  {
    if (request == HID_SET_PROTOCOL) {
      protocol = setup.wValueL;
      return true;
    }
    if (request == HID_SET_IDLE) {
      idle = setup.wValueL;
      return true;
    }
    if (request == HID_SET_REPORT)
    {
      // Check if data has the correct length
      auto length = setup.wLength;
      if (length == sizeof(incoming)) {
        USB_RecvControl(&incoming, length);
      }
      return true;
    }
  }

  return false;
}

void NintendoSwitchController_::press(uint16_t buttons) {
  report.buttons |= buttons;
}

void NintendoSwitchController_::release(uint16_t buttons) {
  report.buttons &= ~buttons;
}

void NintendoSwitchController_::setHat(uint8_t hat) {
  report.hat = hat;
}

void NintendoSwitchController_::setLx(uint8_t pos) {
  report.lx = pos;
}

void NintendoSwitchController_::setLy(uint8_t pos) {
  report.ly = pos;
}

void NintendoSwitchController_::setRx(uint8_t pos) {
  report.rx = pos;
}

void NintendoSwitchController_::setRy(uint8_t pos) {
  report.ry = pos;
}

void NintendoSwitchController_::send(void) {
  if (memcmp(&last_report, &report, sizeof(NintendoSwitch_Report_t)) != 0) {
    USB_Send(pluggedEndpoint | TRANSFER_RELEASE, &report, sizeof(report));
    memcpy(&last_report, &report, sizeof(NintendoSwitch_Report_t));
  }
}

void NintendoSwitchController_::recv(void) {
  // In case we really want to do something with it later on.
  while (USB_Recv(pluggedEndpoint + 1) >= 0);
}

NintendoSwitchController_ NintendoSwitchController;
