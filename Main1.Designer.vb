<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()>
Partial Class Main
    Inherits System.Windows.Forms.Form

    'Form overrides dispose to clean up the component list.
    <System.Diagnostics.DebuggerNonUserCode()>
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()>
    Private Sub InitializeComponent()
        Me.components = New System.ComponentModel.Container()
        Dim resources As System.ComponentModel.ComponentResourceManager = New System.ComponentModel.ComponentResourceManager(GetType(Main))
        Me.ComboBoxCommPort = New System.Windows.Forms.ComboBox()
        Me.SerialPort1 = New System.IO.Ports.SerialPort(Me.components)
        Me.ButtonOpen = New System.Windows.Forms.Button()
        Me.LabelInfo = New System.Windows.Forms.Label()
        Me.ButtonClose = New System.Windows.Forms.Button()
        Me.ButtonStart = New System.Windows.Forms.Button()
        Me.RadioButtonMarkerRed = New System.Windows.Forms.RadioButton()
        Me.PanelMarker = New System.Windows.Forms.Panel()
        Me.Label1 = New System.Windows.Forms.Label()
        Me.RadioButtonMarkerYellow = New System.Windows.Forms.RadioButton()
        Me.RadioButtonMarkerBlue = New System.Windows.Forms.RadioButton()
        Me.RadioButtonMarkerGreen = New System.Windows.Forms.RadioButton()
        Me.TimerWatchdog = New System.Windows.Forms.Timer(Me.components)
        Me.PanelOption = New System.Windows.Forms.Panel()
        Me.RadioButtonOptionHue = New System.Windows.Forms.RadioButton()
        Me.RadioButtonOptionLuminanceB = New System.Windows.Forms.RadioButton()
        Me.RadioButtonOptionLuminanceG = New System.Windows.Forms.RadioButton()
        Me.RadioButtonOptionLuminanceR = New System.Windows.Forms.RadioButton()
        Me.RadioButtonOptionGradient = New System.Windows.Forms.RadioButton()
        Me.RadioButtonOptionLuminanceRGB = New System.Windows.Forms.RadioButton()
        Me.LabelData1 = New System.Windows.Forms.Label()
        Me.LabelData2 = New System.Windows.Forms.Label()
        Me.LabelData3 = New System.Windows.Forms.Label()
        Me.LabelData4 = New System.Windows.Forms.Label()
        Me.Label2 = New System.Windows.Forms.Label()
        Me.Label3 = New System.Windows.Forms.Label()
        Me.Label4 = New System.Windows.Forms.Label()
        Me.Label5 = New System.Windows.Forms.Label()
        Me.LabelData5 = New System.Windows.Forms.Label()
        Me.Label6 = New System.Windows.Forms.Label()
        Me.LabelData6 = New System.Windows.Forms.Label()
        Me.Label7 = New System.Windows.Forms.Label()
        Me.ButtonHLUp = New System.Windows.Forms.Button()
        Me.ButtonHLDown = New System.Windows.Forms.Button()
        Me.ButtonSaveImage = New System.Windows.Forms.Button()
        Me.ButtonVLRight = New System.Windows.Forms.Button()
        Me.ButtonVLLeft = New System.Windows.Forms.Button()
        Me.LabelData7 = New System.Windows.Forms.Label()
        Me.Label10 = New System.Windows.Forms.Label()
        Me.ButtonTest = New System.Windows.Forms.Button()
        Me.LabelTest = New System.Windows.Forms.Label()
        Me.OpenFileDialog1 = New System.Windows.Forms.OpenFileDialog()
        Me.PanelMarker.SuspendLayout()
        Me.PanelOption.SuspendLayout()
        Me.SuspendLayout()
        '
        'ComboBoxCommPort
        '
        Me.ComboBoxCommPort.FormattingEnabled = True
        Me.ComboBoxCommPort.Items.AddRange(New Object() {"COM 1", "COM 2", "COM 3", "COM 4", "COM 5", "COM 6", "COM 7", "COM 8", "COM 9", "COM 10", "COM 11", "COM 12", "COM 13", "COM 14", "COM 15", "COM 16", "COM 17", "COM 18", "COM 19", "COM 20", "COM 21", "COM 22", "COM 23", "COM 24"})
        Me.ComboBoxCommPort.Location = New System.Drawing.Point(12, 24)
        Me.ComboBoxCommPort.Name = "ComboBoxCommPort"
        Me.ComboBoxCommPort.Size = New System.Drawing.Size(118, 21)
        Me.ComboBoxCommPort.TabIndex = 0
        '
        'SerialPort1
        '
        Me.SerialPort1.ReadBufferSize = 1024
        Me.SerialPort1.ReadTimeout = 1000
        '
        'ButtonOpen
        '
        Me.ButtonOpen.Location = New System.Drawing.Point(12, 64)
        Me.ButtonOpen.Name = "ButtonOpen"
        Me.ButtonOpen.Size = New System.Drawing.Size(66, 45)
        Me.ButtonOpen.TabIndex = 2
        Me.ButtonOpen.Text = "Open Port"
        Me.ButtonOpen.UseVisualStyleBackColor = True
        '
        'LabelInfo
        '
        Me.LabelInfo.Location = New System.Drawing.Point(9, 466)
        Me.LabelInfo.Name = "LabelInfo"
        Me.LabelInfo.Size = New System.Drawing.Size(371, 27)
        Me.LabelInfo.TabIndex = 3
        '
        'ButtonClose
        '
        Me.ButtonClose.Location = New System.Drawing.Point(12, 115)
        Me.ButtonClose.Name = "ButtonClose"
        Me.ButtonClose.Size = New System.Drawing.Size(66, 45)
        Me.ButtonClose.TabIndex = 4
        Me.ButtonClose.Text = "Close Port/Stop"
        Me.ButtonClose.UseVisualStyleBackColor = True
        '
        'ButtonStart
        '
        Me.ButtonStart.Location = New System.Drawing.Point(12, 166)
        Me.ButtonStart.Name = "ButtonStart"
        Me.ButtonStart.Size = New System.Drawing.Size(66, 43)
        Me.ButtonStart.TabIndex = 5
        Me.ButtonStart.Text = "Start"
        Me.ButtonStart.UseVisualStyleBackColor = True
        '
        'RadioButtonMarkerRed
        '
        Me.RadioButtonMarkerRed.AutoSize = True
        Me.RadioButtonMarkerRed.Location = New System.Drawing.Point(9, 3)
        Me.RadioButtonMarkerRed.Name = "RadioButtonMarkerRed"
        Me.RadioButtonMarkerRed.Size = New System.Drawing.Size(45, 17)
        Me.RadioButtonMarkerRed.TabIndex = 6
        Me.RadioButtonMarkerRed.TabStop = True
        Me.RadioButtonMarkerRed.Text = "Red"
        Me.RadioButtonMarkerRed.UseVisualStyleBackColor = True
        '
        'PanelMarker
        '
        Me.PanelMarker.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle
        Me.PanelMarker.Controls.Add(Me.Label1)
        Me.PanelMarker.Controls.Add(Me.RadioButtonMarkerYellow)
        Me.PanelMarker.Controls.Add(Me.RadioButtonMarkerBlue)
        Me.PanelMarker.Controls.Add(Me.RadioButtonMarkerGreen)
        Me.PanelMarker.Controls.Add(Me.RadioButtonMarkerRed)
        Me.PanelMarker.Location = New System.Drawing.Point(12, 357)
        Me.PanelMarker.Name = "PanelMarker"
        Me.PanelMarker.Size = New System.Drawing.Size(154, 98)
        Me.PanelMarker.TabIndex = 7
        '
        'Label1
        '
        Me.Label1.AutoSize = True
        Me.Label1.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label1.Location = New System.Drawing.Point(68, 7)
        Me.Label1.Name = "Label1"
        Me.Label1.Size = New System.Drawing.Size(79, 13)
        Me.Label1.TabIndex = 10
        Me.Label1.Text = "Marker Color"
        '
        'RadioButtonMarkerYellow
        '
        Me.RadioButtonMarkerYellow.AutoSize = True
        Me.RadioButtonMarkerYellow.Location = New System.Drawing.Point(9, 72)
        Me.RadioButtonMarkerYellow.Name = "RadioButtonMarkerYellow"
        Me.RadioButtonMarkerYellow.Size = New System.Drawing.Size(56, 17)
        Me.RadioButtonMarkerYellow.TabIndex = 9
        Me.RadioButtonMarkerYellow.TabStop = True
        Me.RadioButtonMarkerYellow.Text = "Yellow"
        Me.RadioButtonMarkerYellow.UseVisualStyleBackColor = True
        '
        'RadioButtonMarkerBlue
        '
        Me.RadioButtonMarkerBlue.AutoSize = True
        Me.RadioButtonMarkerBlue.Location = New System.Drawing.Point(9, 49)
        Me.RadioButtonMarkerBlue.Name = "RadioButtonMarkerBlue"
        Me.RadioButtonMarkerBlue.Size = New System.Drawing.Size(46, 17)
        Me.RadioButtonMarkerBlue.TabIndex = 8
        Me.RadioButtonMarkerBlue.TabStop = True
        Me.RadioButtonMarkerBlue.Text = "Blue"
        Me.RadioButtonMarkerBlue.UseVisualStyleBackColor = True
        '
        'RadioButtonMarkerGreen
        '
        Me.RadioButtonMarkerGreen.AutoSize = True
        Me.RadioButtonMarkerGreen.Location = New System.Drawing.Point(9, 26)
        Me.RadioButtonMarkerGreen.Name = "RadioButtonMarkerGreen"
        Me.RadioButtonMarkerGreen.Size = New System.Drawing.Size(54, 17)
        Me.RadioButtonMarkerGreen.TabIndex = 7
        Me.RadioButtonMarkerGreen.TabStop = True
        Me.RadioButtonMarkerGreen.Text = "Green"
        Me.RadioButtonMarkerGreen.UseVisualStyleBackColor = True
        '
        'TimerWatchdog
        '
        Me.TimerWatchdog.Interval = 10
        '
        'PanelOption
        '
        Me.PanelOption.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle
        Me.PanelOption.Controls.Add(Me.RadioButtonOptionHue)
        Me.PanelOption.Controls.Add(Me.RadioButtonOptionLuminanceB)
        Me.PanelOption.Controls.Add(Me.RadioButtonOptionLuminanceG)
        Me.PanelOption.Controls.Add(Me.RadioButtonOptionLuminanceR)
        Me.PanelOption.Controls.Add(Me.RadioButtonOptionGradient)
        Me.PanelOption.Controls.Add(Me.RadioButtonOptionLuminanceRGB)
        Me.PanelOption.Location = New System.Drawing.Point(12, 215)
        Me.PanelOption.Name = "PanelOption"
        Me.PanelOption.Size = New System.Drawing.Size(148, 136)
        Me.PanelOption.TabIndex = 8
        '
        'RadioButtonOptionHue
        '
        Me.RadioButtonOptionHue.AutoSize = True
        Me.RadioButtonOptionHue.Location = New System.Drawing.Point(3, 112)
        Me.RadioButtonOptionHue.Name = "RadioButtonOptionHue"
        Me.RadioButtonOptionHue.Size = New System.Drawing.Size(45, 17)
        Me.RadioButtonOptionHue.TabIndex = 14
        Me.RadioButtonOptionHue.TabStop = True
        Me.RadioButtonOptionHue.Text = "Hue"
        Me.RadioButtonOptionHue.UseVisualStyleBackColor = True
        '
        'RadioButtonOptionLuminanceB
        '
        Me.RadioButtonOptionLuminanceB.AutoSize = True
        Me.RadioButtonOptionLuminanceB.Location = New System.Drawing.Point(3, 71)
        Me.RadioButtonOptionLuminanceB.Name = "RadioButtonOptionLuminanceB"
        Me.RadioButtonOptionLuminanceB.Size = New System.Drawing.Size(87, 17)
        Me.RadioButtonOptionLuminanceB.TabIndex = 13
        Me.RadioButtonOptionLuminanceB.TabStop = True
        Me.RadioButtonOptionLuminanceB.Text = "Luminance B"
        Me.RadioButtonOptionLuminanceB.UseVisualStyleBackColor = True
        '
        'RadioButtonOptionLuminanceG
        '
        Me.RadioButtonOptionLuminanceG.AutoSize = True
        Me.RadioButtonOptionLuminanceG.Location = New System.Drawing.Point(3, 51)
        Me.RadioButtonOptionLuminanceG.Name = "RadioButtonOptionLuminanceG"
        Me.RadioButtonOptionLuminanceG.Size = New System.Drawing.Size(88, 17)
        Me.RadioButtonOptionLuminanceG.TabIndex = 12
        Me.RadioButtonOptionLuminanceG.TabStop = True
        Me.RadioButtonOptionLuminanceG.Text = "Luminance G"
        Me.RadioButtonOptionLuminanceG.UseVisualStyleBackColor = True
        '
        'RadioButtonOptionLuminanceR
        '
        Me.RadioButtonOptionLuminanceR.AutoSize = True
        Me.RadioButtonOptionLuminanceR.Location = New System.Drawing.Point(3, 28)
        Me.RadioButtonOptionLuminanceR.Name = "RadioButtonOptionLuminanceR"
        Me.RadioButtonOptionLuminanceR.Size = New System.Drawing.Size(88, 17)
        Me.RadioButtonOptionLuminanceR.TabIndex = 11
        Me.RadioButtonOptionLuminanceR.TabStop = True
        Me.RadioButtonOptionLuminanceR.Text = "Luminance R"
        Me.RadioButtonOptionLuminanceR.UseVisualStyleBackColor = True
        '
        'RadioButtonOptionGradient
        '
        Me.RadioButtonOptionGradient.AutoSize = True
        Me.RadioButtonOptionGradient.Location = New System.Drawing.Point(3, 92)
        Me.RadioButtonOptionGradient.Name = "RadioButtonOptionGradient"
        Me.RadioButtonOptionGradient.Size = New System.Drawing.Size(65, 17)
        Me.RadioButtonOptionGradient.TabIndex = 10
        Me.RadioButtonOptionGradient.TabStop = True
        Me.RadioButtonOptionGradient.Text = "Gradient"
        Me.RadioButtonOptionGradient.UseVisualStyleBackColor = True
        '
        'RadioButtonOptionLuminanceRGB
        '
        Me.RadioButtonOptionLuminanceRGB.AutoSize = True
        Me.RadioButtonOptionLuminanceRGB.Location = New System.Drawing.Point(4, 5)
        Me.RadioButtonOptionLuminanceRGB.Name = "RadioButtonOptionLuminanceRGB"
        Me.RadioButtonOptionLuminanceRGB.Size = New System.Drawing.Size(103, 17)
        Me.RadioButtonOptionLuminanceRGB.TabIndex = 9
        Me.RadioButtonOptionLuminanceRGB.TabStop = True
        Me.RadioButtonOptionLuminanceRGB.Text = "Luminance RGB"
        Me.RadioButtonOptionLuminanceRGB.UseVisualStyleBackColor = True
        '
        'LabelData1
        '
        Me.LabelData1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.LabelData1.Location = New System.Drawing.Point(291, 274)
        Me.LabelData1.Name = "LabelData1"
        Me.LabelData1.Size = New System.Drawing.Size(46, 21)
        Me.LabelData1.TabIndex = 10
        '
        'LabelData2
        '
        Me.LabelData2.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.LabelData2.Location = New System.Drawing.Point(172, 273)
        Me.LabelData2.Name = "LabelData2"
        Me.LabelData2.Size = New System.Drawing.Size(46, 22)
        Me.LabelData2.TabIndex = 11
        '
        'LabelData3
        '
        Me.LabelData3.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.LabelData3.Location = New System.Drawing.Point(172, 301)
        Me.LabelData3.Name = "LabelData3"
        Me.LabelData3.Size = New System.Drawing.Size(46, 22)
        Me.LabelData3.TabIndex = 12
        '
        'LabelData4
        '
        Me.LabelData4.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.LabelData4.Location = New System.Drawing.Point(172, 329)
        Me.LabelData4.Name = "LabelData4"
        Me.LabelData4.Size = New System.Drawing.Size(46, 22)
        Me.LabelData4.TabIndex = 13
        '
        'Label2
        '
        Me.Label2.AutoSize = True
        Me.Label2.Location = New System.Drawing.Point(343, 278)
        Me.Label2.Name = "Label2"
        Me.Label2.Size = New System.Drawing.Size(44, 13)
        Me.Label2.TabIndex = 14
        Me.Label2.Text = "1st byte"
        '
        'Label3
        '
        Me.Label3.AutoSize = True
        Me.Label3.Location = New System.Drawing.Point(224, 274)
        Me.Label3.Name = "Label3"
        Me.Label3.Size = New System.Drawing.Size(48, 13)
        Me.Label3.TabIndex = 15
        Me.Label3.Text = "2nd byte"
        '
        'Label4
        '
        Me.Label4.AutoSize = True
        Me.Label4.Location = New System.Drawing.Point(224, 302)
        Me.Label4.Name = "Label4"
        Me.Label4.Size = New System.Drawing.Size(45, 13)
        Me.Label4.TabIndex = 16
        Me.Label4.Text = "3rd byte"
        '
        'Label5
        '
        Me.Label5.AutoSize = True
        Me.Label5.Location = New System.Drawing.Point(227, 332)
        Me.Label5.Name = "Label5"
        Me.Label5.Size = New System.Drawing.Size(45, 13)
        Me.Label5.TabIndex = 17
        Me.Label5.Text = "4th byte"
        '
        'LabelData5
        '
        Me.LabelData5.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.LabelData5.Location = New System.Drawing.Point(172, 352)
        Me.LabelData5.Name = "LabelData5"
        Me.LabelData5.Size = New System.Drawing.Size(46, 22)
        Me.LabelData5.TabIndex = 18
        '
        'Label6
        '
        Me.Label6.AutoSize = True
        Me.Label6.Location = New System.Drawing.Point(227, 361)
        Me.Label6.Name = "Label6"
        Me.Label6.Size = New System.Drawing.Size(123, 13)
        Me.Label6.TabIndex = 19
        Me.Label6.Text = "No. of bytes in RX buffer"
        '
        'LabelData6
        '
        Me.LabelData6.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.LabelData6.Location = New System.Drawing.Point(410, 274)
        Me.LabelData6.Name = "LabelData6"
        Me.LabelData6.Size = New System.Drawing.Size(46, 21)
        Me.LabelData6.TabIndex = 20
        '
        'Label7
        '
        Me.Label7.AutoSize = True
        Me.Label7.Location = New System.Drawing.Point(464, 277)
        Me.Label7.Name = "Label7"
        Me.Label7.Size = New System.Drawing.Size(130, 13)
        Me.Label7.TabIndex = 21
        Me.Label7.Text = "Horizontal Reference Line"
        '
        'ButtonHLUp
        '
        Me.ButtonHLUp.Location = New System.Drawing.Point(410, 346)
        Me.ButtonHLUp.Name = "ButtonHLUp"
        Me.ButtonHLUp.Size = New System.Drawing.Size(65, 30)
        Me.ButtonHLUp.TabIndex = 22
        Me.ButtonHLUp.Text = "HL Up"
        Me.ButtonHLUp.UseVisualStyleBackColor = True
        '
        'ButtonHLDown
        '
        Me.ButtonHLDown.Location = New System.Drawing.Point(410, 382)
        Me.ButtonHLDown.Name = "ButtonHLDown"
        Me.ButtonHLDown.Size = New System.Drawing.Size(65, 30)
        Me.ButtonHLDown.TabIndex = 23
        Me.ButtonHLDown.Text = "HL Down"
        Me.ButtonHLDown.UseVisualStyleBackColor = True
        '
        'ButtonSaveImage
        '
        Me.ButtonSaveImage.Location = New System.Drawing.Point(552, 346)
        Me.ButtonSaveImage.Name = "ButtonSaveImage"
        Me.ButtonSaveImage.Size = New System.Drawing.Size(63, 66)
        Me.ButtonSaveImage.TabIndex = 24
        Me.ButtonSaveImage.Text = "Save Image"
        Me.ButtonSaveImage.UseVisualStyleBackColor = True
        '
        'ButtonVLRight
        '
        Me.ButtonVLRight.Location = New System.Drawing.Point(481, 346)
        Me.ButtonVLRight.Name = "ButtonVLRight"
        Me.ButtonVLRight.Size = New System.Drawing.Size(65, 30)
        Me.ButtonVLRight.TabIndex = 27
        Me.ButtonVLRight.Text = "VL Right"
        Me.ButtonVLRight.UseVisualStyleBackColor = True
        '
        'ButtonVLLeft
        '
        Me.ButtonVLLeft.Location = New System.Drawing.Point(481, 382)
        Me.ButtonVLLeft.Name = "ButtonVLLeft"
        Me.ButtonVLLeft.Size = New System.Drawing.Size(65, 30)
        Me.ButtonVLLeft.TabIndex = 28
        Me.ButtonVLLeft.Text = "VL Left"
        Me.ButtonVLLeft.UseVisualStyleBackColor = True
        '
        'LabelData7
        '
        Me.LabelData7.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.LabelData7.Location = New System.Drawing.Point(410, 306)
        Me.LabelData7.Name = "LabelData7"
        Me.LabelData7.Size = New System.Drawing.Size(46, 21)
        Me.LabelData7.TabIndex = 29
        '
        'Label10
        '
        Me.Label10.AutoSize = True
        Me.Label10.Location = New System.Drawing.Point(464, 310)
        Me.Label10.Name = "Label10"
        Me.Label10.Size = New System.Drawing.Size(118, 13)
        Me.Label10.TabIndex = 30
        Me.Label10.Text = "Vertical Reference Line"
        '
        'ButtonTest
        '
        Me.ButtonTest.Location = New System.Drawing.Point(495, 414)
        Me.ButtonTest.Margin = New System.Windows.Forms.Padding(2)
        Me.ButtonTest.Name = "ButtonTest"
        Me.ButtonTest.Size = New System.Drawing.Size(52, 31)
        Me.ButtonTest.TabIndex = 31
        Me.ButtonTest.Text = "Test"
        Me.ButtonTest.UseVisualStyleBackColor = True
        '
        'LabelTest
        '
        Me.LabelTest.AutoSize = True
        Me.LabelTest.Location = New System.Drawing.Point(343, 423)
        Me.LabelTest.Margin = New System.Windows.Forms.Padding(2, 0, 2, 0)
        Me.LabelTest.Name = "LabelTest"
        Me.LabelTest.Size = New System.Drawing.Size(54, 13)
        Me.LabelTest.TabIndex = 32
        Me.LabelTest.Text = "LabelTest"
        '
        'OpenFileDialog1
        '
        Me.OpenFileDialog1.DefaultExt = "txt"
        Me.OpenFileDialog1.FileName = "Testimage.txt"
        Me.OpenFileDialog1.Title = "Select a file to save the image data"
        '
        'Main
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(622, 502)
        Me.Controls.Add(Me.LabelTest)
        Me.Controls.Add(Me.ButtonTest)
        Me.Controls.Add(Me.Label10)
        Me.Controls.Add(Me.LabelData7)
        Me.Controls.Add(Me.ButtonVLLeft)
        Me.Controls.Add(Me.ButtonVLRight)
        Me.Controls.Add(Me.ButtonSaveImage)
        Me.Controls.Add(Me.ButtonHLDown)
        Me.Controls.Add(Me.ButtonHLUp)
        Me.Controls.Add(Me.Label7)
        Me.Controls.Add(Me.LabelData6)
        Me.Controls.Add(Me.Label6)
        Me.Controls.Add(Me.LabelData5)
        Me.Controls.Add(Me.Label5)
        Me.Controls.Add(Me.Label4)
        Me.Controls.Add(Me.Label3)
        Me.Controls.Add(Me.Label2)
        Me.Controls.Add(Me.LabelData4)
        Me.Controls.Add(Me.LabelData3)
        Me.Controls.Add(Me.LabelData2)
        Me.Controls.Add(Me.LabelData1)
        Me.Controls.Add(Me.PanelOption)
        Me.Controls.Add(Me.PanelMarker)
        Me.Controls.Add(Me.ButtonStart)
        Me.Controls.Add(Me.ButtonClose)
        Me.Controls.Add(Me.LabelInfo)
        Me.Controls.Add(Me.ButtonOpen)
        Me.Controls.Add(Me.ComboBoxCommPort)
        Me.Icon = CType(resources.GetObject("$this.Icon"), System.Drawing.Icon)
        Me.Name = "Main"
        Me.Text = "Robot Eye Monitor"
        Me.PanelMarker.ResumeLayout(False)
        Me.PanelMarker.PerformLayout()
        Me.PanelOption.ResumeLayout(False)
        Me.PanelOption.PerformLayout()
        Me.ResumeLayout(False)
        Me.PerformLayout()

    End Sub
    Friend WithEvents ComboBoxCommPort As System.Windows.Forms.ComboBox
    Friend WithEvents SerialPort1 As System.IO.Ports.SerialPort
    Friend WithEvents ButtonOpen As System.Windows.Forms.Button
    Friend WithEvents LabelInfo As System.Windows.Forms.Label
    Friend WithEvents ButtonClose As System.Windows.Forms.Button
    Friend WithEvents ButtonStart As System.Windows.Forms.Button
    Friend WithEvents RadioButtonMarkerRed As System.Windows.Forms.RadioButton
    Friend WithEvents PanelMarker As System.Windows.Forms.Panel
    Friend WithEvents RadioButtonMarkerBlue As System.Windows.Forms.RadioButton
    Friend WithEvents RadioButtonMarkerGreen As System.Windows.Forms.RadioButton
    Friend WithEvents RadioButtonMarkerYellow As System.Windows.Forms.RadioButton
    Friend WithEvents Label1 As System.Windows.Forms.Label
    Friend WithEvents TimerWatchdog As System.Windows.Forms.Timer
    Friend WithEvents PanelOption As System.Windows.Forms.Panel
    Friend WithEvents RadioButtonOptionGradient As System.Windows.Forms.RadioButton
    Friend WithEvents RadioButtonOptionLuminanceRGB As System.Windows.Forms.RadioButton
    Friend WithEvents LabelData1 As System.Windows.Forms.Label
    Friend WithEvents LabelData2 As System.Windows.Forms.Label
    Friend WithEvents LabelData3 As System.Windows.Forms.Label
    Friend WithEvents LabelData4 As Label
    Friend WithEvents Label2 As Label
    Friend WithEvents Label3 As Label
    Friend WithEvents Label4 As Label
    Friend WithEvents Label5 As Label
    Friend WithEvents LabelData5 As Label
    Friend WithEvents Label6 As Label
    Friend WithEvents LabelData6 As Label
    Friend WithEvents Label7 As Label
    Friend WithEvents ButtonHLUp As Button
    Friend WithEvents ButtonHLDown As Button
    Friend WithEvents ButtonSaveImage As Button
    Friend WithEvents ButtonVLRight As Button
    Friend WithEvents ButtonVLLeft As Button
    Friend WithEvents LabelData7 As Label
    Friend WithEvents Label10 As Label
    Friend WithEvents RadioButtonOptionLuminanceB As RadioButton
    Friend WithEvents RadioButtonOptionLuminanceG As RadioButton
    Friend WithEvents RadioButtonOptionLuminanceR As RadioButton
    Friend WithEvents RadioButtonOptionHue As RadioButton
    Friend WithEvents ButtonTest As Button
    Friend WithEvents LabelTest As Label
    Friend WithEvents OpenFileDialog1 As OpenFileDialog
End Class
