#!/usr/bin/env python3
"""
ClawdMochi — 图形化控制面板
系统托盘 + 迷你控制窗口，替代 claude_monitor.py

用法:
    pythonw control_panel.py    # 无黑窗启动（开机自启用）
    python  control_panel.py    # 带控制台输出（调试用）
"""
import customtkinter as ctk
import pystray
from PIL import Image, ImageDraw
import serial
import serial.tools.list_ports
import threading
import time
import os
import sys

# ── Configuration ──────────────────────────────────────────────
STATUS_FILE = os.path.expanduser(r"~\.claude\claude_status")
BAUD_RATE = 115200
PING_INTERVAL = 5.0       # seconds between PING heartbeats
POLL_INTERVAL = 0.25      # seconds between status-file checks
RECONNECT_INTERVAL = 2.0  # seconds between reconnect attempts

VALID_STATES = {"IDLE", "THINK", "CODE", "RUN", "ERROR"}

# ── Shared State (protected by state_lock) ─────────────────────
state_lock = threading.Lock()
shared = {
    "current_state": "IDLE",
    "mode": "follow",           # "follow" or "manual"
    "serial_connected": False,
    "port_name": "",
    "running": True,            # set False to shut down
}


# ═══════════════════════════════════════════════════════════════
#  SERIAL COMMUNICATION
# ═══════════════════════════════════════════════════════════════

ser = None
ser_lock = threading.Lock()


def find_esp32_port():
    """Auto-detect ESP32-C3 USB serial port. Returns port name or None."""
    ports = serial.tools.list_ports.comports()
    for p in ports:
        lower = (p.description + p.manufacturer).lower()
        if any(kw in lower for kw in
               ("usb serial", "esp32", "silicon labs", "cp210", "ch340", "jtag")):
            return p.device
    # Fallback: any port with "usb" in description
    for p in ports:
        if "usb" in (p.description + p.manufacturer).lower():
            return p.device
    # Last resort: return last available port
    return ports[-1].device if ports else None


def open_serial(port=None):
    """Open serial connection. Returns True on success."""
    global ser
    # Close existing connection first
    with ser_lock:
        if ser and ser.is_open:
            try:
                ser.close()
            except Exception:
                pass
            ser = None

    target = port or find_esp32_port()
    if not target:
        return False

    try:
        with ser_lock:
            ser = serial.Serial(target, BAUD_RATE, timeout=0.1, write_timeout=0.5)
            time.sleep(0.1)
            ser.dtr = False
            ser.rts = False
        # Wait for ESP32 to finish boot (logo animation ~4s)
        time.sleep(4.0)
        with ser_lock:
            if ser and ser.is_open:
                ser.reset_input_buffer()
                ser.reset_output_buffer()
        with state_lock:
            shared["port_name"] = target
            shared["serial_connected"] = True
        print(f"[OK] Connected to {target}", flush=True)
        return True
    except (serial.SerialException, OSError) as e:
        print(f"[!] Serial open failed: {e}", flush=True)
        return False


def send_cmd(cmd):
    """Send a command to ESP32. Returns True on success."""
    global ser
    with ser_lock:
        if not ser or not ser.is_open:
            return False
        try:
            line = (cmd + "\n").encode("ascii")
            n = ser.write(line)
            return n == len(line)
        except (serial.SerialException, OSError):
            pass
    return False


def close_serial():
    """Close serial port and update state."""
    global ser
    with ser_lock:
        if ser and ser.is_open:
            try:
                ser.close()
            except Exception:
                pass
            ser = None
    with state_lock:
        shared["serial_connected"] = False
        shared["port_name"] = ""


# ═══════════════════════════════════════════════════════════════
#  STATUS FILE
# ═══════════════════════════════════════════════════════════════

def read_status_file():
    """Read the Hook status file. Returns state string or None."""
    try:
        if not os.path.exists(STATUS_FILE):
            return None
        with open(STATUS_FILE, "r") as f:
            state = f.read().strip().upper()
        for vs in VALID_STATES:
            if state.startswith(vs):
                return vs
    except (OSError, PermissionError):
        pass
    return None


# ═══════════════════════════════════════════════════════════════
#  DAEMON THREAD
# ═══════════════════════════════════════════════════════════════

def daemon_loop():
    """Background thread: monitor status file, send state to ESP32, heartbeat."""
    last_ping = 0.0
    last_state_sent = None  # track what we last sent to avoid duplicates

    while True:
        with state_lock:
            if not shared["running"]:
                break
            mode = shared["mode"]

        # ── 1. Reconnect if disconnected ──
        with state_lock:
            connected = shared["serial_connected"]
        if not connected:
            time.sleep(RECONNECT_INTERVAL)
            if open_serial():
                last_ping = time.time()
                last_state_sent = None  # force re-sync on reconnect
            continue

        # ── 2. Follow mode: read status file ──
        if mode == "follow":
            file_state = read_status_file()
            if file_state:
                with state_lock:
                    shared["current_state"] = file_state

        # ── 3. Send state change to ESP32 ──
        with state_lock:
            state_to_send = shared["current_state"]
        if state_to_send != last_state_sent:
            if send_cmd(state_to_send):
                last_state_sent = state_to_send
            else:
                # Write failed → mark disconnected
                with state_lock:
                    shared["serial_connected"] = False
                continue

        # ── 4. Periodic PING ──
        now = time.time()
        if now - last_ping >= PING_INTERVAL:
            if not send_cmd("PING"):
                with state_lock:
                    shared["serial_connected"] = False
                continue
            last_ping = now

        # ── 5. Drain ESP32 responses (prevent buffer buildup) ──
        with ser_lock:
            if ser and ser.is_open and ser.in_waiting:
                try:
                    ser.read(ser.in_waiting)
                except Exception:
                    pass

        time.sleep(POLL_INTERVAL)

    # Cleanup on exit
    close_serial()


# ═══════════════════════════════════════════════════════════════
#  GUI — CONTROL PANEL WINDOW
# ═══════════════════════════════════════════════════════════════

class ControlPanel:
    """CustomTkinter mini window with 5 state buttons + mode toggle + face buttons."""

    COLORS = {
        "IDLE":  "#FF6B35",
        "THINK": "#6C5CE7",
        "CODE":  "#00B894",
        "RUN":   "#0984E3",
        "ERROR": "#D63031",
    }
    LABELS = {
        "IDLE":  "IDLE",
        "THINK": "THINK",
        "CODE":  "CODE",
        "RUN":   "RUN",
        "ERROR": "ERROR",
    }

    # ── Face expression buttons ──
    FACE_CMDS = {
        "BLUSH": "BLUSH", "AWE": "AWE", "WORRY": "WORRY",
        "PROUD": "PROUD", "HEART": "HEART", "SLEEP": "SLEEP",
        "WINK": "WINK",  "THK":  "THK",
    }
    FACE_LABELS = {
        "BLUSH": "😊 脸红",
        "AWE":   "😲 敬畏",
        "WORRY": "😟 担忧",
        "PROUD": "😎 骄傲",
        "HEART": "❤️  爱心",
        "SLEEP": "😴 睡眠",
        "WINK":  "😉 眨眼",
        "THK":   "🤔 思考",
    }
    FACE_COLORS = {
        "BLUSH": "#FF6B8A",
        "AWE":   "#FFD93D",
        "WORRY": "#6C5CE7",
        "PROUD": "#00CEC9",
        "HEART": "#FF6B6B",
        "SLEEP": "#74B9FF",
        "WINK":  "#FDCB6E",
        "THK":   "#A29BFE",
    }

    def __init__(self):
        ctk.set_appearance_mode("dark")
        ctk.set_default_color_theme("blue")

        self.window = ctk.CTk()
        self.window.title("ClawdMochi")
        self.window.geometry("220x760")
        self.window.resizable(False, False)
        self.window.protocol("WM_DELETE_WINDOW", self._on_close)

        # ── Connection status ──
        self.status_frame = ctk.CTkFrame(self.window, fg_color="transparent")
        self.status_frame.pack(pady=(10, 0))

        self.conn_dot = ctk.CTkLabel(
            self.status_frame, text="●", font=("", 16),
            text_color="gray", width=20)
        self.conn_dot.pack(side="left")
        self.conn_label = ctk.CTkLabel(
            self.status_frame, text="检测中...", font=("", 12),
            text_color="gray")
        self.conn_label.pack(side="left", padx=(4, 0))

        # ── Mode toggle ──
        self.mode_var = ctk.StringVar(value="follow")
        self.mode_frame = ctk.CTkFrame(self.window, fg_color="transparent")
        self.mode_frame.pack(pady=(10, 6))

        self.btn_follow = ctk.CTkButton(
            self.mode_frame, text="跟随", width=70, height=28,
            fg_color="#00B894", hover_color="#00A381",
            command=lambda: self._set_mode("follow"))
        self.btn_follow.pack(side="left", padx=(0, 4))

        self.btn_manual = ctk.CTkButton(
            self.mode_frame, text="手动", width=70, height=28,
            fg_color="#4A4A5A", hover_color="#5A5A6A",
            command=lambda: self._set_mode("manual"))
        self.btn_manual.pack(side="left")

        # ── State buttons ──
        self.buttons: dict[str, ctk.CTkButton] = {}
        self.btn_frame = ctk.CTkFrame(self.window, fg_color="transparent")
        self.btn_frame.pack(pady=(8, 0))

        for state in ("IDLE", "THINK", "CODE", "RUN", "ERROR"):
            btn = ctk.CTkButton(
                self.btn_frame, text=self.LABELS[state],
                width=160, height=42, font=("", 14, "bold"),
                fg_color=self.COLORS[state],
                hover_color=self._darken(self.COLORS[state]),
                command=lambda s=state: self._on_state_click(s))
            btn.pack(pady=3)
            self.buttons[state] = btn

        # ── Face expression buttons ──
        self.face_sep = ctk.CTkLabel(
            self.window, text="── 表情 ──", font=("", 10), text_color="#666")
        self.face_sep.pack(pady=(8, 2))

        self.face_buttons: dict[str, ctk.CTkButton] = {}
        self.face_frame = ctk.CTkFrame(self.window, fg_color="transparent")
        self.face_frame.pack(pady=(2, 0))

        face_order = ["BLUSH", "AWE", "WORRY", "PROUD",
                      "HEART", "SLEEP", "WINK", "THK"]
        for i, key in enumerate(face_order):
            row = i // 2
            col = i % 2
            btn = ctk.CTkButton(
                self.face_frame, text=self.FACE_LABELS[key],
                width=90, height=34, font=("", 12),
                fg_color=self.FACE_COLORS[key],
                hover_color=self._darken(self.FACE_COLORS[key]),
                command=lambda k=key: self._on_face_click(k))
            btn.grid(row=row, column=col, padx=3, pady=2, sticky="ew")
            self.face_buttons[key] = btn

        # ── Random face button ──
        self.btn_random = ctk.CTkButton(
            self.face_frame, text="🎲 随机表情",
            width=186, height=36, font=("", 13, "bold"),
            fg_color="#636E72", hover_color="#4A5558",
            command=self._on_random_click)
        self.btn_random.grid(row=4, column=0, columnspan=2, padx=3, pady=(6, 0), sticky="ew")

        # ── Status line ──
        self.info_label = ctk.CTkLabel(
            self.window, text="模式：跟随", font=("", 11), text_color="#888")
        self.info_label.pack(pady=(8, 10))

        # ── Start periodic UI refresh ──
        self._refresh_ui()

    # ── Helpers ──

    @staticmethod
    def _darken(hex_color: str, factor: float = 0.8) -> str:
        """Darken a hex color for hover / disabled effect."""
        r = int(hex_color[1:3], 16)
        g = int(hex_color[3:5], 16)
        b = int(hex_color[5:7], 16)
        r, g, b = int(r * factor), int(g * factor), int(b * factor)
        return f"#{r:02x}{g:02x}{b:02x}"

    def _set_mode(self, mode: str):
        """Switch between follow/manual mode."""
        with state_lock:
            shared["mode"] = mode
        if mode == "follow":
            self.btn_follow.configure(fg_color="#00B894")
            self.btn_manual.configure(fg_color="#4A4A5A")
            # Sync current Hook state immediately
            file_state = read_status_file()
            if file_state:
                with state_lock:
                    shared["current_state"] = file_state
        else:
            self.btn_manual.configure(fg_color="#E17055")
            self.btn_follow.configure(fg_color="#4A4A5A")
        self._refresh_ui()

    def _on_state_click(self, state: str):
        """Handle manual state button click — send directly + update state."""
        with state_lock:
            if shared["mode"] != "manual":
                return
            shared["current_state"] = state
        send_cmd(state)  # force-send, ensures it works after face expression
        self._refresh_ui()

    def _on_face_click(self, face_key: str):
        """Send face expression command directly to ESP32."""
        cmd = self.FACE_CMDS[face_key]
        send_cmd(cmd)

    def _on_random_click(self):
        """Send random face command directly to ESP32."""
        send_cmd("RANDOM")

    def _on_close(self):
        """Hide window instead of closing — minimize to tray."""
        self.window.withdraw()

    def _refresh_ui(self):
        """Periodic UI refresh — connection dot, button states, info label."""
        with state_lock:
            mode = shared["mode"]
            connected = shared["serial_connected"]
            port = shared["port_name"]
            current = shared["current_state"]

        # Connection indicator
        if connected:
            self.conn_dot.configure(text_color="#44FF88")
            short_port = port.replace("COM", "") if port else "?"
            self.conn_label.configure(
                text=f"COM{short_port} 已连接", text_color="#CCC")
        else:
            self.conn_dot.configure(text_color="#FF4444")
            self.conn_label.configure(text="未连接", text_color="#FF6666")

        # Button states based on mode
        for state, btn in self.buttons.items():
            if mode == "manual":
                btn.configure(state="normal")
                if state == current:
                    btn.configure(fg_color=self.COLORS[state])
                else:
                    btn.configure(
                        fg_color=self._darken(self.COLORS[state], 0.5))
            else:
                btn.configure(
                    state="disabled",
                    fg_color=self._darken(self.COLORS[state], 0.4))

        # Face buttons — always enabled, dim when disconnected
        for key, btn in self.face_buttons.items():
            if connected:
                btn.configure(state="normal",
                              fg_color=self.FACE_COLORS[key])
            else:
                btn.configure(state="disabled",
                              fg_color=self._darken(self.FACE_COLORS[key], 0.4))

        if connected:
            self.btn_random.configure(state="normal", fg_color="#636E72")
        else:
            self.btn_random.configure(state="disabled",
                                      fg_color=self._darken("#636E72", 0.4))

        # Info line
        if mode == "follow":
            self.info_label.configure(
                text=f"跟随模式 · 当前：{current}", text_color="#888")
        else:
            self.info_label.configure(
                text=f"手动模式 · 当前：{current}", text_color="#E17055")

        self.window.after(200, self._refresh_ui)  # 5 Hz refresh

    def run(self):
        """Start the GUI main loop."""
        self.window.mainloop()


# ═══════════════════════════════════════════════════════════════
#  SYSTEM TRAY
# ═══════════════════════════════════════════════════════════════

def _create_tray_image(color: tuple) -> Image.Image:
    """Create a 64x64 colored circle icon for the tray."""
    img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.ellipse((4, 4, 60, 60), fill=color)
    draw.ellipse((4, 4, 60, 60), outline=(255, 255, 255, 60), width=2)
    return img


def _on_tray_state(icon, state: str):
    """Tray menu: switch state (works in both modes)."""
    with state_lock:
        shared["current_state"] = state
    # If in follow mode, this is a one-shot manual override
    # The daemon won't override until next status file read


def _make_tray_menu(icon) -> pystray.Menu:
    """Build the tray right-click menu with dynamic checkmarks."""
    with state_lock:
        current = shared["current_state"]

    state_items = []
    for s in ("IDLE", "THINK", "CODE", "RUN", "ERROR"):
        check = "✓ " if s == current else "    "
        state_items.append(pystray.MenuItem(
            f"{check}{s}",
            lambda icon=icon, state=s: _on_tray_state(icon, state),
            enabled=True))

    return pystray.Menu(
        pystray.MenuItem(
            "显示控制面板",
            lambda icon=icon: panel.window.deiconify() if panel else None,
            default=True),
        pystray.Menu.SEPARATOR,
        *state_items,
        pystray.Menu.SEPARATOR,
        pystray.MenuItem("退出", _on_tray_exit),
    )


def _on_tray_exit(icon=None):
    """Graceful shutdown: stop daemon, close serial, quit GUI."""
    with state_lock:
        shared["running"] = False
    close_serial()
    if icon:
        icon.stop()
    if panel:
        panel.window.quit()


# Global references for cross-module access
panel: ControlPanel | None = None
tray_icon: pystray.Icon | None = None


def setup_tray(ctrl_panel: ControlPanel):
    """Create and start the system tray icon in a background thread."""
    global panel, tray_icon
    panel = ctrl_panel

    green_icon = _create_tray_image((68, 255, 136))
    red_icon = _create_tray_image((255, 68, 68))

    tray_icon = pystray.Icon(
        "ClawdMochi", green_icon, "ClawdMochi",
        menu=_make_tray_menu(tray_icon))

    # Dynamic menu & icon color updater (runs every second)
    def update_tray():
        if tray_icon and tray_icon.visible:
            tray_icon.menu = _make_tray_menu(tray_icon)
            tray_icon.update_menu()
            with state_lock:
                connected = shared["serial_connected"]
            tray_icon.icon = green_icon if connected else red_icon
        if tray_icon:
            threading.Timer(1.0, update_tray).start()

    update_tray()

    # Run pystray in daemon thread
    tray_thread = threading.Thread(target=tray_icon.run, daemon=True)
    tray_thread.start()


# ═══════════════════════════════════════════════════════════════
#  ENTRY POINT
# ═══════════════════════════════════════════════════════════════

def main():
    """Start daemon thread, GUI window, and system tray."""
    global panel

    print("=" * 50, flush=True)
    print("  ClawdMochi — 图形化控制面板", flush=True)
    print("=" * 50, flush=True)

    # 1. Initial serial connection (non-blocking — daemon will retry)
    found = open_serial()
    if found:
        print(f"[OK] Connected to {shared['port_name']}", flush=True)
    else:
        print("[!] No ESP32 found. Will retry in background.", flush=True)

    # 2. Send initial IDLE
    send_cmd("IDLE")

    # 3. Start daemon thread
    daemon = threading.Thread(target=daemon_loop, daemon=True, name="daemon")
    daemon.start()

    # 4. Start GUI
    panel = ControlPanel()
    setup_tray(panel)

    # 5. Hide window on start (go straight to tray for auto-start)
    # Comment out for debugging:
    # panel.window.withdraw()

    panel.run()

    # 6. Cleanup on exit
    with state_lock:
        shared["running"] = False
    close_serial()
    print("[OK] Closed. Bye!", flush=True)


if __name__ == "__main__":
    main()
