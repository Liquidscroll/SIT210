import speech_recognition as sr
import numpy as np
import soundfile as sf
import io
from faster_whisper import WhisperModel
import RPi.GPIO as GPIO
# Importing sounddevice supresses ALSA lib errors. These do not seem to affect the program.
import sounddevice

# Class to manage GPIO operations
class GPIOSystem:
    def __init__(self, led_pin):
        self.led_pin = led_pin
        self.current_led_state = False

        # Setup the GPIO pin for output
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(self.led_pin, GPIO.OUT)
        GPIO.output(self.led_pin, GPIO.LOW)

    # Toggle LED based upon input and current state.
    def HandleLED(self, new_state):
        if new_state and not self.current_led_state:
            GPIO.output(self.led_pin, GPIO.HIGH)
            self.current_led_state = True
        elif not new_state and self.current_led_state:
            GPIO.output(self.led_pin, GPIO.LOW)
            self.current_led_state = False

# Main class to handle voice recognition and control the LED
class WhisperRecogniserLEDSystem:
    def __init__(self, faster_whisper, model, compute_type, led_pin):
        self.gpio = GPIOSystem(led_pin)
        self.is_faster_whisper = faster_whisper
        self.faster_whisper_model = WhisperModel(model, device="cpu", compute_type=compute_type)
        self.recogniser = sr.Recognizer()
        self.microphone = sr.Microphone

    # Parse and handle text, if recognised as 'on' or 'off' then handle LED appropriately.
    def HandleText(self, input_string):
        words = [word.lower().strip().rstrip('.') for word in input_string.split()]
        for word in words:
            if word == "on":
                self.gpio.HandleLED(True)
            elif word == "off":
                self.gpio.HandleLED(False)

    # Listen to the microphone and handle audio input.
    def Listen(self):
        with self.microphone() as source:
            audio = self.recogniser.listen(source)
        try:
            if self.is_faster_whisper:
                # Recognise speech using Faster Whisper model.
                self.FasterRecognise(audio)
            else:
                # Recognise speech using the default Whisper model
                self.HandleText(self.recogniser.recognize_whisper(audio, language="en", model="tiny.en"))
        except sr.UnknownValueError:
            print("Error, audio could not be understood.")
        except sr.RequestError as e:
            print(f"Could not request results from Whisper; {e}")

    # Process audio using the Faster Whisper model.
    def FasterRecognise(self, audio_data):
        # Extract WAV data from audio
        wav_bytes = audio_data.get_wav_data(convert_rate=16000)
        # Create a stream to process WAV data
        wav_stream = io.BytesIO(wav_bytes)
        # Read audio stream and store bytes into an array
        audio_array, _ = sf.read(wav_stream, dtype='float32')
        # Convert array to numpy array
        audio_array = audio_array.astype(np.float32)
        # Transcribe audio using faster whisper model.
        segments, _ = self.faster_whisper_model.transcribe(audio_array, beam_size=3, vad_filter=True)

        for segment in segments:
            self.HandleText(segment.text)

if __name__ == '__main__':
    #faster_whisper = False
    faster_whisper = True
    wr = WhisperRecogniserLEDSystem(faster_whisper, "tiny.en", "int8", 17)
    try:
        while(True):
            print("Listening...")
            wr.Listen()
    finally:
        # Ensure GPIO resources are properly cleaned on program exit.
        GPIO.cleanup()
