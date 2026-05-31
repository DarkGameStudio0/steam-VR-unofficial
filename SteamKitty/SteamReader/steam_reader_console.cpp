#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <windows.h>
#include <wininet.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "wininet.lib")

using json = nlohmann::json;
using namespace std;

class SteamReader {
private:
    HINTERNET hInternet;
    HINTERNET hConnect;
    
    // Remove HTML tags from text
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
    
    // HTTP GET request
    string HttpGet(const string& url) {
        HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, 
                                          INTERNET_FLAG_RELOAD, 0);
        if (!hUrl) return "";
        
        string response;
        char buffer[4096];
        DWORD bytesRead;
        
        while (InternetReadFile(hUrl, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            response += buffer;
        }
        
        InternetCloseHandle(hUrl);
        return response;
    }
    
    // Format number with commas
    string FormatNumber(int number) {
        string numStr = to_string(number);
        int insertPosition = numStr.length() - 3;
        while (insertPosition > 0) {
            numStr.insert(insertPosition, ",");
            insertPosition -= 3;
        }
        return numStr;
    }

public:
    SteamReader() {
        hInternet = InternetOpenA("SteamReader/1.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    }
    
    ~SteamReader() {
        if (hInternet) InternetCloseHandle(hInternet);
    }
    
    // Get game details from Steam API
    bool GetGameDetails(const string& appId) {
        string url = "https://store.steampowered.com/api/appdetails?appids=" + appId;
        string response = HttpGet(url);
        
        if (response.empty()) {
            cout << "Failed to fetch data from Steam API" << endl;
            return false;
        }
        
        try {
            json data = json::parse(response);
            
            if (data.contains(appId) && data[appId]["success"].get<bool>()) {
                json gameData = data[appId]["data"];
                DisplayGameInfo(gameData);
                return true;
            } else {
                cout << "Game not found or no data available for App ID: " << appId << endl;
                return false;
            }
        } catch (const exception& e) {
            cout << "Error parsing JSON: " << e.what() << endl;
            return false;
        }
    }
    
    // Display formatted game information
    void DisplayGameInfo(const json& gameData) {
        system("cls");
        cout << "\n" << string(60, '=') << endl;
        cout << "  STEAM GAME READER" << endl;
        cout << string(60, '=') << "\n" << endl;
        
        // Game Name
        if (gameData.contains("name")) {
            cout << "  Name: " << gameData["name"].get<string>() << endl;
        }
        
        // Type
        if (gameData.contains("type")) {
            cout << "  Type: " << gameData["type"].get<string>() << endl;
        }
        
        cout << "\n" << string(60, '-') << endl;
        
        // Description
        if (gameData.contains("short_description")) {
            string desc = StripHtml(gameData["short_description"].get<string>());
            cout << "  Description: " << desc << endl;
        }
        
        if (gameData.contains("detailed_description")) {
            string desc = StripHtml(gameData["detailed_description"].get<string>());
            if (desc.length() > 200) {
                desc = desc.substr(0, 200) + "...";
            }
            cout << "  Detailed: " << desc << endl;
        }
        
        cout << "\n" << string(60, '-') << endl;
        
        // Developers
        if (gameData.contains("developers") && gameData["developers"].size() > 0) {
            cout << "  Developer(s): ";
            for (size_t i = 0; i < gameData["developers"].size(); i++) {
                if (i > 0) cout << ", ";
                cout << gameData["developers"][i].get<string>();
            }
            cout << endl;
        }
        
        // Publishers
        if (gameData.contains("publishers") && gameData["publishers"].size() > 0) {
            cout << "  Publisher(s): ";
            for (size_t i = 0; i < gameData["publishers"].size(); i++) {
                if (i > 0) cout << ", ";
                cout << gameData["publishers"][i].get<string>();
            }
            cout << endl;
        }
        
        // Release Date
        if (gameData.contains("release_date")) {
            cout << "  Release Date: " << gameData["release_date"]["date"].get<string>() << endl;
            if (gameData["release_date"].contains("coming_soon") && 
                gameData["release_date"]["coming_soon"].get<bool>()) {
                cout << "  Status: Coming Soon" << endl;
            }
        }
        
        cout << "\n" << string(60, '-') << endl;
        
        // Price Information
        if (gameData.contains("is_free") && gameData["is_free"].get<bool>()) {
            cout << "  Price: FREE TO PLAY" << endl;
        } else if (gameData.contains("price_overview")) {
            auto price = gameData["price_overview"];
            cout << "  Price: " << price["final_formatted"].get<string>() << endl;
            
            if (price.contains("discount_percent") && price["discount_percent"].get<int>() > 0) {
                cout << "  Original Price: " << price["initial_formatted"].get<string>() << endl;
                cout << "  Discount: " << price["discount_percent"].get<int>() << "%" << endl;
            }
        } else {
            cout << "  Price: Not available" << endl;
        }
        
        cout << "\n" << string(60, '-') << endl;
        
        // Categories
        if (gameData.contains("categories") && gameData["categories"].size() > 0) {
            cout << "  Categories: ";
            for (size_t i = 0; i < gameData["categories"].size(); i++) {
                if (i > 0) cout << ", ";
                cout << gameData["categories"][i]["description"].get<string>();
            }
            cout << endl;
        }
        
        // Genres
        if (gameData.contains("genres") && gameData["genres"].size() > 0) {
            cout << "  Genres: ";
            for (size_t i = 0; i < gameData["genres"].size(); i++) {
                if (i > 0) cout << ", ";
                cout << gameData["genres"][i]["description"].get<string>();
            }
            cout << endl;
        }
        
        cout << "\n" << string(60, '-') << endl;
        
        // Platforms
        if (gameData.contains("platforms")) {
            cout << "  Supported Platforms: ";
            vector<string> platforms;
            auto platforms_data = gameData["platforms"];
            if (platforms_data["windows"].get<bool>()) platforms.push_back("Windows");
            if (platforms_data["mac"].get<bool>()) platforms.push_back("Mac");
            if (platforms_data["linux"].get<bool>()) platforms.push_back("Linux");
            
            for (size_t i = 0; i < platforms.size(); i++) {
                if (i > 0) cout << ", ";
                cout << platforms[i];
            }
            cout << endl;
        }
        
        // Metacritic Score
        if (gameData.contains("metacritic")) {
            cout << "  Metacritic Score: " << gameData["metacritic"]["score"].get<int>() << "/100" << endl;
            if (gameData["metacritic"].contains("url")) {
                cout << "  Metacritic URL: " << gameData["metacritic"]["url"].get<string>() << endl;
            }
        } else {
            cout << "  Metacritic Score: N/A" << endl;
        }
        
        // Recommendations
        if (gameData.contains("recommendations")) {
            int total = gameData["recommendations"]["total"].get<int>();
            cout << "  Recommendations: " << FormatNumber(total) << endl;
        }
        
        // Required Age
        if (gameData.contains("required_age")) {
            string age = gameData["required_age"].get<string>();
            if (age != "0") {
                cout << "  Required Age: " << age << "+" << endl;
            }
        }
        
        // Website
        if (gameData.contains("website") && !gameData["website"].is_null()) {
            cout << "  Website: " << gameData["website"].get<string>() << endl;
        }
        
        // Support Info
        if (gameData.contains("support_info")) {
            auto support = gameData["support_info"];
            if (support.contains("url") && !support["url"].is_null()) {
                cout << "  Support: " << support["url"].get<string>() << endl;
            }
        }
        
        // DLC
        if (gameData.contains("dlc") && gameData["dlc"].size() > 0) {
            cout << "\n" << string(60, '-') << endl;
            cout << "  DLC Available: " << gameData["dlc"].size() << " items" << endl;
        }
        
        // Screenshots count
        if (gameData.contains("screenshots") && gameData["screenshots"].size() > 0) {
            cout << "  Screenshots: " << gameData["screenshots"].size() << " available" << endl;
        }
        
        // Movies count
        if (gameData.contains("movies") && gameData["movies"].size() > 0) {
            cout << "  Videos: " << gameData["movies"].size() << " available" << endl;
        }
        
        cout << "\n" << string(60, '=') << endl;
    }
    
    // Display popular Steam game IDs
    void ShowPopularGames() {
        cout << "\n" << string(50, '=') << endl;
        cout << "  POPULAR STEAM GAME IDs" << endl;
        cout << string(50, '=') << "\n" << endl;
        
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
            {"1091500", "Cyberpunk 2077"},
            {"1938090", "Call of Duty: Modern Warfare III"},
            {"814380", "Sekiro: Shadows Die Twice"},
            {"582010", "Monster Hunter: World"},
            {"489830", "The Elder Scrolls V: Skyrim Special Edition"}
        };
        
        cout << left << setw(15) << "App ID" << "Game Name" << endl;
        cout << string(50, '-') << endl;
        
        for (const auto& game : popularGames) {
            cout << left << setw(15) << game.first << game.second << endl;
        }
        
        cout << string(50, '=') << endl;
    }
};

int main() {
    SteamReader reader;
    string appId;
    int choice;
    
    while (true) {
        cout << "\n" << string(40, '=') << endl;
        cout << "  STEAM READER MENU" << endl;
        cout << string(40, '=') << endl;
        cout << "  1. Search Game by App ID" << endl;
        cout << "  2. Show Popular Games" << endl;
        cout << "  3. Exit" << endl;
        cout << string(40, '-') << endl;
        cout << "  Enter choice: ";
        
        cin >> choice;
        cin.ignore();
        
        switch (choice) {
            case 1:
                cout << "\n  Enter Steam App ID: ";
                getline(cin, appId);
                if (!appId.empty()) {
                    reader.GetGameDetails(appId);
                }
                break;
                
            case 2:
                reader.ShowPopularGames();
                break;
                
            case 3:
                cout << "\n  Thank you for using Steam Reader!" << endl;
                return 0;
                
            default:
                cout << "\n  Invalid choice. Please try again." << endl;
        }
    }
    
    return 0;
}