Imports System.Threading
Imports System
Imports System.IO
Imports System.Text

'Description:
'This application uses multi-threading to buffer and display image data from a remote device.
'The remote device communicates with this application via COM port.  The COM port can be a 
'virtual COM port from USB or Bluetooth device or an actual physical COM port device.
'To improve efficiency, one thread will buffer the image data line-by-line while another thread
'display the image data onto the window (in a picturebox in this case).  The image data is
'trasmitted line-by-line by the remote device.  To prevent conflict, we have two line buffers, so 
'that when one is being updated with fresh data from the COM port, the other is being accessed to
'updated the image on the picturebox.
'
'
Public Class Main

    'Global public datatypes declaration 
    Public Const mstrSoftwareTitle = "Machine Vision Monitor - "
    Public Const mstrSoftwareVersion = "Version 1.13"
    Public Const mstrSoftwareAuthor = "Fabian Kung Wai Lee"
    Public Const mstrSoftwareDate = "20 Feb 2018"
    Public Const mstrCompany = "Multimedia University"
    Public Const mstrAddress = "Faculty of Engineering, Jalan Multimedia, Cyberjaya, Selangor, MALAYSIA"
    Public Const mstrWebpage = "http://foe.mmu.edu.my"

    Dim nTemp As Integer
    Dim mintComPort As Integer
    Dim mnRGB565Pixel As Integer

    'Bitmap image global variables
    Dim mbytRxData1(0 To 255) As Byte           'Buffer1 for storing 1 line of bitmap pixel data.
    Dim mbytRxData2(0 To 255) As Byte           'Buffer2 for storing 1 line of bitmap pixel data.
    Dim mblnLine1Mutex As Boolean               'Buffer1 mutex. True = Buffer1 is used by buffering thread.
    '                                           'False = Buffer1 is being accessed by bitmap update thread.
    Dim mblnLine2Mutex As Boolean               'Buffer2 mutex. True = Buffer2 is used by buffering thread.
    'False = Buffer2 is being accessed by bitmap update thread.
    Dim mintCurrentBMPLine1Counter As Integer
    Dim mintCurrentBMPLine2Counter As Integer
    Dim mintDataPayload1Length As Integer
    Dim mintDataPayload2Length As Integer
    Dim mbytTxData(0 To 8) As Byte
    Dim mintOption As Integer

    Dim mintWatchDogCounter As Integer

    Dim mintImageWidth As Integer = 160
    Dim mintImageHeight As Integer = 120
    Dim mintImageScale As Integer = 2         '2 - The displayed image will be 2X the actual size.
    '1 - This will display the image in original size (1x).

    'Threads
    Dim t1 As New Thread(AddressOf ThreadProc1) 'Thread to buffer serial data from remote devices.
    Dim t2 As New Thread(AddressOf ThreadProc2) 'Thread to update the bitmap image.

    'Graphics objects

    Private mptBmpStart As Point    'Position of bitmap (the top left hand corner).
    Private mnColor As Integer

    Private mbytPixel(mintImageWidth, mintImageHeight) As Byte '2D array to store image secondary data.
    Private mbytPixel2(mintImageWidth, mintImageHeight) As Byte '2D array to store image data.
    Private mbmpMainBMP As Bitmap   'Main bitmap display for image
    Private mbmpSecBMP As Bitmap   'Secondary bitmap display for showing the result of image processing
    Private mptSecBmpStart As Point     'Position of bitmap (the top left hand corner).
    Private mintYCoorHL As Integer      'Y coordinate of horizontal line (to be superimposed into image for indication purpose)
    Private mintXCoorVL As Integer      'X coordinate of vertical line (to be superimposed into image for indication purpose)

    'Auxiliary information to be shown together with the image.
    Dim mstrMessage As String
    Dim mnData1 As Integer
    Dim mnData2 As Integer
    Dim mnData3 As Integer
    Dim mnData4 As Integer

    'Private mFilePath As String = "C:\FabianKung\Testimage.txt"
    Private mFilePath As String = ""

    Private Const mnRedMask As Integer = &HF800
    Private Const mnGreenMask As Integer = &H7E0
    Private Const mnBlueMask As Integer = &H1F

    Private Const mnLuminanceRGB As Integer = 0
    Private Const mnLuminanceR As Integer = 1
    Private Const mnLuminanceG As Integer = 2
    Private Const mnLuminanceB As Integer = 3
    Private Const mnLuminanceGradient As Integer = 4
    Private Const mnHue As Integer = 5

    'mnRGB565Pixel = mbytRxData(0)
    'mnRGB565Pixel = mnRGB565Pixel << 8
    'mnRGB565Pixel = mnRGB565Pixel + mbytRxData(1) 'Form a 16 bits pixel data (RGB565 format).
    'nR5 = (mnRGB565Pixel And mnRedMask) >> 11
    'nG6 = (mnRGB565Pixel And mnGreenMask) >> 5
    'nB5 = mnRGB565Pixel And mnBlueMask
    'nR8 = (nR5 * 527 + 23) >> 6     'Convert from RGB565 to RGB888 color format (Alternative 1).
    'nG8 = (nG6 * 259 + 33) >> 6
    'nB8 = (nB5 * 527 + 23) >> 6
    'nR8 = nR5 << 3      'Convert from RGB565 to RGB888 color format (Alternative 2)
    'nG8 = nG6 << 2
    'nB8 = nB5 << 3

    Private Sub Main_Load(sender As Object, e As EventArgs) Handles Me.Load

        Me.Text = mstrSoftwareTitle & " " & mstrSoftwareVersion
        ComboBoxCommPort.SelectedIndex = 10  'Default port 11
        LabelInfo.Text = "No COM Port open"


        'Initialize main picture box and graphic objects.
        mbmpMainBMP = New Bitmap(mintImageWidth + 2, mintImageHeight + 2)      'Create a bitmap image at least twice the image frame resolution.
        mptBmpStart = New Point(160, 20)        'Start position for bitmap (upper left corner).
        If mintImageScale = 2 Then               'This will display the image at 2x original size.
            mbmpMainBMP.SetResolution(50, 50)    'Set resolution to 50 dots per inch.
        End If


        RadioButtonMarkerRed.Checked = True
        mnColor = 1

        'Initialize Option
        RadioButtonOptionLuminanceRGB.Checked = True
        mintOption = 0

        'Initialize Threads parameters
        t1.Name = "RobotEyeMon Buffer"
        t2.Name = "RobotEyeMon Display"

    End Sub

    Private Sub Main_FormClosing(sender As Object, e As FormClosingEventArgs) Handles Me.FormClosing

        If (SerialPort1.IsOpen = True) Then 'Close the COM port if it is still open.
            SerialPort1.Close()
        End If

        If (t1.IsAlive) Then     'Abort the secondary thread if it is still running.
            t1.Abort()
            't1.Join()
        End If
        If (t2.IsAlive) Then    'Abort the secondary thread if it is still running.
            t2.Abort()
            't2.Join()
        End If

    End Sub

    Private Sub ButtonClose_Click(sender As Object, e As EventArgs) Handles ButtonClose.Click
        If SerialPort1.IsOpen = True Then
            SerialPort1.Close()
            LabelInfo.Text = "COM Port Closed!"
            ButtonOpen.Enabled = True
            TimerWatchdog.Enabled = False       'Disable watchdog timer.
        End If

        'If (t1.IsAlive) Then     'Abort the secondary thread if it is still running.
        't1.Abort()
        'End If
        'If (t2.IsAlive) Then    'Abort the secondary thread if it is still running.
        't2.Abort()
        'End If
    End Sub

    Private Sub ButtonOpen_Click(sender As Object, e As EventArgs) Handles ButtonOpen.Click
        Try

            If SerialPort1.IsOpen = False Then
                SerialPort1.PortName = "COM" & CStr(ComboBoxCommPort.SelectedIndex + 1)
                SerialPort1.Parity = IO.Ports.Parity.None
                SerialPort1.BaudRate = 115200
                'SerialPort1.BaudRate = 230400
                SerialPort1.ReceivedBytesThreshold = 1
                SerialPort1.ReadTimeout = 500
                SerialPort1.WriteTimeout = 500
                SerialPort1.Open()
                SerialPort1.DiscardInBuffer() 'Flush buffer after initialization.

                If SerialPort1.IsOpen = True Then
                    ButtonOpen.Enabled = False
                End If

                LabelInfo.Text = "COM " & CStr(ComboBoxCommPort.SelectedIndex + 1) & " Port Opened!"
            End If

        Catch ex As Exception
            MessageBox.Show(ex.Message, "ERROR", MessageBoxButtons.OK)
        End Try

    End Sub

    Private Sub ButtonStart_Click(sender As Object, e As EventArgs) Handles ButtonStart.Click
        Dim nXindex As Integer
        Dim nYindex As Integer

        If SerialPort1.IsOpen = True Then
            If mintOption = 0 Then
                mbytTxData(0) = Asc("L")   'Ask remote unit to send luminance data of pixel (RGB)
            ElseIf mintOption = 1 Then
                mbytTxData(0) = Asc("R")   'Ask remote unit to send luminance data of pixel (R only)
            ElseIf mintOption = 2 Then
                mbytTxData(0) = Asc("G")   'Ask remote unit to send luminance data of pixel (G only)
            ElseIf mintOption = 3 Then
                mbytTxData(0) = Asc("B")   'Ask remote unit to send luminance data of pixel (B only)
            Else
                mbytTxData(0) = Asc("D")   'Ask remote unit to send luminance gradient data of pixel
            End If

            For nYindex = 0 To 119          'Initialize the secondary display buffer (for storing marker and ROI
                For nXindex = 0 To 159      'information)
                    mbytPixel(nXindex, nYindex) = 0
                Next
            Next
            mintYCoorHL = 60                        'Set horizontal marker line location
            mintXCoorVL = 80                        'Set vertical marker line location
            For nXindex = 0 To mintImageWidth - 1
                mbytPixel(nXindex, mintYCoorHL) = 1  'Initialize the horizontal line markers.
            Next
            For nYindex = 0 To mintImageHeight - 1
                mbytPixel(mintXCoorVL, nYindex) = 1  'Initialize the vertical line markers.
            Next

            mbytTxData(1) = 0
            SerialPort1.Write(mbytTxData, 0, 1) 'Send a command byte to remote client to start the process
            SerialPort1.DiscardInBuffer()       'Flush buffer to prepare for incoming data.

            mblnLine1Mutex = True               'Start with buffering pixel data for line 1, while displaying
            mblnLine2Mutex = False              'pixel data for line 2.
            TimerWatchdog.Interval = 100        '100  msec to timeout.
            TimerWatchdog.Enabled = True        'Enable watchdog timer.
            mintWatchDogCounter = 0             'Reset watchdog counter.

            If t1.IsAlive = False Then          'July 2016: Once a thread is started, we cannot start it a 2nd time,
                'else this would cause an exception.
                t1.Start()                      'Start the 2nd thread if it is not running.
            End If
            If t2.IsAlive = False Then
                t2.Start()                      'Start the 3rd thread if it is not running.
            End If

        Else
            LabelInfo.Text = "Please open a COM Port first!"
        End If
    End Sub


    Private Sub SerialPort1_ErrorReceived(sender As Object, e As IO.Ports.SerialErrorReceivedEventArgs) Handles SerialPort1.ErrorReceived

        SerialPort1.DiscardInBuffer() 'Flush buffer to prepare for incoming data.
        LabelInfo.Text = "Serial Port Error!"

    End Sub

    Private Sub RadioButtonMarkerRed_CheckedChanged(sender As Object, e As EventArgs) Handles RadioButtonMarkerRed.CheckedChanged

        mnColor = 1

    End Sub

    Private Sub RadioButtonMarkerGreen_CheckedChanged(sender As Object, e As EventArgs) Handles RadioButtonMarkerGreen.CheckedChanged

        mnColor = 2

    End Sub

    Private Sub RadioButtonMarkerBlue_CheckedChanged(sender As Object, e As EventArgs) Handles RadioButtonMarkerBlue.CheckedChanged

        mnColor = 3

    End Sub

    Private Sub RadioButtonMarkerYellow_CheckedChanged(sender As Object, e As EventArgs) Handles RadioButtonMarkerYellow.CheckedChanged

        mnColor = 4

    End Sub

    'TimerWatchdog overflow event handler.
    'This process is used to update the respective text labels on the main window.

    Private Sub TimerWatchdog_Tick(sender As Object, e As EventArgs) Handles TimerWatchdog.Tick
        Static Dim nState As Integer = 0


        Try

            LabelData1.Text = CStr(mnData1)
            LabelData2.Text = CStr(mnData2)
            LabelData3.Text = CStr(mnData3)
            LabelData4.Text = CStr(mnData4)
            LabelData5.Text = CStr(SerialPort1.BytesToRead)
            LabelData6.Text = CStr(mintYCoorHL)
            LabelData7.Text = CStr(mintXCoorVL)

            mintWatchDogCounter = mintWatchDogCounter + 1

            'Me.CreateGraphics.DrawImage(mbmpMainBMP, mptBmpStart)

        Catch ex As Exception
            MessageBox.Show("TimerWatchdog_Tick: " & ex.Message, "ERROR", MessageBoxButtons.OK)
        End Try

    End Sub

    Private Sub RadioButtonOptionLuminance_CheckedChanged(sender As Object, e As EventArgs) Handles RadioButtonOptionLuminanceRGB.CheckedChanged

        mintOption = mnLuminanceRGB

    End Sub

    Private Sub RadioButtonOptionGradient_CheckedChanged(sender As Object, e As EventArgs) Handles RadioButtonOptionGradient.CheckedChanged

        mintOption = mnLuminanceGradient

    End Sub

    Private Sub RadioButtonOptionLuminanceR_CheckedChanged(sender As Object, e As EventArgs) Handles RadioButtonOptionLuminanceR.CheckedChanged

        mintOption = mnLuminanceR

    End Sub

    Private Sub RadioButtonOptionLuminanceG_CheckedChanged(sender As Object, e As EventArgs) Handles RadioButtonOptionLuminanceG.CheckedChanged

        mintOption = mnLuminanceG

    End Sub

    Private Sub RadioButtonOptionLuminanceB_CheckedChanged(sender As Object, e As EventArgs) Handles RadioButtonOptionLuminanceB.CheckedChanged

        mintOption = mnLuminanceB

    End Sub

    Private Sub RadioButtonOptionHue_CheckedChanged(sender As Object, e As EventArgs) Handles RadioButtonOptionHue.CheckedChanged
        mintOption = mnHue
    End Sub

    'Last updated:  18 Feb 2018
    'Author:        Fabian Kung
    'Subroutine to update the bitmap image one line at a time.  Also superimpose markers or ROI.
    '
    '--- 1. Format for 1 line of pixel data ---
    'The image data Is send to the remote display line-by-line, using a simple RLE (Run-Length 
    'Encoding) compression format. The data format:
    'Byte0: 0xFF (Indicate start-of-line)
    'Byte1: Line number, 0-253, indicate the line number In the bitmap image.
    '       if Byte1 = 254, it indicate the following bytes are secondary data.  
    'Byte2: The length Of the data payload, excluding Byte0-2.
    'Byte3-ByteN: Data payload.
    'The data payload only accepts 7 bits value, from bit0-bit6.  Bit7 Is used to indicate
    'repetition in the RLE algorithm, the example below elaborate further.

    'Example
    'Consider the following byte stream:
    '[0xFF][3][7][90][80][70][0x83][60][0x82][120]
    'Byte2 = 7 indicates that there are 6 data bytes in this data packet.
    'Byte3 = 90 represent the 1st pixel data in line 3.  This value can represent the 
    '         luminance value of other user define data.
    'Byte4 = 80 another pixel data.
    'Byte5 = 70 another pixel data.
    'Byte6 = 0x83 = 0b10000000 + 3.  This indicates the last byte, e.g. Byte5=70 Is to
    '         be repeated 3 times, thus there are a total of 4 pixels with value of 70
    '         after pixel with value of 80. 
    'Byte7 = 60 another pixel data.
    'Byte8 = 0x82 = 0b10000000 + 2.  The pixel with value of 60 Is to be repeated 2 times.
    'Byte9 = 120, another pixel.
    '
    'The complete line of pixels Is as follows:
    'Line3 [90][80][70][70][70][70][60][60][60][120]
    '
    'This subroutine must thus be able to decode the above format and setup the bitmap pixels
    'accordingly
    'The maximumum no. of repetition: Currently the maximum no. Of repetition Is 63.  If 
    'there are 64 consequtive pixels with values of 70, we would write [70][0x80 + 0x3F]
    'Or [70][0x10111111].  The '0' at bit 6 is to prevention a repetition code of 0xFF, which
    'can be confused with start-of-line value.
    '
    '
    ' --- 2. Markers/ROI ---
    ' We can ovellap marker or ROI (region of interest) onto the bitmap image.
    ' A 2D array mbytPixel(x,y) is used to keep track of the shape and size of markers or ROI that 
    ' would be superimposed onto the bitmap image.  
    ' Whenever the value of the element in mbytPixel(x,y) is <> 0, the pixel in the bitmap will 
    ' be colored according to the value in mbytPixel(x,y).  If it is 0, the color of the bitmap
    ' will be determined by the image pixel data received from the external module.
    Private Sub DisplayBitmap()

        Static Dim nR8 As Integer
        Static Dim nHue As Integer
        Dim nData As Integer
        Dim nRepetition As Integer
        Dim nX As Integer
        Dim nY As Integer
        Dim nIndex As Integer
        Dim nIndex2 As Integer
        Dim colorPixel As Color
        Dim colorMarker As Color
        Dim nXCoor As Integer
        Dim nYCoor As Integer

        Try

            If mblnLine1Mutex = False Then           'False means buffering thread is not using the memory associated with Line 1 image data.
                nY = mintCurrentBMPLine1Counter + 1  'Get line number, add 1 since the value starts from 0.  The bitmap coordinate index starts 
            Else                                     'from 1.
                nY = mintCurrentBMPLine2Counter + 1
            End If

            If (nY <= mintImageHeight) Then         'Make sure line no. is less than image height in pixels.

                'window coordinate starts from 1.
                nYCoor = nY   'Form Y coordinate of pixel in bitmap.

                nX = 0
                'Retrive gray scale data and plot on bitmap.  

                For nIndex = 0 To mintDataPayload1Length - 1
                    If mblnLine1Mutex = False Then
                        nData = mbytRxData1(nIndex)    'Get pixel luminance data or repetition number from the correct buffer.
                    Else
                        nData = mbytRxData2(nIndex)
                    End If

                    If (nData > 128) Then               'Check if bit7 is set, this indicate repetition number.
                        nRepetition = nData And &H3F    'Mask out bit7 and bit6.
                        If (nRepetition > 0) Then       'Make sure repetition is not 0.
                            For nIndex2 = 0 To nRepetition - 1

                                nX = nX + 1
                                If (nX > mintImageWidth) Then 'Make sure X position is smaller than image width.
                                    Return
                                End If

                                nXCoor = nX        'Form X coordinate of pixel in bitmap.

                                'Optional - store the pixel for subsequent image processing
                                mbytPixel2(nX - 1, nY - 1) = nR8 >> 1 'Actual gray scale is only 7 bits, so we increase it's effective value!

                                'Form a pixel 
                                If mbytPixel(nX - 1, nY - 1) > 0 Then  'Check if to update the image or marker/ROI.
                                    Select Case mbytPixel(nX - 1, nY - 1)
                                        Case 1
                                            colorMarker = Color.Red
                                        Case 2
                                            colorMarker = Color.Green
                                        Case 3
                                            colorMarker = Color.Blue
                                        Case 4
                                            colorMarker = Color.Yellow
                                        Case 5
                                            colorMarker = Color.Orange
                                        Case Else
                                            colorMarker = Color.Pink
                                    End Select
                                    mbmpMainBMP.SetPixel(nXCoor, nYCoor, colorMarker)    'Update marker/ROI.
                                Else
                                    mbmpMainBMP.SetPixel(nXCoor, nYCoor, colorPixel)    'Update image.
                                End If

                            Next nIndex2
                        End If 'If (nRepetition > 0)
                    Else            'Not repetition, new pixel data.
                        nX = nX + 1
                        If (nX > mintImageWidth) Then 'Make sure X position is smaller than image width.
                            Return
                        End If

                        nXCoor = nX                     'Form X coordinate of pixel in bitmap.
                        nR8 = nData                     'Read pixel data
                        If nR8 > 127 Then               'Limit the maximum value.
                            nR8 = 127
                        End If

                        'Set the image pixel color scheme according to the luminance option selected.
                        If mintOption = mnLuminanceR Then
                                colorPixel = Color.FromArgb(nR8, 0, 0)      'Generate a red scale level.
                            ElseIf mintOption = mnLuminanceG Then
                                colorPixel = Color.FromArgb(0, nR8, 0)      'Generate a green scale level.
                            ElseIf mintOption = mnLuminanceB Then
                                colorPixel = Color.FromArgb(0, 0, nR8)      'Generate a blue scale level.
                            ElseIf mintOption = mnHue Then

                                nHue = 4 * nR8  'Rescale back the hue value to between 0-360 for valid hue.
                                'In the remote unit the hue valus is divide by 4 before
                                'being transmitted.

                                'Algorithm to convert Hue to RGB color representation.
                                'Here valid value of hue is 0-360.
                                'nHue = 400 if no hue due to too dark.
                                'nHue = 420 if no hue due to too bright or near gray scale condition.
                                If nHue < 60 Then
                                    colorPixel = Color.FromArgb(255, (nHue / 60) * 255, 0)
                                ElseIf nHue < 120 Then
                                    colorPixel = Color.FromArgb(255 * (1 - ((nHue - 60) / 60)), 255, 0)
                                ElseIf nHue < 180 Then
                                    colorPixel = Color.FromArgb(0, 255, ((nHue - 120) / 60) * 255)
                                ElseIf nHue < 240 Then
                                    colorPixel = Color.FromArgb(0, 255 * (1 - ((nHue - 180) / 60)), 255)
                                ElseIf nHue < 300 Then
                                    colorPixel = Color.FromArgb(((nHue - 240) / 60) * 255, 0, 255)
                                ElseIf nHue < 361 Then
                                    colorPixel = Color.FromArgb(255, 0, 255 * (1 - ((nHue - 300) / 60)))
                                ElseIf nHue < 404 Then                'Saturation level too low, no color.
                                    colorPixel = Color.FromArgb(0, 0, 0)    'Black
                                Else
                                    colorPixel = Color.FromArgb(255, 255, 255)  'White
                                End If
                            Else                                            'Either luminance or others.
                                nR8 = 2 * nR8                               'Increase the brightness by 2, optional.
                                colorPixel = Color.FromArgb(nR8, nR8, nR8)   'Generate a gray scale level.
                            End If

                            'Optional - store the pixel for subsequent image processing
                            mbytPixel2(nX - 1, nY - 1) = nR8 >> 1 'Actual gray scale is only 7 bits, so we increase it's effective value.

                        'Form a pixel 
                        If mbytPixel(nX - 1, nY - 1) > 0 Then  'Check if to update the image or marker/ROI.
                            Select Case mbytPixel(nX - 1, nY - 1)
                                Case 1
                                    colorMarker = Color.Red
                                Case 2
                                    colorMarker = Color.Green
                                Case 3
                                    colorMarker = Color.Blue
                                Case 4
                                    colorMarker = Color.Yellow
                                Case 5
                                    colorMarker = Color.Orange
                                Case Else
                                    colorMarker = Color.Pink
                            End Select
                            mbmpMainBMP.SetPixel(nXCoor, nYCoor, colorMarker)       'Update marker/ROI.
                        Else
                            mbmpMainBMP.SetPixel(nXCoor, nYCoor, colorPixel)        'Update image.
                        End If

                    End If 'If (nData > 128)

                Next nIndex

                If (nYCoor Mod 2) = 0 Then  'Only update the bitmap display every other lines.
                    Me.CreateGraphics.DrawImage(mbmpMainBMP, mptBmpStart)  'Update the main bitmap display.  This follows the example in MSDN.
                End If

            End If 'End of If (nY <= mintImageHeight) Then

        Catch ex As ThreadAbortException

        Catch ex As Exception
            MessageBox.Show(ex.Message, "DisplayBitmap Error at line " & CStr(nY), MessageBoxButtons.OK)
        End Try

    End Sub

    'Date:      30 Jan 2018
    'Author:    Fabian Kung
    'Subroutine to draw rectangles on the secondary display buffer.  This version supports up to
    'two rectangles.
    'The value stored in 2D array mbytPixel indicates whether this pixel should be
    'highlight or not.  If the value stored is 0, the pixel is not highlighted.
    'Otherwise the pixel will be highlighted, with the color of the pixel corresponds
    'to the value.
    Private Sub DisplaySecondaryInfo()

        Dim nXindex As Integer
        Dim nYindex As Integer
        Dim nMIndex As Integer

        'Rectangles detail
        Dim nXstart(2) As Integer
        Dim nYstart(2) As Integer
        Dim nXsize(2) As Integer
        Dim nYsize(2) As Integer
        Static Dim nXstartold(2) As Integer
        Static Dim nYstartold(2) As Integer
        Static Dim nXsizeold(2) As Integer
        Static Dim nYsizeold(2) As Integer
        Dim nColorSetting(2) As Integer


        Try
            If mblnLine1Mutex = False Then      'Determine which buffer to extract the data (buffer 1 or 2).
                nXsize(0) = mbytRxData1(0)         'Get the location and size of marker 1 if present.    
                nYsize(0) = mbytRxData1(1)
                nXstart(0) = mbytRxData1(2)
                nYstart(0) = mbytRxData1(3)
                nColorSetting(0) = mbytRxData1(4)  'Get color of marker 1.
                nXsize(1) = mbytRxData1(5)         'Get the location and size of marker 2 if present.    
                nYsize(1) = mbytRxData1(6)
                nXstart(1) = mbytRxData1(7)
                nYstart(1) = mbytRxData1(8)
                nColorSetting(1) = mbytRxData1(9)  'Get color of marker 2.

            Else
                nXsize(0) = mbytRxData2(0)         'Get the location and size of marker 1 if present.
                nYsize(0) = mbytRxData2(1)
                nXstart(0) = mbytRxData2(2)
                nYstart(0) = mbytRxData2(3)
                nColorSetting(0) = mbytRxData2(4)  'Get color of marker 1.
                nXsize(1) = mbytRxData2(5)         'Get the location and size of marker 2 if present.    
                nYsize(1) = mbytRxData2(6)
                nXstart(1) = mbytRxData2(7)
                nYstart(1) = mbytRxData2(8)
                nColorSetting(1) = mbytRxData2(9)  'Get color of marker 2.
            End If

            'Debug, send info to Main Window
            mnData1 = nXstart(0)
            mnData2 = nYstart(0)
            mnData3 = nXstart(1)
            mnData4 = nYstart(1)

            'Highlight rectangular marker 1 in the Secondary Display Buffer.
            'We do this by scanning through each pixel in the image frame.  For those pixel which is to be changed to
            'marker color set the correponding value in mbytPixel(x,y) to a non-zero value, the value is the marker color.
            'if mbytPixel(x,y) value is 0, the corresponding pixel takes the value of the image frame.
            For nMIndex = 0 To 1
                If ((nXstart(nMIndex) + nXsize(nMIndex)) < (mintImageWidth - 1)) And ((nYstart(nMIndex) + nYsize(nMIndex)) < (mintImageHeight - 1)) Then

                    For nYindex = nYstartold(nMIndex) To (nYstartold(nMIndex) + nYsizeold(nMIndex) - 1)     'Clear previous marker/ROI
                        For nXindex = nXstartold(nMIndex) To (nXstartold(nMIndex) + nXsizeold(nMIndex) - 1)
                            mbytPixel(nXindex, nYindex) = 0
                        Next
                    Next

                    'Check if bit 7 of the color setting is set.  If set then only the outline of the ROI
                    'is drawn, else fill the ROI.
                    If (nColorSetting(nMIndex) > 127) Then 'Bit 7 = 128 in decimal
                        'Draw the 4 borders on the square region.
                        nColorSetting(nMIndex) = nColorSetting(nMIndex) - 128 'Clear bit7 to extract the color
                        nYindex = nYstart(nMIndex)
                        For nXindex = nXstart(nMIndex) To (nXstart(nMIndex) + nXsize(nMIndex) - 1)  'Horizontal border 1.
                            mbytPixel(nXindex, nYindex) = nColorSetting(nMIndex)
                        Next
                        nYindex = nYstart(nMIndex) + nYsize(nMIndex) - 1
                        For nXindex = nXstart(nMIndex) To (nXstart(nMIndex) + nXsize(nMIndex) - 1)  'Horizontal border 2.
                            mbytPixel(nXindex, nYindex) = nColorSetting(nMIndex)
                        Next
                        nXindex = nXstart(nMIndex)
                        For nYindex = nYstart(nMIndex) To (nYstart(nMIndex) + nYsize(nMIndex) - 1)  'Vertical border 1.
                            mbytPixel(nXindex, nYindex) = nColorSetting(nMIndex)
                        Next
                        nXindex = nXstart(nMIndex) + nXsize(nMIndex) - 1
                        For nYindex = nYstart(nMIndex) To (nYstart(nMIndex) + nYsize(nMIndex) - 1)  'Vertical border 2.
                            mbytPixel(nXindex, nYindex) = nColorSetting(nMIndex)
                        Next
                    Else                                                     'Fill a rectangular region.   
                        For nYindex = nYstart(nMIndex) To (nYstart(nMIndex) + nYsize(nMIndex) - 1)     'Highlight current marker/ROI
                            For nXindex = nXstart(nMIndex) To (nXstart(nMIndex) + nXsize(nMIndex) - 1)
                                mbytPixel(nXindex, nYindex) = nColorSetting(nMIndex)
                            Next
                        Next
                    End If

                    nXstartold(nMIndex) = nXstart(nMIndex)                'Store current parameters.
                    nYstartold(nMIndex) = nYstart(nMIndex)
                    nXsizeold(nMIndex) = nXsize(nMIndex)
                    nYsizeold(nMIndex) = nYsize(nMIndex)
                End If
            Next

        Catch ex As ThreadAbortException

        Catch ex As Exception
            MessageBox.Show(ex.Message, "DisplaySecondaryInfo Error ", MessageBoxButtons.OK)
        End Try
    End Sub


    'Thread 1: RobotEyeMon Receive Buffer
    'This thread uses a state machine to read one line of pixel data into the display buffer.
    '
    'Each packet consists of:
    'Byte0: 0xFF (Start-of-line code)
    'Byte1: Line number for bitmap
    'Byte2: No. of bytes in the payload
    'Byte3 to last byte: payload.
    'For instance if Byte2 = 50, then last byte will be Byte53.
    '
    'The state machine reads the first 3 bytes, then decides how many bytes to 
    'anticipate for the payload.  After reading the payload, it will send an
    'instruction back to the remote device to send the next line of pixel data.
    'The payload data is compressed using simple RLE (Run-Length Encoding)
    'method. See further details on the DisplayBitmap() subroutine.
    '
    'There are two dispay buffers, mbytRxData1[] and mbytRxData2[].  This is to improve
    'the efficiency and prevent conflict for the buffering and displaying process.  
    'Which buffers will be used by this thread is determined by the mutex 
    'For instance if this thread is buffering pixel data into mbytRxData1[], 
    'the DisplayBitmap() subroutine will access the pixel data in mbytRxData2[] and 
    'update the picturebox.  Which display buffer will be used by Thread1 and
    'DisplayBitmap() is determined by the mutexes mblnLine1Mutex and mblnLine2Mutex.
    '
    'For instance if mblnLine1Mutex = True and mblnLine2Mutex = False, this indicates
    'DisplayBitmap() subroutine is updating the picturebox using the pixel data stored
    'in mbytRxData2[], so buffering of data from COM port should be to mbytRxData1[].

    Public Sub ThreadProc1()
        Static Dim nState As Integer = 0
        Static Dim nLine As Integer
        Static Dim nLength As Integer
        Dim nRxData As Integer

        Try
            While 1
                If SerialPort1.IsOpen = True Then
                    Select Case nState
                        Case 0 'Line 1: Check for start-of-line code and get the line no. and packet length.
                            If (SerialPort1.BytesToRead > 2) Then 'Start-of-line code, line number and packet length, 3 bytes.
                                nRxData = SerialPort1.ReadByte()
                                If (nRxData = &HFF) Then                'Start-of-line code?
                                    nLine = SerialPort1.ReadByte()      'Yes, proceed to get image line no. and
                                    nLength = SerialPort1.ReadByte()    'payload lenght in bytes.
                                    nState = 1
                                Else
                                    SerialPort1.DiscardInBuffer() 'Flush buffer, wrong format
                                    nState = 0
                                End If
                            Else
                                nState = 0
                            End If

                        Case 1 'Line 1: Get the rest of the payload.
                            If SerialPort1.BytesToRead > (nLength - 1) Then   'Wait until full line of pixel data has arrived
                                If mblnLine1Mutex = True Then                               'Make sure the receive buffer1 is not in used
                                    mintCurrentBMPLine1Counter = nLine
                                    mintDataPayload1Length = nLength
                                    SerialPort1.Read(mbytRxData1, 0, mintDataPayload1Length) 'Read single line pixel data into buffer1
                                    SerialPort1.DiscardInBuffer()                           'Flush buffer to prepare for incoming data.
                                    'Ask the remote client to send another line of pixel data.
                                    If mintOption = 0 Then          'Check option and send the correct request.
                                        mbytTxData(0) = Asc("L")    'Ask remote unit to send luminance data of pixel
                                    ElseIf mintOption = 1 Then
                                        mbytTxData(0) = Asc("R")   'Ask remote unit to send luminance data of pixel (R only)
                                    ElseIf mintOption = 2 Then
                                        mbytTxData(0) = Asc("G")   'Ask remote unit to send luminance data of pixel (G only)
                                    ElseIf mintOption = 3 Then
                                        mbytTxData(0) = Asc("B")   'Ask remote unit to send luminance data of pixel (B only)
                                    ElseIf mintOption = 4 Then
                                        mbytTxData(0) = Asc("D")   'Ask remote unit to send luminance gradient data of pixel
                                    Else
                                        mbytTxData(0) = Asc("H")   'Ask remote unit to send hue data of pixel
                                    End If
                                    mbytTxData(1) = 0
                                    SerialPort1.Write(mbytTxData, 0, 1) 'Send a command byte to remote client to start the process
                                    nState = 2
                                Else
                                    nState = 1
                                End If
                            Else
                                nState = 1
                            End If

                        Case 2 'Line 2: Check for start-of-line
                            If (SerialPort1.BytesToRead > 2) Then
                                nRxData = SerialPort1.ReadByte()
                                If (nRxData = &HFF) Then                'Start-of-line code?
                                    nLine = SerialPort1.ReadByte()      'Yes, proceed to get image line no. and
                                    nLength = SerialPort1.ReadByte()    'payload lenght in bytes.
                                    nState = 3
                                Else
                                    SerialPort1.DiscardInBuffer() 'Flush buffer, wrong format
                                    nState = 2
                                End If
                            Else
                                nState = 0
                            End If

                        Case 3 'Line 2: Get the rest of the payload.
                            If SerialPort1.BytesToRead > (nLength - 1) Then   'Wait until full line of pixel data has arrived
                                If mblnLine2Mutex = True Then                               'Make sure the receive buffer2 is not in used
                                    mintCurrentBMPLine2Counter = nLine
                                    mintDataPayload2Length = nLength
                                    SerialPort1.Read(mbytRxData2, 0, mintDataPayload2Length) 'Read single line pixel data into buffer1
                                    SerialPort1.DiscardInBuffer()                           'Flush buffer to prepare for incoming data.

                                    'Ask the remote client to send another line of pixel data.
                                    If mintOption = 0 Then          'Check option and send the correct request.
                                        mbytTxData(0) = Asc("L")    'Ask remote unit to send luminance data of pixel
                                    ElseIf mintOption = 1 Then
                                        mbytTxData(0) = Asc("R")   'Ask remote unit to send luminance data of pixel (R only)
                                    ElseIf mintOption = 2 Then
                                        mbytTxData(0) = Asc("G")   'Ask remote unit to send luminance data of pixel (G only)
                                    ElseIf mintOption = 3 Then
                                        mbytTxData(0) = Asc("B")   'Ask remote unit to send luminance data of pixel (B only)
                                    ElseIf mintOption = 4 Then
                                        mbytTxData(0) = Asc("D")   'Ask remote unit to send luminance gradient data of pixel
                                    Else
                                        mbytTxData(0) = Asc("H")   'Ask remote unit to send hue data of pixel
                                    End If
                                    mbytTxData(1) = 0
                                    SerialPort1.Write(mbytTxData, 0, 1) 'Send a command byte to remote client to start the process
                                    nState = 0
                                Else
                                    nState = 3
                                End If
                            Else
                                nState = 3
                            End If

                        Case Else
                            nState = 0
                    End Select
                End If 'If SerialPort1.IsOpen = True
            End While

            'Catch ex As Exception
        Catch ex As ThreadAbortException
            'MessageBox.Show("Thread1: " & CStr(nState) & ": " & ex.Message, "ERROR", MessageBoxButtons.OK)
        End Try

    End Sub

    'Thread 2: RobotEyeMon Image Display
    'This thread uses a state machine to update one line of pixel data in the picturebox.
    'Odd and even line number are updated alternately.  E.g. when Odd line is being displayed on the bitmap
    'even line is being buffered from the camera.
    'Mutex is used to prevent conflict of resouces.
    '
    Public Sub ThreadProc2()
        Static Dim nState As Integer = 0

        Try

            While 1
                Select Case nState
                    Case 0 'Display line 2
                        If mintCurrentBMPLine2Counter < mintImageHeight Then 'Make sure pixel line is within display area.
                            DisplayBitmap()
                        ElseIf mintCurrentBMPLine2Counter = 254 Then         'If equal 254 then proceed to show secondary information.
                            DisplaySecondaryInfo()
                        End If
                        mblnLine1Mutex = False
                        mblnLine2Mutex = True   'Data in buffer2 is used, can replenish with new pixel data.
                        nState = 1
                    Case 1 'Display line 1
                        If mintCurrentBMPLine1Counter < mintImageHeight Then 'Make sure pixel line is within display area.
                            DisplayBitmap()
                        ElseIf mintCurrentBMPLine1Counter = 254 Then        'If equal 254 then proceed to show secondary information.
                            DisplaySecondaryInfo()
                        End If
                        mblnLine1Mutex = True   'Data in buffer1 is used, can replenish with new pixel data.
                        mblnLine2Mutex = False
                        nState = 0

                    Case Else
                        nState = 0
                End Select

            End While
        Catch ex As ThreadAbortException
            'MessageBox.Show("Thread2: " & CStr(nState) & ": " & ex.Message, "ERROR", MessageBoxButtons.OK)
        End Try
    End Sub

    Private Sub ButtonHLUp_Click(sender As Object, e As EventArgs) Handles ButtonHLUp.Click

        Dim nIndex As Integer

        If mintYCoorHL > 1 Then
            mintYCoorHL = mintYCoorHL - 1
            For nIndex = 0 To mintImageWidth - 1
                If nIndex <> mintXCoorVL Then
                    mbytPixel(nIndex, mintYCoorHL + 1) = 0      'Clear the current pixels of marker.
                    mbytPixel(nIndex, mintYCoorHL) = mnColor    'Mark the next row of pixels.
                End If
            Next

        End If

    End Sub

    Private Sub ButtonHLDown_Click(sender As Object, e As EventArgs) Handles ButtonHLDown.Click

        Dim nIndex As Integer

        If mintYCoorHL < mintImageHeight - 1 Then
            mintYCoorHL = mintYCoorHL + 1
            For nIndex = 0 To mintImageWidth - 1
                If nIndex <> mintXCoorVL Then
                    mbytPixel(nIndex, mintYCoorHL - 1) = 0      'Clear the current pixels of marker.
                    mbytPixel(nIndex, mintYCoorHL) = mnColor    'Mark the next row of pixels.
                End If
            Next
        End If

    End Sub

    Private Sub ButtonVLRight_Click(sender As Object, e As EventArgs) Handles ButtonVLRight.Click
        Dim nIndex As Integer

        If mintXCoorVL < mintImageHeight - 1 Then
            mintXCoorVL = mintXCoorVL + 1
            For nIndex = 0 To mintImageHeight - 1
                If nIndex <> mintYCoorHL Then
                    mbytPixel(mintXCoorVL - 1, nIndex) = 0      'Clear the current pixels of marker.
                    mbytPixel(mintXCoorVL, nIndex) = mnColor    'Mark the next column of pixels.
                End If
            Next

        End If
    End Sub

    Private Sub ButtonVLLeft_Click(sender As Object, e As EventArgs) Handles ButtonVLLeft.Click
        Dim nIndex As Integer

        If mintXCoorVL > 1 Then
            mintXCoorVL = mintXCoorVL - 1
            For nIndex = 0 To mintImageHeight - 1
                If nIndex <> mintYCoorHL Then
                    mbytPixel(mintXCoorVL + 1, nIndex) = 0      'Clear the current pixels of marker.
                    mbytPixel(mintXCoorVL, nIndex) = mnColor    'Mark the next column of pixels.
                End If
            Next

        End If
    End Sub

    Private Sub ButtonSaveImage_Click(sender As Object, e As EventArgs) Handles ButtonSaveImage.Click
        Dim nYindex As Integer
        Dim nXindex As Integer
        Dim strString As String

        If mFilePath = "" Then

            OpenFileDialog1.ShowDialog()
            mFilePath = OpenFileDialog1.FileName

        End If
        'Note: The image frame is stored as an array of 2D pixels, each with value from 0-255.
        'The x index corresponds to the row, and y index corresponds to the height.
        'Here we write the pixel data to the file row-by-row.
        If mFilePath <> "" Then
            Using sw As StreamWriter = File.CreateText(mFilePath)
                For nYindex = 1 To mintImageHeight - 1
                    strString = ""                  'Clear the string first.
                    For nXindex = 1 To mintImageWidth - 1
                        strString = strString & ChrW(mbytPixel2(nXindex, nYindex)) 'Form a line representing a line of pixel data.  Somehow 
                        'the binary value in mbytPixel() array is store as it
                        'is into the strString array with ChrW() function is used.
                    Next
                    sw.WriteLine(strString)     'Write the pixel data to file, a NL/CR character will be appended at the end of each line.
                Next
                LabelInfo.Text = "Save complete"
            End Using
        End If


        '---Alternative approach using WriteAllBytes() method---
        'Dim bytData(0 To 160) As Byte

        '-Write 1st line to file-
        'nYindex = 1
        'For nXindex = 1 To mintImageWidth - 1
        '  bytData(nXindex) = mbytPixel2(nXindex, nYindex)
        'Next
        'bytData(160) = 255  'A value to indicate end-of-line (EOL).
        'My.Computer.FileSystem.WriteAllBytes(mFilePath, bytData, False)

        'For nYindex = 2 To mintImageHeight - 1
        '  For nXindex = 1 To mintImageWidth - 1
        '    bytData(nXindex) = mbytPixel2(nXindex, nYindex)
        '  Next
        '  bytData(160) = 255  'A value to indicate end-of-line (EOL).
        '  My.Computer.FileSystem.WriteAllBytes(mFilePath, bytData, True)
        'Next

    End Sub

End Class

