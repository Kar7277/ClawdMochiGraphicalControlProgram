# Clawd Mochi — USB Mode Setup

让 ESP32 小屏幕根据 Claude Code 工作状态自动变表情。

## 1. 烧录固件

VSCode 里用 PlatformIO 编译上传到 ESP32-C3。
USB 插上后屏幕显示 "USB Mode Ready" 即可。

## 2. 安装 Python 依赖

```bash
pip install -r companion/requirements.txt
```

## 3. 启动守护脚本

```bash
# 自动检测 COM 口
python companion/claude_monitor.py

# 或指定端口
python companion/claude_monitor.py COM6

# 查看可用端口
python companion/claude_monitor.py --list
```

脚本会一直运行，把 Claude 的状态变化实时发到 ESP32。

## 4. 配置 Claude Code Hooks

在 Claude Code 的 settings.json 里加上 hooks（`~/.claude/settings.json`）：

```json
{
  "hooks": {
    "Stop": [
      {
        "command": "python \"C:\\desk\\project things\\ClawdMochi\\companion\\status_setter.py\" IDLE"
      }
    ],
    "UserPromptSubmit": [
      {
        "command": "python \"C:\\desk\\project things\\ClawdMochi\\companion\\status_setter.py\" THINK"
      }
    ],
    "PreToolUse": [
      {
        "matcher": "Bash",
        "command": "python \"C:\\desk\\project things\\ClawdMochi\\companion\\status_setter.py\" RUN"
      },
      {
        "matcher": "Edit|Write",
        "command": "python \"C:\\desk\\project things\\ClawdMochi\\companion\\status_setter.py\" CODE"
      }
    ],
    "PostToolUse": [
      {
        "matcher": "Bash|Edit|Write",
        "command": "python \"C:\\desk\\project things\\ClawdMochi\\companion\\status_setter.py\" THINK"
      }
    ]
  }
}
```

## 状态映射

| Claude 状态 | 触发时机 | 屏幕表情 |
|---|---|---|
| IDLE | Claude 回复完毕，等你输入 | 眼睛四处看 + 偶尔眨眼 |
| THINK | 你发了消息，Claude 开始思考 | 点点动画 (… 等等) |
| CODE | 正在编辑/写文件 | 笑眼动画 (笑笑) |
| RUN | 正在执行 Shell 命令 | 心跳动画 |
| ERROR | 出错了 | 生气表情 + 叉叉闪烁 |

## 工作原理

```
你发消息
  ↓ UserPromptSubmit hook
状态文件 → "THINK"
  ↓ Python 脚本检测到变化
COM 口 → "THINK\n"
  ↓ ESP32 收到
屏幕显示 → 点点动画循环

Claude 调用 Bash
  ↓ PreToolUse(Bash) hook
状态文件 → "RUN"
  ...
屏幕显示 → 心跳动画循环

Claude 回复完毕
  ↓ Stop hook
状态文件 → "IDLE"
  ...
屏幕显示 → 眼睛眨眼循环
```

## 自动恢复

- ESP32 15 秒没收到 PING → 自动回到 IDLE（防止死锁）
- Python 脚本串口断了 → 自动重连
- 重新插拔 USB → Python 脚本自动找回 COM 口
