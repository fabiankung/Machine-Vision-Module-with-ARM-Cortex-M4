# Machine-Vision-Module-with-ARM-Cortex-M4
In this project I am using an ARM Cortex-M4 micro-controller, in particular Atmel's SAM4SD16 to interface with a VGA CMOS camera.
1. RobotEyeMon1_VB_Codes_Manual.zip - Contains the Visual Basic .NET sourcecodes for the PC monitoring software, i.e. the Machine Vision Monitor, build using Visual Studio Express 2017.  Also include a simple user manual for how to use the software.  This version uses the more efficient Bitmap.Lockbits() to update the bitmap display instead of the Bitmap.Setpixel(), improving the refreshing rate a bit.
2. User_Tasks.c and User_Tasks.h - Source and header files for user processes implementing simple real-time image processing algorithm.  See remarks in the sourcecodes for more information.
3. Driver_TCM8230.c and Driver_TCM8230.h are the routines to initialize the VGA CMOS camera TCM8230 from Toshiba.  For other camera type the user will need to develop their own routines based on the templates in these files.
4. ATSAM4SD16B.c and osmain.h - Contains Main( ) function for the firmware and the main header file.  
5. User_Tasks.c and Driver_TCM8230.c are to be used in conjuction with the other driver sourcecodes (for UART, UASRT, I2C peripherals) in the ATSAM4S ARM Cortex M4 Core as described in https://github.com/fabiankung/Atmel-ATSAM4S-Cortex-M4-Core.
5. Please refer to the demo video to see the system in action: https://www.youtube.com/watch?v=C5EAwsnuIdM
