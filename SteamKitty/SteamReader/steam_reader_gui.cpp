#include <windows.h>
#include <wininet.h>
#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>
#include <sstream>
#include <commctrl.h>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "comctl32.lib")

using json = nlohmann::json;
using namespace std;

// Global variables
HINSTANCE hInst;
HWND hAppIdEdit, hSearchButton, hPopularButton, hInfoEdit, hStatusBar;
string currentGameData;

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void LoadGameInfo(const string& appId);
string HttpGet(const string& url);
string StripHtml(const string& html);

// Windows entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    hInst = hInstance;
    
    // Register window class
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(27, 40, 56));
    wc.lpszClassName = "SteamReaderClass";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    
    RegisterClassEx(&wc);
    
    // Create window
    HWND hWnd = CreateWindowEx(
        0, "SteamReaderClass", "Steam Game Reader",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 700,
        NULL, NULL, hInstance, NULL
    );
    
    if (!hWnd) return 0;
    
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return msg.wParam;
}

// Window Procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
        {
            // Set dark theme colors
            COLORREF bgColor = RGB(27, 40, 56);
            COLORREF panelColor = RGB(23, 33, 46);
            COLORREF buttonColor = RGB(102, 192, 244);
            COLORREF greenButton = RGB(76, 175, 80);
            
            // Title Label
            CreateWindow("STATIC", "Steam Game Reader", 
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                10, 10, 760, 40, hWnd, NULL, hInst, NULL);
            
            // App ID input
            CreateWindow("STATIC", "Enter App ID:", 
                WS_CHILD | WS_VISIBLE,
                20, 60, 80, 25, hWnd, NULL, hInst, NULL);
            
            hAppIdEdit = CreateWindow("EDIT", "730",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                100, 58, 150, 25, hWnd, (HMENU)1, hInst, NULL);
            
            // Search Button
            hSearchButton = CreateWindow("BUTTON", "Search Game",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                260, 56, 120, 30, hWnd, (HMENU)2, hInst, NULL);
            
            // Popular Games Button
            hPopularButton = CreateWindow("BUTTON", "Popular Games",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                390, 56, 130, 30, hWnd, (HMENU)3, hInst, NULL);
            
            // Info Display (Multiline Edit)
            hInfoEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | 
                ES_READONLY | ES_AUTOVSCROLL,
                10, 100, 760, 520, hWnd, (HMENU)4, hInst, NULL);
            
            // Status Bar
            hStatusBar = CreateWindow("STATIC", "Ready...",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                10, 630, 760, 25, hWnd, NULL, hInst, NULL);
            
            // Set fonts and colors
            HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas");
            
            SendMessage(hInfoEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            break;
        }
        
        case WM_CTLCOLORSTATIC:
        {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(255, 255, 255));
            SetBkColor(hdcStatic, RGB(27, 40, 56));
            return (LRESULT)CreateSolidBrush(RGB(27, 40, 56));
        }
        
        case WM_COMMAND:
        {
            switch (LOWORD(wParam)) {
                case 2: // Search Button
                {
                    char buffer[256];
                    GetWindowText(hAppIdEdit, buffer, 256);
                    string appId(buffer);
                    
                    if (!appId.empty()) {
                        SetWindowText(hStatusBar, "Loading game information...");
                        LoadGameInfo(appId);
                    } else {
                        MessageBox(hWnd, "Please enter a Steam App ID", "Error", 
                            MB_OK | MB_ICONWARNING);
                    }
                    break;
                }
                
                case 3: // Popular Games Button
                {
                    map<string, string> popularGames = {
                        {"730", "Counter-Strike 2"},
                        {"570", "Dota 2"},
                        {"1172470", "Apex Legends"},
                        {"271590", "Grand Theft Auto V"},
                        {"578080", "PUBG: BATTLEGROUNDS"},
                        {"1085660", "Destiny 2"},
                        {"252490", "Rust"},
                        {"440", "Team Fortress 2"},
                        {"292030", "The Witcher 3: Wild Hunt"},
                        {"1245620", "ELDEN RING"},
                        {"1091500", "Cyberpunk 2077"}
                    };
                    
                    stringstream ss;
                    ss << "POPULAR STEAM GAME IDs\r\n";
                    ss << "========================\r\n\r\n";
                    
                    for (const auto& game : popularGames) {
                        ss << game.first << " - " << game.second << "\r\n";
                    }
                    
                    ss << "\r\nEnter an App ID above and click Search";
                    
                    SetWindowText(hInfoEdit, ss.str().c_str());
                    SetWindowText(hStatusBar, "Showing popular games list");
                    break;
                }
            }
            break;
        }
        
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// HTTP GET request using WinINet
string HttpGet(const string& url) {
    HINTERNET hInternet = InternetOpenA("SteamReader/1.0", 
        INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    
    if (!hInternet) return "";
    
    HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, 
        INTERNET_FLAG_RELOAD, 0);
    
    if (!hUrl) {
        InternetCloseHandle(hInternet);
        return "";
    }
    
    string response;
    char buffer[4096];
    DWORD bytesRead;
    
    while (InternetReadFile(hUrl, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        response += buffer;
    }
    
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);
    return response;
}

// Strip HTML tags
string StripHtml(const string& html) {
    string result;
    bool inTag = false;
    for (char c : html) {
        if (c == '<') inTag = true;
        else if (c == '>') inTag = false;
        else if (!inTag) result += c;
    }
    return result;
}

// Load and display game info
void LoadGameInfo(const string& appId) {
    string url = "https://store.steampowered.com/api/appdetails?appids=" + appId;
    string response = HttpGet(url);
    
    if (response.empty()) {
        SetWindowText(hInfoEdit, "Failed to fetch data from Steam API");
        SetWindowText(hStatusBar, "Error: No response from API");
        return;
    }
    
    try {
        json data = json::parse(response);
        
        if (!data.contains(appId) || !data[appId]["success"].get<bool>()) {
            stringstream ss;
            ss << "Game not found for App ID: " << appId << "\r\n";
            ss << "Please check the ID and try again.";
            SetWindowText(hInfoEdit, ss.str().c_str());
            SetWindowText(hStatusBar, "Game not found");
            return;
        }
        
        json gameData = data[appId]["data"];
        stringstream ss;
        
        // Build formatted output
        ss << "========================================\r\n";
        ss << "  STEAM GAME INFORMATION\r\n";
        ss << "========================================\r\n\r\n";
        
        // Name
        if (gameData.contains("name")) {
            ss << "NAME: " << gameData["name"].get<string>() << "\r\n\r\n";
        }
        
        // Description
        if (gameData.contains("short_description")) {
            string desc = StripHtml(gameData["short_description"].get<string>());
            ss << "DESCRIPTION:\r\n" << desc << "\r\n\r\n";
        }
        
        // Developer & Publisher
        if (gameData.contains("developers") && gameData["developers"].size() > 0) {
            ss << "DEVELOPER: ";
            for (size_t i = 0; i < gameData["developers"].size(); i++) {
                if (i > 0) ss << ", ";
                ss << gameData["developers"][i].get<string>();
            }
            ss << "\r\n";
        }
        
        if (gameData.contains("publishers") && gameData["publishers"].size() > 0) {
            ss << "PUBLISHER: ";
            for (size_t i = 0; i < gameData["publishers"].size(); i++) {
                if (i > 0) ss << ", ";
                ss << gameData["publishers"][i].get<string>();
            }
            ss << "\r\n";
        }
        
        // Release Date
        if (gameData.contains("release_date")) {
            ss << "RELEASE DATE: " << gameData["release_date"]["date"].get<string>() << "\r\n";
        }
        
        // Price
        if (gameData.contains("is_free") && gameData["is_free"].get<bool>()) {
            ss << "PRICE: Free to Play\r\n";
        } else if (gameData.contains("price_overview")) {
            auto price = gameData["price_overview"];
            ss << "PRICE: " << price["final_formatted"].get<string>() << "\r\n";
            if (price.contains("discount_percent") && price["discount_percent"].get<int>() > 0) {
                ss << "DISCOUNT: " << price["discount_percent"].get<int>() << "% off\r\n";
            }
        }
        
        ss << "\r\n--- Game Details ---\r\n\r\n";
        
        // Genres
        if (gameData.contains("genres")) {
            ss << "GENRES: ";
            for (size_t i = 0; i < gameData["genres"].size(); i++) {
                if (i > 0) ss << ", ";
                ss << gameData["genres"][i]["description"].get<string>();
            }
            ss << "\r\n";
        }
        
        // Categories
        if (gameData.contains("categories")) {
            ss << "CATEGORIES: ";
            for (size_t i = 0; i < min(gameData["categories"].size(), (size_t)5); i++) {
                if (i > 0) ss << ", ";
                ss << gameData["categories"][i]["description"].get<string>();
            }
            ss << "\r\n";
        }
        
        // Platforms
        if (gameData.contains("platforms")) {
            ss << "PLATFORMS: ";
            vector<string> platforms;
            if (gameData["platforms"]["windows"].get<bool>()) platforms.push_back("Windows");
            if (gameData["platforms"]["mac"].get<bool>()) platforms.push_back("Mac");
            if (gameData["platforms"]["linux"].get<bool>()) platforms.push_back("Linux");
            
            for (size_t i = 0; i < platforms.size(); i++) {
                if (i > 0) ss << ", ";
                ss << platforms[i];
            }
            ss << "\r\n";
        }
        
        // Metacritic
        if (gameData.contains("metacritic")) {
            ss << "METACRITIC: " << gameData["metacritic"]["score"].get<int>() << "/100\r\n";
        }
        
        // Recommendations
        if (gameData.contains("recommendations")) {
            ss << "RECOMMENDATIONS: " << gameData["recommendations"]["total"].get<int>() << "\r\n";
        }
        
        // Additional Info
        if (gameData.contains("required_age") && gameData["required_age"].get<string>() != "0") {
            ss << "REQUIRED AGE: " << gameData["required_age"].get<string>() << "+\r\n";
        }
        
        if (gameData.contains("dlc")) {
            ss << "DLC COUNT: " << gameData["dlc"].size() << "\r\n";
        }
        
        ss << "\r\n========================================\r\n";
        
        SetWindowText(hInfoEdit, ss.str().c_str());
        SetWindowText(hStatusBar, "Game information loaded successfully");
        
    } catch (const exception& e) {
        stringstream ss;
        ss << "Error parsing game data: " << e.what();
        SetWindowText(hInfoEdit, ss.str().c_str());
        SetWindowText(hStatusBar, "Error loading data");
    }
}