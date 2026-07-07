// ═══════════════════════════════════════════════
// Clawd Mochi 扩展表情 — 大尺寸动画版 v2
// 像素块: 6x6  网格: 40x40  画布: 240x240
// ═══════════════════════════════════════════════

// ── BLUSH (脸红) — 大竖眼 + 腮红(无嘴) ──────────────────────

void drawFace_blush1() {
  tft.fillScreen(animBgColor);
  // 大竖眼 (24x48)
  tft.fillRect(54, 48, 24, 48, C_BLACK);
  tft.fillRect(162, 48, 24, 48, C_BLACK);
  // 红色腮红
  tft.fillRect(18, 108, 36, 24, C_RED);
  tft.fillRect(186, 108, 36, 24, C_RED);
}

void drawFace_blush2() {
  tft.fillScreen(animBgColor);
  // 眨眼(眼睛变窄)
  tft.fillRect(54, 54, 24, 36, C_BLACK);
  tft.fillRect(162, 54, 24, 36, C_BLACK);
  // 腮红扩大
  tft.fillRect(12, 102, 42, 30, C_RED);
  tft.fillRect(186, 102, 42, 30, C_RED);
}

// ── AWE (敬畏/震惊) — 超大竖眼 + O嘴 ─────────────────────────

void drawFace_awe1() {
  tft.fillScreen(animBgColor);
  // 超大竖眼 (30x60)
  tft.fillRect(48, 42, 30, 60, C_BLACK);
  tft.fillRect(162, 42, 30, 60, C_BLACK);
  // O形嘴
  tft.fillRect(96, 156, 48, 6, C_BLACK);
  tft.fillRect(90, 162, 6, 6, C_BLACK);
  tft.fillRect(144, 162, 6, 6, C_BLACK);
  tft.fillRect(96, 168, 48, 6, C_BLACK);
}

void drawFace_awe2() {
  tft.fillScreen(animBgColor);
  // 眼睛稍宽
  tft.fillRect(42, 42, 36, 60, C_BLACK);
  tft.fillRect(162, 42, 36, 60, C_BLACK);
  // O形嘴更大
  tft.fillRect(90, 150, 60, 6, C_BLACK);
  tft.fillRect(84, 156, 6, 6, C_BLACK);
  tft.fillRect(150, 156, 6, 6, C_BLACK);
  tft.fillRect(84, 162, 6, 6, C_BLACK);
  tft.fillRect(150, 162, 6, 6, C_BLACK);
  tft.fillRect(90, 168, 60, 6, C_BLACK);
}

// ── WORRY (担忧) — 黑眉 + 放大方眼(无嘴) ────────────────────

void drawFace_worry1() {
  tft.fillScreen(animBgColor);
  // 大方眼 (24x24)
  tft.fillRect(54, 84, 24, 24, C_BLACK);
  tft.fillRect(162, 84, 24, 24, C_BLACK);
  // 左八字眉 (黑色)
  tft.fillRect(48, 66, 12, 6, C_BLACK);
  tft.fillRect(60, 60, 12, 6, C_BLACK);
  tft.fillRect(72, 66, 6, 6, C_BLACK);
  // 右八字眉 (黑色)
  tft.fillRect(162, 66, 6, 6, C_BLACK);
  tft.fillRect(168, 60, 12, 6, C_BLACK);
  tft.fillRect(180, 66, 12, 6, C_BLACK);
}

void drawFace_worry2() {
  tft.fillScreen(animBgColor);
  // 眯眼(矮一点)
  tft.fillRect(54, 90, 24, 18, C_BLACK);
  tft.fillRect(162, 90, 24, 18, C_BLACK);
  // 眉毛下压
  tft.fillRect(48, 72, 12, 6, C_BLACK);
  tft.fillRect(60, 66, 12, 6, C_BLACK);
  tft.fillRect(72, 72, 6, 6, C_BLACK);
  tft.fillRect(162, 72, 6, 6, C_BLACK);
  tft.fillRect(168, 66, 12, 6, C_BLACK);
  tft.fillRect(180, 72, 12, 6, C_BLACK);
}

// ── PROUD (骄傲) — 放大墨镜(反光=背景色) + 黑色W嘴 ──────────

void drawFace_proud1() {
  tft.fillScreen(animBgColor);
  // 墨镜顶梁(加宽)
  tft.fillRect(18, 42, 204, 6, C_BLACK);
  // 左右大镜片 (72x48)
  tft.fillRect(24, 48, 72, 48, C_BLACK);
  tft.fillRect(144, 48, 72, 48, C_BLACK);
  // 桥架
  tft.fillRect(96, 60, 48, 6, C_BLACK);
  // 反光用背景色 = 消失效果
  tft.fillRect(36, 54, 6, 24, animBgColor);
  tft.fillRect(156, 54, 6, 24, animBgColor);
  // W嘴(黑色)
  tft.fillRect(90, 144, 6, 6, C_BLACK);
  tft.fillRect(96, 150, 6, 6, C_BLACK);
  tft.fillRect(102, 156, 6, 6, C_BLACK);
  tft.fillRect(108, 150, 6, 6, C_BLACK);
  tft.fillRect(114, 144, 6, 6, C_BLACK);
  tft.fillRect(120, 150, 6, 6, C_BLACK);
  tft.fillRect(126, 156, 6, 6, C_BLACK);
  tft.fillRect(132, 150, 6, 6, C_BLACK);
  tft.fillRect(138, 144, 6, 6, C_BLACK);
}

void drawFace_proud2() {
  tft.fillScreen(animBgColor);
  // 墨镜(同样)
  tft.fillRect(18, 42, 204, 6, C_BLACK);
  tft.fillRect(24, 48, 72, 48, C_BLACK);
  tft.fillRect(144, 48, 72, 48, C_BLACK);
  tft.fillRect(96, 60, 48, 6, C_BLACK);
  // 反光移位(背景色)
  tft.fillRect(36, 72, 6, 24, animBgColor);
  tft.fillRect(156, 72, 6, 24, animBgColor);
  // W嘴下移
  tft.fillRect(90, 150, 6, 6, C_BLACK);
  tft.fillRect(96, 156, 6, 6, C_BLACK);
  tft.fillRect(102, 162, 6, 6, C_BLACK);
  tft.fillRect(108, 156, 6, 6, C_BLACK);
  tft.fillRect(114, 150, 6, 6, C_BLACK);
  tft.fillRect(120, 156, 6, 6, C_BLACK);
  tft.fillRect(126, 162, 6, 6, C_BLACK);
  tft.fillRect(132, 156, 6, 6, C_BLACK);
  tft.fillRect(138, 150, 6, 6, C_BLACK);
}

// ── HEART (爱心眼) — 标准心形(无嘴) ──────────────────────────

void drawFace_heart1() {
  tft.fillScreen(animBgColor);
  // 左心形 — 像素爱心
  tft.fillRect(54, 66, 12, 6, C_BLACK);    // 左圆弧顶
  tft.fillRect(78, 66, 12, 6, C_BLACK);    // 右圆弧顶
  tft.fillRect(42, 72, 60, 6, C_BLACK);    // 顶部横线
  tft.fillRect(36, 78, 66, 6, C_BLACK);    // 最宽
  tft.fillRect(42, 84, 54, 6, C_BLACK);    // 收窄
  tft.fillRect(48, 90, 42, 6, C_BLACK);    // 再窄
  tft.fillRect(54, 96, 30, 6, C_BLACK);    // 更窄
  tft.fillRect(60, 102, 18, 6, C_BLACK);   // 底部尖
  // 高光
  tft.fillRect(48, 78, 6, 6, C_WHITE);

  // 右心形 — 镜像
  tft.fillRect(150, 66, 12, 6, C_BLACK);
  tft.fillRect(174, 66, 12, 6, C_BLACK);
  tft.fillRect(138, 72, 60, 6, C_BLACK);
  tft.fillRect(138, 78, 66, 6, C_BLACK);
  tft.fillRect(144, 84, 54, 6, C_BLACK);
  tft.fillRect(150, 90, 42, 6, C_BLACK);
  tft.fillRect(156, 96, 30, 6, C_BLACK);
  tft.fillRect(162, 102, 18, 6, C_BLACK);
  // 高光
  tft.fillRect(150, 78, 6, 6, C_WHITE);
}

void drawFace_heart2() {
  tft.fillScreen(animBgColor);
  // 左心形脉冲(整体放大一圈)
  tft.fillRect(48, 60, 12, 6, C_BLACK);
  tft.fillRect(84, 60, 12, 6, C_BLACK);
  tft.fillRect(36, 66, 72, 6, C_BLACK);
  tft.fillRect(30, 72, 84, 6, C_BLACK);
  tft.fillRect(30, 78, 78, 6, C_BLACK);    // 最宽行
  tft.fillRect(36, 84, 66, 6, C_BLACK);
  tft.fillRect(42, 90, 54, 6, C_BLACK);
  tft.fillRect(48, 96, 42, 6, C_BLACK);
  tft.fillRect(54, 102, 30, 6, C_BLACK);
  tft.fillRect(60, 108, 18, 6, C_BLACK);
  // 高光
  tft.fillRect(42, 72, 6, 6, C_WHITE);

  // 右心形脉冲
  tft.fillRect(144, 60, 12, 6, C_BLACK);
  tft.fillRect(180, 60, 12, 6, C_BLACK);
  tft.fillRect(132, 66, 72, 6, C_BLACK);
  tft.fillRect(126, 72, 84, 6, C_BLACK);
  tft.fillRect(126, 78, 78, 6, C_BLACK);    // 最宽行
  tft.fillRect(138, 84, 66, 6, C_BLACK);
  tft.fillRect(144, 90, 54, 6, C_BLACK);
  tft.fillRect(150, 96, 42, 6, C_BLACK);
  tft.fillRect(156, 102, 30, 6, C_BLACK);
  tft.fillRect(162, 108, 18, 6, C_BLACK);
  // 高光
  tft.fillRect(138, 72, 6, 6, C_WHITE);
}

// ── SLEEP (睡眠) — 闭眼 + Zz气泡 + 口水泡 ──────────────────

void drawFace_sleep1() {
  tft.fillScreen(animBgColor);
  // 闭眼横线
  tft.fillRect(48, 84, 42, 6, C_BLACK);
  tft.fillRect(150, 84, 42, 6, C_BLACK);
  // 口水泡(小)
  tft.fillRect(60, 108, 18, 6, C_WHITE);
  tft.fillRect(54, 114, 30, 6, C_WHITE);
  tft.fillRect(54, 120, 30, 6, C_WHITE);
  tft.fillRect(60, 126, 18, 6, C_WHITE);
  // 大Z(红色)
  tft.fillRect(168, 102, 24, 6, C_RED);
  tft.fillRect(180, 108, 6, 6, C_RED);
  tft.fillRect(168, 114, 24, 6, C_RED);
  // 小z
  tft.fillRect(192, 126, 18, 6, C_RED);
  tft.fillRect(204, 132, 6, 6, C_RED);
  tft.fillRect(192, 138, 18, 6, C_RED);
}

void drawFace_sleep2() {
  tft.fillScreen(animBgColor);
  // 闭眼
  tft.fillRect(48, 84, 42, 6, C_BLACK);
  tft.fillRect(150, 84, 42, 6, C_BLACK);
  // 口水泡(胀大)
  tft.fillRect(54, 102, 30, 6, C_WHITE);
  tft.fillRect(48, 108, 42, 6, C_WHITE);
  tft.fillRect(48, 114, 42, 6, C_WHITE);
  tft.fillRect(48, 120, 42, 6, C_WHITE);
  tft.fillRect(54, 126, 30, 6, C_WHITE);
  // Zz上浮
  tft.fillRect(174, 90, 24, 6, C_RED);
  tft.fillRect(186, 96, 6, 6, C_RED);
  tft.fillRect(174, 102, 24, 6, C_RED);
  tft.fillRect(198, 114, 18, 6, C_RED);
  tft.fillRect(210, 120, 6, 6, C_RED);
  tft.fillRect(198, 126, 18, 6, C_RED);
}

// ── WINK (眨眼) — 交替眨眼 + 歪嘴 ──────────────────────────

void drawFace_wink1() {
  tft.fillScreen(animBgColor);
  // 左眼睁开(大竖眼)
  tft.fillRect(60, 54, 18, 42, C_BLACK);
  // 右眼眨眼(弧形闭合)
  tft.fillRect(150, 84, 6, 6, C_BLACK);
  tft.fillRect(156, 78, 6, 6, C_BLACK);
  tft.fillRect(162, 72, 18, 6, C_BLACK);
  tft.fillRect(180, 78, 6, 6, C_BLACK);
  tft.fillRect(186, 84, 6, 6, C_BLACK);
  // 歪嘴笑(偏左)
  tft.fillRect(84, 150, 48, 6, C_BLACK);
  tft.fillRect(90, 156, 36, 6, C_BLACK);
}

void drawFace_wink2() {
  tft.fillScreen(animBgColor);
  // 左眼眨眼
  tft.fillRect(54, 84, 6, 6, C_BLACK);
  tft.fillRect(60, 78, 6, 6, C_BLACK);
  tft.fillRect(66, 72, 18, 6, C_BLACK);
  tft.fillRect(84, 78, 6, 6, C_BLACK);
  tft.fillRect(90, 84, 6, 6, C_BLACK);
  // 右眼睁开
  tft.fillRect(162, 54, 18, 42, C_BLACK);
  // 歪嘴笑(偏右)
  tft.fillRect(108, 150, 48, 6, C_BLACK);
  tft.fillRect(114, 156, 36, 6, C_BLACK);
}

// ── THINKING (思考猫猫) — 放大版猫耳 + 大圆眼 + 括号脸 + 气泡 ─

void drawFace_thinking1() {
  tft.fillScreen(animBgColor);
  // 猫耳(加宽)
  tft.fillRect(48, 18, 6, 36, C_BLACK);     // 左耳外
  tft.fillRect(54, 12, 6, 6, C_BLACK);      // 左耳尖
  tft.fillRect(60, 18, 6, 30, C_BLACK);     // 左耳内
  tft.fillRect(162, 18, 6, 30, C_BLACK);    // 右耳内
  tft.fillRect(168, 12, 6, 6, C_BLACK);     // 右耳尖
  tft.fillRect(174, 18, 6, 36, C_BLACK);    // 右耳外
  // 头顶尖 ^
  tft.fillRect(108, 42, 24, 6, C_BLACK);
  tft.fillRect(102, 48, 6, 6, C_BLACK);
  tft.fillRect(132, 48, 6, 6, C_BLACK);
  // 大圆眼(36x36)
  tft.fillRect(42, 66, 36, 36, C_BLACK);
  tft.fillRect(162, 66, 36, 36, C_BLACK);
  // 眼内高光
  tft.fillRect(54, 72, 6, 6, C_WHITE);
  tft.fillRect(174, 72, 6, 6, C_WHITE);
  // 括号脸 (加粗)
  tft.fillRect(24, 78, 6, 30, C_BLACK);
  tft.fillRect(30, 78, 6, 30, C_BLACK);
  tft.fillRect(204, 78, 6, 30, C_BLACK);
  tft.fillRect(210, 78, 6, 30, C_BLACK);
  // W小嘴
  tft.fillRect(102, 120, 36, 6, C_BLACK);
  // 思考气泡(大,右上)
  tft.fillRect(168, 0, 24, 6, C_WHITE);
  tft.fillRect(162, 6, 36, 6, C_WHITE);
  tft.fillRect(156, 12, 48, 6, C_WHITE);
  tft.fillRect(156, 18, 48, 6, C_WHITE);
  tft.fillRect(162, 24, 36, 6, C_WHITE);
  // 小气泡
  tft.fillRect(180, 30, 12, 6, C_WHITE);
}

void drawFace_thinking2() {
  tft.fillScreen(animBgColor);
  // 猫耳(微动)
  tft.fillRect(48, 24, 6, 30, C_BLACK);
  tft.fillRect(54, 18, 6, 6, C_BLACK);
  tft.fillRect(60, 24, 6, 24, C_BLACK);
  tft.fillRect(162, 24, 6, 24, C_BLACK);
  tft.fillRect(168, 18, 6, 6, C_BLACK);
  tft.fillRect(174, 24, 6, 30, C_BLACK);
  // 头顶
  tft.fillRect(108, 48, 24, 6, C_BLACK);
  // 大圆眼(偏右看)
  tft.fillRect(48, 66, 36, 36, C_BLACK);
  tft.fillRect(168, 66, 36, 36, C_BLACK);
  // 高光偏移
  tft.fillRect(60, 72, 6, 6, C_WHITE);
  tft.fillRect(180, 72, 6, 6, C_WHITE);
  // 括号脸
  tft.fillRect(24, 78, 6, 30, C_BLACK);
  tft.fillRect(30, 78, 6, 30, C_BLACK);
  tft.fillRect(204, 78, 6, 30, C_BLACK);
  tft.fillRect(210, 78, 6, 30, C_BLACK);
  // W小嘴
  tft.fillRect(102, 120, 36, 6, C_BLACK);
  // 气泡缩小移位
  tft.fillRect(174, 6, 18, 6, C_WHITE);
  tft.fillRect(168, 12, 30, 6, C_WHITE);
  tft.fillRect(168, 18, 30, 6, C_WHITE);
  tft.fillRect(192, 24, 12, 6, C_WHITE);
}
