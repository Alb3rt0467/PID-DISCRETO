
import tkinter as tk
from tkinter import ttk
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import serial
import time
import pandas as pd


ser = serial.Serial('COM5', 9600) #Escoger el COM del Arduino
time.sleep(2)
ser.reset_input_buffer()


root = tk.Tk()
root.title("Control de PID y Gráfica de Temperatura")


fig, ax = plt.subplots()
ax.set_xlabel('Tiempo (s)')
ax.set_ylabel('Temperatura (°C)')


times = []
temp1_values = []
temp2_values = []


setpoint = 0


def update_graph():
    if ser.in_waiting > 0:
        line = ser.readline().decode('utf-8').strip()
        if line:
            try:
                # Volver a usar split() para separar los valores
                millis, temp1, temp2 = map(float, line.split(','))

                # Convertir el tiempo de millis a segundos
                seconds = millis / 1000
                times.append(seconds)
                temp1_values.append(temp1)
                temp2_values.append(temp2)

                # Actualizar la gráfica
                ax.clear()
                ax.set_xlabel('Tiempo (s)')
                ax.set_ylabel('Temperatura (°C)')
                ax.set_xlim(0, max(times) + 1)
                ax.set_ylim(min(min(temp1_values), min(temp2_values)) - 5, max(max(temp1_values), max(temp2_values)) + 5)
                ax.plot(times, temp1_values, label='Setpoint')
                ax.plot(times, temp2_values, label='Salida')

                ax.legend()
                canvas.draw()
            except ValueError:
                pass
    root.after(50, update_graph)


def send_pid_values():
    global setpoint
    kp = kp_entry.get()
    ti = ti_entry.get()
    td = td_entry.get()
    setpoint = setpoint_entry.get()
    
    if kp and ti and td and setpoint:

        data_to_send = f"{setpoint},{kp},{ti},{td}\n"
        ser.write(data_to_send.encode())


def save_data():

    data = {'Time (s)': times, 'Temp1 (°C)': temp1_values, 'Temp2 (°C)': temp2_values}
    df = pd.DataFrame(data)
    df.to_excel('data.xlsx', index=False)
    print("Datos guardados en data.xlsx")


def terminate():
    ser.close()
    root.destroy()

# Crear la figura para la gráfica
canvas = FigureCanvasTkAgg(fig, master=root)
canvas.get_tk_widget().pack()


frame = ttk.Frame(root)
frame.pack()


setpoint_label = ttk.Label(frame, text="Setpoint:")
setpoint_label.grid(row=0, column=0)
setpoint_entry = ttk.Entry(frame)
setpoint_entry.insert(0, "0")  
setpoint_entry.grid(row=0, column=1)

kp_label = ttk.Label(frame, text="Kp:")
kp_label.grid(row=1, column=0)
kp_entry = ttk.Entry(frame)
kp_entry.grid(row=1, column=1)

ti_label = ttk.Label(frame, text="Ti:")
ti_label.grid(row=2, column=0)
ti_entry = ttk.Entry(frame)
ti_entry.grid(row=2, column=1)

td_label = ttk.Label(frame, text="Td:")
td_label.grid(row=3, column=0)
td_entry = ttk.Entry(frame)
td_entry.grid(row=3, column=1)


send_button = ttk.Button(frame, text="Actualizar PID", command=send_pid_values)
send_button.grid(row=4, columnspan=2)


save_button = ttk.Button(root, text="Guardar Datos", command=save_data)
save_button.pack()


terminate_button = ttk.Button(root, text="Terminar", command=terminate)
terminate_button.pack()


update_graph()


root.mainloop()
