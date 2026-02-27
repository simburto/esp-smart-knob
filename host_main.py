import socket
from pynput.keyboard import Key, Controller

UDP_IP = "0.0.0.0"
UDP_PORT = 8080

keyboard = Controller()
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Listening for wireless volume commands on port {UDP_PORT}...")

while True:
    try:
        data, addr = sock.recvfrom(1024)
        msg = data.decode('utf-8').strip()

        delta = int(msg)

        key = Key.media_volume_up if delta > 0 else Key.media_volume_down
        for _ in range(abs(delta)):
            keyboard.press(key)
            keyboard.release(key)
            
    except ValueError:
        pass
    except KeyboardInterrupt:
        print("Closing listener.")
        break