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
The created thread uses every usefull function.  
It should be taken as an example for working with the Display.  
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
  
1. Display and touch bottons initialisization
```
    disp_init();
    disp_init_buttons(disp_pid);
```
2. predefine your space with position == 255 for showing free usage
```
    static showText_t showDown[SPEICHER]; // define static space
    uint8_t i;
    for (i = 0; i < SPEICHER; i++)
    { // freier Speicherplatz wird mit Position 255 definiert
        showDown[i].position = SPEICHER;
    }
```
3. adding ficitious data for testing and presentaion
```
    
    disp_addParam(&showDown[2], "Hight:", "15");
    disp_addParam(&showDown[3], "Batt:", "82");
    disp_addParam(&showDown[4], "State:", "save");
```
4. Showing up first page
```
    disp_changePage(0);
```
5. waiting for bottons to be pressed
```
        msg_receive(&msg);
        if (msg.content.value == 0)
        {
            curr_page -= 1;
        }
        else if (msg.content.value == 1)
        {
            curr_page += 1;
        }
```
5.1 and change diplay page
```
disp_changePage(curr_page);
```
6. delete a parameter if not longer needed
```
disp_deleteParam(&showDown[3]); 
```
6.1 deleting a parameter with only the title
```
uint8_t temp_pos = getPosition(title);
        if (temp_pos != SPEICHER)
        {
            disp_deleteParam(getshowText(temp_pos));
	}
```
7. updating a variable of a space 
```
disp_changeVar(&showDown[0], "newData");
```
7.1 updating variable with only the title
```
uint8_t temp_pos = getPosition(title);
        if (temp_pos != SPEICHER)
        {
            disp_changeVar(getshowText(temp_pos), variable);
	}
```
8 creating new space
```
showText_t *space = getnewspace();
```
8.1 creating space and adding to the list
```
if (title == NULL)
            return -1;
if (variable == NULL)
            return -1;
showText_t *space = getnewspace();
if (space != NULL)
	{
            disp_addParam(space, title, variable);
            return 0;
        }
```
GCOAP
=====
The "title" is the identifier for the parameter, every dublicated parameter must be deleted manaully.  
Gcoap handler used for:  

* GET "title": returns the values based on the send title  
* PUT "title;variable": update the internal variable based on the title  
* POST "title;variable": addding/creating parameter with given title and variable  
* DELETE "title": deletes the first up come of the parameter with the title "title"  

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
		
