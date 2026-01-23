#!/bin/bash

# Script to generate QR codes for WiFi credentials and configuration URL
# This script creates PNG images of QR codes and converts them to C header files
# for embedding in the M5Stick-C Plus 2 firmware.

# WiFi access point settings
SSID="stickc"
PASS="helloworld"
IP="192.168.4.1"

# Generate QR code for WiFi credentials
# '-v 3': Version 3 (37x37 modules), '-s 1': 1 pixel per module
qrencode -v 3 -s 1 "WIFI:S:${SSID};T:WPA;P:${PASS};;" -o credential_qr.png

# Convert the PNG to a C header file containing the image data
xxd -i credential_qr.png > credential_qr.h

# Generate QR code for the configuration URL
qrencode -v 3 -s 1 "http://${IP}/" -o url_qr.png

# Convert the URL QR code PNG to a C header file
xxd -i url_qr.png > url_qr.h
