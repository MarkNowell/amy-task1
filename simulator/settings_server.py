# settings_server.py
import socket
import json
import threading

SETTINGS = {
    "target_temperature": 22.5,
    "target_humidity": 55.0,
    "target_co2": 400.0
}

HOST = '127.0.0.1'
PORT = 5001

def handle_client(conn, addr):
    with conn:
        print(f"Connected by {addr}")
        message = json.dumps(SETTINGS).encode()
        conn.sendall(message)
        print(f"Sent settings to {addr}")

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print(f"Settings server listening on {HOST}:{PORT}...")

    while True:
        conn, addr = s.accept()
        threading.Thread(target=handle_client, args=(conn, addr), daemon=True).start()
