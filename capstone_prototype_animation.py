import numpy as np
import matplotlib.pyplot as plt
import serial
import time

# --- SERIAL SETUP (adjust COM port) ---
ser = serial.Serial('COM3', 9600, timeout=0.1)
time.sleep(2)

# --- SIGNAL PARAMETERS ---
f = 60
w = 2 * np.pi * f
t_window = 0.25
fs = 5000
t = np.linspace(0, t_window, int(fs * t_window))
dt = 0.0002

N = len(t)

Ia1a_buf = np.zeros(N); Ib1a_buf = np.zeros(N); Ic1a_buf = np.zeros(N)
Ia1b_buf = np.zeros(N); Ib1b_buf = np.zeros(N); Ic1b_buf = np.zeros(N)
Ia2_buf  = np.zeros(N); Ib2_buf  = np.zeros(N); Ic2_buf  = np.zeros(N)

# --- INITIAL AMPLITUDES ---
A_normal = 1
A_fault  = 2

fault1a_active = False
fault1b_active = False
fault2_active  = False

fault1a_start_time = 0
fault1b_start_time = 0
fault2_start_time  = 0

# --- PLOT SETUP ---
plt.ion()
fig, (ax1a, ax1b, ax2) = plt.subplots(3, 1, figsize=(10, 9))

# Zone 1a
line_a1a, = ax1a.plot(t, np.zeros_like(t), label='Ia1a')
line_b1a, = ax1a.plot(t, np.zeros_like(t), label='Ib1a')
line_c1a, = ax1a.plot(t, np.zeros_like(t), label='Ic1a')
dot_a1a,  = ax1a.plot([t[-1]], [0], 'bo')
dot_b1a,  = ax1a.plot([t[-1]], [0], 'ro')
dot_c1a,  = ax1a.plot([t[-1]], [0], 'go')
ax1a.set_ylim(-2.5, 2.5)
ax1a.set_xlim(0, t_window)
ax1a.legend()
ax1a.set_title("Current Entering Zone 1a")

# Zone 1b
line_a1b, = ax1b.plot(t, np.zeros_like(t), label='Ia1b')
line_b1b, = ax1b.plot(t, np.zeros_like(t), label='Ib1b')
line_c1b, = ax1b.plot(t, np.zeros_like(t), label='Ic1b')
dot_a1b,  = ax1b.plot([t[-1]], [0], 'bo')
dot_b1b,  = ax1b.plot([t[-1]], [0], 'ro')
dot_c1b,  = ax1b.plot([t[-1]], [0], 'go')
ax1b.set_ylim(-2.5, 2.5)
ax1b.set_xlim(0, t_window)
ax1b.legend()
ax1b.set_title("Current Entering Zone 1b")

# Zone 2
line_a2, = ax2.plot(t, np.zeros_like(t), label='Ia2')
line_b2, = ax2.plot(t, np.zeros_like(t), label='Ib2')
line_c2, = ax2.plot(t, np.zeros_like(t), label='Ic2')
dot_a2,  = ax2.plot([t[-1]], [0], 'bo')
dot_b2,  = ax2.plot([t[-1]], [0], 'ro')
dot_c2,  = ax2.plot([t[-1]], [0], 'go')
ax2.set_ylim(-2.5, 2.5)
ax2.set_xlim(0, t_window)
ax2.legend()
ax2.set_title("Current Entering Zone 2")

plt.tight_layout()
fig.canvas.draw()
current_time = 0

while True:
    # --- SERIAL READ ---
    if ser.in_waiting:
        try:
            data = ser.readline().decode().strip()

            if data == "Zone1aFault":
                fault1a_active = True
                fault1a_start_time = time.time()
            elif data == "Zone1aNoFault":
                fault1a_active = False
            elif data == "Zone1bFault":
                fault1b_active = True
                fault1b_start_time = time.time()
            elif data == "Zone1bNoFault":
                fault1b_active = False
            elif data == "Zone2Fault":
                fault2_active = True
                fault2_start_time = time.time()
            elif data == "Zone2NoFault":
                fault2_active = False
        except:
            pass

    # --- SHIFT BUFFERS ---
    Ia1a_buf[:-1] = Ia1a_buf[1:]; Ib1a_buf[:-1] = Ib1a_buf[1:]; Ic1a_buf[:-1] = Ic1a_buf[1:]
    Ia1b_buf[:-1] = Ia1b_buf[1:]; Ib1b_buf[:-1] = Ib1b_buf[1:]; Ic1b_buf[:-1] = Ic1b_buf[1:]
    Ia2_buf[:-1]  = Ia2_buf[1:];  Ib2_buf[:-1]  = Ib2_buf[1:];  Ic2_buf[:-1]  = Ic2_buf[1:]

    # --- AMPLITUDE LOGIC: Zone 1a ---
    if fault1a_active:
        elapsed = time.time() - fault1a_start_time
        if elapsed > 6:
            A_a1a = A_b1a = A_c1a = 0
        else:
            A_a1a = A_fault; A_b1a = A_normal; A_c1a = A_normal
    else:
        A_a1a = A_b1a = A_c1a = A_normal

    # --- AMPLITUDE LOGIC: Zone 1b ---
    if fault1b_active:
        elapsed = time.time() - fault1b_start_time
        if elapsed > 6:
            A_a1b = A_b1b = A_c1b = 0
        else:
            A_a1b = A_fault; A_b1b = A_normal; A_c1b = A_normal
    else:
        A_a1b = A_b1b = A_c1b = A_normal

    # --- AMPLITUDE LOGIC: Zone 2 ---
    if fault2_active:
        elapsed = time.time() - fault2_start_time
        if elapsed > 6:
            A_a2 = A_b2 = A_c2 = 0
        else:
            A_a2 = A_fault; A_b2 = A_normal; A_c2 = A_normal
    else:
        A_a2 = A_b2 = A_c2 = A_normal

    # --- TIME INCREMENT ---
    current_time = 0 if current_time >= 0.05 else current_time + dt

    # --- NEW SAMPLES ---
    Ia1a_buf[-1] = A_a1a * np.sin(w * current_time)
    Ib1a_buf[-1] = A_b1a * np.sin(w * current_time - 2 * np.pi / 3)
    Ic1a_buf[-1] = A_c1a * np.sin(w * current_time + 2 * np.pi / 3)

    Ia1b_buf[-1] = A_a1b * np.sin(w * current_time)
    Ib1b_buf[-1] = A_b1b * np.sin(w * current_time - 2 * np.pi / 3)
    Ic1b_buf[-1] = A_c1b * np.sin(w * current_time + 2 * np.pi / 3)

    Ia2_buf[-1]  = A_a2  * np.sin(w * current_time)
    Ib2_buf[-1]  = A_b2  * np.sin(w * current_time - 2 * np.pi / 3)
    Ic2_buf[-1]  = A_c2  * np.sin(w * current_time + 2 * np.pi / 3)

    # --- UPDATE LINES AND DOTS ---
    line_a1a.set_ydata(Ia1a_buf); line_b1a.set_ydata(Ib1a_buf); line_c1a.set_ydata(Ic1a_buf)
    dot_a1a.set_data([t[-1]], [Ia1a_buf[-1]])
    dot_b1a.set_data([t[-1]], [Ib1a_buf[-1]])
    dot_c1a.set_data([t[-1]], [Ic1a_buf[-1]])

    line_a1b.set_ydata(Ia1b_buf); line_b1b.set_ydata(Ib1b_buf); line_c1b.set_ydata(Ic1b_buf)
    dot_a1b.set_data([t[-1]], [Ia1b_buf[-1]])
    dot_b1b.set_data([t[-1]], [Ib1b_buf[-1]])
    dot_c1b.set_data([t[-1]], [Ic1b_buf[-1]])

    line_a2.set_ydata(Ia2_buf); line_b2.set_ydata(Ib2_buf); line_c2.set_ydata(Ic2_buf)
    dot_a2.set_data([t[-1]], [Ia2_buf[-1]])
    dot_b2.set_data([t[-1]], [Ib2_buf[-1]])
    dot_c2.set_data([t[-1]], [Ic2_buf[-1]])

    # --- DRAW ---
    for ax, artists in [
        (ax1a, [line_a1a, line_b1a, line_c1a, dot_a1a, dot_b1a, dot_c1a]),
        (ax1b, [line_a1b, line_b1b, line_c1b, dot_a1b, dot_b1b, dot_c1b]),
        (ax2,  [line_a2,  line_b2,  line_c2,  dot_a2,  dot_b2,  dot_c2]),
    ]:
        for artist in artists:
            ax.draw_artist(artist)
        fig.canvas.blit(ax.bbox)

    fig.canvas.flush_events()
