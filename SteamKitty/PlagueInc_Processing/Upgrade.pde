// ============================================================
// Upgrade  —  单个升级项
// ============================================================

class Upgrade {
  int id;
  String name;
  int cost;
  String category;  // "trans" | "symp" | "abil"
  int level;        // 同一类中的等级
  String desc;
  boolean bought;
  
  Upgrade(int id, String name, int cost, String category, int level, String desc) {
    this.id = id;
    this.name = name;
    this.cost = cost;
    this.category = category;
    this.level = level;
    this.desc = desc;
    this.bought = false;
  }
}
