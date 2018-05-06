//////////////////////////////////////////////////////////////////////////////////////////////
//
//	USER DRIVER ROUTINES DECLARATION (PROCESSOR DEPENDENT)
//
//  (c) Copyright 2015-2018, Fabian Kung Wai Lee, Selangor, MALAYSIA
//  All Rights Reserved  
//   
//////////////////////////////////////////////////////////////////////////////////////////////
//
// File				: User_Task.c
// Author(s)		: Fabian Kung
// Last modified	: 27 April 2018
// Tool-suites		: Atmel Studio 7.0 or later
//                    GCC C-Compiler

#include "osmain.h"
#include "./C_Library/Driver_I2C_V100.h"
#include "./C_Library/Driver_UART_V100.h"
#include "./C_Library/Driver_USART_V100.h"
#include "./C_Library/Driver_TCM8230.h"

// NOTE: Public function prototypes are declared in the corresponding *.h file.

//
//
// --- PUBLIC VARIABLES ---
//
typedef struct StructMARKER
{
	unsigned int nAttrib;				// Attribute of marker
	unsigned int nX;					// X coordinate of marker.
	unsigned int nY;					// y coordinate of marker.
} MARKER;

MARKER	objMarker1;
MARKER	objMarker2;
MARKER	objMarker3;

typedef struct StructRec
{
	unsigned int nHeight;				// Height in pixels.
	unsigned int nWidth;				// Width in pixels.
	unsigned int nX;					// X coordinate of top left corner.
	unsigned int nY;					// y coordinate of top left corner.
	unsigned int nColor;				// Bit7 = 1 - No fill.
										// Bit7 = 0 - Fill.
										// Bit0-6 = 1 to 127, determines color.
										// If nColor = 0, the ROI will not be displayed in the remote monitor.
} MARKER_RECTANGLE;

MARKER_RECTANGLE	gobjRec1;			// Marker objects.
MARKER_RECTANGLE	gobjRec2;
MARKER_RECTANGLE	gobjRec3;


unsigned int	gunPtoALuminance;		// Peak-to-average luminance value of each frame.
unsigned int	gunMaxLuminance;		// Maximum luminance in curret frame, for debug purpose.
unsigned int	gunMinLuminance;		// Minimum luminance in current frame, for debug purpose.
unsigned int	gunHue;					// For debug purpose.

int		gnSendSecondaryInfo = 0;		// Set to 1 (in Image Processing Tasks) to transmit secondary info such as marker location, 
										// size attribute and string to remote display.  Else set to 0.  If this is set 
										// to 1, after the transmission of a line of pixel data, the secondary info
										// packet will be send, and this will be clear to 0 by Proce_MessageLoop_StreamImage().
										// In present implementation the Proce_MessageLoop_StreamImage() will automatically 
										// set this to 1 after transmitting a frame to send secondary info automatically.
int		gnImageProcessingAlgorithm = 1;	// Current active image processing algorithm. 
//int		gnImageProcessingAlgorithm = 3;	// Current active image processing algorithm. 
int		gnImageProcessingArg = 0;		// Argument for image processing algorithm.

int     gnEyeLEDEffect = 0;				// 0 = No effect.
#define _EYE_LED_EFFECT_NONE			0
#define _EYE_LED_EFFECT_BLINK_0_4s		1	// 1 = Blink both eye LEDs at 0.4 second interval.
#define _EYE_LED_EFFECT_BLINK_0_12s		2	// 2 = Blink both eye LEDs at 0.12 second interval.
#define _EYE_LED_EFFECT_ALT_BLINK_0_3s	3	// 3 = Alternate blink eye LEDs at 0.3 second interval.
										

// --- Local function prototype ---

#define     _DEVICE_RESET               0x00
#define     _RES_PARAM                  0x01
#define     _SET_PARAM                  0x02

#define PIN_HC_05_RESET_CLEAR   PIOA->PIO_ODSR |= PIO_ODSR_P7                    // Pin PA7 = Reset pin for HC-05 bluetooth module.
#define PIN_HC_05_RESET_SET		PIOA->PIO_ODSR &= ~PIO_ODSR_P7					 // Active low.

///
/// Function name	: Proce_MessageLoop_StreamImage
///
/// Author			: Fabian Kung
///
/// Last modified	: 21 Feb 2018
///
/// Code Version	: 1.03
///
/// Processor		: ARM Cortex-M4 family
///
/// Processor/System Resource
/// PINS			: Pin PA7 = Reset pin for HC-05 BlueTooth module.
///
/// MODULES			: UART0 (Internal), PDC (Internal), USART0 (Internal).
///
/// RTOS			: Ver 1 or above, round-robin scheduling.
///
/// Global Variables    :

#ifdef __OS_VER			// Check RTOS version compatibility.
	#if __OS_VER < 1
		#error "Proce_MessageLoop_StreamImage: Incompatible OS version"
	#endif
#else
	#error "Proce_MessageLoop_StreamImage: An RTOS is required with this function"
#endif

///
/// Description	:
/// This process performs three (3) key tasks:
/// 1. Message clearing loop for instructions from external controller via USART0 port.
/// 2. Message clearing loop and stream video image captured by the camera via UART0 port.
///
/// --- (1) Message clearing ---
/// Basically this process waits for 1-byte command from external controller. The commands are as 
/// follows:
/// Command:		Action:
/// [CMD]	        0 - Nothing, deactivate all activities of external controller.
///                 1 to 15 - Activate image processing algorithm 1 to 15.
///                 16 - Turn off eye LED2.
///                 17 - Set eye LED2 to low intensity.
///                 18 - Set eye LED2 to medium intensity.
///                 19 - Set eye LED to high intensity.
///                 20 - Blink eye LED2, low intensity.
///                 21 - Blink eye LED2, medium intensity.
///                 22 - Blink eye LED2, high intensity
///  
/// --- (2) Stream video image ---
/// Stream the image captured by camera to remote display via UART port.  Note that the UART port
/// can be connected to a wireless module such as the HC-05 Blue tooth module.
/// Here our main remote display is a computer running Windows OS.  
///
/// The remote display initiates the transfer by sending a single byte command to the processor.
/// 1. If the command is 'L', 'R', 'G', 'B', then luminance data will be stream to the remote processor.
///    The characters determine how the luminance is computed from the RGB components of each pixel.
///    See the codes for the camera driver for details.
/// 2. If the command is 'D', then luminance gradient data will be streamed to the remote processor.
/// 3. If the command is 'H', then hue data will be streamed to the remote processor.  Here we
///    compress the Hue range from 0-360 to 0-90 (e.g. divide by 4) so that it will fit into 7 bits.
///
/// The image data is send to the remote display line-by-line, using a simple RLE (Run-Length 
/// Encoding) compression format. The data format:
///
/// Byte0: 0xFF (Indicate start-of-line)
/// Byte1: Line number, 0-253, indicate the line number in the bitmap image.
///        If Byte1 = 254, it indicate the subsequent bytes are secondary info such
///        as ROI location and size and any other info the user wish to transmit to the host.
///        At present auxiliary info is only 10 bytes.
/// Byte2: The length of the data payload, excluding Byte0-2.
/// Byte3-ByteN: Data payload.
/// The data payload only accepts 7 bits value, from bit0-bit6.  Bit7 is used to indicate
/// repetition in the RLE algorithm, the example below elaborate further.
/// 
/// Example 1 - One line of pixel data.
/// Consider the following byte stream:
/// [0xFF][3][7][90][80][70][0x83][60][0x82][120]
/// 
/// 
/// Byte1 = 3 is the line no. of the current pixel data in a 2D image.
/// Byte2 = 7 indicates that there are 6 data bytes in this data packet.
/// Byte3 = 90 represent the 1st pixel data in line 3.  This value can represent the 
///         luminance value of other user define data.
/// Byte4 = 80 another pixel data.
/// Byte5 = 70 another pixel data.
/// Byte6 = 0x83 = 0b10000000 + 3.  This indicates the last byte, e.g. Byte5=70 is to
///         be repeated 3 times, thus there are a total of 4 pixels with value of 70
///         after pixel with value of 80. 
/// Byte7 = 60 another pixel data.
/// Byte8 = 0x82 = 0b10000000 + 2.  The pixel with value of 60 is to be repeated 2 times.
/// Byte9 = 120, another pixel.
///
/// The complete line of pixels is as follows:
/// Line3: [90][80][70][70][70][70][60][60][60][120]
/// 
/// The maximum no. of repetition: Currently the maximum no. of repetition is 63.  If 
/// there are 64 consecutive pixels with values of 70, we would write [70][0x80 + 0x3F]
/// or [70][0x10111111].  The '0' at bit 6 is to prevention a repetition code of 0xFF, which
/// can be confused with start-of-line value.
///
/// Example 2 - Secondary information
/// Consider the following byte stream:
/// [0xFF][255][5][40][30][60][60][1]
/// The length of the payload is 5 bytes.
/// This instructs the remote monitor to highlight a region of 40x30 pixels, starting at position
/// (x,y) = (60,60) with color code = 1.
/// 
/// 25 Dec 2015: What I discovered is that there is a latency in Windows 8.1 in terms.
/// of processing the data from UART-to-USB converter.  Initially I send only 1 line of
/// pixel data per packet.  My observation using oscilloscope is that after the packet is 
/// received, there is a delay on the order of 10-20 msec before the application is notified.
/// It seems this delay is not dependent of the number of bytes in the packet.  Example if we
/// transmit only 2 bytes, there is still a latency of this amount.  Thus it is more efficient
/// to transmit more bytes per packet.  The actual duration to display the data in the 
/// application window is on the order of 1-3 msec.  In order to improve efficiency, we can
/// transmit 2 or more lines of pixel data in one packet of data.  


void Proce_MessageLoop_StreamImage(TASK_ATTRIBUTE *ptrTask)
{
	static unsigned char bytData;
	static int nLineCounter;
	int nXposCounter;
	int nCurrentPixelData;
	int nRefPixelData;
	int nRepetition;
	int nTemp;
	int nIndex;
	
    if (ptrTask->nTimer == 0)
    {
		switch (ptrTask->nState)
		{
            case 0: // State 0 - Initialization.
				objMarker1.nAttrib = 0;			// Disable plotting of all markers in the graphic window of Remote host.
				objMarker2.nAttrib = 0;
				objMarker3.nAttrib = 0;
				PIN_HC_05_RESET_CLEAR;          // De-assert reset pin for BlueTooth module.
				PIN_LED2_CLEAR;					// Turn off indicator LED2.
				nLineCounter = 0;
						
				OSSetTaskContext(ptrTask, 10, 1000*__NUM_SYSTEMTICK_MSEC);     // Next state = 10, timer = 1000 msec.
            break;

		    case 1: // State 1 - a) Message clearing for USART0, and b) wait for start signal from remote host on UART0.  
					// The start signal is the character 'L' (for luminance data using RGB) 
					// 'R' (for luminance data using R only)
					// 'G' (for luminance data using G only)
					// 'B' (for luminance data using B only)
					// or 'G' (for gradient data).
			
			    // --- Message clearing for USART0 ---
				if (gbytRXbufptr2 > 0)	// Check if USART0 receive any 1 byte of data.
				{
					if (gSCIstatus2.bRXOVF == 0)		// Make sure no overflow error.
					{
						PIN_LED2_CLEAR;	
						switch (gbytRXbuffer2[0])		// Decode command for MVM.
						{
							case 1:
								gnImageProcessingAlgorithm = 1;
							break;
							
							case 2:
								gnImageProcessingAlgorithm = 2;
							break;
							
							case 3:
								gnImageProcessingAlgorithm = 3;
							break;			

							case 4:
								gnImageProcessingAlgorithm = 4;
							break;
											
							case 16:
								gnEyeLED2 = 0;
								break;
								
							case 17:
								gnEyeLED2 = 1;
								break;
							
							case 18:
								gnEyeLED2 = 3;
								break;
								
							case 19:
								gnEyeLED2 = 5;
								break;								

							case 20:
								gnEyeLED2 = 9;
							break;
							
							case 21:
								gnEyeLED2 = 11;
							break;
							
							case 22:
								gnEyeLED2 = 13;
							break;														
																												
							default:
								gnCameraLED = 0;
								gnImageProcessingAlgorithm = 0;
								break;
								
						}
					}
					else
					{
						gSCIstatus2.bRXOVF = 0; 	// Reset overflow error flag.
					}
					gSCIstatus2.bRXRDY = 0;			// Reset valid data flag.
					gbytRXbufptr2 = 0; 				// Reset pointer.
					PIN_LED2_CLEAR;					// Turn off indicator LED2.
				}
				
				// --- Message clearing and stream video image for UART0 ---
				if (gSCIstatus.bRXRDY == 1)	// Check if UART receive any data.
				{
					if (gSCIstatus.bRXOVF == 0) // Make sure no overflow error.
					{
						bytData = gbytRXbuffer[0];	// Get 1 byte and ignore all others.
						if (bytData == 'L')			// Send luminance data computed from RGB components			
						{
							gnLuminanceMode = 0;
							OSSetTaskContext(ptrTask, 2, 1);     // Next state = 2, timer = 1.
						}
						else if (bytData == 'D')	// gradient data
						{					
							OSSetTaskContext(ptrTask, 2, 1);     // Next state = 2, timer = 1.
						}			
						else if (bytData == 'H')	// Hue data
						{
							OSSetTaskContext(ptrTask, 2, 1);     // Next state = 2, timer = 1.
						}
						else if (bytData == 'R')	// Send luminance data computed from R component.
						{
							gnLuminanceMode = 1;
							OSSetTaskContext(ptrTask, 2, 1);     // Next state = 2, timer = 1.
						}
						else if (bytData == 'G')	// Send luminance data computed from G component.
						{
							gnLuminanceMode = 2;
							OSSetTaskContext(ptrTask, 2, 1);     // Next state = 2, timer = 1.
						}
						else if (bytData == 'B')	// Send luminance data computed from B component.
						{
							gnLuminanceMode = 3;
							OSSetTaskContext(ptrTask, 2, 1);     // Next state = 2, timer = 1.
						}						
						else                        // Wrong command.
						{
							OSSetTaskContext(ptrTask, 1, 1);     // Next state = 1, timer = 1.
						}
						
					}
					else
					{
						gSCIstatus.bRXOVF = 0; 	// Reset overflow error flag.
						OSSetTaskContext(ptrTask, 1, 1);     // Next state = 1, timer = 1.
					}
					gSCIstatus.bRXRDY = 0;	// Reset valid data flag.
					gbytRXbufptr = 0; 		// Reset pointer.
					PIN_LED2_CLEAR;			// Turn off indicator LED2.
				}
				else
				{
					OSSetTaskContext(ptrTask, 1, 1);     // Next state = 1, timer = 1.
				}

			break;
							
			case 2: // State 2 - Send a line of pixel data to remote host, using RLE data compression.	    
				//gnCameraLED = 2;

				if (gSCIstatus.bTXRDY == 0)					// Check if  UART port is busy.
				{
					gbytTXbuffer[0] = 0xFF;					// Start of line code.
					gbytTXbuffer[1] = nLineCounter;			// Line number.
															// gbytTXbuffer[2] stores the payload length.
					// --- Implement simple RLE algorithm ---
					nRepetition = 0;						// Initialize repetition counter.
					nXposCounter = 1;						// Initialize x-position along a line of pixels.
															// Initialize the first data byte.
					// Get 1st byte/pixel.										
					if (bytData != 'D') 					// Read pixel data based on format selection.
					{
						if (bytData != 'H')					// 'L', 'R', 'G', 'B', send luminance data.
						{
							nCurrentPixelData = gunImgAtt[0][nLineCounter] & _LUMINANCE_MASK; // Get pixel luminance data.
						}
						else                                // 'H', send scaled hue info.  Here we rescale the hue (0-360)
						{									// to between 0-90, e.g. Hue/4.	
							nTemp = gunImgAtt[0][nLineCounter] & _HUE_MASK;
							nCurrentPixelData = nTemp >> (_HUE_SHIFT + 2);  
						}
					}
					else     // bytData == 'D'.
					{
						nCurrentPixelData = gunImgAtt[0][nLineCounter] >> (_GRAD_SHIFT + 1); // Get pixel gradient data and divide
																			// by 2, as gradient value can hit 255.
						nCurrentPixelData = 0x000000FF & nCurrentPixelData; // Mask out all except lower 8 bits.
					}						
					
					nRefPixelData = nCurrentPixelData;		// Setup the reference value.
					gbytTXbuffer[3] = nCurrentPixelData;	// First byte of the data payload.
					
					// Get subsequent bytes/pixels in the line of pixels data.
					for (nIndex = 1; nIndex < gnImageWidth; nIndex++)
					{
						if (bytData != 'D')					// Read pixel data based on format selection.	
						{
							if (bytData != 'H')				// 'L', 'R', 'G', 'B', send luminance data.
							{
								nCurrentPixelData = gunImgAtt[nIndex][nLineCounter] & _LUMINANCE_MASK; // Get pixel luminance data.							
							}
							else                            // 'H', send scaled hue info.  Here we rescale the hue (0-360)
							{								// to between 0-90, e.g. Hue/4.
								nTemp = gunImgAtt[nIndex][nLineCounter] & _HUE_MASK;
								nCurrentPixelData = nTemp >> (_HUE_SHIFT + 2);
							}
							
						}
						else     // bytData == 'D'.
						{
							nCurrentPixelData = gunImgAtt[nIndex][nLineCounter] >> (_GRAD_SHIFT + 1); // Get pixel gradient data and divide 
																				// by 2, as the gradient value can hit 255.
							nCurrentPixelData = 0x0000007F & nCurrentPixelData; // Mask out all except lower 7 bits.
						}
						
						if (nCurrentPixelData == nRefPixelData) // Current and previous pixels share similar value.
						{
							nRepetition++;
							if (nIndex == gnImageWidth-1)	   // Check for last pixel in line.
							{
								gbytTXbuffer[nXposCounter+3] = 128 + nRepetition; // Set bit7 and add the repetition count.
								nXposCounter++;
							}							
							else if (nRepetition == 64)			// The maximum no. of repetition allowed is 63, see explanation 
							{									// in 'Description'.
								gbytTXbuffer[nXposCounter+3] = 128 + 63; // Set bit7 and add the repetition count.
								nXposCounter++;					// Point to next byte in the array.
								nRepetition = 0;				// Reset repetition counter.
								gbytTXbuffer[nXposCounter+3] = nCurrentPixelData;
								nXposCounter++;					// Point to next byte in the array.
							}
						}
						else									// Current and previous pixel do not share similar value.
						{
							nRefPixelData = nCurrentPixelData;	// Update reference pixel value.
						    if (nRepetition > 0)				// Check if we have multiple pixels of similar value.
							{
								gbytTXbuffer[nXposCounter+3] = 128 + nRepetition; // Set bit7 and add the repetition count.
								nXposCounter++;					// Point to next byte in the array.
								nRepetition = 0;				// Reset repetition counter.
								gbytTXbuffer[nXposCounter+3] = nCurrentPixelData;
								nXposCounter++;					// Point to next byte in the array.
							}
							else                                // No multiple pixels of similar value.
							{
								gbytTXbuffer[nXposCounter+3] = nCurrentPixelData;		
								nXposCounter++;
							}
						} // if (nCurrentPixelData == nRefPixelData)
					}  // Loop index

					gbytTXbuffer[2] = nXposCounter;   // No. of bytes in the data packet, excluding start-of-line code, line number and payload length.

					PDC_UART0->PERIPH_TPR = gbytTXbuffer;	// Setup DMA for UART0 transmit operation.
					PDC_UART0->PERIPH_TCR = nXposCounter+3;
					PDC_UART0->PERIPH_TNPR = gbytTXbuffer;
					PDC_UART0->PERIPH_TNCR = 0;
					PDC_UART0->PERIPH_PTCR = PDC_UART0->PERIPH_PTCR | PERIPH_PTCR_TXTEN;	// Enable transmitter transfer.
					gSCIstatus.bTXDMAEN = 1;			// Indicate UART transmit with DMA.
					gSCIstatus.bTXRDY = 1;				// Initiate TX.
					PIN_LED2_SET;						// Lights up indicator LED2.

					nLineCounter++;						// Point to next line of image pixel data.
					if (nLineCounter == gnImageHeight)	// Check if reach end of line.
					{
						gnSendSecondaryInfo = 1;		// Set flag to transmit secondary info to host at the end of each frame.
						nLineCounter = 0;				// Reset line counter.
					}
					
					OSSetTaskContext(ptrTask, 3, 1);    // Next state = 3, timer = 1.
				}
				else
				{
					//nLineCounter--;					// Restore line counter back to original value.
					OSSetTaskContext(ptrTask, 1, 1);    // Next state = 1, timer = 1.
				}
			break;
										
			case 3: // State 3 - Wait for transmission of line pixel to end.
				
				if (gSCIstatus.bTXRDY == 0)	// Check if still got any data to send via UART.
				{				
					if (gnSendSecondaryInfo == 1)			// Check if any secondary info to transmit to remote display.
					{
						OSSetTaskContext(ptrTask, 4, 1);     // Next state = 4, timer = 1.		
					}
					else                                    // No secondary info to send to remote display, next line.
					{
						OSSetTaskContext(ptrTask, 1, 1);     // Next state = 1, timer = 1.
					}								
				}
				else  // Yes, still has pending data to send via UART.
				{
					OSSetTaskContext(ptrTask, 3, 1);     // Next state = 3, timer = 1.				
				}
			break;

		    case 4: // State 4 - Wait for start signal from remote host.  The start signal is the character
		    // 'L' (for luminance data using RGB)
			// 'R' (for luminance data using R only)
			// 'G' (for luminance data using G only)
			// 'B' (for luminance data using B only)
			// 'D' (for gradient data).
			// 'H' (for hue data).  
		    if (gSCIstatus.bRXRDY == 1)	// Check if UART receive any data.
		    {
			    if (gSCIstatus.bRXOVF == 0) // Make sure no overflow error.
			    {
				    bytData = gbytRXbuffer[0];	// Get 1 byte and ignore all others.
				    if ((bytData == 'L') || (bytData == 'R') || (bytData == 'G') || (bytData == 'B'))	// If send luminance data.	
												// info.
				    {
					    OSSetTaskContext(ptrTask, 5, 1);     // Next state = 5, timer = 1.
				    }
				    else if (bytData == 'D')
				    {
					    OSSetTaskContext(ptrTask, 5, 1);     // Next state = 5, timer = 1.
				    }
				    else if (bytData == 'H')
				    {
					    OSSetTaskContext(ptrTask, 5, 1);     // Next state = 5, timer = 1.
				    }					
				    else
				    {
					    OSSetTaskContext(ptrTask, 1, 1);     // Next state = 1, timer = 1.
				    }
			    }
			    else
			    {
				    gSCIstatus.bRXOVF = 0; 					// Reset overflow error flag.
					OSSetTaskContext(ptrTask, 1, 1);		// Next state = 1, timer = 1.
			    }
			    gSCIstatus.bRXRDY = 0;						// Reset valid data flag.
			    gbytRXbufptr = 0; 							// Reset pointer.
			    PIN_LED2_CLEAR;								// Turn off indicator LED2.
		    }
		    else
		    {
			    OSSetTaskContext(ptrTask, 4, 1);     // Next state = 4, timer = 1.
		    }
		    break;

			case 5: // State 5 - Send auxiliary data (markers, texts etc).
				//gnCameraLED = 0;
				
				gbytTXbuffer[0] = 0xFF;					// Start of line code.
				gbytTXbuffer[1] = 254;					// Line number of 254 indicate auxiliary data.
				gbytTXbuffer[2] = 11;					// Payload length is 11 bytes.
				
				gbytTXbuffer[3] = gobjRec1.nWidth;		// 0. Get the info for rectangle to be highlighted in remote display.
				gbytTXbuffer[4] = gobjRec1.nHeight;		// 1. Height and width.
				gbytTXbuffer[5] = gobjRec1.nX;			// 2. Starting point (top left hand corner) along x and y axes.
				gbytTXbuffer[6] = gobjRec1.nY;			// 3.
				gbytTXbuffer[7] = gobjRec1.nColor;		// 4. Set the color of the marker 1 .
				gbytTXbuffer[8] = gobjRec2.nWidth;		// 0. Get the info for rectangle to be highlighted in remote display.
				gbytTXbuffer[9] = gobjRec2.nHeight;		// 1. Height and width.
				gbytTXbuffer[10] = gobjRec2.nX;			// 2. Starting point (top left hand corner) along x and y axes.
				gbytTXbuffer[11] = gobjRec2.nY;			// 3.				
				gbytTXbuffer[12] = gobjRec2.nColor;		// 4. Set the color of the marker 2.				
				gbytTXbuffer[13] = gunHue;				// Some tag along parameter for debugging purpose.
				
				PDC_UART0->PERIPH_TPR = gbytTXbuffer;	// Setup DMA for UART0 transmit operation.
				PDC_UART0->PERIPH_TCR = 14;				// Total length is 14 bytes including payload.
				PDC_UART0->PERIPH_TNPR = gbytTXbuffer;
				PDC_UART0->PERIPH_TNCR = 0;
				PDC_UART0->PERIPH_PTCR = PDC_UART0->PERIPH_PTCR | PERIPH_PTCR_TXTEN;	// Enable transmitter transfer.
				gSCIstatus.bTXDMAEN = 1;				// Indicate UART transmit with DMA.
				gSCIstatus.bTXRDY = 1;					// Initiate TX.
				PIN_LED2_SET;							// Lights up indicator LED2.
				OSSetTaskContext(ptrTask, 6, 1);		// Next state = 6, timer = 1.
			break;

			case 6: // State 6 - Wait for transmission of line pixel to end.
				if (gSCIstatus.bTXRDY == 0)	// Check if still any data to send via UART.
				{
					gnSendSecondaryInfo = 0;			// Clear flag.
					OSSetTaskContext(ptrTask, 1, 1);    // Next state = 1, timer = 1.
				}
				else  // Yes, still has pending data to send via UART.
				{
					OSSetTaskContext(ptrTask, 6, 1);    // Next state = 6, timer = 1.
				}
				break;
			
			case 10: // State 10 - Reset HC-05 Bluetooth module (if attached).  Note that if we keep the HC-05 module in
					 // reset state, it will consume little power.  This trick can be used when we wish to power down
					 // HC-05 to conserve power.
				PIN_HC_05_RESET_SET;					// Reset Bluetooth module.				

				gnEyeLED1 = 1;							// Turn on Eye LED1s.
				gnEyeLED2 = 1;							// Turn on Eye LED2s.
				gnEyeLEDEffect = _EYE_LED_EFFECT_NONE;
				OSSetTaskContext(ptrTask, 11, 10*__NUM_SYSTEMTICK_MSEC);     // Next state = 11, timer = 10 msec.
			break;
			
			case 11: // State 11 - Reset HC-05 Bluetooth module.
				PIN_HC_05_RESET_CLEAR;					// Clear Reset to Bluetooth module.
				OSSetTaskContext(ptrTask, 1, 10*__NUM_SYSTEMTICK_MSEC);     // Next state = 1, timer = 10 msec.
			break;			
			
			default:
				OSSetTaskContext(ptrTask, 0, 1); // Back to state = 0, timer = 1.
            break;
        }
    }
}


///
/// Function name	: Proce_Image1
///
/// Author			: Fabian Kung
///
/// Last modified	: 10 April 2018
///
/// Code Version	: 0.93
///
/// Processor		: ARM Cortex-M4 family
///
/// Processor/System Resource
/// PINS			: None
///
/// MODULES			: Camera driver.
///					  Proce_USART_Driver
///
/// RTOS			: Ver 1 or above, round-robin scheduling.
///
/// Global Variables    :

#ifdef __OS_VER			// Check RTOS version compatibility.
#if __OS_VER < 1
#error "Proce_Image1: Incompatible OS version"
#endif
#else
#error "Proce_Image1: An RTOS is required with this function"
#endif

///
/// Description	:
/// A simple image processing task to find brightest and darkest spots in an image frame.
/// Here we make use of the image luminance histogram from the camera driver to determine
/// the highest and lowest luminance values.
/// 1) Find the average coordinate for the brightest spot and the darkest spot.
/// 2) Send command to remote controller.

#define		_MAX_NUMBER_LUMINANCE_REF	2

void Proce_Image1(TASK_ATTRIBUTE *ptrTask)
{
	static	int	nCurrentFrame;
	static	int	nYindex, nXindex;
	static	int	nxmax[_MAX_NUMBER_LUMINANCE_REF], nymax[_MAX_NUMBER_LUMINANCE_REF];
	static	int	nxmin, nymin;
	static	unsigned int nLuminanceMax[_MAX_NUMBER_LUMINANCE_REF] = {0, 0};	// Variable to store max luminance value.
	static	unsigned int	nLuminanceMin = 127;	// Variable to store min luminance value.
	int		unsigned	nLuminance;
	static	int nCounter = 0;

	static  int nXMoment = 0;
    static  int nXCount = 0;
	static  int nYMoment = 0;
	static  int nYCount = 0;	

	int nTemp, nTemp2, nIndex;

	if (ptrTask->nTimer == 0)
	{
		switch (ptrTask->nState)
		{
			case 0: // State 0 - Initialization.
			if ((gnImageProcessingAlgorithm == 1) && (gnCameraReady == _CAMERA_READY))
			{
				gnCameraLED = 1;					// Set camera LED intensity to low.
				nCurrentFrame = gnFrameCounter;
				OSSetTaskContext(ptrTask, 1, 10*__NUM_SYSTEMTICK_MSEC);     // Next state = 1, timer = 10 msec.
			}
			else
			{
				OSSetTaskContext(ptrTask, 0, 1*__NUM_SYSTEMTICK_MSEC);     // Next state = 0, timer = 1 msec.
			}
			break;

			case 1: // State 1 - Wait until a new frame is acquired in the image buffer before start processing.
			if (gnFrameCounter != nCurrentFrame)		// Check if new image frame has been captured.
			{
				nCurrentFrame = gnFrameCounter;			// Update current frame counter.
				nXindex = 0;							// Initialize all parameters.
				nYindex = 1;							// 13 Dec 2016: Start with line 2 of the image.  Line 1 of the
														// image is all black (luminance = 0), not sure why.
				// 8 Feb 2018: Image contains some error as stated, probably due to the camera TCM8230.  So we
				// start with line 2.  Because of this if the actual image is quite bright, then there would be
				// no pixel with luminance = 0, and there would be no valid minimum luminance coordinate.  Thus
				// we can inform the robot controller connected to the MVM that no valid minimum luminance is
				// detected.
				// The above restriction can be removed in future if a better camera module is used.
								
				nxmax[0] = gnImageWidth>>1;				// At the start of each frame initialized all (x,y) coordinates
				nymax[0] = gnImageHeight>>1;			// to the center of the frame.
				nxmax[1] = gnImageWidth>>1;
				nymax[1] = gnImageHeight>>1;
				nxmin = gnImageWidth>>1;
				nymin = gnImageHeight>>1;
				
				nCounter = 0;
				
                for (nIndex = 127; nIndex > 0; nIndex--)	// Search for the maximum luminance value in the luminance
															// histogram of the current frame.  Start from the largest
				{											// index, max luminance is assumed to be +127.
					if (gunIHisto[nIndex] > 4)				// At least 3 or more pixels having this luminance to qualify as a 
															// valid maximum.
															// Note: 10 April 2018 - I discover that if we set to > 0, e.g. just
															// 1 pixel having a luminance value, the output of this process fluctuates
															// wildly.  It could be due to noise or some unknown error in the 
															// camera sensor elements that cause a wrong luminance value to be 
															// registered.  Thus setting a higher threshold eliminates this
															// effect, leading to a stable output as long as the scene captured
															// is the same.  The threshold can be adjusted based on 
															// image resolution and camera type and EMI.
					{
						nLuminanceMax[nCounter] = nIndex;	// The first instance a non-zero location is found, this 
															// represents the largest luminance.
						nCounter++;							// Point to next largest luminance.
						if (nCounter == _MAX_NUMBER_LUMINANCE_REF)					
															// Here nLuminanceMax[0] stores the max luminance value,
						{									// nLuminanceMax[1] stores the next largest luminance value								
															// and so forth.
							nIndex = 0;						// Once reach max. number of items, break the for-loop.
							nCounter = 0;					// Reset counter.
						}
					}
				}
                for (nIndex = 0; nIndex < 127; nIndex++)	// Search for the minimum luminance value in the luminance
															// histogram of the current frame.  Start from the smallest
                {											// index.
	                if (gunIHisto[nIndex] > 4)				// At least 3 or more pixels having this luminance to qualify as a 
															// valid minimum.  See comments above.
	                {				
		                nLuminanceMin = nIndex;				// The first instance a non-zero location is found, this
															// represents the minimum luminance.
		                nIndex = 1000;						// Break the for-loop.
	                }
                }				
				OSSetTaskContext(ptrTask, 2, 1);			// Next state = 2, timer = 1.
				
				nXMoment = 0;
				nXCount = 0;
				nYMoment = 0;
				nYCount = 0;				
			}
			else
			{
				OSSetTaskContext(ptrTask, 1, 1);     // Next state = 1, timer = 1.
			}
			break;

			case 2: // State 2 - Find the brightest and dimmest spots in the frame.
			//		Here we scan 2 lines of pixel data at one go, and do this multiple times until we cover the whole
			//		image frame.  This is to ensure that we don't hog the processor.  All processes should complete
			//		within 1 system tick.
			PIOA->PIO_ODSR |= PIO_ODSR_P19;											// Set flag, this is optional, for debugging purpose.
			
			for (nXindex = 0; nXindex < gnImageWidth; nXindex++)					// Scan 1st row of image frame relative to nYindex.
			{
				nLuminance = gunImgAtt[nXindex][nYindex] & _LUMINANCE_MASK;			// Extract the 7-bits luminance value.
				if (nLuminance == nLuminanceMax[0])									// Search for highest luminance pixel location.
				{
					nXMoment = nXMoment + nXindex;									// Sum the x and y coordinates of all pixels whose luminance
					nXCount++;														// corresponds to the maximum luminance.  As we do this
					nYMoment = nYMoment + nYindex;									// we also keep track of the number of pixels.
					nYCount++;					
				}
				else if (nLuminance == nLuminanceMax[1])							// Search for next highest luminance pixel 
				{																	// location.
					nxmax[1] = nXindex;
					nymax[1] = nYindex;
				}
				if (nLuminance == nLuminanceMin)									// Search for lowest luminance pixel location.
				{
					nxmin = nXindex;
					nymin = nYindex;
				}
			}
			
			nYindex++;
			if (nYindex == gnImageHeight)											// Is this the last row?
			{
				PIOA->PIO_ODSR &= ~PIO_ODSR_P19;									// Clear flag, this is optional, for debugging purpose.	
				OSSetTaskContext(ptrTask, 3, 1);									// Next state = 3, timer = 1.
				break;																// Exit current state.			
			}
			
			for (nXindex = 0; nXindex < gnImageWidth; nXindex++)					// Scan 2nd of image frame.
			{
				nLuminance = gunImgAtt[nXindex][nYindex] & _LUMINANCE_MASK;			// Extract the 7-bits luminance value.
				if (nLuminance == nLuminanceMax[0])									// Search for highest luminance pixel location.
				{
					nXMoment = nXMoment + nXindex;									// Sum the x and y coordinates of all pixels whose luminance
					nXCount++;														// corresponds to the maximum luminance.  As we do this
					nYMoment = nYMoment + nYindex;									// we also keep track of the number of pixels.
					nYCount++;													
				}
				else if (nLuminance == nLuminanceMax[1])							// Search for next highest luminance pixel
				{																	// location.
					nxmax[1] = nXindex;
					nymax[1] = nYindex;
				}
				if (nLuminance == nLuminanceMin)									// Search for lowest luminance pixel location.
				{
					nxmin = nXindex;
					nymin = nYindex;
				}
			}
			
			nYindex++;
			PIOA->PIO_ODSR &= ~PIO_ODSR_P19;										// Clear flag, this is optional, for debugging purpose.
			
			if (nYindex == gnImageHeight)											// Is it last row?
			{																		// Yes
				// in the external display.
				OSSetTaskContext(ptrTask, 3, 1);									// Next state = 3, timer = 1.
			}
			else																	// Not yet last row.
			{
				OSSetTaskContext(ptrTask, 2, 1);									// Next state = 2, timer = 1.
			}
			break;
			
			case 3: // State 3 - Add marker 1 to the external display to highlight the result, also find peak-to-average value.
			nxmax[0] = nXMoment / nXCount;											// Compute the average location of the brightest pixels.
			nymax[0] = nYMoment / nYCount;
			gobjRec1.nHeight = 5 ;													// Enable a square marker to be displayed in remote monitor 
			gobjRec1.nWidth = 5 ;													// software.
			gobjRec1.nX = nxmax[0];													// Set the location of the marker.			
			gobjRec1.nY = nymax[0];
			gobjRec1.nColor = 3;
			
			gobjRec2.nHeight = 5 ;													// Enable a square marker to be displayed in remote monitor
			gobjRec2.nWidth = 5 ;													// software.
			gobjRec2.nX = nxmin;
			gobjRec2.nY = nymin;	
			gobjRec2.nColor = 2;	
					
			gunMaxLuminance = nLuminanceMax[0];										// Peak luminance value for this frame.
			gunPtoALuminance = gunMaxLuminance - gunAverageLuminance;				// Peak-to-Average luminance for this frame.

			OSSetTaskContext(ptrTask, 4, 1);										// Next state = 4, timer = 1.
			break;
			
			case 4: // State 4 - Transmit status to external controller, part 1 (transmit first 2 bytes).
			// NOTE: 28 Dec 2016.  To prevent overflow error on the remote controller (as it does not use DMA on the UART),
			// we avoid sending all 4 bytes one shot, but split the data packet into two packets or 2 bytes each.  A 1 msec
			// delay is inserted between each two bytes packet.  As the algorithm in the remote controller improves in future
			// this artificial restriction can be removed.

			if (gSCIstatus2.bTXRDY == 0)	// Check if any data to send via UART.
			{
				gbytTXbuffer2[0] = gnImageProcessingAlgorithm;	// Load data, process ID.
				gbytTXbuffer2[1] = 1;							// 
				gbytTXbuflen2 = 2;								// Set TX frame length.
				gSCIstatus2.bTXRDY = 1;							// Initiate TX.
				OSSetTaskContext(ptrTask, 5, 1*__NUM_SYSTEMTICK_MSEC);     // Next state = 5, timer = 1 msec.
			}
			else
			{
				OSSetTaskContext(ptrTask, 4, 1);     // Next state = 4, timer = 1.
			}
			break;

			case 5: // State 5 - Transmit status to external controller, part 2 (transmit last 2 bytes).
			if (gSCIstatus2.bTXRDY == 0)	// Check if any data to send via UART.
			{
				//gbytTXbuffer2[0] = nxmax[1];
				//gbytTXbuffer2[1] = nymax[1];				
				
				gbytTXbuffer2[0] = nxmax[0];				
				gbytTXbuffer2[1] = nymax[0];
				gbytTXbuflen2 = 2;					// Set TX frame length.
				gSCIstatus2.bTXRDY = 1;				// Initiate TX.
				OSSetTaskContext(ptrTask, 6, 1*__NUM_SYSTEMTICK_MSEC);    // Next state = 6, timer = 1 msec.
			}
			else
			{
				OSSetTaskContext(ptrTask, 5, 1);    // Next state = 5, timer = 1.
			}
			break;

			case 6:	// State 6 - Check if process ID has changed.
			if (gnImageProcessingAlgorithm == 1)	// Check process ID.
			{
				OSSetTaskContext(ptrTask, 1, 1);	// Next state = 1, timer = 1.
			}
			else									// Process ID changed.
			{
				OSSetTaskContext(ptrTask, 0, 1);	// Next state = 0, timer = 1.
			}
			break;

			default:
			OSSetTaskContext(ptrTask, 0, 1); // Back to state = 0, timer = 1.
			break;
		}
	}
}



///
/// Function name	: Proce_Image2
///
/// Author			: Fabian Kung
///
/// Last modified	: 28 March 2018
///
/// Code Version	: 0.61
///
/// Processor		: ARM Cortex-M4 family
///
/// Processor/System Resource
/// PINS			:
///
/// MODULES			: Camera driver.
///                   Proce_USART_Driver
///
/// RTOS			: Ver 1 or above, round-robin scheduling.
///
/// Global Variables    :

#ifdef __OS_VER			// Check RTOS version compatibility.
#if __OS_VER < 1
#error "Proce_Image2: Incompatible OS version"
#endif
#else
#error "Proce_Image2: An RTOS is required with this function"
#endif

///
/// Description	:
/// 1. Perform mathematical morphology on the ROI in the image using LBP (local binary pattern)
/// approach.  Here we assume a binary image (black and white) with threshold be selected properly.
/// The threshold is the average luminance level of each frame.
/// The mathematical morphology procedure helps to de-noise the binary image.
/// 2. After de-noising, we inspect the ROI, whenever it contains a number of black pixels above  
/// a certain threshold, we interpret that as object present in the ROI.  
/// 3. Here we have 9 ROIs, 3 each on the left, middle and on the right vision field, to ascertain
/// if there are objects in the left, front, or right of the camera.
/// 4. The process will report the status of each ROIs to the remote controller via the 
/// following protocol:
///
/// The status of each row of ROIs is represented by 1 byte, as follows:
/// bit2: 1 = Left ROI contain objects, else 0.
/// bit1: 1 = Middle ROI contain objects, else 0.
/// bit0: 1 = Right ROI contain objects, else 0.
///
/// A 4 bytes code is transmitted via USART to the remote controller to 
/// indicate the status of the machine vision module.
/// Byte 0: The process ID (PID) of this process (e.g. 2).
/// Byte 1: The status of ROIs row 2.
/// Byte 2: The status of ROIs row 1.
/// Byte 3: The status of ROIs row 0.
/// Note that we transmit the last row first! (Big endian)
/// Thus a packet from this process is as follows:
/// [PID] + [ROW3 status] + [ROW2 status] + [ROW1 status]
///
/// The ROI is sub-divided into 9 sub-regions, as follows:
///                      
/// ----------------------------> x (horizontal)
/// |
/// |   ROI00	ROI10	ROI20
/// |   ROI01	ROI11	ROI21
/// |   ROI02	ROI12	ROI22
/// |
/// |
/// V
/// y (vertical)
/// 
/// nROIStartX and nROIStartY: set the start pixel (x,y) position.
/// nROIWIdth and nROIHeight: set the width and height of each sub-region.
/// nROIXOffset: This is an offset value added to nROIStartX, to cater for 
/// the inherent mechanical error in the azimuth direction.  Due to the limited
/// resolution of the coupling, sometimes the camera is not pointing straight 
/// ahead even though the azimuth angle of the motor is set to zero.  We can
/// adjust this offset to move all the ROIs to the left or right.  

#define     __IP2_MAX_ROIX		3		// Set the number of ROI along x axis.
#define     __IP2_MAX_ROIY		3		// Set the number of ROI along y axis.
#define		__IP2_ROI00_XSTART	52		// Start position in pixels for ROI (upper left corner).
#define		__IP2_ROI00_YSTART	88	
#define     __IP2_ROI01_XSTART  47
#define		__IP2_ROI01_YSTART  97 
#define		__IP2_ROI02_XSTART  43
#define		__IP2_ROI02_YSTART  106
#define		__IP2_ROI00_WIDTH	19		// The width of a single ROI region in pixels.
#define		__IP2_ROI01_WIDTH	22
#define		__IP2_ROI02_WIDTH	25
#define		__IP2_ROI_WIDTH_TOTAL	 __IP2_MAX_ROIX*__IP2_ROI_WIDTH	// Total ROI width
#define		__IP2_ROI_HEIGHT	9		// The height of a single ROI region in pixels.
#define		__IP2_ROI_HEIGHT_TOTAL	__IP2_MAX_ROIY*__IP2_ROI_HEIGHT
#define     __IP2_ROI_THRESHOLD 10		// The no. of pixels to trigger the object recognition.
//#define     __IP2_ROIX_START_OFFSET 10	// For V2T2A camera
#define     __IP2_ROIX_START_OFFSET 0	// For V2T1A camera


typedef struct StructSquareROI
{
	int nStartX;				// Start pixel coordinate (top left hand corner).
	int nStartY;				// 
	unsigned int unWidth;		// Width and heigth of ROI.
	unsigned int unHeigth;
	
} SQUAREROI;

void Proce_Image2(TASK_ATTRIBUTE *ptrTask)
{
	static int	nCurrentFrame;
	static int  nXindex, nYindex;
	static int	nLumReference;
	
	static unsigned int	unLumCumulativeROI = 0;
	static unsigned int unAverageLumROI = 0;
	static int	nPixelCount = 0;
	
	int nLuminance;
	int nTemp;
	int	nColEnd, nColStart;
	int	nRowEnd, nRowStart;
	static	int ni = 0;
	static	int nj = 0;
	int nColPixelCount;
	static int nCounter;
	static int nROIThreshold[3][3];
	static int nLBP;
	static int nROIStartX, nROIStartY;
	static int nROIWidth, nROIHeight;
	static int nROIXOffset;
	
	static	SQUAREROI	objROI[3][3];

	if (ptrTask->nTimer == 0)
	{
		switch (ptrTask->nState)
		{
			case 0: // State 0 - Initialize ROI parameters.
				objROI[0][0].nStartX = __IP2_ROI00_XSTART;
				objROI[0][0].nStartY = __IP2_ROI00_YSTART;
				objROI[0][0].unWidth = __IP2_ROI00_WIDTH;
				objROI[0][0].unHeigth = __IP2_ROI_HEIGHT;
				objROI[1][0].nStartX = __IP2_ROI00_XSTART + __IP2_ROI00_WIDTH;
				objROI[1][0].nStartY = __IP2_ROI00_YSTART;
				objROI[1][0].unWidth = __IP2_ROI00_WIDTH;
				objROI[1][0].unHeigth = __IP2_ROI_HEIGHT;
				objROI[2][0].nStartX = __IP2_ROI00_XSTART + (2*__IP2_ROI00_WIDTH);
				objROI[2][0].nStartY = __IP2_ROI00_YSTART;
				objROI[2][0].unWidth = __IP2_ROI00_WIDTH;
				objROI[2][0].unHeigth = __IP2_ROI_HEIGHT;
				objROI[0][1].nStartX = __IP2_ROI01_XSTART;
				objROI[0][1].nStartY = __IP2_ROI01_YSTART;
				objROI[0][1].unWidth = __IP2_ROI01_WIDTH;
				objROI[0][1].unHeigth = __IP2_ROI_HEIGHT;
				objROI[1][1].nStartX = __IP2_ROI01_XSTART + __IP2_ROI01_WIDTH;
				objROI[1][1].nStartY = __IP2_ROI01_YSTART;
				objROI[1][1].unWidth = __IP2_ROI01_WIDTH;
				objROI[1][1].unHeigth = __IP2_ROI_HEIGHT;
				objROI[2][1].nStartX = __IP2_ROI01_XSTART + (2*__IP2_ROI01_WIDTH);
				objROI[2][1].nStartY = __IP2_ROI01_YSTART;
				objROI[2][1].unWidth = __IP2_ROI01_WIDTH;
				objROI[2][1].unHeigth = __IP2_ROI_HEIGHT;	
				objROI[0][2].nStartX = __IP2_ROI02_XSTART;
				objROI[0][2].nStartY = __IP2_ROI02_YSTART;
				objROI[0][2].unWidth = __IP2_ROI02_WIDTH;
				objROI[0][2].unHeigth = __IP2_ROI_HEIGHT;
				objROI[1][2].nStartX = __IP2_ROI02_XSTART + __IP2_ROI02_WIDTH;
				objROI[1][2].nStartY = __IP2_ROI02_YSTART;
				objROI[1][2].unWidth = __IP2_ROI02_WIDTH;
				objROI[1][2].unHeigth = __IP2_ROI_HEIGHT;
				objROI[2][2].nStartX = __IP2_ROI02_XSTART + (2*__IP2_ROI02_WIDTH);
				objROI[2][2].nStartY = __IP2_ROI02_YSTART;
				objROI[2][2].unWidth = __IP2_ROI02_WIDTH;
				objROI[2][2].unHeigth = __IP2_ROI_HEIGHT;							
				OSSetTaskContext(ptrTask, 1, 1);     // Next state = 1, timer = 1.							
			break;
			
			case 1: // State 1 - Wait for the correct process ID, then start initialization.
				if ((gnImageProcessingAlgorithm == 2) && (gnCameraReady == _CAMERA_READY))
				{
					nCurrentFrame = gnFrameCounter;
					gnCameraLED = 4;						// Set camera LED intensity to high.
					unAverageLumROI = gunAverageLuminance;	// Initialize the local average luminance as equal to global (e.g. 
															// whole frame) average luminance.
																			
					gnEyeLED1 = 1;
					gnEyeLED2 = 1;
					gnEyeLEDEffect = _EYE_LED_EFFECT_NONE;	
					nLumReference = 127;					// Set this to the maximum! (for 7 bits unsigned integer)
					nPixelCount = 0;
					OSSetTaskContext(ptrTask, 2, 10*__NUM_SYSTEMTICK_MSEC);     // Next state = 2, timer = 10 msec.
				}
				else
				{
					OSSetTaskContext(ptrTask, 1, 1*__NUM_SYSTEMTICK_MSEC);     // Next state = 1, timer = 1 msec.
				}			
			break;

			case 2: // State 2 - Wait until a new frame is acquired in the image buffer, initialize de-noising.
				if (gnFrameCounter != nCurrentFrame)						// Check if new image frame has been captured.
				{
					nCurrentFrame = gnFrameCounter;							// Update current frame counter.
					nYindex = __IP2_ROI00_YSTART-1;							// Set the y start position for the de-noising process.
					nROIXOffset =  __IP2_ROIX_START_OFFSET;										
																			// Calibration or offset in x pixels (This value can 
																			// be positive or negative).  Due to the resolution of
																			// the motor controlling the azimuth angle, sometime
																			// the camera is not pointing straight ahead even though
																			// the azimuth angle is zero.
					nXindex = __IP2_ROI00_XSTART+__IP2_ROIX_START_OFFSET;	// Set the x start position for the de-noising process.
					ni = 0;
					nj = 0;
					OSSetTaskContext(ptrTask, 3, 1);						// Next state = 3, timer = 1.
				}
				else                                                        // Is still old frame, keep polling.
				{
					OSSetTaskContext(ptrTask, 2, 1);						// Next state = 2, timer = 1.
				}						
			break;

			case 3: // State 3 - Perform morphology operation on image frame, one ROI at a time until all ROIs are processed. 
					
				PIOA->PIO_ODSR |= PIO_ODSR_P19;								// Set flag, this is optional, for debugging purpose.
				
				nRowStart = objROI[ni][nj].nStartY;							// Set up the start (x,y) pixel coordinates, and 
				nColStart = objROI[ni][nj].nStartX;							// the end limits of the current ROI.
				nRowEnd = nRowStart + objROI[ni][nj].unHeigth;
				nColEnd = nColStart + objROI[ni][nj].unWidth;
					
				for (nYindex = nRowStart; nYindex < nRowEnd; nYindex++)
				{
					for (nXindex = nColStart; nXindex < nColEnd; nXindex++)
					{
						nCounter = 0;										// Reset LBP counter.
						nLBP = 0;
										
						// Get the P=8, R=1 square LBP neighborhood luminance pattern.  First we reduce the luminance to two-level
						// by comparing with average luminance for current frame, then only perform the LBP extraction.
						//                    \
						// |-------------------- 'X'
						// |                  /
						// |  0 1 2
						// |  7 P 3   P=Pixel of interest
						//\|/ 6 5 4
						// 'Y'
						// LBP = (a0*2^0) + (a1*2^1) + (a2*2^2) + ... + (a7*2^7)
						// where ai = 1 if the corresponding position is white or greater than the binary threshold.
						//
							
						// Position 0.
						if ((gunImgAtt[nXindex - 1][nYindex - 1] & _LUMINANCE_MASK) >= unAverageLumROI)
						//if ((gunImgAtt[nXindex - 1][nYindex - 1] & _LUMINANCE_MASK) >= gunAverageLuminance)
						{
							nLBP = 1;									// Set bit0.
							nCounter++;
						}

						// Position 1.
						if ((gunImgAtt[nXindex][nYindex - 1] & _LUMINANCE_MASK) >= unAverageLumROI)
						//if ((gunImgAtt[nXindex][nYindex - 1] & _LUMINANCE_MASK) >= gunAverageLuminance)
						{
							nLBP = nLBP + 2;							// Set bit1.
							nCounter++;
						}

						// Position 2.
						if ((gunImgAtt[nXindex + 1][nYindex - 1] & _LUMINANCE_MASK) >= unAverageLumROI)
						//if ((gunImgAtt[nXindex + 1][nYindex - 1] & _LUMINANCE_MASK) >= gunAverageLuminance)
						{
							nLBP = nLBP + 4;							// Set bit2.
							nCounter++;
						}

						// Position 3.
						if ((gunImgAtt[nXindex + 1][nYindex] & _LUMINANCE_MASK) >= unAverageLumROI)
						//if ((gunImgAtt[nXindex + 1][nYindex] & _LUMINANCE_MASK) >= gunAverageLuminance)
						{
							nLBP = nLBP + 8;							// Set bit3.
							nCounter++;
						}

						// Position 4.
						if ((gunImgAtt[nXindex + 1][nYindex + 1] & _LUMINANCE_MASK) >= unAverageLumROI)
						//if ((gunImgAtt[nXindex + 1][nYindex + 1] & _LUMINANCE_MASK) >= gunAverageLuminance)
						{
							nLBP = nLBP + 16;							// Set bit4.
							nCounter++;
						}

						// Position 5.
						if ((gunImgAtt[nXindex][nYindex + 1] & _LUMINANCE_MASK) >= unAverageLumROI)
						//if ((gunImgAtt[nXindex][nYindex + 1] & _LUMINANCE_MASK) >= gunAverageLuminance)
						{
							nLBP = nLBP + 32;							// Set bit5.
							nCounter++;
						}

						// Position 6.
						if ((gunImgAtt[nXindex - 1][nYindex + 1] & _LUMINANCE_MASK) >= unAverageLumROI)
						//if ((gunImgAtt[nXindex - 1][nYindex + 1] & _LUMINANCE_MASK) >= gunAverageLuminance)
						{
							nLBP = nLBP + 64;							// Set bit6.
							nCounter++;
						}

						// Position 7.
						if ((gunImgAtt[nXindex - 1][nYindex ] & _LUMINANCE_MASK) >= unAverageLumROI)
						//if ((gunImgAtt[nXindex - 1][nYindex ] & _LUMINANCE_MASK) >= gunAverageLuminance)
						{
							nLBP = nLBP + 128;							// Set bit7.
							nCounter++;
						}
							
						// Perform de-noising based on LBP texture and update luminance attribute of current pixel.
						nTemp = gunImgAtt[nXindex][nYindex];					// Get current pixel attributes.
						nLuminance = nTemp & _LUMINANCE_MASK;					// Extract current pixel luminance.
						if (nLuminance >= unAverageLumROI)						// If current pixel is white.
						{
							if (nCounter < 3)									// If less than 3 neighboring pixels of the is white,
							{													// this means the pixel could be isolated white pixel, so we
								// make it black (set the luminance to 0).
								gunImgAtt[nXindex][nYindex] = (nTemp & 0xFFFFFF80) + 0;	// Set bit6-0 to 0;
							}
							else
							{
								gunImgAtt[nXindex][nYindex] = (nTemp & 0xFFFFFF80) + nLumReference;	// Set bit6-0 to value of nLumReference;
							}
						}
						else													// If current pixel is black.
						{
							if (nCounter > 3)									// If more than 3 neighboring pixels is white, this
							{													// means it could be a notch, so we make the pixel white.
								gunImgAtt[nXindex][nYindex] = (nTemp & 0xFFFFFF80) + nLumReference;	// Set bit6-0 to value of nLumReference;
							}
							else
							{
								gunImgAtt[nXindex][nYindex] = (nTemp & 0xFFFFFF80) + 0;	// Set bit6-0 to 0;
							}
						}
							
						unLumCumulativeROI = unLumCumulativeROI + nLuminance;	// Accumulate the luminance within ROI.
						nPixelCount++;											// Increment the pixel counter;			
										
					}
				}
				
				ni++;														// Proceed to next ROI.
				if (ni == __IP2_MAX_ROIX)									// Increment the x and y indices of the ROI.
				{
					ni = 0;
					nj++;
					if (nj == __IP2_MAX_ROIY)
					{
						nj = 0;
						unAverageLumROI = unLumCumulativeROI/nPixelCount;		// Compute average luminance in the ROI.
						unLumCumulativeROI = 0;									// Reset ROI cumulative luminance.
						nPixelCount = 0;										// Reset the pixel counter.		
						OSSetTaskContext(ptrTask, 4, 1);						// Next state = 4, timer = 1.
					}
					else
					{
						OSSetTaskContext(ptrTask, 3, 1);						// Next state = 3, timer = 1.
					}
				}
			break;

			case 4: // State 4 - Check if any object detected (e.g. luminance is 0) within each sub-ROI.

				nRowStart = objROI[ni][nj].nStartY;							// Set up the start (x,y) pixel coordinates, and
				nColStart = objROI[ni][nj].nStartX;							// the end limits of the current ROI.
				nRowEnd = nRowStart + objROI[ni][nj].unHeigth;
				nColEnd = nColStart + objROI[ni][nj].unWidth;					

				nROIThreshold[ni][nj] = 0;
				
				for (nYindex = nRowStart; nYindex < nRowEnd; nYindex++)
				{
					for (nXindex = nColStart; nXindex < nColEnd; nXindex++)
					{
						if ((gunImgAtt[nXindex][nYindex] & _LUMINANCE_MASK) < nLumReference)	// Check for 'dark spots' in ROI.
						{
							nROIThreshold[ni][nj]++;	// Update the counter for 'dark spots'.	
						}
					}
				}

				ni++;														// Proceed to next ROI.
				if (ni == __IP2_MAX_ROIX)									// Increment the x and y indices of the ROI.
				{
					ni = 0;
					nj++;
					if (nj == __IP2_MAX_ROIY)
					{
						nj = 0;
						OSSetTaskContext(ptrTask, 6, 1);						// Next state = 6, timer = 1.
					}
					else
					{
						OSSetTaskContext(ptrTask, 4, 1);						// Next state = 4, timer = 1.
					}
				}
				break;

			case 6: // State 6 - Check if any object detected (e.g. luminance is 0) within the ROIs, report to remote controller.
					//
					// The status of each row of ROIs is represented by 1 byte, as follows:
					// bit2: 1 = Left ROI contain objects, else 0.
					// bit1: 1 = Middle ROI contain objects, else 0.
					// bit0: 1 = Right ROI contain objects, else 0.
					//
					// A 4 bytes code is transmitted via USART to indicate the status of the machine vision module.
					// Byte 0: The process ID (PID) of this process (e.g. 2).
					// Byte 1: The status of ROIs row 2.
					// Byte 2: The status of ROIs row 1.
					// Byte 3: The status of ROIs row 0.
					// Note that we transmit the last row first! (Big endian)
					// Thus a packet from this process is as follows:
					// [PID] + [ROW2 status] + [ROW1 status] + [ROW0 status]
					//
					// NOTE: 28 Dec 2016.  To prevent overflow error on the remote controller (as it does not use DMA on the UART),
					// we avoid sending all 4 bytes one shot, but split the data packet into two packets or 2 bytes each.  A 1 msec
					// delay is inserted between each two bytes packet.  As the algorithm in the remote controller improves, this
					// artificial restriction can be removed. 

				gobjRec1.nHeight = __IP2_ROI_HEIGHT*__IP2_MAX_ROIY;		// Mark the ROI.
				gobjRec1.nWidth = __IP2_ROI00_WIDTH*__IP2_MAX_ROIX;
				gobjRec1.nX = __IP2_ROI00_XSTART + nROIXOffset;
				gobjRec1.nY = __IP2_ROI00_YSTART;
				gobjRec1.nColor = 130;						// Set ROI to green, no fill, 128 + 2 = 130.
															// Bit7 = 1 - No fill.
															// Bit7 = 0 - Fill.
															// Bit0-6 = 1 to 127, determines color.
															// If nColor = 0, the ROI will not be displayed in the remote monitor.

				// 27 July 2017: The scenario considered here:
				// (a) When the floor is bright or light color than the surrounding.  In this case the floor
				// will appear as white pixels and the surrounding as black pixels.  Thus any black pixels 
				// within the floor area will be interpreted as objects. The threshold should not be more than 
				// 5% of the total pixels within the ROI.


				nTemp = 0;
				gnEyeLEDEffect = _EYE_LED_EFFECT_NONE;
				// Check left thresholds.
				if (nROIThreshold[0][2] > __IP2_ROI_THRESHOLD)	
				//if ((nROIThreshold[0][2] > __IP2_ROI_THRESHOLD) && (nROIThreshold[0][2] < 200))
				{
					//gobjRec1.nColor = 1;
					gnEyeLEDEffect = _EYE_LED_EFFECT_ALT_BLINK_0_3s;
					nTemp = 0x04;				// Set bit 2.
				}
				// Check middle threshold.
				if (nROIThreshold[1][2] > __IP2_ROI_THRESHOLD) 
				//if ((nROIThreshold[1][2] > __IP2_ROI_THRESHOLD) && (nROIThreshold[1][2] < 200))
				{
					//gobjRec1.nColor = 2;
					gnEyeLEDEffect = _EYE_LED_EFFECT_ALT_BLINK_0_3s;
					nTemp = nTemp | 0x02;		// Set bit 1.
				}
				// Check right threshold.
				if (nROIThreshold[2][2] > __IP2_ROI_THRESHOLD)
				//if ((nROIThreshold[2][2] > __IP2_ROI_THRESHOLD) && (nROIThreshold[2][2] < 200))
				{
					//gobjRec1.nColor = 3;
					gnEyeLEDEffect = _EYE_LED_EFFECT_ALT_BLINK_0_3s;
					nTemp = nTemp | 0x01;		// Set bit 0.
				}
				gbytTXbuffer2[1] = nTemp;		// Load data.

				if (gSCIstatus2.bTXRDY == 0)	// Check if any data to send via USART.
				{
					gbytTXbuffer2[0] = 2;		//Load image processing algorithm ID.
					gbytTXbuflen2 = 2;			// Set TX frame length.
					gSCIstatus2.bTXRDY = 1;		// Initiate TX.
					OSSetTaskContext(ptrTask, 7, 1*__NUM_SYSTEMTICK_MSEC);     // Next state = 7, timer = 1 msec.
				}
				else
				{
					OSSetTaskContext(ptrTask, 6, 1);						// Next state = 6, timer = 1.
				}
			break;

			case 7: // State 7 - Continue checking if any object detected (e.g. luminance is 0) within the right ROI and report
					// status to remote controller.
				nTemp = 0;
				// Check left thresholds.
				if (nROIThreshold[0][1] > __IP2_ROI_THRESHOLD) 
				//if ((nROIThreshold[0][1] > __IP2_ROI_THRESHOLD) && (nROIThreshold[0][1] < 200)) 
				{
					//gobjRec1.nColor = 1;
					gnEyeLEDEffect = _EYE_LED_EFFECT_ALT_BLINK_0_3s;
					nTemp = 0x04;				// Set bit 2.
				}
				// Check middle threshold.
				if (nROIThreshold[1][1] > __IP2_ROI_THRESHOLD) 
				//if ((nROIThreshold[1][1] > __IP2_ROI_THRESHOLD) && (nROIThreshold[1][1] < 200))
				{
					//gobjRec1.nColor = 2;
					gnEyeLEDEffect = _EYE_LED_EFFECT_ALT_BLINK_0_3s;
					nTemp = nTemp | 0x02;		// Set bit 1.
				}
				// Check right threshold.
				if (nROIThreshold[2][1] > __IP2_ROI_THRESHOLD)
				//if ((nROIThreshold[2][1] > __IP2_ROI_THRESHOLD) && (nROIThreshold[2][1] < 200)) 
				{
					//gobjRec1.nColor = 3;
					gnEyeLEDEffect = _EYE_LED_EFFECT_ALT_BLINK_0_3s;
					nTemp = nTemp | 0x01;		// Set bit 0.
				}
				gbytTXbuffer2[0] = nTemp;		// Load data.

				nTemp = 0;
				// Check left thresholds.
				if (nROIThreshold[0][0] > __IP2_ROI_THRESHOLD) 
				//if ((nROIThreshold[0][0] > __IP2_ROI_THRESHOLD) && (nROIThreshold[0][0] < 200))
				{
					//gobjRec1.nColor = 1;
					gnEyeLEDEffect = _EYE_LED_EFFECT_ALT_BLINK_0_3s;
					nTemp = 0x04;				// Set bit 2.
				}
				// Check middle threshold.
				if (nROIThreshold[1][0] > __IP2_ROI_THRESHOLD)
				//if ((nROIThreshold[1][0] > __IP2_ROI_THRESHOLD) && (nROIThreshold[1][0] < 200))
				{
					//gobjRec1.nColor = 2;
					gnEyeLEDEffect = _EYE_LED_EFFECT_ALT_BLINK_0_3s;
					nTemp = nTemp | 0x02;		// Set bit 1.
				}
				// Check right threshold.
				if (nROIThreshold[2][0] > __IP2_ROI_THRESHOLD)
				//if ((nROIThreshold[2][0] > __IP2_ROI_THRESHOLD)	&& (nROIThreshold[2][0] < 200))
				{
					//gobjRec1.nColor = 3;
					gnEyeLEDEffect = _EYE_LED_EFFECT_ALT_BLINK_0_3s;
					nTemp = nTemp | 0x01;		// Set bit 0.
				}
				gbytTXbuffer2[1] = nTemp;		// Load data.

				if (gSCIstatus2.bTXRDY == 0)	// Check if any data to send via USART.
				{
					gbytTXbuflen2 = 2;			// Set TX frame length.
					gSCIstatus2.bTXRDY = 1;		// Initiate TX.
					OSSetTaskContext(ptrTask, 8, 1*__NUM_SYSTEMTICK_MSEC);		// Next state = 8, timer = 1 msec.
				}
				else
				{
					OSSetTaskContext(ptrTask, 7, 1);							// Next state = 7, timer = 1.
				}
			break;

			case 8: // State 8 - Check if process ID changed.
				if (gnImageProcessingAlgorithm == 2)
				{
					OSSetTaskContext(ptrTask, 1, 1);							// Next state = 1, timer = 1.
				}
				else
				{
					OSSetTaskContext(ptrTask, 0, 1);							// Next state = 0, timer = 1.
				}
				PIOA->PIO_ODSR &= ~PIO_ODSR_P19;							// Clear flag, this is optional, for debugging purpose.
			break;

			default:
				OSSetTaskContext(ptrTask, 0, 1);							// Back to state = 0, timer = 1.
			break;
		}
	}
}

///
/// Function name	: Proce_Image3
///
/// Author			: Fabian Kung
///
/// Last modified	: 23 April 2018
///
/// Code Version	: 0.20
///
/// Processor		: ARM Cortex-M4 family
///
/// Processor/System Resource
/// PINS			: None.
///
/// MODULES			: Camera driver.
///                   Proce_USART_Driver
///
/// RTOS			: Ver 1 or above, round-robin scheduling.
///
/// Global Variables    :

#ifdef __OS_VER			// Check RTOS version compatibility.
#if __OS_VER < 1
#error "Proce_Image3: Incompatible OS version"
#endif
#else
#error "Proce_Image3: An RTOS is required with this function"
#endif

///
/// Description	:
/// Color object location.  Here we are using the algorithm described by T. Braunl, "Embedded robotics", 2nd edition, 2006, Springer.
/// The proposed algorithm performs a search along column for pixels matching certain color (hue).  A histogram is constructed for all
/// the columns in the image frame, and the column with highest number of pixel match is identified from the histogram.  This is then
/// repeated for each row to identify the row with highest number of pixel match.  The row and column with highest pixel match, and the
/// largest value of pixel match per row or column are then transmitted to external controller.  The largest value of pixel match can 
/// be interpreted as an indication of the size of the object matching the required color in terms of pixel (assuming the pixels within
/// the row/column are continuous).

#define		_IP3_HUEOFINTEREST_HIGH_LIMIT	78		// Between 0 to 359, refer to hue table.
#define		_IP3_HUEOFINTEREST_LOW_LIMIT	58

void Proce_Image3(TASK_ATTRIBUTE *ptrTask)
{
	static	int		nCurrentFrame;
	static	int		nHOIHistoCol[_IMAGE_HRESOLUTION];		// Hue-of-interest (HOI) histogram for each column.
	static  int		nHOIHistoRow[_IMAGE_VRESOLUTION];		// Hue-of-interest (HOI) histogram for each row.
	static	int		nXindex, nYindex;
	static	unsigned int	unHOIHigh, unHOILow;
	unsigned int	unHueTemp;
	static	int		nMaxValueCol, nMaxCol;
	static	int		nMaxValueRow, nMaxRow;
	int	nIndex, nTemp;
	static	int     nHueFCol, nHueFColPrev, nHueCounterCol, nCntMaxCol;

	if (ptrTask->nTimer == 0)
	{
		switch (ptrTask->nState)
		{
			case 0: // State 0 - Initialization.
				if ((gnImageProcessingAlgorithm == 3) && (gnCameraReady == _CAMERA_READY))
				{					
					nCurrentFrame = gnFrameCounter;
					OSSetTaskContext(ptrTask, 1, 10*__NUM_SYSTEMTICK_MSEC);     // Next state = 1, timer = 10 msec.
				}
				else
				{
					OSSetTaskContext(ptrTask, 0, 1*__NUM_SYSTEMTICK_MSEC);     // Next state = 0, timer = 1 msec.
				}			
			break;

			case 1: // State 1 - Wait until a new frame is acquired in the image buffer, initialize de-noising.
				if (gnFrameCounter != nCurrentFrame)							// Check if new image frame has been captured.
				{
					gnCameraLED = 1;											// Set camera LED intensity to low.
					nCurrentFrame = gnFrameCounter;								// Update current frame counter.
					
					for (nIndex = 0; nIndex < _IMAGE_HRESOLUTION-1; nIndex++)	// Clear the HOI column histogram.
					{
						nHOIHistoCol[nIndex] = 0;
					}
					for (nIndex = 0; nIndex < _IMAGE_VRESOLUTION-1; nIndex++)	// Clear the HOI row histogram.
					{
						nHOIHistoRow[nIndex] = 0;
					}					
					nXindex = 1;												// Ignore 1st column.
					unHOIHigh = _IP3_HUEOFINTEREST_HIGH_LIMIT<<_HUE_SHIFT;		// Set the upper and lower range of the
					unHOILow = _IP3_HUEOFINTEREST_LOW_LIMIT<<_HUE_SHIFT;		// hue of interest.
					OSSetTaskContext(ptrTask, 2, 1);							// Next state = 2, timer = 1.
				}
				else															// Is still old frame, keep polling.
				{
					OSSetTaskContext(ptrTask, 1, 1);							// Next state = 1, timer = 1.
				}
			break;
			
			case 2: // State 2 - Compute the HOI histogram for each column of current frame, four columns at a time.
				PIOA->PIO_ODSR |= PIO_ODSR_P19;									// Set flag, this is optional, for debugging purpose.
				
				// --- Column 1 ---
				nHueCounterCol = 0;												// Clear all column related flags and counters
				nHueFCol = 0;													// before we analyze the color matching in the
				nHueFColPrev = 0;												// column.
				nCntMaxCol = 0;
				for (nYindex = 1; nYindex < gnImageHeight-1; nYindex++)			// Scan through each column, ignore the 1st and
																				// last columns, and 1st and last rows.
				{
					unHueTemp = gunImgAtt[nXindex][nYindex] & _HUE_MASK;		// Extract the hue for current pixel.
					if ((unHueTemp < unHOIHigh) && (unHueTemp > unHOILow))		// Check if current pixel is within the upper
																				// and lower limits of the hue of interest.
					{
						nHueFCol = 1;											// Set current hue match flag.
						nHOIHistoRow[nYindex]++;								// Update row histogram.
					}
					else
					{
						nHueFCol = 0;											// Clear current hue match flag.
					}
					// Decision logic based on current and previous hue match flag status.
					if ((nHueFCol == 0) && (nHueFColPrev == 0))
					{
						nHueCounterCol = 0;
					}
					else if ((nHueFCol == 0) && (nHueFColPrev == 1))
					{						
						nHueCounterCol = 0;
					}
					else if ((nHueFCol == 1) && (nHueFColPrev == 0))
					{
						nHueCounterCol = 1;
						if (nCntMaxCol < nHueCounterCol)
						{
							nCntMaxCol = nHueCounterCol;
						}
					}
					else
					{
						nHueCounterCol++;
						if (nCntMaxCol < nHueCounterCol)
						{
							nCntMaxCol = nHueCounterCol;
						}						
					}
					nHueFColPrev = nHueFCol;									// Update previous hue match flag.
				}
				nHOIHistoCol[nXindex] = nCntMaxCol; 							// Update column histogram to maximum count value.
																				// of current column.		
				nXindex++;														// Next column.				
				if (nXindex == gnImageWidth-2)									// Is this the last column?
				{
					OSSetTaskContext(ptrTask, 3, 1);							// Next state = 3, timer = 1.
					break;														// Exit current state.
				}			
				// --- Column 2 ---
				nHueCounterCol = 0;												// Clear all column related flags and counters
				nHueFCol = 0;													// before we analyze the color matching in the
				nHueFColPrev = 0;												// column.
				nCntMaxCol = 0;
				for (nYindex = 1; nYindex < gnImageHeight-1; nYindex++)			// Scan through each column, ignore the 1st and
																				// last columns, and 1st and last rows.
				{
					unHueTemp = gunImgAtt[nXindex][nYindex] & _HUE_MASK;		// Extract the hue for current pixel.
					if ((unHueTemp < unHOIHigh) && (unHueTemp > unHOILow))		// Check if current pixel is within the upper
																				// and lower limits of the hue of interest.
					{
						nHueFCol = 1;											// Set current hue match flag.
						nHOIHistoRow[nYindex]++;								// Update row histogram.
					}
					else
					{
						nHueFCol = 0;											// Clear current hue match flag.
					}					
					// Decision logic based on current and previous hue match flag status.					
					if ((nHueFCol == 0) && (nHueFColPrev == 0))
					{
						nHueCounterCol = 0;
					}
					else if ((nHueFCol == 0) && (nHueFColPrev == 1))
					{
						
						nHueCounterCol = 0;
					}
					else if ((nHueFCol == 1) && (nHueFColPrev == 0))
					{
						nHueCounterCol = 1;
						if (nCntMaxCol < nHueCounterCol)
						{
							nCntMaxCol = nHueCounterCol;
						}
					}
					else
					{
						nHueCounterCol++;
						if (nCntMaxCol < nHueCounterCol)
						{
							nCntMaxCol = nHueCounterCol;
						}
					}
					nHueFColPrev = nHueFCol;									// Update previous hue match flag.
				}
				nHOIHistoCol[nXindex] = nCntMaxCol; 							// Update column histogram to maximum count value.
																				// of current column.
				nXindex++;														// Next column.
				if (nXindex == gnImageWidth-2)									// Is this the last column?
				{
					OSSetTaskContext(ptrTask, 3, 1);							// Next state = 3, timer = 1.
					break;														// Exit current state.
				}
				// --- Column 3 ---
				nHueCounterCol = 0;												// Clear all column related flags and counters
				nHueFCol = 0;													// before we analyze the color matching in the
				nHueFColPrev = 0;												// column.
				nCntMaxCol = 0;
				for (nYindex = 1; nYindex < gnImageHeight-1; nYindex++)			// Scan through each column, ignore the 1st and
				// last columns, and 1st and last rows.
				{
					unHueTemp = gunImgAtt[nXindex][nYindex] & _HUE_MASK;		// Extract the hue for current pixel.
					if ((unHueTemp < unHOIHigh) && (unHueTemp > unHOILow))		// Check if current pixel is within the upper
																				// and lower limits of the hue of interest.
					{
						nHueFCol = 1;											// Set current hue match flag.
						nHOIHistoRow[nYindex]++;								// Update row histogram.
					}
					else
					{
						nHueFCol = 0;											// Clear current hue match flag.
					}
					// Decision logic based on current and previous hue match flag status.
					if ((nHueFCol == 0) && (nHueFColPrev == 0))
					{
						nHueCounterCol = 0;
					}
					else if ((nHueFCol == 0) && (nHueFColPrev == 1))
					{
						
						nHueCounterCol = 0;
					}
					else if ((nHueFCol == 1) && (nHueFColPrev == 0))
					{
						nHueCounterCol = 1;
						if (nCntMaxCol < nHueCounterCol)
						{
							nCntMaxCol = nHueCounterCol;
						}
					}
					else
					{
						nHueCounterCol++;
						if (nCntMaxCol < nHueCounterCol)
						{
							nCntMaxCol = nHueCounterCol;
						}
					}
					nHueFColPrev = nHueFCol;									// Update previous hue match flag.
				}
				nHOIHistoCol[nXindex] = nCntMaxCol; 							// Update column histogram to maximum count value.
																				// of current column.
				nXindex++;														// Next column.
				if (nXindex == gnImageWidth-2)									// Is this the last column?
				{
					OSSetTaskContext(ptrTask, 3, 1);							// Next state = 3, timer = 1.
					break;														// Exit current state.
				}
				// --- Column 4 ---
				nHueCounterCol = 0;												// Clear all column related flags and counters
				nHueFCol = 0;													// before we analyze the color matching in the
				nHueFColPrev = 0;												// column.
				nCntMaxCol = 0;
				for (nYindex = 1; nYindex < gnImageHeight-1; nYindex++)			// Scan through each column, ignore the 1st and
				// last columns, and 1st and last rows.
				{
					unHueTemp = gunImgAtt[nXindex][nYindex] & _HUE_MASK;		// Extract the hue for current pixel.
					if ((unHueTemp < unHOIHigh) && (unHueTemp > unHOILow))		// Check if current pixel is within the upper
																				// and lower limits of the hue of interest.
					{
						nHueFCol = 1;											// Set current hue match flag.
						nHOIHistoRow[nYindex]++;								// Update row histogram.
					}
					else
					{
						nHueFCol = 0;											// Clear current hue match flag.
					}
					// Decision logic based on current and previous hue match flag status.					
					if ((nHueFCol == 0) && (nHueFColPrev == 0))
					{
						nHueCounterCol = 0;
					}
					else if ((nHueFCol == 0) && (nHueFColPrev == 1))
					{
						
						nHueCounterCol = 0;
					}
					else if ((nHueFCol == 1) && (nHueFColPrev == 0))
					{
						nHueCounterCol = 1;
						if (nCntMaxCol < nHueCounterCol)
						{
							nCntMaxCol = nHueCounterCol;
						}
					}
					else
					{
						nHueCounterCol++;
						if (nCntMaxCol < nHueCounterCol)
						{
							nCntMaxCol = nHueCounterCol;
						}
					}
					nHueFColPrev = nHueFCol;									// Update previous hue match flag.
				}
				nHOIHistoCol[nXindex] = nCntMaxCol; 							// Update column histogram to maximum count value.
																				// of current column.
				nXindex++;														// Next column.
				if (nXindex == gnImageWidth-2)									// Is this the last column?
				{
					OSSetTaskContext(ptrTask, 3, 1);							// Next state = 3, timer = 1.
				}
				else
				{
					OSSetTaskContext(ptrTask, 2, 1);							// Next state = 2, timer = 1.
				}								
			break;

			case 3: // State 3 - Check for the column with large histogram value.  Add marker 1 to the external display to highlight the result.		
			nMaxValueCol = 6;													// Initialize the maximum histogram value and column.
			nMaxCol = -1;														// Note: The initial value serves as a threshold.  Only when
																				// the values in the HOI histogram above this threshold will the
																				// values in the histogram array be considered.  This threshold can
																				// be set in following manner:
																				// Say the number of pixels in a column is 120 pixels.  We could 
																				// select a threshold of 5% of the column pixels, e.g. 6 pixels.
																				// This rule-of-thumb allow sufficient guard against noise.
																				
			for (nIndex = 1; nIndex < gnImageWidth-2; nIndex++)					// Scan through each column histogram, and compare with the maximum.
																				// histogram value.  Ignore Column 0 and last column.
			{
				if (nHOIHistoCol[nIndex] > nMaxValueCol)						// If current histogram value is larger than nMaxValue,
				{																// update nMaxValue and the corresponding index.
					nMaxValueCol = nHOIHistoCol[nIndex];
					nMaxCol = nIndex;
				}
			}
			nMaxValueRow = 9;
			nMaxRow = -1;
			for (nIndex = 1; nIndex < gnImageHeight-2; nIndex++)				// Scan through each row histogram, and compare with the maximum.
																				// histogram value.  Ignore Column 0 and last column.
			{
				if (nHOIHistoRow[nIndex] > nMaxValueRow)						// If current histogram value is larger than nMaxValue,
				{																// update nMaxValue and the corresponding index.
					nMaxValueRow = nHOIHistoRow[nIndex];
					nMaxRow = nIndex;
				}
			}
						
			if ((nMaxCol > 0) && (nMaxRow > 0))									// If both valid column or row value exists.
			{
				if (nMaxValueRow > nMaxValueCol)								// Set the marker size to corresponds to the
				{																// region in the image frame matching the 
					nTemp = nMaxValueRow>>1;									// HOI.  We divide the largest dimension by 2
				}																// so that the marker will not completely overshadow
				else                                                            // the object.
				{
					nTemp = nMaxValueCol>>1;
				}
				if (nTemp == 0)
				{
					nTemp = 3;													// Minimum size is 3x3 pixels.
				}
				gnCameraLED = 4;												// Set camera LED to high, to signal that object of interest is detected
																				// in current frame.
				gobjRec1.nHeight = nTemp;										// Enable a square marker to be displayed in remote monitor
				gobjRec1.nWidth = nTemp;										// software.
				gobjRec1.nX = nMaxCol;
				gobjRec1.nY = nMaxRow;
			}
			else
			{				
				gnCameraLED = 1;												// Set camera LED to low, to signal that no object of interest (with matching color)
																				// is detected.
																				// Invalid column value.
				gobjRec1.nHeight = 0 ;											// Disable square marker to be displayed in remote monitor
				gobjRec1.nWidth = 0 ;											// software.
				gobjRec1.nX = 0;												// Set the location of the marker.
				gobjRec1.nY = 0;				
			}
			gobjRec1.nColor = 4;
			
			gobjRec2.nHeight = 3 ;												// Enable a square marker to be displayed in remote monitor
			gobjRec2.nWidth = 3 ;												// software.  This marker marks the point where the hue will
			gobjRec2.nX = 79;													// be transmitted to the remote monitor for debugging purpose.
			gobjRec2.nY = 59;
			gobjRec2.nColor = 2;
			gunHue = (gunImgAtt[80][60] & _HUE_MASK)>>(_HUE_SHIFT+1);			// Send the hue/2 at the center pixel, for debugging purpose.
			OSSetTaskContext(ptrTask, 4, 1);									// Next state = 4, timer = 1.
			break;
			
			case 4: // State 4 - Transmit status to external controller, part 1 (transmit first 2 bytes).
			// NOTE: 28 Dec 2016.  To prevent overflow error on the remote controller (as it does not use DMA on the UART),
			// we avoid sending all 4 bytes one shot, but split the data packet into two packets or 2 bytes each.  A 1 msec
			// delay is inserted between each two bytes packet.  As the algorithm in the remote controller improves in future
			// this artificial restriction can be removed.

			if (gSCIstatus2.bTXRDY == 0)	// Check if any data to send via UART.
			{
				gbytTXbuffer2[0] = gnImageProcessingAlgorithm;	// Load data, process ID.
				if (nMaxValueRow > nMaxValueCol)
				{
					gbytTXbuffer2[1] = nMaxValueRow;			// The max value is an indication of the object size.
				}
				else
				{
					gbytTXbuffer2[1] = nMaxValueCol;			//
				}				
				gbytTXbuflen2 = 2;								// Set TX frame length.
				gSCIstatus2.bTXRDY = 1;							// Initiate TX.
				OSSetTaskContext(ptrTask, 5, 1*__NUM_SYSTEMTICK_MSEC);     // Next state = 5, timer = 1 msec.
				//OSSetTaskContext(ptrTask, 5, 3);				// Next state = 5, timer = 3c.
			}
			else
			{
				OSSetTaskContext(ptrTask, 4, 1);     // Next state = 4, timer = 1.
			}
			break;

			case 5: // State 5 - Transmit status to external controller, part 2 (transmit last 2 bytes).
			if (gSCIstatus2.bTXRDY == 0)	// Check if any data to send via UART.
			{
				if ((nMaxCol > 0) && (nMaxRow > 0))	// Check if the coordinate is valid, e.g. object matching HOI is detected.
				{
					gbytTXbuffer2[0] = nMaxCol;
					gbytTXbuffer2[1] = nMaxRow;
				}
				else
				{
					gbytTXbuffer2[0] = 255;			// Indicate invalid coordinate.  Basically 0xFF = -1 in 8-bits 2's complement.	
					gbytTXbuffer2[1] = 255;				
				}
				
				//gbytTXbuffer2[1] = 50;				// Set y coordinate to center, matching the firmware in the robot controller.
				gbytTXbuflen2 = 2;					// Set TX frame length.
				gSCIstatus2.bTXRDY = 1;				// Initiate TX.
				OSSetTaskContext(ptrTask, 6, 1*__NUM_SYSTEMTICK_MSEC);    // Next state = 6, timer = 1 msec.
				//OSSetTaskContext(ptrTask, 6, 3);    // Next state = 6, timer = 3.
			}
			else
			{
				OSSetTaskContext(ptrTask, 5, 1);    // Next state = 5, timer = 1.
			}
			break;
						
			case 6: // State 6 - Check if process ID changed.
				if (gnImageProcessingAlgorithm == 3)
				{
					//gnCameraLED = 0;
					OSSetTaskContext(ptrTask, 1, 1);							// Next state = 1, timer = 1.
				}
				else
				{
					OSSetTaskContext(ptrTask, 0, 1);							// Next state = 0, timer = 1.
				}
				PIOA->PIO_ODSR &= ~PIO_ODSR_P19;							// Clear flag, this is optional, for debugging purpose.
			break;			
			
			default:
				OSSetTaskContext(ptrTask, 0, 1);							// Back to state = 0, timer = 1.
			break;
		}
	}
}