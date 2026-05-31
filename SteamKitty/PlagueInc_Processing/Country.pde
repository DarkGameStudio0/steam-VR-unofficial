// ============================================================
// Country  —  国家 / 区域模型
// ============================================================

class Country {
  String name;
  float x, y;
  int population;           // 总人口
  float infected, dead;    // 当前感染 / 死亡数
  float wealth;            // 0~1  富裕度（影响治愈速度贡献）
  int   climate;           // 0=热带  1=温带  2=寒带
  boolean hasPort, hasAirport, isIsland;
  ArrayList<Country> neighbors;

  Country(String name, float x, float y, int pop, 
          float wealth, int climate,
          boolean port, boolean airport, boolean island) {
    this.name = name;
    this.x = x; this.y = y;
    this.population = pop;
    this.wealth = wealth;
    this.climate = climate;
    this.hasPort = port;
    this.hasAirport = airport;
    this.isIsland = island;
    neighbors = new ArrayList<Country>();
    infected = 0; dead = 0;
  }

  // ---- 健康人口 ----
  int healthy() { return max(0, population - (int)infected - (int)dead); }

  // ---- 感染比例 ----
  float infectPct() { return min(1, infected / (float)population); }
  float deadPct()   { return min(1, dead / (float)population); }

  // ============================================================
  // 国内传播
  // ============================================================
  void spreadInternal(Disease d) {
    if (infected <= 0) return;
    int healthyPop = healthy();
    if (healthyPop <= 0) return;

    // 基础传播率
    float rate = 0.006;
    // 人口密度加成（人口越多密度越高）
    rate += min(0.004, population / 500000.0 * 0.002);
    // 传播升级加成
    rate *= (1.0 + d.spreadBonus);
    // 症状传播加成
    rate *= (1.0 + d.symptomSpread);
    // 气候适应性
    rate *= climateFactor(d);
    // 港口/机场加成
    if (hasPort)    rate *= 1.2;
    if (hasAirport) rate *= 1.3;
    
    // 感染人数
    float newInfect = healthyPop * rate;
    // 随机扰动 ±20%
    newInfect *= random(0.8, 1.2);
    infected = min(population - dead, infected + newInfect);
  }

  // ============================================================
  // 跨国传播
  // ============================================================
  void spreadExternal(Disease d, ArrayList<Country> all) {
    // 只传播给相邻国家
    for (Country nb : neighbors) {
      if (nb.deadPct() > 0.5) continue; // 死人太多的国家不收
      
      float baseChance = 0.0004;
      // 感染比例越高越容易溢出
      baseChance *= infectPct() * 3;
      // 传播升级
      baseChance *= (1.0 + d.spreadBonus);
      // 港口/航线加成
      if (hasPort && nb.hasPort)   baseChance *= 2.0;
      if (hasAirport && nb.hasAirport) baseChance *= 2.5;
      // 岛屿免疫（除非对方有港口）
      if (nb.isIsland && !(hasPort && nb.hasPort)) baseChance *= 0.1;
      
      if (random(1) < baseChance) {
        float seed = infected * random(0.0001, 0.001);
        nb.infected = min(nb.population - nb.dead, nb.infected + max(10, seed));
      }
    }
    
    // 远距离传播（港口→港口 / 机场→机场）
    if (hasPort || hasAirport) {
      for (Country nb : all) {
        if (nb == this || nb.neighbors.contains(this)) continue;
        if (dist(x, y, nb.x, nb.y) < 200) continue; // 太近不算远距离
        if (nb.deadPct() > 0.5) continue;
        
        float farRate = 0.0001;
        if (hasPort && nb.hasPort) farRate = 0.0006;
        if (hasAirport && nb.hasAirport) farRate = 0.0008;
        farRate *= infectPct() * 3;
        farRate *= (1.0 + d.spreadBonus);
        
        if (random(1) < farRate) {
          nb.infected = min(nb.population - nb.dead, 
                           nb.infected + max(5, infected * random(0.00001, 0.0001)));
        }
      }
    }
  }

  // ============================================================
  // 致死
  // ============================================================
  void killPeople(Disease d) {
    if (infected <= 0) return;
    float lethal = d.lethality;
    // 症状致死加成
    lethal += d.symptomLethal;
    
    float killRate = 0.0005 * lethal;
    // 感染人口越多致死越快
    killRate *= min(3, infectPct() * 5);
    // 随机扰动
    killRate *= random(0.7, 1.3);
    
    float newDead = infected * killRate;
    newDead = min(newDead, infected * 0.05); // 单次最多杀 5%
    dead = min(population, dead + newDead);
    infected = max(0, infected - newDead * 0.6); // 死亡会减少感染数
  }

  // ============================================================
  // 气候适应系数
  // ============================================================
  float climateFactor(Disease d) {
    if (climate == 0) return 0.8 + d.heatResist * 0.4;   // 热带
    if (climate == 1) return 1.0;                         // 温带
    return 0.6 + d.coldResist * 0.6;                      // 寒带
  }
}
