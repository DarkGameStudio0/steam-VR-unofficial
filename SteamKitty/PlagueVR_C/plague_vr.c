 /**
 * Plague Inc. VR  —  C 语言 + Raylib 3D 复刻
 * ========================================================
 * 一个 C 语言实现的 Plague Inc. 类游戏，带 3D 地球仪视角
 *
 * 操作：
 *   鼠标拖动  → 旋转地球
 *   滚轮      → 拉近/拉远
 *   左键点击  → 选中国家 / 购买升级
 *   空格      → 暂停
 *   R         → 重新开局
 *   ESC       → 退出
 *
 * 玩法：
 *   1. 初始感染一个随机国家
 *   2. 用 DNA 点数购买升级（传播/症状/能力）
 *   3. 感染全球，在治愈完成前灭绝全人类
 *
 * 编译（Windows MinGW + raylib 5.0）：
 *   1. 下载 raylib: https://github.com/raysan5/raylib/releases
 *      选择 raylib-5.0_win64_mingw-w64.zip
 *   2. 解压到 raylib/ 目录
 *   3. 编译：
 *      gcc plague_vr.c -o plague_vr.exe ^
 *          -Iraylib/include -Lraylib/lib ^
 *          -lraylib -lopengl32 -lgdi32 -lwinmm
 *
 * 编译（Linux）：
 *   gcc plague_vr.c -o plague_vr -lraylib -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* ============================================================
 * 数学宏
 * ============================================================ */
#define PI         3.141592653589793f
#define DEG2RAD    (PI / 180.0f)
#define MIN(a,b)   (((a) < (b)) ? (a) : (b))
#define MAX(a,b)   (((a) > (b)) ? (a) : (b))
#define CLAMP(v,l,h) MIN(MAX((v),(l)),(h))

#include "raylib.h"
#include "raymath.h"

/* ============================================================
 * 常量
 * ============================================================ */
#define COUNTRY_CNT  14
#define UPGRADE_CNT  30
#define GLOBE_R      5.0f
#define PANEL_W      270

/* ============================================================
 * 升级结构体
 * ============================================================ */
typedef struct {
    int id;
    char name[24];
    int cost;
    char cat[8];   /* "trans" "symp" "abil" */
    int lvl;
    int bought;
} Upgrade;

/* ============================================================
 * 病原体
 * ============================================================ */
typedef struct {
    float spreadBonus;
    float lethality;
    float heatResist, coldResist;
    float drugResist, geneticArmor;
    float sympSpread, sympLethal;
} Disease;

/* ============================================================
 * 国家
 * ============================================================ */
typedef struct {
    char name[12];
    Vector3 pos;
    int population;
    float infected, dead;
    float wealth;
    int climate;           /* 0=热带 1=温带 2=寒带 */
    int hasPort, hasAirport, isIsland;
    int nbr[COUNTRY_CNT];
    int nbrCnt;
} Country;

/* ============================================================
 * 游戏主状态
 * ============================================================ */
typedef struct {
    Country  cty[COUNTRY_CNT];
    Upgrade  ug[UPGRADE_CNT];
    Disease  dis;
    float    curePct;
    int      dna, gameDay;
    int      totalPop, deadPop, infectedPop;
    int      gameOver, paused;
    char     resultMsg[80];
    int      selIdx;
    /* 相机 */
    float    camYaw, camPitch, camDist;
} Game;

static Game g;

/* ============================================================
 * 3D 位置转换
 * ============================================================ */
static Vector3 latlon(float latDeg, float lonDeg) {
    float la = latDeg * DEG2RAD, lo = lonDeg * DEG2RAD;
    return (Vector3){
        GLOBE_R * cosf(la) * cosf(lo),
        GLOBE_R * sinf(la),
        GLOBE_R * cosf(la) * sinf(lo)
    };
}

/* ============================================================
 * 初始化
 * ============================================================ */
static void initGame(void) {
    srand((unsigned)time(NULL));
    g.curePct = 0; g.dna = 15; g.gameDay = 0;
    g.gameOver = 0; g.paused = 0; g.selIdx = -1;
    g.resultMsg[0] = '\0';
    
    /* disease */
    Disease *d = &g.dis;
    d->spreadBonus = 0; d->lethality = 0.5f;
    d->heatResist = d->coldResist = 0;
    d->drugResist = d->geneticArmor = 0;
    d->sympSpread = d->sympLethal = 0;
    
    /* countries: name, lat, lon, pop(M), wealth, climate, port, airport, island */
    struct { const char *n; float la, lo; int p; float w; int cl,po,ai,is; } raw[] = {
        {"北美",   40,-100, 370, 0.8,1, 1,1,0},
        {"南美",  -15, -60, 220, 0.5,0, 1,1,0},
        {"格陵兰", 72, -40,   5, 0.5,2, 1,1,1},
        {"西欧",   48,   3, 290, 0.9,1, 1,1,0},
        {"东欧",   52,  25, 220, 0.6,1, 1,1,0},
        {"非洲",    2,  20, 650, 0.3,0, 1,1,0},
        {"中东",   26,  45, 180, 0.5,0, 1,1,0},
        {"俄罗斯", 60,  60, 144, 0.4,2, 1,1,0},
        {"中国",   35, 105,1400, 0.6,1, 1,1,0},
        {"日本",   36, 138, 125, 0.7,1, 1,1,1},
        {"印度",   20,  78,1400, 0.5,0, 1,1,0},
        {"东南亚",  5, 105, 380, 0.4,0, 1,1,0},
        {"北欧",   60,  10,  28, 0.7,2, 1,1,0},
        {"澳洲",  -25, 135,  26, 0.7,1, 1,1,1},
    };
    
    int edges[][2] = {
        {0,1},{0,2},{0,3},{1,5},{2,3},{2,12},{3,4},{3,12},{3,13},
        {4,5},{4,6},{4,7},{5,6},{6,7},{6,8},{7,8},{7,9},{8,9},
        {8,10},{8,11},{9,10},{10,11},{12,2},{12,3},{13,3},{13,4}
    };
    int edgeCnt = sizeof(edges)/sizeof(edges[0]);
    
    for (int i = 0; i < COUNTRY_CNT; i++) {
        Country *c = &g.cty[i];
        strcpy(c->name, raw[i].n);
        c->pos = latlon(raw[i].la, raw[i].lo);
        c->population = raw[i].p * 1000000;
        c->wealth = raw[i].w; c->climate = raw[i].cl;
        c->hasPort = raw[i].po; c->hasAirport = raw[i].ai;
        c->isIsland = raw[i].is;
        c->infected = c->dead = 0;
        c->nbrCnt = 0;
        for (int j = 0; j < COUNTRY_CNT; j++) c->nbr[j] = -1;
    }
    for (int i = 0; i < edgeCnt; i++) {
        int a = edges[i][0], b = edges[i][1];
        Country *ca = &g.cty[a], *cb = &g.cty[b];
        int dup = 0;
        for (int j = 0; j < ca->nbrCnt; j++) if (ca->nbr[j] == b) { dup=1; break; }
        if (!dup) { ca->nbr[ca->nbrCnt++] = b; cb->nbr[cb->nbrCnt++] = a; }
    }
    
    int si = rand() % COUNTRY_CNT;
    g.cty[si].infected = g.cty[si].population * (0.01f + (rand()%5)*0.01f);
    
    g.totalPop = 0;
    for (int i = 0; i < COUNTRY_CNT; i++) g.totalPop += g.cty[i].population;
    
    /* upgrades */
    struct { int id; const char *n; int c; const char *cat; int l; } ugR[] = {
        {0, "水源传播 I",   2, "trans",0},{1, "水源传播 II",  4, "trans",1},
        {2, "水源传播 III", 7, "trans",2},{3, "空气传播 I",   3, "trans",0},
        {4, "空气传播 II",  5, "trans",1},{5, "空气传播 III", 9, "trans",2},
        {6, "血液传播 I",   3, "trans",0},{7, "血液传播 II",  6, "trans",1},
        {8, "牲畜传播",     4, "trans",0},{9, "极端人畜",    10, "trans",0},
        {10,"咳嗽",         2, "symp",0},{11,"发热",         2, "symp",1},
        {12,"皮疹",         3, "symp",2},{13,"恶心",         3, "symp",3},
        {14,"肺炎",         5, "symp",4},{15,"癫痫",         5, "symp",5},
        {16,"瘫痪",         7, "symp",6},{17,"昏迷",         8, "symp",7},
        {18,"器官衰竭",    12, "symp",8},{19,"坏死",        15, "symp",9},
        {20,"耐寒 I",       3, "abil",0},{21,"耐寒 II",      5, "abil",1},
        {22,"耐热 I",       3, "abil",0},{23,"耐热 II",      5, "abil",1},
        {24,"耐药 I",       4, "abil",0},{25,"耐药 II",      7, "abil",1},
        {26,"耐药 III",    11, "abil",2},{27,"基因强化 I",   6, "abil",0},
        {28,"基因强化 II", 10, "abil",1},{29,"基因强化 III",15, "abil",2},
    };
    for (int i = 0; i < UPGRADE_CNT; i++) {
        g.ug[i] = (Upgrade){ugR[i].id,"",ugR[i].c,"",ugR[i].l,0};
        strcpy(g.ug[i].name, ugR[i].n);
        strcpy(g.ug[i].cat, ugR[i].cat);
    }
    
    /* camera */
    g.camYaw = 0; g.camPitch = 20 * DEG2RAD; g.camDist = 18;
}

/* ============================================================
 * 前置条件检查
 * ============================================================ */
static int canBuy(Upgrade *u) {
    if (u->bought) return 0;
    switch (u->id) {
        case 1:  return g.ug[0].bought;
        case 2:  return g.ug[1].bought;
        case 4:  return g.ug[3].bought;
        case 5:  return g.ug[4].bought;
        case 7:  return g.ug[6].bought;
        case 21: return g.ug[20].bought;
        case 23: return g.ug[22].bought;
        case 25: return g.ug[24].bought;
        case 26: return g.ug[25].bought;
        case 28: return g.ug[27].bought;
        case 29: return g.ug[28].bought;
        default: return 1;
    }
}

/* ============================================================
 * 应用升级
 * ============================================================ */
static void applyUpgrade(Upgrade *u) {
    Disease *d = &g.dis;
    switch (u->id) {
        case 0:  d->spreadBonus+=0.15f; break;
        case 1:  d->spreadBonus+=0.20f; break;
        case 2:  d->spreadBonus+=0.30f; break;
        case 3:  d->spreadBonus+=0.20f; break;
        case 4:  d->spreadBonus+=0.30f; break;
        case 5:  d->spreadBonus+=0.40f; break;
        case 6:  d->spreadBonus+=0.15f; break;
        case 7:  d->spreadBonus+=0.25f; break;
        case 8:  d->spreadBonus+=0.20f; break;
        case 9:  d->spreadBonus+=0.50f; break;
        case 10: d->sympSpread +=0.10f; break;
        case 11: d->sympSpread +=0.10f; break;
        case 12: d->sympSpread +=0.15f; break;
        case 13: d->sympSpread+=0.10f; d->sympLethal+=0.05f; break;
        case 14: d->sympSpread +=0.30f; break;
        case 15: d->sympLethal+=0.15f; break;
        case 16: d->sympLethal+=0.25f; break;
        case 17: d->sympLethal+=0.35f; break;
        case 18: d->sympLethal+=0.50f; break;
        case 19: d->sympLethal+=0.60f; break;
        case 20: d->coldResist = MAX(d->coldResist,1); break;
        case 21: d->coldResist = MAX(d->coldResist,2); break;
        case 22: d->heatResist = MAX(d->heatResist,1); break;
        case 23: d->heatResist = MAX(d->heatResist,2); break;
        case 24: d->drugResist = MAX(d->drugResist,1); break;
        case 25: d->drugResist = MAX(d->drugResist,2); break;
        case 26: d->drugResist = MAX(d->drugResist,3); break;
        case 27: d->geneticArmor = MAX(d->geneticArmor,1); break;
        case 28: d->geneticArmor = MAX(d->geneticArmor,2); break;
        case 29: d->geneticArmor = MAX(d->geneticArmor,3); break;
    }
}

/* ============================================================
 * 气候系数
 * ============================================================ */
static float climateFactor(Country *c) {
    Disease *d = &g.dis;
    switch (c->climate) {
        case 0: return 0.8f + d->heatResist * 0.4f;
        case 2: return 0.6f + d->coldResist * 0.6f;
        default: return 1.0f;
    }
}

/* ============================================================
 * 国内传播
 * ============================================================ */
static void spreadInternal(Country *c) {
    if (c->infected <= 0) return;
    int healthy = c->population - (int)c->infected - (int)c->dead;
    if (healthy <= 0) return;
    
    float rate = 0.006f;
    rate += MIN(0.004f, c->population / 500000.0f * 0.002f);
    rate *= (1 + g.dis.spreadBonus) * (1 + g.dis.sympSpread);
    rate *= climateFactor(c);
    if (c->hasPort)    rate *= 1.2f;
    if (c->hasAirport) rate *= 1.3f;
    rate *= 0.8f + (rand()%41) * 0.01f;
    
    float ni = healthy * rate;
    c->infected = MIN(c->population - c->dead, c->infected + ni);
}

/* ============================================================
 * 跨国传播
 * ============================================================ */
static void spreadExternal(Country *c) {
    if (c->infected <= 100) return;
    Disease *d = &g.dis;
    float ip = c->infected / c->population;
    
    for (int i = 0; i < c->nbrCnt; i++) {
        if (c->nbr[i] < 0) continue;
        Country *nb = &g.cty[c->nbr[i]];
        if (nb->dead / nb->population > 0.5f) continue;
        
        float ch = 0.0004f * ip * 3 * (1 + d->spreadBonus);
        if (c->hasPort && nb->hasPort)   ch *= 2.0f;
        if (c->hasAirport && nb->hasAirport) ch *= 2.5f;
        if (nb->isIsland && !(c->hasPort && nb->hasPort)) ch *= 0.1f;
        
        if ((rand() / (float)RAND_MAX) < ch) {
            float seed = c->infected * (0.0001f + (rand()%10)*0.0001f);
            nb->infected = MIN(nb->population-nb->dead, nb->infected + MAX(10,seed));
        }
    }
    
    /* long-distance via ports/airports */
    if (c->hasPort || c->hasAirport) {
        for (int i = 0; i < COUNTRY_CNT; i++) {
            Country *nb = &g.cty[i];
            if (nb == c) continue;
            int isNbr = 0;
            for (int j = 0; j < c->nbrCnt; j++) if (c->nbr[j]==i) { isNbr=1; break; }
            if (isNbr) continue;
            if (Vector3Distance(c->pos, nb->pos) < 3.0f) continue;
            if (nb->dead / nb->population > 0.5f) continue;
            
            float fr = 0.0001f;
            if (c->hasPort && nb->hasPort)   fr = 0.0006f;
            if (c->hasAirport && nb->hasAirport) fr = 0.0008f;
            fr *= ip * 3 * (1 + d->spreadBonus);
            
            if ((rand()/(float)RAND_MAX) < fr) {
                float seed = c->infected * (0.00001f + (rand()%10)*0.00001f);
                nb->infected = MIN(nb->population-nb->dead, nb->infected + MAX(5,seed));
            }
        }
    }
}

/* ============================================================
 * 致死
 * ============================================================ */
static void killPeople(Country *c) {
    if (c->infected <= 0) return;
    float lethal = g.dis.lethality + g.dis.sympLethal;
    float kr = 0.0005f * lethal * MIN(3, c->infected/c->population * 5);
    kr *= 0.7f + (rand()%7) * 0.1f;
    float nd = MIN(c->infected * kr, c->infected * 0.05f);
    c->dead = MIN(c->population, c->dead + nd);
    c->infected = MAX(0, c->infected - nd * 0.6f);
}

/* ============================================================
 * 治愈 / 统计 / 胜负判定
 * ============================================================ */
static void updateStats(void) {
    g.infectedPop = g.deadPop = 0;
    for (int i = 0; i < COUNTRY_CNT; i++) {
        g.infectedPop += (int)g.cty[i].infected;
        g.deadPop     += (int)g.cty[i].dead;
    }
}
static void updateCure(void) {
    if (g.curePct >= 100) return;
    float sp = 0.02f + (g.infectedPop/(float)g.totalPop)*0.08f;
    sp += (g.dis.sympSpread + g.dis.sympLethal)/3.0f * 0.03f;
    sp *= 1.0f - (g.dis.drugResist*0.08f + g.dis.geneticArmor*0.06f);
    g.curePct = MIN(100, g.curePct + sp);
}
static void checkGameOver(void) {
    if (g.gameOver) return;
    if (g.curePct >= 100) {
        g.gameOver = 1;
        int alive = g.totalPop - g.deadPop;
        if (alive <= 0) sprintf(g.resultMsg,"灭绝！你赢了！   按 R 重新开局");
        else sprintf(g.resultMsg,"治愈完成！%dM 幸存" , alive/1000000);
        return;
    }
    if (g.deadPop >= g.totalPop - 1) {
        g.gameOver = 1;
        sprintf(g.resultMsg,"灭绝！你赢了！   按 R 重新开局");
    }
}

/* ============================================================
 * 游戏 tick
 * ============================================================ */
static int frameCnt = 0;
static void tickGame(void) {
    if (g.gameOver || g.paused) return;
    g.gameDay++;
    if (frameCnt % 4 != 0) return;  /* 每 4 帧更新一次 */
    
    for (int i = 0; i < COUNTRY_CNT; i++) spreadInternal(&g.cty[i]);
    for (int i = 0; i < COUNTRY_CNT; i++) spreadExternal(&g.cty[i]);
    for (int i = 0; i < COUNTRY_CNT; i++) killPeople(&g.cty[i]);
    
    updateCure();
    
    /* DNA */
    g.dna++;
    int ic = 0;
    for (int i = 0; i < COUNTRY_CNT; i++) if (g.cty[i].infected > 0) ic++;
    g.dna += ic / 3;
    if (g.deadPop > 0) g.dna += MAX(1, g.deadPop/10000000);
    
    updateStats();
    checkGameOver();
}

/* ============================================================
 * 3D 渲染 —— 地球
 * ============================================================ */
static void drawGlobe(void) {
    /* 大气辉光 */
    rlDisableDepthMask();
    DrawSphere((Vector3){0,0,0}, GLOBE_R*1.02f, (Color){30,50,100,15});
    rlEnableDepthMask();
    
    /* 半透明球体 */
    DrawSphere((Vector3){0,0,0}, GLOBE_R*0.98f, (Color){15,25,50,60});
    
    /* 经纬网 */
    for (int i = 0; i < 12; i++) {
        float lat = -PI/2 + (i+1)*PI/13;
        float r = GLOBE_R*cosf(lat)*1.005f, y = GLOBE_R*sinf(lat)*1.005f;
        DrawCircle3D((Vector3){0,y,0}, r, (Vector3){1,0,0}, 90, (Color){40,60,100,25});
    }
    for (int i = 0; i < 16; i++) {
        float lo = i * 2*PI/16;
        DrawCircle3D((Vector3){0,0,0}, GLOBE_R*1.005f,
                     (Vector3){cosf(lo),0,sinf(lo)}, 90, (Color){40,60,100,20});
    }
}

/* ============================================================
 * 3D 渲染 —— 连接线
 * ============================================================ */
static void drawConnections(void) {
    int edges[][2] = {
        {0,1},{0,2},{0,3},{1,5},{2,3},{2,12},{3,4},{3,12},{3,13},
        {4,5},{4,6},{4,7},{5,6},{6,7},{6,8},{7,8},{7,9},{8,9},
        {8,10},{8,11},{9,10},{10,11},{12,2},{12,3},{13,3},{13,4}
    };
    int ec = sizeof(edges)/sizeof(edges[0]);
    
    for (int e = 0; e < ec; e++) {
        Country *a = &g.cty[edges[e][0]], *b = &g.cty[edges[e][1]];
        float ip = MAX(a->infected/a->population, b->infected/b->population);
        unsigned char br = 80 + (unsigned char)(ip*120);
        unsigned char bg = 100 - (unsigned char)(ip*80);
        unsigned char bb = 160 - (unsigned char)(ip*120);
        unsigned char ba = 60 + (unsigned char)(ip*80);
        Color col = {br, bg, bb, ba};
        
        Vector3 mid = Vector3Scale(Vector3Normalize(Vector3Add(a->pos,b->pos)), GLOBE_R*1.3f);
        int seg = 12;
        for (int i = 0; i < seg; i++) {
            float t1 = i/(float)seg, t2 = (i+1)/(float)seg;
            float s1=1-t1, s2=1-t2;
            Vector3 p1 = {s1*s1*a->pos.x+2*s1*t1*mid.x+t1*t1*b->pos.x,
                          s1*s1*a->pos.y+2*s1*t1*mid.y+t1*t1*b->pos.y,
                          s1*s1*a->pos.z+2*s1*t1*mid.z+t1*t1*b->pos.z};
            Vector3 p2 = {s2*s2*a->pos.x+2*s2*t2*mid.x+t2*t2*b->pos.x,
                          s2*s2*a->pos.y+2*s2*t2*mid.y+t2*t2*b->pos.y,
                          s2*s2*a->pos.z+2*s2*t2*mid.z+t2*t2*b->pos.z};
            DrawLine3D(p1, p2, col);
        }
    }
}

/* ============================================================
 * 3D 渲染 —— 国家节点
 * ============================================================ */
static void drawCountryNodes(void) {
    for (int i = 0; i < COUNTRY_CNT; i++) {
        Country *c = &g.cty[i];
        float r = 0.3f + (c->population/1400000000.0f)*0.5f;
        float ip = c->infected / c->population;
        float dp = c->dead / c->population;
        
        /* 选中光环 */
        if (i == g.selIdx)
            DrawSphere(c->pos, r*2.2f, (Color){255,255,255,20});
        
        /* 感染弧线 */
        if (ip > 0.01f) {
            int seg = 20;
            int ds = (int)(seg * ip);
            if (ds > 0) {
                for (int j = 0; j < ds; j++) {
                    float a1 = (j/(float)seg)*2*PI;
                    float a2 = ((j+1)/(float)seg)*2*PI;
                    Vector3 p1 = {c->pos.x + r*1.2f*cosf(a1), c->pos.y + r*1.2f*sinf(a1), c->pos.z};
                    Vector3 p2 = {c->pos.x + r*1.2f*cosf(a2), c->pos.y + r*1.2f*sinf(a2), c->pos.z};
                    
                    /* 旋转到球面切面 */
                    Vector3 up = {0,1,0};
                    Vector3 axis = Vector3Normalize(Vector3CrossProduct(up, c->pos));
                    float ang = acosf(Vector3DotProduct(up, Vector3Normalize(c->pos)));
                    p1 = Vector3RotateByAxisAngle(p1, axis, ang);
                    p2 = Vector3RotateByAxisAngle(p2, axis, ang);
                    
                    Color rc = {200,40,40,(unsigned char)(80+ip*120)};
                    DrawLine3D(p1, p2, rc);
                }
            }
        }
        
        /* 主球 */
        Color col;
        if (dp > 0.3f) {
            float t = MIN(1,dp);
            col = (Color){(unsigned char)LERP(100,60,t),(unsigned char)LERP(120,60,t),
                          (unsigned char)LERP(160,60,t),200};
        } else {
            col = (Color){(unsigned char)LERP(100,200,ip*0.7f),
                          (unsigned char)LERP(120,60,ip*0.5f),
                          (unsigned char)LERP(160,60,ip*0.5f),200};
        }
        DrawSphere(c->pos, r, col);
        
        /* 标签（3D 文字近似——用线框小球标记） */
        Vector3 lblPos = {c->pos.x, c->pos.y + r + 0.3f, c->pos.z};
        /* 实际文字在 2D overlay 中投影，这里只画一个指示点 */
        DrawSphere(lblPos, 0.05f, (Color){255,255,255,100});
    }
}

/* ============================================================
 * 2D 叠加 —— 标签文字（国家名称投影到屏幕）
 * ============================================================ */
static void drawLabels(Camera3D cam) {
    for (int i = 0; i < COUNTRY_CNT; i++) {
        Country *c = &g.cty[i];
        float r = 0.3f + (c->population/1400000000.0f)*0.5f;
        Vector3 wp = {c->pos.x, c->pos.y + r + 0.5f, c->pos.z};
        Vector2 sp = GetWorldToScreen(wp, cam);
        if (sp.y > 0 && sp.y < GetScreenHeight() && sp.x > 0 && sp.x < GetScreenWidth()-PANEL_W) {
            float ip = c->infected / c->population;
            Color tc = {200,210,230,200};
            if (ip > 0.5f) { tc.r=230; tc.g=120; tc.b=80; }
            DrawText(c->name, (int)sp.x-20, (int)sp.y-8, 10, tc);
        }
    }
}

/* ============================================================
 * 2D 渲染 —— 右侧面板
 * ============================================================ */
static void drawPanel(void) {
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    int px = sw - PANEL_W;
    
    DrawRectangle(px, 0, PANEL_W, sh, (Color){20,24,40,220});
    DrawText("🧬 基因进化", px+12, 8, 18, (Color){240,208,96,255});
    
    char buf[32];
    sprintf(buf, "DNA: %d", g.dna);
    DrawText(buf, px+12, 32, 16, (Color){240,208,96,255});
    
    int sy = 64;
    for (int i = 0; i < UPGRADE_CNT; i++) {
        Upgrade *u = &g.ug[i];
        int bx = px + 6, by = sy, bw = PANEL_W - 12, bh = 20;
        
        if (u->bought) {
            DrawRectangle(bx, by, bw, bh, (Color){40,50,80,80});
            DrawText(TextFormat("✓ %s", u->name), bx+4, by+2, 11, (Color){90,106,138,255});
        } else {
            int ok = (g.dna >= u->cost && canBuy(u));
            Color bg = ok ? (Color){26,32,52,220} : (Color){20,24,40,180};
            Color bd = ok ? (Color){240,208,96,100} : (Color){60,70,90,80};
            DrawRectangle(bx, by, bw, bh, bg);
            DrawRectangleLines(bx, by, bw, bh, bd);
            Color tc = ok ? (Color){240,208,96,255} : (Color){100,110,130,200};
            DrawText(u->name, bx+4, by+2, 11, tc);
            sprintf(buf, "$%d", u->cost);
            DrawText(buf, bx+bw-32, by+2, 11, tc);
        }
        sy += 22;
        if (sy > sh - 50) break;
    }
}

/* ============================================================
 * 2D 渲染 —— 底部信息栏
 * ============================================================ */
static void drawBottomBar(void) {
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    int by = sh - 44;
    int bw = sw - PANEL_W - 20, bx = 10;
    
    DrawRectangle(0, by, sw - PANEL_W, 44, (Color){20,24,40,220});
    DrawRectangle(bx, by+12, bw, 20, (Color){50,60,80,100});
    
    Color cc = {74,186,112,255};
    DrawRectangle(bx, by+12, (int)(bw * g.curePct/100), 20, cc);
    
    DrawText(TextFormat("🧪 %5.1f%%", g.curePct), bx+4, by+13, 14, WHITE);
    
    float sx = (float)(bx + bw + 16);
    DrawText(TextFormat("☠ %.1fM", g.deadPop/1000000.f), (int)sx, by+13, 14,
             (Color){224,64,64,255});
    sx += MeasureText(TextFormat("☠ %.1fM", g.deadPop/1000000.f), 14) + 14;
    DrawText(TextFormat("🧑 %.1fM", (g.totalPop-g.deadPop)/1000000.f), (int)sx, by+13, 14,
             (Color){106,154,90,255});
    sx += MeasureText(TextFormat("🧑 %.1fM", (g.totalPop-g.deadPop)/1000000.f), 14) + 14;
    DrawText(TextFormat("Day %d", g.gameDay), (int)sx, by+13, 14,
             (Color){120,130,150,255});
}

/* ============================================================
 * 2D 渲染 —— 选中国家信息
 * ============================================================ */
static void drawSelectedInfo(void) {
    if (g.selIdx < 0 || g.selIdx >= COUNTRY_CNT) return;
    Country *c = &g.cty[g.selIdx];
    
    int ix = 20, iy = GetScreenHeight() - 164;
    DrawRectangle(ix, iy, 230, 120, (Color){20,24,40,220});
    DrawRectangleLines(ix, iy, 230, 120, (Color){60,70,100,150});
    
    DrawText(c->name, ix+8, iy+4, 18, WHITE);
    int h = c->population - (int)c->infected - (int)c->dead;
    
    DrawText(TextFormat("总人口: %dM", c->population/1000000), ix+8, iy+28, 12,
             (Color){180,190,210,255});
    DrawText(TextFormat("健康:   %dM", h/1000000), ix+8, iy+44, 12,
             (Color){106,154,90,255});
    DrawText(TextFormat("感染:   %dM", (int)c->infected/1000000), ix+8, iy+60, 12,
             (Color){224,64,64,255});
    DrawText(TextFormat("死亡:   %dM", (int)c->dead/1000000), ix+8, iy+76, 12,
             (Color){140,140,140,255});
    DrawText(TextFormat("感染率: %.1f%%", c->infected/c->population*100), ix+8, iy+94, 12,
             (Color){200,160,80,255});
}

/* ============================================================
 * 2D 渲染 —— 游戏结束
 * ============================================================ */
static void drawGameOver(void) {
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    DrawRectangle(0,0,sw-PANEL_W,sh, (Color){0,0,0,180});
    DrawText(g.resultMsg, 20, sh/2-20, 28, WHITE);
}

/* ============================================================
 * 3D 射线拾取国家
 * ============================================================ */
static Camera3D getCam(void) {
    Camera3D cam = {0};
    cam.up = (Vector3){0,1,0};
    cam.fovy = 60;
    cam.projection = CAMERA_PERSPECTIVE;
    cam.position = (Vector3){
        g.camDist * cosf(g.camPitch) * sinf(g.camYaw),
        g.camDist * sinf(g.camPitch),
        g.camDist * cosf(g.camPitch) * cosf(g.camYaw)
    };
    cam.target = (Vector3){0,0,0};
    return cam;
}

static void pickCountry(Vector2 mp) {
    Camera3D cam = getCam();
    Ray ray = GetMouseRay(mp, cam);
    
    g.selIdx = -1;
    float best = 1e9f;
    for (int i = 0; i < COUNTRY_CNT; i++) {
        Country *c = &g.cty[i];
        float r = 0.3f + (c->population/1400000000.0f)*0.5f;
        RayCollision col = GetRayCollisionSphere(ray, c->pos, r);
        if (col.hit && col.distance < best) {
            best = col.distance;
            g.selIdx = i;
        }
    }
}

/* ============================================================
 * 点击升级
 * ============================================================ */
static void handleUpgradeClick(int mx, int my) {
    if (g.gameOver) return;
    int px = GetScreenWidth() - PANEL_W;
    if (mx < px) return;
    
    int sy = 64;
    for (int i = 0; i < UPGRADE_CNT; i++) {
        Upgrade *u = &g.ug[i];
        if (u->bought) { sy += 22; continue; }
        int bx = px + 6;
        if (mx >= bx && mx <= px+PANEL_W-6 && my >= sy && my <= sy+20) {
            if (g.dna >= u->cost && canBuy(u)) {
                g.dna -= u->cost;
                u->bought = 1;
                applyUpgrade(u);
            }
            return;
        }
        sy += 22;
    }
}

/* ============================================================
 * 主函数
 * ============================================================ */
int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1200, 720, "Plague Inc. VR  —  C Language + Raylib");
    SetTargetFPS(60);
    
    initGame();
    
    while (!WindowShouldClose()) {
        frameCnt++;
        
        /* 相机控制 */
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 delta = GetMouseDelta();
            if (Vector2Length(delta) > 2.0f && GetMouseX() < GetScreenWidth()-PANEL_W) {
                g.camYaw   += delta.x * 0.005f;
                g.camPitch += delta.y * 0.005f;
                g.camPitch  = CLAMP(g.camPitch, -PI/2+0.1f, PI/2-0.1f);
            }
        }
        
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            g.camDist -= wheel * 1.5f;
            g.camDist = CLAMP(g.camDist, 6.0f, 40.0f);
        }
        
        /* 点击 */
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            int mx = GetMouseX(), my = GetMouseY();
            if (mx >= GetScreenWidth() - PANEL_W) {
                handleUpgradeClick(mx, my);
            } else {
                pickCountry((Vector2){(float)mx, (float)my});
            }
        }
        
        /* 键盘 */
        if (IsKeyPressed(KEY_SPACE)) g.paused = !g.paused;
        if (IsKeyPressed(KEY_R))     initGame();
        
        tickGame();
        
        /* ---- 渲染 ---- */
        Camera3D cam = getCam();
        
        BeginDrawing();
        ClearBackground((Color){10, 14, 26, 255});
        
        BeginMode3D(cam);
        drawGlobe();
        drawConnections();
        drawCountryNodes();
        EndMode3D();
        
        /* 2D 叠加层 */
        drawLabels(cam);
        drawPanel();
        drawBottomBar();
        drawSelectedInfo();
        
        if (g.gameOver) drawGameOver();
        
        if (g.paused) {
            DrawRectangle(0, 0, GetScreenWidth()-PANEL_W, GetScreenHeight(),
                          (Color){0,0,0,160});
            DrawText("⏸ 暂停中", GetScreenWidth()/2-100, GetScreenHeight()/2, 40, WHITE);
        }
        
        DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 11, (Color){100,110,130,200});
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
