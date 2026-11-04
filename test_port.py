from pathlib import Path
import hashlib
import json
import re
import unittest
import importlib.util

ROOT=Path(__file__).parent
class Tests(unittest.TestCase):
    def test_payload_integrity_and_chat_limits(self):
        raw=(ROOT/'payload.txt').read_bytes()
        info=json.loads((ROOT/'payload-info.json').read_text())
        self.assertEqual(hashlib.sha256(raw).hexdigest(),info['sha256'])
        lines=raw.decode().splitlines()
        self.assertEqual(len(lines),87)
        self.assertTrue(all(0<len(line)<=254 and all(32<=ord(c)<=126 for c in line) for line in lines))
    def test_macro_matches_every_payload_character(self):
        spec=importlib.util.spec_from_file_location('macro_builder',ROOT/'tools/payload_to_macro.py')
        builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder)
        source=(ROOT/'CivTyper/macro_script.h').read_text()
        # Each batch's comments precede its generated keystrokes.
        batches=re.split(r'    // Batch \d+: type ',source)[1:]
        lines=(ROOT/'payload.txt').read_text().splitlines()
        self.assertEqual(len(batches),len(lines))
        for batch,line in zip(batches,lines):
            typed=batch.split('// Press Esc, then gamepad A',1)[1].split('// Press Enter twice',1)[0]
            keys=re.findall(r'KEYS\(32, ([^,]+), ([^,]+),',typed)
            self.assertEqual(keys[0],('0','HID_KEY_ESCAPE'))
            expected=[('KEYBOARD_MODIFIER_LEFTSHIFT' if builder.ASCII_TO_KEY[c][0] else '0',builder.ASCII_TO_KEY[c][1]) for c in line]
            self.assertEqual(keys[1:],expected)
            self.assertEqual(len(re.findall(r'KEYS\(32, 0, HID_KEY_ENTER,',batch)),2)
        self.assertEqual(source.count('PAD_BTN(50, SWITCH_USB_MASK_A)'),178)
    def test_two_interfaces_and_manual_start(self):
        firmware=(ROOT/'CivTyper/CivTyper.ino').read_text()
        driver=(ROOT/'CivTyper/hid_device_dual.c').read_text()
        self.assertIn('#define CFG_TUD_HID 2',driver)
        self.assertIn('bool running=false',firmware)
        self.assertIn('command=="START"&&!running&&tud_mounted()',firmware)
        self.assertIn('stopMacro("USB report timeout")',firmware)
if __name__=='__main__':unittest.main()
