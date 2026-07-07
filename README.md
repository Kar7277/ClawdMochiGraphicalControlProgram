# Clawd Mochi 🐾

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

> ESP32-C3 + ST7789 桌面伴侣 —— Claude Code 工作时自动切换表情动画

## 它能做什么

通过 USB 连接电脑，根据 Claude Code 的工作状态**自动切换 5 种表情动画**：

| 状态 | 触发场景 | 动画 |
|------|---------|------|
| 🟠 IDLE | 空闲 / 15s 无心跳 | 左右摆眼 + 眨眼 |
| 💜 THINK | 用户发消息 | 三点依次出现 |
| 💚 CODE | 编辑/写入文件 | 笑脸交替 |
| 💙 RUN | 执行 Shell 命令 | 心跳 |
| ❤️ ERROR | 出错 | 生气 + X 闪烁 |

附带一个 **Python 图形化控制面板**，支持系统托盘、手动切换、开机自启。

## 硬件

| 物料 | 型号 |
|------|------|
| MCU | ESP32-C3 Super Mini (HW-466AB) |
| 屏幕 | ST7789 1.54" 240×240 IPS |
| 连接 | USB 数据线（内置 USB-JTAG） |

### 接线

| 屏幕 | ESP32-C3 GPIO |
|------|--------------|
| SDA (MOSI) | 10 |
| SCL (SCK) | 8 |
| RST | 2 |
| DC | 1 |
| CS | 4 |
| BL | 3 |
| VCC | 3.3V |
| GND | GND |

## 从零开始复刻

### 1. 克隆仓库
```bash
git clone <repo-url>
cd ClawdMochi
```

### 2. 烧录固件
- 安装 [VSCode](https://code.visualstudio.com/) + [PlatformIO](https://platformio.org/) 扩展
- 打开项目根目录，PlatformIO 自动安装依赖
- ESP32-C3 通过 USB 连电脑
- Build → Upload（端口 COM6）

### 3. 安装 Python 依赖
```bash
pip install -r companion/requirements.txt
```
> Python ≥ 3.10，依赖：customtkinter、pystray、Pillow、pyserial

### 4. 配置 Claude Code Hooks

在 `~/.claude/settings.json` 中添加：

```json
{
  "hooks": {
    "Stop": [{
      "hooks": [{
        "type": "command",
        "command": "python \"<项目路径>/companion/status_setter.py\" IDLE"
      }]
    }],
    "UserPromptSubmit": [{
      "hooks": [{
        "type": "command",
        "command": "python \"<项目路径>/companion/status_setter.py\" THINK"
      }]
    }],
    "PreToolUse": [
      {
        "matcher": "Bash",
        "hooks": [{
          "type": "command",
          "command": "python \"<项目路径>/companion/status_setter.py\" RUN"
        }]
      },
      {
        "matcher": "Edit|Write",
        "hooks": [{
          "type": "command",
          "command": "python \"<项目路径>/companion/status_setter.py\" CODE"
        }]
      }
    ],
    "PostToolUse": [{
      "matcher": "Bash|Edit|Write",
      "hooks": [{
        "type": "command",
        "command": "python \"<项目路径>/companion/status_setter.py\" THINK"
      }]
    }]
  }
}
```

### 5. 启动控制面板
```bash
python companion/control_panel.py
```
窗口弹出，接上 ESP32 后状态灯变绿。关窗缩到托盘，开机自启见下方。

### 6. 开机自启（Windows）
将以下内容保存为 `ClawdMochi.vbs`，放到 `shell:startup` 文件夹：
```vbs
Set WshShell = CreateObject("WScript.Shell")
WshShell.Run """<pythonw.exe路径>"" ""<项目路径>/companion/control_panel.py""", 0, False
```

## 通信协议

PC → ESP32 通过 USB CDC 串口（115200bps）发送单行文本命令：

| 命令 | 效果 |
|------|------|
| `IDLE` | 空闲表情 |
| `THINK` | 思考表情 |
| `CODE` | 编码表情 |
| `RUN` | 运行表情 |
| `ERROR` | 错误表情 |
| `PING` | 心跳（ESP32 返回 PONG） |

ESP32 超过 15s 未收到 PING 自动切回 IDLE。

## 文件说明

```
├── src/                 # ESP32 固件
│   ├── main.cpp         # 主程序（动画系统 + 串口解析）
│   ├── face_code.h      # 基础表情像素绘制
│   └── face_code2.h     # 扩展表情像素绘制（大尺寸动画版）
├── companion/           # PC 端 Python
│   ├── control_panel.py # 图形化控制面板（主程序）
│   ├── status_setter.py # Hook 辅助脚本
│   ├── test_serial.py   # 串口测试工具
│   └── requirements.txt # Python 依赖
├── platformio.ini       # PlatformIO 配置
├── LICENSE              # MIT 许可证
└── SUMMARY.md           # 开发笔记
```

## 许可

MIT — 详见 [LICENSE](LICENSE)
