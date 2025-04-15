# settings_server.py
import socket
import json
import threading
import os
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import time

ROOT_DIR = os.path.dirname(os.path.abspath(__file__))

SETTINGS_FILE = 'settings.json'
SETTINGS_PATH = ROOT_DIR + '/' + SETTINGS_FILE
HOST = '127.0.0.1'
PORT = 5001

SETTINGS_LOCK = threading.Lock()
SETTINGS = {}

def load_settings():
    global SETTINGS
    try:
        with open(SETTINGS_PATH, 'r') as f:
            settings = json.load(f)
        with SETTINGS_LOCK:
            SETTINGS = settings
        print("Settings loaded:", SETTINGS)
    except Exception as e:
        print("Error loading settings:", e)

class SettingsFileHandler(FileSystemEventHandler):
    def on_modified(self, event):
        if event.src_path.endswith(SETTINGS_FILE):
            print(f"{SETTINGS_FILE} modified, reloading...")
            load_settings()

def handle_client(conn, addr):
    with conn:
        print(f"Connected by {addr}")
        with SETTINGS_LOCK:
            message = json.dumps(SETTINGS).encode()
        conn.sendall(message)
        print(f"Sent settings to {addr}")

def start_file_watcher():
    event_handler = SettingsFileHandler()
    observer = Observer()
    observer.schedule(event_handler, path=ROOT_DIR, recursive=False)
    observer.start()
    return observer

# Main server logic
if __name__ == "__main__":
    load_settings()
    observer = start_file_watcher()

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        print(f"Settings server listening on {HOST}:{PORT}...")

        try:
            while True:
                conn, addr = s.accept()
                threading.Thread(target=handle_client, args=(conn, addr), daemon=True).start()
        except KeyboardInterrupt:
            print("Shutting down...")
        finally:
            observer.stop()
            observer.join()
