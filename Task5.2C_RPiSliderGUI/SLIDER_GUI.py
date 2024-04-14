from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QHBoxLayout, QRadioButton, QPushButton, QSlider, QLabel

import RPi.GPIO as GPIO

# Class to hold the group of widgets to control particular LED and their layouts.
class LedSliderGroup(QWidget):
    def __init__(self, colour):
        super().__init__()
        self.colour = colour
        self.ledPWM = GPIO.PWM(ledPins[colour], 100)
        self.ledPWM.start(0)
        self.initUI(f"{colour} LED")

    def initUI(self, title):
    # We use a VBox layout so we can position the title for each
    # slider above the slider itself.
        mainLayout = QVBoxLayout()

        titleLabel = QLabel(title)
        titleLabel.setAlignment(Qt.AlignCenter)
        mainLayout.addWidget(titleLabel)

    # We Use a HBox layout so we can display the value of the slider
    # next to the slider itself.
        sliderLayout = QHBoxLayout()

        self.valueLabel = QLabel('0')
        self.valueLabel.setAlignment(Qt.AlignCenter)
        # We set a min width so it doesn't squash the slider as more 
        # digits are displayed.
        self.valueLabel.setMinimumWidth(45)
        sliderLayout.addWidget(self.valueLabel)

        self.slider = QSlider(Qt.Horizontal)
        self.slider.setMinimumWidth(150)
        self.slider.setMinimum(0)
        self.slider.setMaximum(100)
        self.slider.setValue(0)
        self.slider.setTickPosition(QSlider.TicksBelow)
        self.slider.setTickInterval(10)
        self.slider.valueChanged.connect(self.sliderUpdate)
        sliderLayout.addWidget(self.slider)

        mainLayout.addLayout(sliderLayout)
        self.setLayout(mainLayout)

    def sliderUpdate(self, value):
        self.valueLabel.setText(f"{value} %")
        self.updateLed(value)

    def updateLed(self, value):
        self.ledPWM.ChangeDutyCycle(value)

# Class to define the layout for entire window.
class MainWindow(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()

    def onExitButtonClicked(self):
        self.close()
        
    def initUI(self):
    # We use VBox layout so we can stack the sliders vertically.
        layout = QVBoxLayout()

        self.redSlider = LedSliderGroup('Red')
        self.blueSlider = LedSliderGroup('Blue')
        self.greenSlider = LedSliderGroup('Green')

        layout.addWidget(self.redSlider)
        layout.addWidget(self.blueSlider)
        layout.addWidget(self.greenSlider)

        exitButton = QPushButton('Exit')
        exitButton.clicked.connect(self.onExitButtonClicked)
        layout.addWidget(exitButton)

        self.setLayout(layout)
        self.setWindowTitle('LED Control')
        self.show()

if __name__ == '__main__':
    # Init GPIO
    GPIO.setmode(GPIO.BCM)

    ledPins = { 'Red': 17, 'Blue': 27, 'Green': 22 }

    for pin in ledPins.values():
        GPIO.setup(pin, GPIO.OUT)
        GPIO.output(pin, GPIO.LOW)

    app = QApplication([])
    wind = MainWindow()
    try:
        app.exec_()
    finally:
        print("Cleaning GPIO Pins...")
        wind.redSlider.ledPWM.stop()
        wind.blueSlider.ledPWM.stop()
        wind.greenSlider.ledPWM.stop()
        GPIO.cleanup()
