// ============================================================
// GameState  —  主游戏状态机 + 更新逻辑
// ============================================================

class GameState {
  ArrayList<Country> countries;
  ArrayList<Upgrade> upgrades;
  Disease disease;
  float curePct = 0;           // 0~100
  int dna = 10;
  int totalPop, deadPop, infectedPop;
  int gameDay = 0;
  int dnaTimer = 0;
  String resultMsg = "";
  boolean gameOver = false;

  // ---- 地图布局 ----
  final int[][] EDGES = {
    {0,1},{0,12},{0,3},      // NA→SA, NA→Greenland, NA→西欧
    {1,5},                    // SA→非洲
    {2,12},{2,3},{2,13},     // 格陵兰→NA, 格陵兰→西欧, 格陵兰→北欧
    {3,13},{3,4},{3,6},      // 西欧→北欧, 西欧→东欧, 西欧→中东
    {13,4},{13,6},           // 北欧→东欧, 北欧→俄罗斯
    {4,5},{4,6},{4,7},       // 东欧→非洲, 东欧→中东, 东欧→俄罗斯
    {6,5},{6,7},{6,8},       // 中东→非洲, 中东→俄罗斯, 中东→中国
    {7,8},{7,9},             // 俄罗斯→中国, 俄罗斯→日本
    {8,9},{8,10},{8,11},     // 中国→日本, 中国→印度, 中国→东南亚
    {9,10},                   // 日本→印度 (sea)
    {10,11}                   // 印度→东南亚
  };

  // ---- 初始化 ----
  void init() {
    disease = new Disease(this);
    buildCountries();
    buildUpgrades();
    // 初始感染一个国家
    int startIdx = int(random(COUNTRIES));
    Country c = countries.get(startIdx);
    c.infected = c.population * random(0.01, 0.05);
    dna = 15;
    gameDay = 0;
    curePct = 0;
    gameOver = false;
    resultMsg = "";
  }

  void buildCountries() {
    countries = new ArrayList<Country>();
    // name, x, y, pop(M), wealth, climate, port, airport, island, connections
    countries.add(new Country("北美",    160, 140, 370, 0.8, 1, true, true, false));
    countries.add(new Country("南美",    200, 320, 220, 0.5, 0, true, true, false));
    countries.add(new Country("格陵兰",  240, 70,  5,   0.5, 2, true, true, true));
    countries.add(new Country("西欧",    360, 180, 290, 0.9, 1, true, true, false));
    countries.add(new Country("东欧",    440, 200, 220, 0.6, 1, true, true, false));
    countries.add(new Country("非洲",    380, 340, 650, 0.3, 0, true, true, false));
    countries.add(new Country("中东",    480, 270, 180, 0.5, 0, true, true, false));
    countries.add(new Country("俄罗斯",  540, 120, 144, 0.4, 2, true, true, false));
    countries.add(new Country("中国",    600, 240, 1400, 0.6, 1, true, true, false));
    countries.add(new Country("日本",    670, 200, 125, 0.7, 1, true, true, true));
    countries.add(new Country("印度",    640, 300, 1400, 0.5, 0, true, true, false));
    countries.add(new Country("东南亚",  660, 360, 380, 0.4, 0, true, true, false));
    countries.add(new Country("北欧",    340, 110, 28,  0.7, 2, true, true, false));
    countries.add(new Country("澳洲",    700, 420, 26,  0.7, 1, true, true, true));

    // 连接
    for (int[] e : EDGES) {
      Country a = countries.get(e[0]);
      Country b = countries.get(e[1]);
      a.neighbors.add(b);
      b.neighbors.add(a);
    }
    
    // 计算总人口
    totalPop = 0;
    for (Country c : countries) totalPop += c.population;
  }

  void buildUpgrades() {
    upgrades = new ArrayList<Upgrade>();
    int idx = 0;
    // 传播
    upgrades.add(new Upgrade(idx++, "水源传播 I",  2,  "trans", 0, "通过受污染水源传播"));
    upgrades.add(new Upgrade(idx++, "水源传播 II", 4,  "trans", 1, "水源传播效率提升"));
    upgrades.add(new Upgrade(idx++, "水源传播 III",7,  "trans", 2, "极大提升水源传播"));
    upgrades.add(new Upgrade(idx++, "空气传播 I",  3,  "trans", 0, "通过空气飞沫传播"));
    upgrades.add(new Upgrade(idx++, "空气传播 II", 5,  "trans", 1, "空气传播效率提升"));
    upgrades.add(new Upgrade(idx++, "空气传播 III",9,  "trans", 2, "极强空气传播能力"));
    upgrades.add(new Upgrade(idx++, "血液传播 I",  3,  "trans", 0, "通过血液/体液传播"));
    upgrades.add(new Upgrade(idx++, "血液传播 II", 6,  "trans", 1, "血液传播效率提升"));
    upgrades.add(new Upgrade(idx++, "牲畜传播",    4,  "trans", 0, "通过牲畜传播"));
    upgrades.add(new Upgrade(idx++, "极端人畜",    10, "trans", 0, "解锁全部动物传播路径"));
    // 症状
    upgrades.add(new Upgrade(idx++, "咳嗽",        2,  "symp", 0, "轻度传播加成"));
    upgrades.add(new Upgrade(idx++, "发热",        2,  "symp", 1, "轻度传播加成"));
    upgrades.add(new Upgrade(idx++, "皮疹",        3,  "symp", 2, "中度传播加成"));
    upgrades.add(new Upgrade(idx++, "恶心",        3,  "symp", 3, "中度传播+致死"));
    upgrades.add(new Upgrade(idx++, "肺炎",        5,  "symp", 4, "强力传播加成"));
    upgrades.add(new Upgrade(idx++, "癫痫",        5,  "symp", 5, "中度致死"));
    upgrades.add(new Upgrade(idx++, "瘫痪",        7,  "symp", 6, "重度致死"));
    upgrades.add(new Upgrade(idx++, "昏迷",        8,  "symp", 7, "极高致死"));
    upgrades.add(new Upgrade(idx++, "器官衰竭",    12, "symp", 8, "极端致死"));
    upgrades.add(new Upgrade(idx++, "坏死",        15, "symp", 9, "终极致死"));
    // 能力
    upgrades.add(new Upgrade(idx++, "耐寒 I",      3,  "abil", 0, "寒冷地区传播"));
    upgrades.add(new Upgrade(idx++, "耐寒 II",     5,  "abil", 1, "极寒地区传播"));
    upgrades.add(new Upgrade(idx++, "耐热 I",      3,  "abil", 0, "炎热地区传播"));
    upgrades.add(new Upgrade(idx++, "耐热 II",     5,  "abil", 1, "极热地区传播"));
    upgrades.add(new Upgrade(idx++, "耐药 I",      4,  "abil", 0, "减缓治愈研究"));
    upgrades.add(new Upgrade(idx++, "耐药 II",     7,  "abil", 1, "大幅减缓治愈"));
    upgrades.add(new Upgrade(idx++, "耐药 III",    11, "abil", 2, "极大减缓治愈"));
    upgrades.add(new Upgrade(idx++, "基因强化 I",  6,  "abil", 0, "减缓治愈研究"));
    upgrades.add(new Upgrade(idx++, "基因强化 II", 10, "abil", 1, "大幅减缓治愈"));
    upgrades.add(new Upgrade(idx++, "基因强化 III",15, "abil", 2, "极大阻碍治愈"));
  }

  // ============================================================
  // 每帧更新
  // ============================================================
  void tick() {
    if (gameOver) return;
    gameDay++;
    
    // 每 3 帧做一次更新（≈ 1 天 = 20 frames）
    if (frameCount % 4 != 0) return;
    
    // 1. 国内传播
    for (Country c : countries) {
      c.spreadInternal(disease);
    }
    
    // 2. 跨国传播
    for (Country c : countries) {
      if (c.infected > 100) {
        c.spreadExternal(disease, countries);
      }
    }
    
    // 3. 致死
    for (Country c : countries) {
      c.killPeople(disease);
    }
    
    // 4. 治愈研究
    updateCure();
    
    // 5. DNA 收入
    dnaTimer++;
    if (dnaTimer >= 15) { // 约每 15 tick = 1 DNA
      dnaTimer = 0;
      dna += 1;
      // 感染奖励
      int infectedCount = 0;
      for (Country c : countries) if (c.infected > 0) infectedCount++;
      dna += infectedCount / 3;
      // 致死奖励
      if (deadPop > 0) dna += max(1, deadPop / 10000000);
    }
    
    // 6. 统计数据
    updateStats();
    
    // 7. 胜 / 负判定
    checkGameOver();
  }

  void updateStats() {
    infectedPop = 0;
    deadPop = 0;
    for (Country c : countries) {
      infectedPop += (int)c.infected;
      deadPop += (int)c.dead;
    }
  }

  void updateCure() {
    // 基准速度
    float speed = 0.02;
    // 全球感染率加速
    float infectRate = (float)infectedPop / totalPop;
    speed += infectRate * 0.08;
    // 症状严重度加速
    float sev = disease.severity();
    speed += sev * 0.03;
    // 耐药/基因强化减速
    float resist = disease.resistance();
    speed *= (1.0 - resist);
    
    curePct = min(100, curePct + speed);
  }

  void checkGameOver() {
    if (curePct >= 100) {
      gameOver = true;
      // 检查是否还有活人
      int alive = totalPop - deadPop;
      if (alive <= 0) {
        resultMsg = "💀 全人类灭绝！你赢了！     按 R 重新开始";
      } else {
        resultMsg = "🧪 治愈完成！人类幸存 " + alive/1000000 + "M    按 R 重新开始";
      }
      return;
    }
    
    // 所有人都死了？
    if (deadPop >= totalPop - 1) {
      gameOver = true;
      resultMsg = "💀 全人类灭绝！你赢了！     按 R 重新开始";
    }
  }

  // ============================================================
  // 交互
  // ============================================================
  Country selected = null;

  void clickCountry(float mx, float my) {
    selected = null;
    for (Country c : countries) {
      if (dist(mx, my, c.x, c.y) < 22) {
        selected = c;
        break;
      }
    }
  }

  void clickUpgrade(float mx, float my) {
    if (gameOver) return;
    int sy = 80;
    for (Upgrade u : upgrades) {
      if (u.bought) { sy += 24; continue; }
      int bx = width - 260;
      // 检查点击区域
      if (mx >= bx && mx <= width - 10 && my >= sy && my <= sy + 20) {
        if (dna >= u.cost && disease.canBuy(u)) {
          dna -= u.cost;
          u.bought = true;
          disease.apply(u);
        }
        return;
      }
      sy += 24;
    }
  }

  // ============================================================
  // 渲染
  // ============================================================
  void render() {
    drawMap();
    drawPanel();
    drawBottomBar();
    if (gameOver) drawGameOver();
  }

  void drawMap() {
    // 连接线
    stroke(CLR_DIM, 60);
    strokeWeight(1.5);
    for (int[] e : EDGES) {
      Country a = countries.get(e[0]);
      Country b = countries.get(e[1]);
      color col = lerpColor(CLR_NODE_N, CLR_NODE_I, 
        max(a.infectPct(), b.infectPct()) * 0.7);
      stroke(col, 100);
      line(a.x, a.y, b.x, b.y);
    }

    // 国家节点
    for (Country c : countries) {
      float r = 12 + sqrt((float)c.population / 1400) * 14;
      color fillCol;
      float ip = c.infectPct();
      if (c.deadPct() > 0.3) fillCol = lerpColor(CLR_NODE_I, CLR_NODE_D, c.deadPct());
      else fillCol = lerpColor(CLR_NODE_N, CLR_NODE_I, ip);
      
      // 边框 - 选中高亮
      noStroke();
      if (c == selected) {
        fill(255, 80);
        ellipse(c.x, c.y, r+8, r+8);
      }
      
      fill(fillCol, 180);
      ellipse(c.x, c.y, r, r);
      
      // 内圈 - 感染比例
      if (ip > 0.01) {
        noFill();
        stroke(CLR_ACCENT, 60 + ip * 100);
        strokeWeight(2);
        arc(c.x, c.y, r-2, r-2, -PI/2, -PI/2 + ip * TWO_PI);
      }
      
      // 国家名
      fill(CLR_TEXT, 200);
      textAlign(CENTER, CENTER);
      textFont(fntSmall);
      text(c.name, c.x, c.y + r/2 + 10);
    }
    textAlign(LEFT, CENTER);
    textFont(fntUI);
  }

  void drawPanel() {
    int px = width - 270;
    noStroke();
    fill(CLR_PANEL, 200);
    rect(px, 0, 270, height);
    
    // 标题
    fill(CLR_DNA);
    textFont(fntUI);
    text("🧬 基因进化", px + 15, 28);
    
    // DNA 余额
    fill(CLR_DNA);
    text("DNA: " + dna, px + 15, 52);
    
    // 升级列表
    textFont(fntSmall);
    int sy = 80;
    for (Upgrade u : upgrades) {
      float bx = px + 8;
      float by = sy;
      float bw = 244;
      float bh = 20;
      
      if (u.bought) {
        fill(CLR_DIM, 40);
        noStroke();
        rect(bx, by, bw, bh, 3);
        fill(CLR_DIM);
        text("✓ " + u.name, bx + 6, by + 11);
      } else {
        boolean canBuy = dna >= u.cost && disease.canBuy(u);
        fill(canBuy ? CLR_PANEL : CLR_PANEL, 200);
        stroke(canBuy ? CLR_DNA : CLR_DIM, 80);
        strokeWeight(1);
        rect(bx, by, bw, bh, 3);
        
        fill(canBuy ? CLR_DNA : CLR_DIM);
        text(u.name, bx + 6, by + 11);
        text("$" + u.cost, bx + bw - 34, by + 11);
      }
      sy += 24;
    }
    textFont(fntUI);
  }

  void drawBottomBar() {
    int by = height - 44;
    noStroke();
    fill(CLR_PANEL, 220);
    rect(0, by, width - 270, 44);
    
    // 治愈条
    float barW = width - 300;
    fill(CLR_DIM, 60);
    rect(10, by + 14, barW, 16, 4);
    
    fill(lerpColor(#4a3a3a, CLR_CURE, curePct/100));
    rect(10, by + 14, barW * curePct / 100, 16, 4);
    
    fill(CLR_TEXT);
    textFont(fntUI);
    text("🧪 治愈研究: " + nf(curePct, 1, 1) + "%", 16, by + 23);
    
    // 统计
    float sx = barW + 30;
    fill(CLR_ACCENT);
    text("☠ " + nf(deadPop/1000000.0, 1, 1) + "M", sx, by + 23);
    sx += textWidth("☠ 999.9M ") + 10;
    fill(#6a9a5a);
    text("🧑 " + nf((totalPop-deadPop)/1000000.0, 1, 1) + "M", sx, by + 23);
    sx += textWidth("🧑 999.9M ") + 10;
    fill(CLR_DIM);
    text("Day " + gameDay, sx, by + 23);
  }

  void drawGameOver() {
    noStroke();
    fill(0, 180);
    rect(0, 0, width - 270, height);
    fill(255);
    textAlign(CENTER, CENTER);
    textFont(fntBig);
    text(resultMsg, (width - 270)/2, height/2);
    textAlign(LEFT, CENTER);
    textFont(fntUI);
  }
}
