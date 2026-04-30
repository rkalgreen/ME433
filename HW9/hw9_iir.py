import matplotlib.pyplot as plt
import numpy as np
import csv

t_A = [] # column 0
data1_A = [] # column 1

with open('sigA.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        t_A.append(round(float(row[0]), 4)) # leftmost column
        data1_A.append(float(row[1])) # second column

# for i in range(len(t_A)):
#     # print the data to verify it was read
#     print(str(t_A[i]) + ", " + str(data1_A[i]))

t_B = [] # column 0
data1_B = [] # column 1

with open('sigB.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        t_B.append(round(float(row[0]), 4)) # leftmost column
        data1_B.append(float(row[1])) # second column

# for i in range(len(t)):
#     # print the data to verify it was read
#     print(str(t[i]) + ", " + str(data1[i]) + ", " + str(data2[i]))

t_C = [] # column 0
data1_C = [] # column 1

with open('sigC.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        t_C.append(round(float(row[0]), 4)) # leftmost column
        data1_C.append(float(row[1])) # second column

# for i in range(len(t)):
#     # print the data to verify it was read
#     print(str(t[i]) + ", " + str(data1[i]) + ", " + str(data2[i]))

t_D = [] # column 0
data1_D = [] # column 1

with open('sigD.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        t_D.append(round(float(row[0]), 4)) # leftmost column
        data1_D.append(float(row[1])) # second column

# for i in range(len(t)):
#     # print the data to verify it was read
#     print(str(t[i]) + ", " + str(data1[i]) + ", " + str(data2[i]))

# iir filter
A_A = .002
B_A = 1 - A_A
filter_A = []
for i in range(len(t_A)):
    if i < 1:
        filter_A.append(data1_A[i])
    else: 
        filter_A.append(A_A * data1_A[i-1] + B_A * filter_A[i-1])

A_B = .015
B_B = 1 - A_B
filter_B = []
for i in range(len(t_B)):
    if i < 1:
        filter_B.append(data1_B[i])
    else: 
        filter_B.append(A_B * data1_B[i-1] + B_B * filter_B[i-1])

A_C = .2
B_C = 1 - A_C
filter_C = []
for i in range(len(t_C)):
    if i < 1:
        filter_C.append(data1_C[i])
    else: 
        filter_C.append(A_C * data1_C[i-1] + B_C * filter_C[i-1])

A_D = .1
B_D = 1 - A_D
filter_D = []
for i in range(len(t_D)):
    if i < 1:
        filter_D.append(data1_D[i])
    else: 
        filter_D.append(A_D * data1_D[i-1] + B_D * filter_D[i-1])

dt_A = t_A[1] - t_A[0]
dt_B = t_B[1] - t_B[0]
dt_C = t_C[1] - t_C[0]
dt_D = t_D[1] - t_D[0]
 
Fs_A = 1/dt_A # sample rate
Fs_B = 1/dt_B # sample rate
Fs_C = 1/dt_C # sample rate
Fs_D = 1/dt_D # sample rate

Ts_A = 1.0/Fs_A; # sampling interval
Ts_B = 1.0/Fs_B; # sampling interval
Ts_C = 1.0/Fs_C; # sampling interval
Ts_D = 1.0/Fs_D; # sampling interval

ts_A = np.arange(0,t_A[-1],Ts_A) # time vector
ts_B = np.arange(0,t_B[-1],Ts_B) # time vector
ts_C = np.arange(0,t_C[-1],Ts_C) # time vector
ts_D = np.arange(0,t_D[-1],Ts_D) # time vector

y_A = data1_A # the data to make the fft from
y_B = data1_B # the data to make the fft from
y_C = data1_C # the data to make the fft from
y_D = data1_D # the data to make the fft from

y_A_filter = filter_A
y_B_filter = filter_B
y_C_filter = filter_C
y_D_filter = filter_D

n_A = len(y_A) # length of the signal
n_B = len(y_B) # length of the signal
n_C = len(y_C) # length of the signal
n_D = len(y_D) # length of the signal

k_A = np.arange(n_A)
k_B = np.arange(n_B)
k_C = np.arange(n_C)
k_D = np.arange(n_D)

T_A = n_A/Fs_A
T_B = n_B/Fs_B
T_C = n_C/Fs_C
T_D = n_D/Fs_D

frq_A = k_A/T_A # two sides frequency range
frq_B = k_B/T_B # two sides frequency range
frq_C = k_C/T_C # two sides frequency range
frq_D = k_D/T_D # two sides frequency range

frq_A = frq_A[range(int(n_A/2))] # one side frequency range
frq_B = frq_B[range(int(n_B/2))] # one side frequency range
frq_C = frq_C[range(int(n_C/2))] # one side frequency range
frq_D = frq_D[range(int(n_D/2))] # one side frequency range

Y_A = np.fft.fft(y_A)/n_A # fft computing and normalization
Y_B = np.fft.fft(y_B)/n_B # fft computing and normalization
Y_C = np.fft.fft(y_C)/n_C # fft computing and normalization
Y_D = np.fft.fft(y_D)/n_D # fft computing and normalization

Y_A = Y_A[range(int(n_A/2))]
Y_B = Y_B[range(int(n_B/2))]
Y_C = Y_C[range(int(n_C/2))]
Y_D = Y_D[range(int(n_D/2))]

Y_A_filter = np.fft.fft(y_A_filter)/n_A # fft computing and normalization
Y_B_filter = np.fft.fft(y_B_filter)/n_B # fft computing and normalization
Y_C_filter = np.fft.fft(y_C_filter)/n_C # fft computing and normalization
Y_D_filter = np.fft.fft(y_D_filter)/n_D # fft computing and normalization

Y_A_filter = Y_A_filter[range(int(n_A/2))]
Y_B_filter = Y_B_filter[range(int(n_B/2))]
Y_C_filter = Y_C_filter[range(int(n_C/2))]
Y_D_filter = Y_D_filter[range(int(n_D/2))]

fig_A, (ax1, ax2) = plt.subplots(2, 1)
fig_A.suptitle(f'Signal A, IIR filter with A={A_A:.3f}, B={B_A:.3f}')
ax1.plot(t_A,y_A,'k',t_A,y_A_filter,'r', lw=1)
ax1.set_xlabel('Time')
ax1.set_ylabel('Amplitude')
ax2.loglog(frq_A,abs(Y_A),'k',frq_A,abs(Y_A_filter),'r', lw=1) # plotting the fft
ax2.set_xlabel('Freq (Hz)')
ax2.set_ylabel('|Y(freq)|')
# plt.show()

fig_B, (ax1, ax2) = plt.subplots(2, 1)
fig_B.suptitle(f'Signal B, IIR filter with A={A_B:.3f}, B={B_B:.3f}')
ax1.plot(t_B,y_B,'k',t_B,y_B_filter,'r', lw=1)
ax1.set_xlabel('Time')
ax1.set_ylabel('Amplitude')
ax2.loglog(frq_B,abs(Y_B),'k',frq_B,abs(Y_B_filter),'r', lw=1) # plotting the fft
ax2.set_xlabel('Freq (Hz)')
ax2.set_ylabel('|Y(freq)|')

# plt.show()

fig_C, (ax1, ax2) = plt.subplots(2, 1)
fig_C.suptitle(f'Signal C, IIR filter with A={A_C:.3f}, B={B_C:.3f}')
ax1.plot(t_C,y_C,'k',t_C,y_C_filter,'r', lw=1)
ax1.set_xlabel('Time')
ax1.set_ylabel('Amplitude')
ax2.loglog(frq_C,abs(Y_C),'k',frq_C,abs(Y_C_filter),'r', lw=1) # plotting the fft
ax2.set_xlabel('Freq (Hz)')
ax2.set_ylabel('|Y(freq)|')

# plt.show()

fig_D, (ax1, ax2) = plt.subplots(2, 1)
fig_D.suptitle(f'Signal D, IIR filter with A={A_D:.3f}, B={B_D:.3f}')
ax1.plot(t_D,y_D,'k',t_D,y_D_filter,'r', lw=1)
ax1.set_xlabel('Time')
ax1.set_ylabel('Amplitude')
ax2.loglog(frq_D,abs(Y_D),'k',frq_D,abs(Y_D_filter),'r', lw=1) # plotting the fft
ax2.set_xlabel('Freq (Hz)')
ax2.set_ylabel('|Y(freq)|')

plt.show()