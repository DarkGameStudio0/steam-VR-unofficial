#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <ctime>
using namespace std;
using namespace chrono;

void gotoSleep(int ms) { this_thread::sleep_for(milliseconds(ms)); }
void clear() { system("clear"); }

struct Steam {
    float x, y, life;
    Steam(float x, float y) : x(x), y(y), life(30 + rand()%20) {}
    void update() { y -= 0.4f; x += (rand()%100-50)/200.0f; life--; }
    bool alive() { return life > 0 && y > 0; }
};

class SteamCat {
    vector<Steam> particles;
    float x, y;
    int dir, frame, breath;
    string cat[1] = {
            "     /\\_/\\   \n"
            "     ( o.o )   \n"
            "      > ^ <    \n"
            "     /     \\  \n"
            "    (       )  \n"
            "     \\_____/  \n"
            "      U   U      ",
    };
public:
    SteamCat() : x(40), y(12), dir(1), frame(0), breath(0) { srand(time(0)); }
    
    void move() {
        if(rand()%100 < 3) dir = rand()%4;
        float nx = x, ny = y;
        dir==0?nx-=0.5f:dir==1?nx+=0.5f:dir==2?ny-=0.5f:ny+=0.5f;
        if(nx>5 && nx<65 && ny>3 && ny<20) { x=nx; y=ny; }
        else dir = rand()%4;
    }
    
    void update() {
        move();
        for(auto& p : particles) p.update();
        particles.erase(remove_if(particles.begin(), particles.end(),
            [](Steam& p){ return !p.alive(); }), particles.end());
        
        frame = (frame+1)%(dir<2?8:12);
        if(++breath>40) {
            for(int i=0;i<3;i++) particles.emplace_back(x+12+(dir==0?-3:dir==1?3:0), y+5);
            breath = -20;
        }
    }
    
    void draw() {
        clear();
        vector<string> grid(24, string(80, ' '));
        string c = cat[frame/4];
        int cx=x, cy=y;
        for(int i=0,row=0; i<c.size(); i++) {
            if(c[i]=='\n') row++;
            else if(c[i]!=' ') grid[cy+row][cx+(i%14)] = c[i];
        }
        for(auto& p : particles) {
            int px=p.x, py=p.y;
            if(px>0 && px<80 && py>0 && py<24) grid[py][px] = p.life>20?'~':'.';
        }
        for(int i=0;i<24;i++) cout << grid[i] << '\n';
        cout << "\n🐱 CRAWLING STEAM CAT | Dir:" << "←↓→↑"[dir] 
             << " | Steam:" << particles.size() << " 💨\n";
    }
    
    void run() {
        while(true) {
            update();
            draw();
            gotoSleep(1000/30);
        }
    }
};

int main() {
    cout << "\033[?25l";
    SteamCat().run();
    return 0;
}