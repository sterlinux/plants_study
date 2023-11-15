import pandas as pd
import numpy as np
from scipy.signal import butter, lfilter
import matplotlib.pyplot as plt

# Carica i dati dal file CSV (supponendo che il file abbia due colonne: tempo e valore del segnale)
data = pd.read_csv('test.csv')
time = data['millisec']
signal = data['plant']

# Specifiche del filtro passa-basso
cut_off_frequency = 40.0  # Frequenza di taglio (Hz)
order = 4  # Ordine del filtro

# Progettazione del filtro passa-basso (Butterworth)
# nyquist_freq = 0.5 * 1.0 / np.mean(np.diff(time))  # Frequenza di Nyquist (metà della frequenza di campionamento)
nyquist_freq = 50
normal_cutoff = cut_off_frequency / nyquist_freq
b, a = butter(order, normal_cutoff, btype='low', analog=False)

# Applicazione del filtro passa-basso ai dati campionati
filtered_signal = lfilter(b, a, signal)

# Plot dei dati originali e del segnale filtrato
plt.figure(figsize=(10, 6))
plt.plot(time, signal, label='Dati originali')
plt.plot(time, filtered_signal, label='Segnale filtrato (passa-basso)')
plt.xlabel('Tempo')
plt.ylabel('Valore del segnale')
plt.legend()
plt.grid(True)
plt.show()
