#include <Arduino.h>
#include "USB.h"
#include "esp32-hal-tinyusb.h"
#include "tusb.h"
#include "macro_script.h"
#include "test_script.h"
const MacroStep *activeScript=MACRO_SCRIPT;
size_t activeCount=sizeof(MACRO_SCRIPT)/sizeof(MacroStep);

// Dedicated CivSwitch firmware. Never starts typing on power-on.
bool running=false, releasing=true, sent=false;
size_t stepIndex=0;
uint32_t deadline=0, blockedAt=0;
uint8_t sentMask=0;
volatile bool capsKnown=false, capsOn=false;
uint8_t capsStage=0;
uint32_t capsDeadline=0;
String command, message="Ready";
bool bootRaw=false, bootStable=false, bootConsumed=true;
uint32_t bootChangedAt=0, bootPressedAt=0;
const SwitchUsbReport neutralPad={0,8,128,128,128,128,0};
const KeyboardReport neutralKeys={0,0,{0,0,0,0,0,0}};

uint16_t descriptors(uint8_t *dst,uint8_t *itf) {
  // Upstream's two HID interfaces, with no report IDs.
  memcpy(dst,switch_usb_configuration_descriptor+9,sizeof(switch_usb_configuration_descriptor)-9);
  dst[2]=*itf;dst[34]=*itf+1;*itf+=2;
  return sizeof(switch_usb_configuration_descriptor)-9;
}
extern "C" const uint8_t *tud_hid_descriptor_report_cb(uint8_t instance) {
  return instance==0?switch_usb_report_descriptor:keyboard_usb_report_descriptor;
}
extern "C" uint16_t tud_hid_get_report_cb(uint8_t,uint8_t,hid_report_type_t,uint8_t*,uint16_t) {return 0;}
extern "C" void tud_hid_set_report_cb(uint8_t instance,uint8_t,hid_report_type_t type,uint8_t const *data,uint16_t length) {
  if(instance==1&&type==HID_REPORT_TYPE_OUTPUT&&length){
    capsOn=(data[0]&KEYBOARD_LED_CAPSLOCK)!=0;
    capsKnown=true;
  }
}

void stopMacro(const char *reason) {
  running=false;releasing=true;sentMask=0;sent=false;message=reason;
}
void startMacro(bool test=false) {
  activeScript=test?TEST_SCRIPT:MACRO_SCRIPT;
  activeCount=test?sizeof(TEST_SCRIPT)/sizeof(MacroStep):sizeof(MACRO_SCRIPT)/sizeof(MacroStep);
  capsStage=0;
  stepIndex=0;sentMask=0;sent=false;releasing=true;running=true;
  deadline=millis()+10000;blockedAt=0;message="Starting in 10 seconds";
  // A button held through START must be released before it can stop the run.
  bootConsumed=true;
}
void pollBoot() {
  const uint32_t now=millis();
  const bool pressed=digitalRead(0)==LOW;
  if(pressed!=bootRaw){bootRaw=pressed;bootChangedAt=now;}
  if(now-bootChangedAt<30)return;
  if(bootStable!=bootRaw){
    bootStable=bootRaw;
    if(bootStable){
      bootPressedAt=now;
      if(running){stopMacro("Stopped with BOOT");bootConsumed=true;}
    }
  }
  if(!bootStable){bootConsumed=false;return;}
  if(!bootConsumed&&!running&&now-bootPressedAt>=2000){
    bootConsumed=true;
    if(tud_mounted())startMacro();
    else message="Connect USB, release BOOT, then hold again";
  }
}
bool sendPair(const SwitchUsbReport &pad,const KeyboardReport &keys) {
  if(!(sentMask&1)&&tud_hid_n_ready(0)&&tud_hid_n_report(0,0,&pad,sizeof(pad)))sentMask|=1;
  if(!(sentMask&2)&&tud_hid_n_ready(1)&&tud_hid_n_report(1,0,&keys,sizeof(keys)))sentMask|=2;
  return sentMask==3;
}
void repeatPair(const SwitchUsbReport &pad,const KeyboardReport &keys) {
  // Pico-style: refresh each interface independently whenever it is ready.
  if(tud_hid_n_ready(0))tud_hid_n_report(0,0,&pad,sizeof(pad));
  if(tud_hid_n_ready(1))tud_hid_n_report(1,0,&keys,sizeof(keys));
}
void status() {
  Serial0.printf("{\"active\":%s,\"usb\":%s,\"step\":%u,\"total\":%u,\"message\":\"%s\"}\n",
    running?"true":"false",tud_mounted()?"true":"false",(unsigned)stepIndex,
    (unsigned)activeCount,message.c_str());
}
void setup() {
  pinMode(0,INPUT_PULLUP);
  Serial0.begin(115200,SERIAL_8N1,44,43);
  USB.VID(0x0f0d);USB.PID(0x0092);USB.usbClass(0);USB.usbSubClass(0);USB.usbProtocol(0);
  USB.manufacturerName("ESP32 CivSwitch");USB.productName("CivSwitch Controller + Keyboard");
  tinyusb_enable_interface(USB_INTERFACE_CUSTOM,sizeof(switch_usb_configuration_descriptor)-9,descriptors);
  USB.begin();
}
void loop() {
  unsigned budget=128;
  while(Serial0.available()&&budget--) {
    char c=Serial0.read();
    if(c=='\n') {
      if(command=="STATUS")status();
      else if(command=="STOP") {stopMacro("Stopped");status();}
      else if((command=="START"||command=="TEST")&&!running&&tud_mounted()) {
        startMacro(command=="TEST");status();
      } else Serial0.println("{\"error\":\"Busy, USB disconnected, or unknown command\"}");
      command="";
    } else if(c!='\r') {if(command.length()<32)command+=c;else command="INVALID";}
  }
  pollBoot();
  if(!tud_mounted()) {if(running)stopMacro("USB disconnected");delay(1);return;}
  if(releasing) {
    if(sendPair(neutralPad,neutralKeys)){releasing=false;sentMask=0;}
    delay(1);return;
  }
  if(!running){repeatPair(neutralPad,neutralKeys);delay(1);return;}
  if((int32_t)(millis()-deadline)<0){
    if(sent)repeatPair(activeScript[stepIndex].gamepad,activeScript[stepIndex].keyboard);
    else repeatPair(neutralPad,neutralKeys);
    delay(1);return;
  }
  if(sent){stepIndex++;sent=false;sentMask=0;}
  if(capsStage<3){
    if(!capsKnown){stopMacro("Caps state unavailable; toggle Caps Lock with a keyboard then retry");return;}
    if(capsStage==0){
      if(!capsOn){capsStage=3;}
      else {
        const KeyboardReport toggle={0,0,{HID_KEY_CAPS_LOCK,0,0,0,0,0}};
        if(!sendPair(neutralPad,toggle)){delay(1);return;}
        sentMask=0;capsDeadline=millis()+80;capsStage=1;
      }
    }
    if(capsStage==1){
      if((int32_t)(millis()-capsDeadline)<0){delay(1);return;}
      if(!sendPair(neutralPad,neutralKeys)){delay(1);return;}
      sentMask=0;capsDeadline=millis()+1000;capsStage=2;
    }
    if(capsStage==2){
      if(capsOn){
        if((int32_t)(millis()-capsDeadline)>=0)stopMacro("Caps Lock did not turn off; check keyboard settings");
        delay(1);return;
      }
      capsStage=3;
    }
  }
  const MacroStep &s=activeScript[stepIndex];
  if(s.op==MACRO_OP_END){stopMacro("Completed; verify loader on console");return;}
  if(s.op==MACRO_OP_KEY_TIMES){stopMacro("Unsupported macro operation");return;}
  if(sendPair(s.gamepad,s.keyboard)) {
    // Wait from actual transmission; never skip steps when USB is delayed.
    deadline=millis()+s.durationMs;sent=true;blockedAt=0;message="Running";
  } else {
    if(!blockedAt)blockedAt=millis();
    if(millis()-blockedAt>2000)stopMacro("USB report timeout");
  }
  delay(1);
}




