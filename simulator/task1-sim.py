import json
import time
import os
import random

ROOT_DIR = os.path.dirname(os.path.abspath(__file__))

CONFIG_FILE = ROOT_DIR + "/config.json"
OUTPUTS_FILE = ROOT_DIR + "/../shared/outputs.json"
SENSORS_FILE = ROOT_DIR + "/../shared/sensors.json"
LOOP_INTERVAL = 1.0  # seconds (10Hz)

class GreenhouseSim:
    def __init__(self):
        self.load_config()

        self.temp = [22.0, 22.0, 22.0]
        self.humidity = [50.0, 50.0]
        self.co2 = 400.0

        self.stuck_values = {
            "temperature": self.temp.copy(),
            "humidity": self.humidity.copy(),
            "co2": self.co2
        }

        self.drift_offsets = {
            "temperature": [0.0, 0.0, 0.0],
            "humidity": [0.0, 0.0],
            "co2": 0.0
        }

    def load_config(self):
        try:
            with open(CONFIG_FILE, "r") as f:
                config = json.load(f)
        except Exception as e:
            print("Failed to load config.json:", e)
            config = {}

        self.ambient_temp = config.get("ambient_temp", 15.0)
        self.noise = config.get("noise", {})
        self.faults = config.get("faults", {})
        self.drift_speed = config.get("drift_speed", 0.01)

    def apply_fault(self, sensor_type, values):
        fault = self.faults.get(sensor_type, "ok")

        if sensor_type == "temperature":
            result = []
            for i, val in enumerate(values):
                f = fault[i] if isinstance(fault, list) else fault
                result.append(self.fault_value("temperature", i, val, f))
            return result

        elif sensor_type == "humidity":
            result = []
            for i, val in enumerate(values):
                f = fault[i] if isinstance(fault, list) else fault
                result.append(self.fault_value("humidity", i, val, f))
            return result

        elif sensor_type == "co2":
            return self.fault_value("co2", None, values, fault)

    def fault_value(self, sensor_type, index, value, fault_mode):
        if fault_mode == "stuck":
            return self.stuck_values[sensor_type][index] if index is not None else self.stuck_values[sensor_type]
        elif fault_mode == "flatline":
            return 0.0
        elif fault_mode == "drift":
            if index is not None:
                self.drift_offsets[sensor_type][index] += self.drift_speed
                return value + self.drift_offsets[sensor_type][index]
            else:
                self.drift_offsets[sensor_type] += self.drift_speed
                return value + self.drift_offsets[sensor_type]
        else:
            return value  # 'ok'

    def update(self, outputs, dt):
        heater = outputs.get("heater", 0.0)
        fan1 = outputs.get("fan1", 0.0)
        fan2 = outputs.get("fan2", 0.0)
        humidifier = outputs.get("humidifier", 0.0)
        co2_injector = outputs.get("co2_injector", 0.0)

        for i in range(3):
            heat_input = (heater / 10.0) * 2.0
            cooling = ((self.temp[i] - self.ambient_temp) * 0.05 +
                       (fan1 + fan2) / 20.0 * 1.5)
            self.temp[i] += (heat_input - cooling) * dt

        avg_fan = (fan1 + fan2) / 2.0
        humidity_change = (humidifier / 10.0 * 5.0 - avg_fan / 10.0 * 3.0) * dt
        for i in range(2):
            self.humidity[i] += humidity_change
            self.humidity[i] = max(0.0, min(100.0, self.humidity[i]))

        co2_change = (co2_injector / 10.0 * 50.0 - avg_fan / 10.0 * 30.0) * dt
        self.co2 += co2_change
        self.co2 = max(300.0, self.co2)

    def read_outputs(self):
        try:
            with open(OUTPUTS_FILE, "r") as f:
                return json.load(f)
        except:
            return {}

    def write_sensors(self):
        temp = self.apply_fault("temperature", self.temp)
        humidity = self.apply_fault("humidity", self.humidity)
        co2 = self.apply_fault("co2", self.co2)

        noisy_temp = [round(t + random.gauss(0, self.noise.get("temperature", 0.2)), 2) for t in temp]
        noisy_humidity = [round(h + random.gauss(0, self.noise.get("humidity", 1.0)), 2) for h in humidity]
        noisy_co2 = round(co2 + random.gauss(0, self.noise.get("co2", 5.0)), 2)

        data = {
            "temperature": noisy_temp,
            "humidity": noisy_humidity,
            "co2": noisy_co2
        }

        with open(SENSORS_FILE, "w") as f:
            json.dump(data, f, indent=2)

    def step(self, dt):
        outputs = self.read_outputs()
        self.update(outputs, dt)
        self.write_sensors()

def main():
    sim = GreenhouseSim()
    print("Greenhouse simulator running with fault simulation...")

    last_time = time.time()
    while True:
        now = time.time()
        dt = now - last_time
        last_time = now

        sim.step(dt)
        time.sleep(max(0.0, LOOP_INTERVAL - (time.time() - now)))

if __name__ == "__main__":
    main()
