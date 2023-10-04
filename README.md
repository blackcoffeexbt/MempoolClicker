# Mempool Clicker

Make a click sound on a piezo buzzer when a new bitcoin block is mined.

This project was built using a LilyGo TTGO T-Display board.

It connects to WiFi, then opens a websocket to mempool.space, then subscribes to "block" messages from the mempool.space server. When a new block is mined, the TTGO tells the attached piezo buzzer to make a "click" sound.

## Instructions
1. Flash this to the ESP32
1. Connect a piezo buzzer to pins 2 and GND
1. ...
1. Profit!!!!