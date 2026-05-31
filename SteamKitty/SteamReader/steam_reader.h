#ifndef STEAM_READER_H
#define STEAM_READER_H

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

class SteamGameInfo {
public:
    string name;
    string description;
    string developer;
    string publisher;
    string releaseDate;
    string price;
    vector<string> genres;
    vector<string> categories;
    vector<string> platforms;
    int metacriticScore;
    int recommendations;
    bool isFree;
    
    static SteamGameInfo FromJson(const json& data) {
        SteamGameInfo info;
        
        if (data.contains("name"))
            info.name = data["name"].get<string>();
            
        if (data.contains("short_description"))
            info.description = data["short_description"].get<string>();
            
        if (data.contains("developers") && data["developers"].size() > 0)
            info.developer = data["developers"][0].get<string>();
            
        if (data.contains("publishers") && data["publishers"].size() > 0)
            info.publisher = data["publishers"][0].get<string>();
            
        if (data.contains("release_date"))
            info.releaseDate = data["release_date"]["date"].get<string>();
            
        if (data.contains("is_free"))
            info.isFree = data["is_free"].get<bool>();
            
        if (data.contains("price_overview"))
            info.price = data["price_overview"]["final_formatted"].get<string>();
            
        if (data.contains("genres")) {
            for (const auto& genre : data["genres"]) {
                info.genres.push_back(genre["description"].get<string>());
            }
        }
        
        if (data.contains("categories")) {
            for (const auto& category : data["categories"]) {
                info.categories.push_back(category["description"].get<string>());
            }
        }
        
        if (data.contains("platforms")) {
            auto platforms = data["platforms"];
            if (platforms["windows"].get<bool>()) info.platforms.push_back("Windows");
            if (platforms["mac"].get<bool>()) info.platforms.push_back("Mac");
            if (platforms["linux"].get<bool>()) info.platforms.push_back("Linux");
        }
        
        if (data.contains("metacritic"))
            info.metacriticScore = data["metacritic"]["score"].get<int>();
            
        if (data.contains("recommendations"))
            info.recommendations = data["recommendations"]["total"].get<int>();
            
        return info;
    }
};

#endif // STEAM_READER_H