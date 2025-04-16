from collections import deque
from matplotlib import pyplot as plt
from matplotlib.animation import FuncAnimation
import json
import time
import os

ROOT_DIR = os.path.dirname(os.path.abspath(__file__))

BUFFER_SIZE = 300 #30s at 10Hz
INTERVAL_MS = 100 #update interval

#rolling buffers
time_buffer = deque(maxlen=BUFFER_SIZE)
temp_buffers = [deque(maxlen=BUFFER_SIZE) for _ in range(3)]
humid_buffers = [deque(maxlen=BUFFER_SIZE) for _ in range(2)]
co2_buffers = deque(maxlen=BUFFER_SIZE)
heater_buffer = deque(maxlen=BUFFER_SIZE)
fan1_buffer = deque(maxlen=BUFFER_SIZE)
fan2_buffer = deque(maxlen=BUFFER_SIZE)
humidifier_buffer = deque(maxlen=BUFFER_SIZE)
co2injector_buffer = deque(maxlen=BUFFER_SIZE)

#set up the graph
fig,(ax1,ax2,ax3) = plt.subplots(3,1,sharex=True)
lines1 = [ax1.plot([],[],label=f"Temp {i+1}")[0] for i in range(3)]
heater_line = ax1.plot([], [], label="Heater", linestyle="--", color="red")[0]
fan1_line = ax1.plot([], [], label="Fan 1", linestyle="--", color="blue")[0]
fan2_line = ax1.plot([], [], label="Fan 2", linestyle="--", color="green")[0]

lines2 = [ax2.plot([],[],label=f"Humidity {i+1}")[0] for i in range(2)]
humidifier_line = ax2.plot([], [], label="Humidifier", linestyle="--", color="purple")[0]
lines3 = [ax3.plot([], [], label="CO₂")[0]]  # Convert to list for consistency
co2injector_line = ax3.plot([], [], label="CO₂ Injector", linestyle="--", color="orange")[0]

ax1.set_ylim(10,50)
ax2.set_ylim(20,90)
ax3.set_ylim(200,800)

ax1.set_xlim(0,BUFFER_SIZE)
ax1.legend()
ax2.legend()
ax3.legend()
ax1.set_title("Temperature Sensors")
ax1.set_xlabel("Time")
ax1.set_ylabel("°C")
ax2.set_ylabel("%")
ax3.set_ylabel("ppm")

def update_plot(frame):
    try:
        with open(ROOT_DIR + "/../shared/sensors.json") as f:
            sensors = json.load(f)
            temps=sensors["temperature"]
            humid =sensors["humidity"]
            co2=sensors["co2"]
    except Exception as e:
        print("Sensor read error:",e)
        return
    try:
        with open(ROOT_DIR + "/../shared/outputs.json") as f:
            outputs = json.load(f)
            heater = outputs["heater"]
            fan1 = outputs["fan1"]
            fan2 = outputs["fan2"]
            humidifier = outputs["humidifier"]       
            co2injector = outputs["co2_injector"] 
    except Exception as e:
        print("Output read error:",e)
        return

    timestamp=time.time()
    time_buffer.append(timestamp)
    for i in range(len(temps)):
        temp_buffers[i].append(temps[i])
    for i, line in enumerate(lines1):
        line.set_data(range(len(temp_buffers[i])),temp_buffers[i])
    ax1.set_xlim(max(0,len(temp_buffers[0])-BUFFER_SIZE),len(temp_buffers[0]))
    # Shifted logic output lines up visually for clarity (range 36–39)
    heater_buffer.append(36 + heater * 2)  # Appears around 36–38°C
    fan1_buffer.append(36 + fan1 * 2)
    fan2_buffer.append(36 + fan2 * 2)
    humidifier_buffer.append(80 + humidifier * 2)   
    co2injector_buffer.append(480 + co2injector * 2)

    heater_line.set_data(range(len(heater_buffer)), heater_buffer)
    fan1_line.set_data(range(len(fan1_buffer)), fan1_buffer)
    fan2_line.set_data(range(len(fan2_buffer)), fan2_buffer)
    humidifier_line.set_data(range(len(humidifier_buffer)), humidifier_buffer)

    for i in range(len(humid)):
        humid_buffers[i].append(humid[i])
    for i, line in enumerate(lines2):
        line.set_data(range(len(humid_buffers[i])),humid_buffers[i])
    ax2.set_xlim(max(0,len(humid_buffers[0])-BUFFER_SIZE),len(humid_buffers[0]))
    
    co2_buffers.append(co2)
    lines3[0].set_data(range(len(co2_buffers)), co2_buffers)
    co2injector_line.set_data(range(len(co2injector_buffer)), co2injector_buffer)

    ax3.set_xlim(max(0, len(co2_buffers) - BUFFER_SIZE), len(co2_buffers))

ani = FuncAnimation(fig, update_plot, interval=INTERVAL_MS)
plt.tight_layout()
plt.show()
