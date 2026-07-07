"""Quick test: read ESP32 serial, send command, read response."""
import serial, time

# Open port — ESP32 may reset on first open via DTR, so we wait
s = serial.Serial("COM6", 115200, timeout=2)
# Re-assert DTR to prevent further resets
s.dtr = False

# Flush any pending data, then wait a bit for new data to arrive
time.sleep(1)

print("[1] Reading ESP32 initial output...")
if s.in_waiting:
    data = s.read(s.in_waiting).decode("ascii", errors="replace")
    print(f"    ESP32 says:\n{data}")
else:
    print("    (no data yet — ESP32 might already be running)")

# Send PING
print("[2] Sending PING...")
s.write(b"PING\n")
print("    Sent 5 bytes")

# Wait and read response
time.sleep(0.5)
if s.in_waiting:
    resp = s.read(s.in_waiting).decode("ascii", errors="replace")
    print(f"[3] Response: {resp.strip()}")
else:
    print("[3] No response — PC→ESP32 direction not working")

s.close()
print("[4] Done.")
