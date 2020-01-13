Usefull git commands
================
Get:
```
    $ git clone https://github.com/BalooSLU/RIOT.git
    $ git branch
    $ git checkout Display
    $ git branch
    $ git config --global user.name "UserName"
    $ git config --global user.email "Email"
    $ git config --global color.ui "auto"
```
Push:
```
    $ git add DATEI
    $ git add *
    $ git commit -m "Changes Comment"
    $ git push
```
Usage
=====
The created thread uses every usefull function.//
It should be taken as an example for working with the Display.//
To Build, flash and start the application on an ESP32:
```
  $ BUILD_IN_DOCKER=1 BOARD=esp32-wroom-32 make all flash term
  $ sudo BUILD_IN_DOCKER=1 BOARD=esp32-wroom-32 make all flash term ESPTOOL=$(which esptool) FLASH=$(which esptool) PREFLASHER=$(which esptool)
```
Further Information on the ESP32 --> https://api.riot-os.org/group__cpu__esp32.html#esp32_spi_interfaces
The most usefull customizable defines are:
- The display size with DISP_WIDT/DISP_HIGHT
- The rows and columns number COLS/LINES
After changing the variable or title fonts the defines VAR_TEXT_HIGHT/TITLE_TEXT_HIGHT must be adjusted.

Pinout
======
Pin out for ESP-WROOM-32
          			Diplay  	-	MCU
				-----------------------------
				LED	      	-	3V3
			  	SCK	      	-	D18
			  	SDA	      	-	D23
			  	A0	      	-	D14
			  	RESET	    	-	D2
			  	CS	      	-	D5
			  	GND	      	-	GND
			  	VCC	      	-	VIN
        			Up_Botton   	- 	D22
        			Down_Bott   	-	D21
		
