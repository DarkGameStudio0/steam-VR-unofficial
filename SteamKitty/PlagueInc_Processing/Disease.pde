// ============================================================
// Disease  —  病原体模型 + 升级效果
// ============================================================

class Disease {
  // ---- 基础属性 ----
  float spreadBonus   = 0;   // 传播升级累加
  float lethality     = 0.5; // 基础致死率
  float heatResist    = 0;   // 耐热 0~2
  float coldResist    = 0;   // 耐寒 0~2
  float drugResist    = 0;   // 耐药 0~3
  float geneticArmor  = 0;   // 基因强化 0~3
  
  // ---- 症状属性 ----
  float symptomSpread = 0;   // 症状带来的传播加成
  float symptomLethal = 0;   // 症状带来的致死加成
  
  // ---- 升级追踪 ----
  int transLevels = 0;       // 总传播升级数
  
  // ---- 索引映射 ----
  // 升级列表索引范围:
  //   trans: 0-9
  //   symp:  10-19
  //   abil:  20-29
  
  GameState gs;
  
  Disease(GameState gs) {
    this.gs = gs;
  }
  
  // ============================================================
  // 检查购买条件
  // ============================================================
  boolean canBuy(Upgrade u) {
    if (u.bought) return false;
    
    // 传播类有前置条件
    if (u.id == 1 && !gs.upgrades.get(0).bought) return false;  // 水源 II → 水源 I
    if (u.id == 2 && !gs.upgrades.get(1).bought) return false;  // 水源 III → 水源 II
    if (u.id == 4 && !gs.upgrades.get(3).bought) return false;  // 空气 II → 空气 I
    if (u.id == 5 && !gs.upgrades.get(4).bought) return false;  // 空气 III → 空气 II
    if (u.id == 7 && !gs.upgrades.get(6).bought) return false;  // 血液 II → 血液 I
    
    // 症状类无前置（直接可买）
    // 能力类
    if (u.id == 21 && !gs.upgrades.get(20).bought) return false; // 耐寒 II → 耐寒 I
    if (u.id == 23 && !gs.upgrades.get(22).bought) return false; // 耐热 II → 耐热 I
    if (u.id == 25 && !gs.upgrades.get(24).bought) return false; // 耐药 II → 耐药 I
    if (u.id == 26 && !gs.upgrades.get(25).bought) return false; // 耐药 III → 耐药 II
    if (u.id == 28 && !gs.upgrades.get(27).bought) return false; // 基因 II → 基因 I
    if (u.id == 29 && !gs.upgrades.get(28).bought) return false; // 基因 III → 基因 II
    
    return true;
  }
  
  // ============================================================
  // 应用升级效果
  // ============================================================
  void apply(Upgrade u) {
    switch(u.id) {
      // ----- 传播 -----
      case 0:  spreadBonus += 0.15; transLevels++; break; // 水源 I
      case 1:  spreadBonus += 0.20; transLevels++; break; // 水源 II
      case 2:  spreadBonus += 0.30; transLevels++; break; // 水源 III
      case 3:  spreadBonus += 0.20; transLevels++; break; // 空气 I
      case 4:  spreadBonus += 0.30; transLevels++; break; // 空气 II
      case 5:  spreadBonus += 0.40; transLevels++; break; // 空气 III
      case 6:  spreadBonus += 0.15; transLevels++; break; // 血液 I
      case 7:  spreadBonus += 0.25; transLevels++; break; // 血液 II
      case 8:  spreadBonus += 0.20; transLevels++; break; // 牲畜
      case 9:  spreadBonus += 0.50; transLevels++; break; // 极端人畜
      
      // ----- 症状 -----
      case 10: symptomSpread += 0.10; break; // 咳嗽
      case 11: symptomSpread += 0.10; break; // 发热
      case 12: symptomSpread += 0.15; break; // 皮疹
      case 13: symptomSpread += 0.10; symptomLethal += 0.05; break; // 恶心
      case 14: symptomSpread += 0.30; break; // 肺炎
      case 15: symptomLethal += 0.15; break; // 癫痫
      case 16: symptomLethal += 0.25; break; // 瘫痪
      case 17: symptomLethal += 0.35; break; // 昏迷
      case 18: symptomLethal += 0.50; break; // 器官衰竭
      case 19: symptomLethal += 0.60; break; // 坏死
      
      // ----- 能力 -----
      case 20: coldResist = max(coldResist, 1); break;
      case 21: coldResist = max(coldResist, 2); break;
      case 22: heatResist = max(heatResist, 1); break;
      case 23: heatResist = max(heatResist, 2); break;
      case 24: drugResist = max(drugResist, 1); break;
      case 25: drugResist = max(drugResist, 2); break;
      case 26: drugResist = max(drugResist, 3); break;
      case 27: geneticArmor = max(geneticArmor, 1); break;
      case 28: geneticArmor = max(geneticArmor, 2); break;
      case 29: geneticArmor = max(geneticArmor, 3); break;
    }
  }
  
  // ============================================================
  // 症状严重度（影响治愈加速）
  // ============================================================
  float severity() {
    return (symptomSpread + symptomLethal) / 3.0;
  }
  
  // ============================================================
  // 耐药性/基因强化阻值（影响治愈减速）
  // ============================================================
  float resistance() {
    return (drugResist * 0.08 + geneticArmor * 0.06);
  }
}
