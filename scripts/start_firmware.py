#!/usr/bin/env python3
"""Start ESP32-S3 firmware after flashing through its native USB bootloader."""
import argparse

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--port', required=True, help='Bootloader port, e.g. COM5 or /dev/cu.usbmodem1101')
args = parser.parse_args()

import serial
from esptool.targets.esp32s3 import ESP32S3ROM

port = serial.Serial(port=None, baudrate=115200, timeout=1)
port.dtr = False
port.rts = False
port.port = args.port
try:
    port.open()
    esp = ESP32S3ROM(port)
    esp.connect('no_reset')
    esp.write_reg(esp.RTC_CNTL_OPTION1_REG, 0, esp.RTC_CNTL_FORCE_DOWNLOAD_BOOT_MASK)
    esp.watchdog_reset()
    print('Firmware start requested. The USB port may change after reboot.')
finally:
    port.close()
