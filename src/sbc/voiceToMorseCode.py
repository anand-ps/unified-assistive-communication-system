from vosk import Model, KaldiRecognizer
import pyaudio

CHARS_TO_MORSE_CODE_MAP = {'A': '.-', 'B': '-...',
                           'C': '-.-.', 'D': '-..', 'E': '.',
                           'F': '..-.', 'G': '--.', 'H': '....',
                           'I': '..', 'J': '.---', 'K': '-.-',
                           'L': '.-..', 'M': '--', 'N': '-.',
                           'O': '---', 'P': '.--.', 'Q': '--.-',
                           'R': '.-.', 'S': '...', 'T': '-',
                           'U': '..-', 'V': '...-', 'W': '.--',
                           'X': '-..-', 'Y': '-.--', 'Z': '--..',
                           '1': '.----', '2': '..---', '3': '...--',
                           '4': '....-', '5': '.....', '6': '-....',
                           '7': '--...', '8': '---..', '9': '----.',
                           '0': '-----', ', ': '--..--', '.': '.-.-.-',
                           '?': '..--..', '/': '-..-.', '-': '-....-',
                           '(': '-.--.', ')': '-.--.-'}
notDefined = ["'"]

result = ""
model = Model(r"C:\Users\anand\Downloads\vosk-model-small-en-in-0.4\vosk-model-small-en-in-0.4")
recognizer = KaldiRecognizer(model, 16000)
mic = pyaudio.PyAudio()

stream = mic.open(format=pyaudio.paInt16, channels=1, rate=16000, input=True, frames_per_buffer=8192)
stream.start_stream()


def to_morse_code(english_plain_text):
    morseCode = ''

    for char in english_plain_text:
        # checking for space
        # to add single space after every character and double space after every word
        if char == ' ':
            morseCode += '  '
        else:
            # adding encoded morse code to the result
            if char in notDefined:
                morseCode += ' '
                continue
            morseCode += CHARS_TO_MORSE_CODE_MAP[char.upper()] + ' '
    return morseCode


while True:
    data = stream.read(4096)
    # if len(data) == 0:
    #   break
    if recognizer.AcceptWaveform(data):
        text = recognizer.Result()
        # print(text)
        result = text[14:-3]
        print(result)
        morse_code = to_morse_code(result)
        print(morse_code)