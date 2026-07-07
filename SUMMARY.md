# Clawd Mochi — 项目开发总结

> 最后更新：2025-06-09  
> 状态：✅ 全部功能正常工作

## 项目简介

ESP32-C3 Super Mini (HW-466AB) + ST7789 1.54" 240×240 显示屏制作的 Claude Code 桌面伴侣。
通过 USB 连接电脑，根据 Claude Code 工作状态自动切换表情动画。

---

## 五个状态动画

| 状态 | 触发 | 动画效果 |
|------|------|---------|
| **IDLE** | 空闲 / 15s 无心跳 | 左右摆眼 + 眨眼 |
| **THINK** | 用户发消息 / Claude 思考 | 三点依次出现 |
| **CODE** | 编辑/写入文件 | 笑脸交替 |
| **RUN** | 执行 Shell 命令 | 心跳 💓 |
| **ERROR** | 出错 | 生气 + X 闪烁 |

---

## 串口通信方案

**通信路径：** ESP32-C3 内置 USB-JTAG CDC → COM 口 (VID:303A PID:1001)

**关键配置：**

```ini
build_flags =
    -DARDUINO_USB_MODE=1
    -DARDUINO_USB_CDC_ON_BOOT=1
```

```cpp
Serial.begin(115200);  // USB CDC，无需指定 GPIO
```

---

## 文件结构

```
ClawdMochi/
├── README.md                   ← 开源说明 + 快速开始
├── SUMMARY.md                  ← 本文件（开发笔记）
├── platformio.ini              ← USB_MODE=1 + USB_CDC_ON_BOOT=1
├── .gitignore
├── LICENSE                     ← MIT
├── .vscode/
│   └── extensions.json         ← 推荐 VSCode 插件
├── src/
│   ├── main.cpp                ← USB CDC 固件（动画 + 串口指令解析）
│   ├── face_code.h             ← 表情绘制函数（v1）
│   └── face_code2.h            ← 扩展表情绘制函数（v2，大尺寸动画版）
└── companion/
    ├── control_panel.py        ← 图形化控制面板（系统托盘 + GUI）
    ├── status_setter.py        ← Claude Code Hook 调用（写入状态文件）
    ├── test_serial.py          ← 串口连通性快速测试
    └── requirements.txt        ← Python 依赖
```

---

## Claude Code Hooks 配置

`~/.claude/settings.json`:

| Hook | → 状态 |
|------|--------|
| `Stop` | IDLE |
| `UserPromptSubmit` | THINK |
| `PreToolUse(Bash)` | RUN |
| `PreToolUse(Edit\|Write)` | CODE |
| `PostToolUse` | THINK |

---

## 图形化控制面板

替代原来的命令行守护进程 `claude_monitor.py`，提供：

- 🪟 **迷你控制窗口** — 5 个状态按钮 + 跟随/手动模式切换 + 串口连接状态灯
- 📋 **系统托盘** — 关闭缩到托盘，右键菜单切换表情，托盘图标颜色指示连接状态
- 🔄 **双模式** — 跟随模式（自动跟 Hook）/ 手动模式（按钮控制）
- 🔌 **自动检测 COM 口** — 断线自动重连，PING 心跳保活
- 🚀 **开机自启** — 启动文件夹 VBS 脚本

---

## 🔧 踩坑记录

| 问题 | 原因 | 解决 |
|------|------|------|
| COM 口不通 | 板子上没有独立 USB 转串口芯片 | 用内置 USB-JTAG CDC 端口 |
| CDC 超时 | Windows usbser.sys 不兼容 | `USB_MODE=1` + `USB_CDC_ON_BOOT=1` |
| python 命令无反应 | Windows 应用商店假 python | 用完整 Python 路径或 uv |
| 屏幕频闪/黑屏 | 调试用背光关断代码 | 删除 loop() 和 checkSerial() 里的背光调试 |
| ERROR 表情看不到 | Hook 竞态（PostToolUse 覆盖） | 延迟写入状态文件 |

---

## 🔮 后续可做

- [ ] 安装 Espressif 官方 USB 驱动替代 Windows 默认驱动
- [ ] WiFi 回退模式（UDP 通信，摆脱 USB 线）
- [ ] 屏幕黑屏问题进一步调试（SPI 通信）
