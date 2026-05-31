#include <stdio.h>
#include <math.h>

int main() {
    // 常数定义
    const float sqrt3 = sqrt(3.0);
    const float inv_sqrt3 = 1.0 / sqrt3;               // 归一化因子（用于模为√3的点）
    const float phi = (1.0 + sqrt(5.0)) / 2.0;         // 黄金比例 (1+√5)/2
    const float inv_phi = 1.0 / phi;                  // 1/φ
    const float sqrt_phi2p1 = sqrt(phi * phi + 1.0);  // √(φ²+1)
    const float inv_sqrt_phi2p1 = 1.0 / sqrt_phi2p1;  // 二十面体归一化因子

    // 1. 正四面体 (4个顶点)
    printf("Tetrahedron (4 vertices):\n");
    double tetra[4][3] = {
        { 1,  1,  1},
        { 1, -1, -1},
        {-1,  1, -1},
        {-1, -1,  1}
    };
    for (int i = 0; i < 4; ++i) {
        printf("  (%.6f, %.6f, %.6f)\n",
               tetra[i][0] * inv_sqrt3,
               tetra[i][1] * inv_sqrt3,
               tetra[i][2] * inv_sqrt3);
    }
    printf("\n");

    // 2. 正六面体 (立方体, 8个顶点)
    printf("Cube (8 vertices):\n");
    for (int ix = -1; ix <= 1; ix += 2) {
        for (int iy = -1; iy <= 1; iy += 2) {
            for (int iz = -1; iz <= 1; iz += 2) {
                printf("  (%.6f, %.6f, %.6f)\n",
                       ix * inv_sqrt3,
                       iy * inv_sqrt3,
                       iz * inv_sqrt3);
            }
        }
    }
    printf("\n");

    // 3. 正八面体 (6个顶点)
    printf("Octahedron (6 vertices):\n");
    double octa[6][3] = {
        { 1,  0,  0}, {-1,  0,  0},
        { 0,  1,  0}, { 0, -1,  0},
        { 0,  0,  1}, { 0,  0, -1}
    };
    for (int i = 0; i < 6; ++i) {
        printf("  (%.6f, %.6f, %.6f)\n",
               octa[i][0], octa[i][1], octa[i][2]);
    }
    printf("\n");

    // 4. 正十二面体 (20个顶点)
    printf("Dodecahedron (20 vertices):\n");
    // 第一部分: 8个顶点 (±1,±1,±1) / √3
    for (int ix = -1; ix <= 1; ix += 2) {
        for (int iy = -1; iy <= 1; iy += 2) {
            for (int iz = -1; iz <= 1; iz += 2) {
                printf("  (%.6f, %.6f, %.6f)\n",
                       ix * inv_sqrt3,
                       iy * inv_sqrt3,
                       iz * inv_sqrt3);
            }
        }
    }
    // 第二部分: 12个顶点 (0, ±1/φ, ±φ) 及其循环排列，再除以 √3
    // 排列1: (0, ±1/φ, ±φ)
    for (int s1 = -1; s1 <= 1; s1 += 2) {
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            double x = 0.0;
            double y = s1 * inv_phi;
            double z = s2 * phi;
            printf("  (%.6f, %.6f, %.6f)\n",
                   x * inv_sqrt3, y * inv_sqrt3, z * inv_sqrt3);
        }
    }
    // 排列2: (±1/φ, 0, ±φ)
    for (int s1 = -1; s1 <= 1; s1 += 2) {
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            double x = s1 * inv_phi;
            double y = 0.0;
            double z = s2 * phi;
            printf("  (%.6f, %.6f, %.6f)\n",
                   x * inv_sqrt3, y * inv_sqrt3, z * inv_sqrt3);
        }
    }
    // 排列3: (±φ, ±1/φ, 0)
    for (int s1 = -1; s1 <= 1; s1 += 2) {
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            double x = s1 * phi;
            double y = s2 * inv_phi;
            double z = 0.0;
            printf("  (%.6f, %.6f, %.6f)\n",
                   x * inv_sqrt3, y * inv_sqrt3, z * inv_sqrt3);
        }
    }
    printf("\n");

    // 5. 正二十面体 (12个顶点)
    printf("Icosahedron (12 vertices):\n");
    // 顶点形式 (0, ±1, ±φ) 及其循环排列，再除以 √(φ²+1)
    // 排列1: (0, ±1, ±φ)
    for (int s1 = -1; s1 <= 1; s1 += 2) {
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            double x = 0.0;
            double y = s1 * 1.0;
            double z = s2 * phi;
            printf("  (%.6f, %.6f, %.6f)\n",
                   x * inv_sqrt_phi2p1,
                   y * inv_sqrt_phi2p1,
                   z * inv_sqrt_phi2p1);
        }
    }
    // 排列2: (±1, 0, ±φ)
    for (int s1 = -1; s1 <= 1; s1 += 2) {
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            double x = s1 * 1.0;
            double y = 0.0;
            double z = s2 * phi;
            printf("  (%.6f, %.6f, %.6f)\n",
                   x * inv_sqrt_phi2p1,
                   y * inv_sqrt_phi2p1,
                   z * inv_sqrt_phi2p1);
        }
    }
    // 排列3: (±φ, ±1, 0)
    for (int s1 = -1; s1 <= 1; s1 += 2) {
        for (int s2 = -1; s2 <= 1; s2 += 2) {
            double x = s1 * phi;
            double y = s2 * 1.0;
            double z = 0.0;
            printf("  (%.6f, %.6f, %.6f)\n",
                   x * inv_sqrt_phi2p1,
                   y * inv_sqrt_phi2p1,
                   z * inv_sqrt_phi2p1);
        }
    }
    printf("\n");

    return 0;
}