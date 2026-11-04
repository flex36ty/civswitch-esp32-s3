// GENERATED from payload.txt by tools/payload_to_macro.py.
// Do not edit by hand - edit payload.txt and rebuild instead.
#ifndef test_script_h
#define test_script_h

#include "MacroEngine.h"

static const MacroStep TEST_SCRIPT[] = {
    // Preamble: press A 5 times
    PAD_BTN(50, SWITCH_USB_MASK_A),
    WAIT(550),
    PAD_BTN(50, SWITCH_USB_MASK_A),
    WAIT(550),
    PAD_BTN(50, SWITCH_USB_MASK_A),
    WAIT(550),
    PAD_BTN(50, SWITCH_USB_MASK_A),
    WAIT(550),
    PAD_BTN(50, SWITCH_USB_MASK_A),
    WAIT(550),

    // Batch 1: type "1/2 TEST Alpha 123"
    // Press Esc, then gamepad A
    KEYS(32, 0, HID_KEY_ESCAPE, 0, 0, 0, 0, 0),
    WAIT(750),
    PAD_BTN(50, SWITCH_USB_MASK_A),
    WAIT(2000),

    KEYS(32, 0, HID_KEY_1, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_SLASH, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_2, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_SPACE, 0, 0, 0, 0, 0),
    KEYS(32, KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_T, 0, 0, 0, 0, 0),
    KEYS(32, KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_E, 0, 0, 0, 0, 0),
    KEYS(32, KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_S, 0, 0, 0, 0, 0),
    KEYS(32, KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_T, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_SPACE, 0, 0, 0, 0, 0),
    KEYS(32, KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_A, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_L, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_P, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_H, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_A, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_SPACE, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_1, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_2, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_3, 0, 0, 0, 0, 0),

    // Press Enter twice
    KEYS(32, 0, HID_KEY_ENTER, 0, 0, 0, 0, 0),
    WAIT(200),
    KEYS(32, 0, HID_KEY_ENTER, 0, 0, 0, 0, 0),

    WAIT(750),
    PAD_BTN(50, SWITCH_USB_MASK_A),
    WAIT(2000),

    // Batch 2: type "2/2 TEST Bravo 456"
    // Press Esc, then gamepad A
    KEYS(32, 0, HID_KEY_ESCAPE, 0, 0, 0, 0, 0),
    WAIT(750),
    PAD_BTN(50, SWITCH_USB_MASK_A),
    WAIT(2000),

    KEYS(32, 0, HID_KEY_2, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_SLASH, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_2, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_SPACE, 0, 0, 0, 0, 0),
    KEYS(32, KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_T, 0, 0, 0, 0, 0),
    KEYS(32, KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_E, 0, 0, 0, 0, 0),
    KEYS(32, KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_S, 0, 0, 0, 0, 0),
    KEYS(32, KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_T, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_SPACE, 0, 0, 0, 0, 0),
    KEYS(32, KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_B, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_R, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_A, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_V, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_O, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_SPACE, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_4, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_5, 0, 0, 0, 0, 0),
    KEYS(32, 0, HID_KEY_6, 0, 0, 0, 0, 0),

    // Press Enter twice
    KEYS(32, 0, HID_KEY_ENTER, 0, 0, 0, 0, 0),
    WAIT(200),
    KEYS(32, 0, HID_KEY_ENTER, 0, 0, 0, 0, 0),

    END(),
};

#endif

