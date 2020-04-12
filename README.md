# FPGA-FPS
FPS Game built for **ARM7 processor on the De1Soc**
Inspired by javidx9's console game engine (http://www.onelonecoder.com/)

* Emulator
* Search cpulator or go to https://cpulator.01xz.net/
* Pick the Armv7 architecture and the De1-Soc system and pres go (or go to https://cpulator.01xz.net/?sys=arm-de1soc)
* In the languages tab pick c and upload either project_2.3.1 or project 2.4
  * project 2.4 uses the whole vga display buffer (640\*480) while 2.3.1 uses 1/4 of it
  * 2.3.1 is much faster than 2.4 but requires extra set up of the display area
* Hit compile and load and resize the vga window if you like


**If using 2.3.1** 
* SET UP DISPLAY AREA}
* Step 1:  Load c files into CPUlator and turn of all device specific warnings
* Step 2: Hit the “Compile and Load” button
* Step 3: Navigate to “VGA pixel buffer” window and click on drop down arrow
* Step 4: Change “Zoom” to 2.0 and click “Show in separate box”
* Step 5: Resize VGA pixel buffer window to only show top left quadrant
* Step 6: Hit the “Continue” button

* {CONTROL}
* Pushbutton 0: Shoot Fireball
* Pushbutton 1: Start Game and End Game

* Switch 0: Rotate Right
* Switch 1: Move Forward
* Switch 2: Move Backward
* Switch 3: Rotate Left
* Switch 4: Strafe Right
* Switch 5: Strafe Left

