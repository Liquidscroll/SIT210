from PyQt5.QtWidgets import QApplication, QWidget, QRadioButton, QPushButton, QVBoxLayout

import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)

ledPins = { 'Red': 17, 'Blue': 27, 'Green': 22 }

for pin in ledPins.values():
    GPIO.setup(pin, GPIO.OUT)
    GPIO.output(pin, GPIO.LOW)

class MainWindow(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()

    def onExitButtonClicked(self):
        self.close()

    def changeLed(self, colour):
        for pin in ledPins.values():
            GPIO.output(pin, GPIO.LOW)

        GPIO.output(ledPins[colour], GPIO.HIGH)

    def initUI(self):

        self.redRadioButton = QRadioButton('Red')
        self.blueRadioButton = QRadioButton('Blue')
        self.greenRadioButton = QRadioButton('Green')
        self.exitButton = QPushButton('Exit')

        self.redRadioButton.clicked.connect(lambda: self.changeLed('Red'))
        self.blueRadioButton.clicked.connect(lambda: self.changeLed('Blue'))
        self.greenRadioButton.clicked.connect(lambda: self.changeLed('Green'))
        self.exitButton.clicked.connect(self.onExitButtonClicked)


        layout = QVBoxLayout()
        layout.addWidget(self.redRadioButton)
        layout.addWidget(self.blueRadioButton)
        layout.addWidget(self.greenRadioButton)
        layout.addWidget(self.exitButton)

        self.setLayout(layout)
        self.setWindowTitle('LED Control')
        self.show()


        #exitButton = QPushButton('Exit')
        #exitButton.clicked.connect(on_quit_button_clicked)
        #layout.addWidget(exitButton)

        #widget.setLayout(layout)
        #widget.show()



if __name__ == '__main__':
    app = QApplication([])
    wind = MainWindow()

    app.exec_()

