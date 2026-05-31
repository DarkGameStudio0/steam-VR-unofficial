/**
 * Plague Inc.  —  Processing 自制复刻版
 * ==========================================
 * 感染全球 → 进化病原体 → 在治愈完成前消灭全人类
 *
 * 操作:
 *   鼠标悬停 → 查看国家信息
 *   点击右侧升级项 → 消耗 DNA 购买
 *   空格 → 暂停 / 继续
 *   R 键 → 重新开局
 */

final String VERSION = "1.0";

// ---------- 颜色 ----------
final color CLR_BG          = #0a0e1a;
final color CLR_PANEL       = #141828;
final color CLR_TEXT        = #c8d0e0;
final color CLR_DIM         = #5a6a8a;
final color CLR_ACCENT      = #e04040;
final color CLR_CURE        = #4aba70;
final color CLR_DNA         = #f0d060;
final color CLR_NODE_N      = #2a3a5c;
final color CLR_NODE_I      = #8a1a2a;
final color CLR_NODE_D      = #3a3a3a;

// ---------- 全局 ----------
GameState gs;
boolean paused = false;
PFont  fntUI, fntSmall, fntBig;

void setup() {
  size(1200, 720);
  fntUI    = createFont("Consolas", 14, true);
  fntSmall = createFont("Consolas", 11, true);
  fntBig   = createFont("Consolas", 20, true);
  textFont(fntUI);
  gs = new GameState();
  gs.init();
}

void draw() {
  background(CLR_BG);
  if (!paused) gs.tick();
  gs.render();

  // 暂停蒙层
  if (paused) {
    noStroke(); fill(0, 160); rect(0,0,width,height);
    fill(255); textAlign(CENTER, CENTER); textFont(fntBig);
    text("⏸ 暂停中", width/2, height/2);
    textFont(fntUI); textAlign(LEFT, CENTER);
  }
}

void keyPressed() {
  if (key == ' ')  paused = !paused;
  if (key == 'r' || key == 'R') { gs = new GameState(); gs.init(); }
}

void mousePressed() {
  if (mouseX > width - 270) gs.clickUpgrade(mouseX, mouseY);
  else                      gs.clickCountry(mouseX, mouseY);
}
