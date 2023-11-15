# This is a sample Python script.

# Press ⌃R to execute it or replace it with your code.
# Press Double ⇧ to search everywhere for classes, files, tool windows, actions, and settings.

import numpy as np
import matplotlib.pyplot as plt

def print_hi(name):
    # Use a breakpoint in the code line below to debug your script.
    print(f'Hi, {name}')  # Press ⌘F8 to toggle the breakpoint.


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    print_hi('PyCharm')

# See PyCharm help at https://www.jetbrains.com/help/pycharm/


# Carica i dati dal file CSV (supponendo che il file abbia due colonne: tempo e valore del segnale)
data = np.loadtxt('vivaldi-four-season-request-5.csv', delimiter=',')
#data = np.loadtxt('test_86.csv', delimiter=',')

# Estrai il segnale dalla quarta colonna (se il file CSV ha solo due colonne)
signal = data[:, 3]

# Applica la FFT al segnale
fft_result = np.fft.fft(signal)

# Calcola il modulo dell'FFT per ottenere l'ampiezza delle frequenze
amplitudes = np.abs(fft_result)

# Calcola le frequenze corrispondenti
sample_rate = 100.0  # Frequenza di campionamento (Hz)
frequencies = np.fft.fftfreq(len(signal), d=1/sample_rate)

# Crea un grafico delle frequenze
plt.plot(frequencies, amplitudes)
plt.xlabel('Frequenza (Hz)')
plt.ylabel('Ampiezza')
plt.title('Spettro delle frequenze')
plt.grid(True)
plt.show()