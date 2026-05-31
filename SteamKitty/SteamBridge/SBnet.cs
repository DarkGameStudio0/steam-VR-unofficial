// ============================================================================
// SteamWebCrawler.cs - Steam Platform Data Aggregation & Integration Module
// Part of Steam Bridge v3.0 Ecosystem
//
// A comprehensive C# implementation for crawling Steam platform data,
// integrating with the MetaVerse-BlockChain bridge system.
//
// Compile: csc /target:exe /out:SteamWebCrawler.exe SteamWebCrawler.cs /r:System.Net.Http.dll
// Run:     SteamWebCrawler.exe
//
// "Bridging Steam's digital economy to the decentralized future."
// ============================================================================

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Text.RegularExpressions;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.IO;
using System.IO.Compression;
using System.Collections.Concurrent;
using System.Diagnostics;
using System.Security.Cryptography;
using System.Web;

// ============================================================================
// SECTION 1: Core Data Models for Steam Platform
// ============================================================================

namespace SteamBridge.Crawler
{
    #region Data Models

    
    /// Represents a Steam application/game
    
    public class SteamApp
    {
        [JsonPropertyName("appid")]
        public uint AppId { get; set; }

        [JsonPropertyName("name")]
        public string Name { get; set; }

        public string Description { get; set; }
        public string Developer { get; set; }
        public string Publisher { get; set; }
        public decimal Price { get; set; }
        public string Currency { get; set; } = "USD";
        public List<string> Categories { get; set; } = new List<string>();
        public List<string> Genres { get; set; } = new List<string>();
        public Dictionary<string, int> Achievements { get; set; } = new Dictionary<string, int>();
        public int TotalAchievements { get; set; }
        public DateTime ReleaseDate { get; set; }
        public string HeaderImageUrl { get; set; }
        public uint PeakPlayers { get; set; }
        public uint CurrentPlayers { get; set; }
        public Dictionary<string, decimal> PriceHistory { get; set; } = new Dictionary<string, decimal>();
        public float Rating { get; set; }
        public uint TotalReviews { get; set; }
        public uint PositiveReviews { get; set; }
        public uint NegativeReviews { get; set; }
        public List<string> SupportedLanguages { get; set; } = new List<string>();
        public Dictionary<string, string> Requirements { get; set; } = new Dictionary<string, string>();
        public List<SteamDLC> DLCs { get; set; } = new List<SteamDLC>();
        public DateTime LastUpdated { get; set; } = DateTime.UtcNow;

        public override string ToString() => $"SteamApp[{AppId}] {Name} (${Price})";
    }

    
    /// Represents a Steam DLC (Downloadable Content)
    
    public class SteamDLC
    {
        [JsonPropertyName("id")]
        public uint Id { get; set; }

        public string Name { get; set; }
        public decimal Price { get; set; }
        public DateTime ReleaseDate { get; set; }
        public bool IsOwned { get; set; }

        public override string ToString() => $"DLC[{Id}] {Name} (${Price})";
    }

    
    /// Represents a Steam user profile
    
    public class SteamUser
    {
        [JsonPropertyName("steamid")]
        public string SteamId { get; set; }

        public string PersonaName { get; set; }
        public string ProfileUrl { get; set; }
        public string AvatarUrl { get; set; }
        public string RealName { get; set; }
        public string Location { get; set; }
        public DateTime MemberSince { get; set; }
        public uint SteamLevel { get; set; }
        public uint FriendCount { get; set; }
        public uint GameCount { get; set; }
        public List<SteamApp> OwnedGames { get; set; } = new List<SteamApp>();
        public Dictionary<uint, uint> Playtime { get; set; } = new Dictionary<uint, uint>();
        public Dictionary<uint, uint> AchievementsEarned { get; set; } = new Dictionary<uint, uint>();
        public List<SteamBadge> Badges { get; set; } = new List<SteamBadge>();
        public uint TotalPlaytimeHours { get; set; }
        public bool IsProfilePublic { get; set; }
        public DateTime LastCrawled { get; set; } = DateTime.UtcNow;

        public override string ToString() => $"SteamUser[{SteamId}] {PersonaName} (Level {SteamLevel})";
    }

    
    /// Represents a Steam inventory item
    
    public class SteamInventoryItem
    {
        public ulong Id { get; set; }
        public uint AppId { get; set; }
        public uint ContextId { get; set; }
        public string MarketHashName { get; set; }
        public string MarketName { get; set; }
        public string Type { get; set; }
        public string Rarity { get; set; }
        public decimal MarketPrice { get; set; }
        public bool IsTradable { get; set; }
        public bool IsMarketable { get; set; }
        public uint Quantity { get; set; }
        public string IconUrl { get; set; }
        public Dictionary<string, string> Attributes { get; set; } = new Dictionary<string, string>();
        public DateTime AcquiredAt { get; set; }

        public override string ToString() => $"InventoryItem[{Id}] {MarketName} ({Rarity}) - ${MarketPrice}";
    }

    
    /// Represents a Steam community market listing
    
    public class SteamMarketListing
    {
        public string ListingId { get; set; }
        public uint AppId { get; set; }
        public string MarketHashName { get; set; }
        public decimal Price { get; set; }
        public string Currency { get; set; } = "USD";
        public uint Quantity { get; set; }
        public DateTime ListedAt { get; set; }
        public string SellerSteamId { get; set; }
        public SteamInventoryItem Item { get; set; }

        public override string ToString() => $"MarketListing[{ListingId}] {MarketHashName} - ${Price}";
    }

    
    /// Represents a Steam trading card
    
    public class SteamTradingCard
    {
        public uint AppId { get; set; }
        public string Name { get; set; }
        public string Series { get; set; }
        public uint CardNumber { get; set; }
        public decimal MarketPrice { get; set; }
        public string IconUrl { get; set; }
        public bool IsFoil { get; set; }
        public uint BadgeLevel { get; set; }

        public override string ToString() => $"TradingCard[{AppId}] {Name} #{CardNumber}" + 
                                             (IsFoil ? " (Foil)" : "") + $" - ${MarketPrice}";
    }

    
    /// Represents a Steam badge
    
    public class SteamBadge
    {
        public uint BadgeId { get; set; }
        public uint AppId { get; set; }
        public string Name { get; set; }
        public uint Level { get; set; }
        public uint MaxLevel { get; set; }
        public uint XP { get; set; }
        public DateTime CompletedAt { get; set; }
        public string IconUrl { get; set; }

        public override string ToString() => $"Badge[{BadgeId}] {Name} - Level {Level}/{MaxLevel}";
    }

    
    /// Represents a Steam workshop item
    
    public class SteamWorkshopItem
    {
        public ulong PublishedFileId { get; set; }
        public uint AppId { get; set; }
        public string Title { get; set; }
        public string Description { get; set; }
        public string CreatorSteamId { get; set; }
        public DateTime CreatedAt { get; set; }
        public DateTime UpdatedAt { get; set; }
        public uint Subscriptions { get; set; }
        public uint Favorites { get; set; }
        public uint Views { get; set; }
        public List<string> Tags { get; set; } = new List<string>();
        public string PreviewUrl { get; set; }
        public string FileUrl { get; set; }
        public ulong FileSize { get; set; }

        public override string ToString() => $"WorkshopItem[{PublishedFileId}] {Title} ({Subscriptions} subs)";
    }

    
    /// Represents a Steam review
    
    public class SteamReview
    {
        public string ReviewId { get; set; }
        public uint AppId { get; set; }
        public string AuthorSteamId { get; set; }
        public string AuthorName { get; set; }
        public string Content { get; set; }
        public bool IsPositive { get; set; }
        public uint PlaytimeAtReview { get; set; }
        public DateTime CreatedAt { get; set; }
        public DateTime UpdatedAt { get; set; }
        public uint HelpfulVotes { get; set; }
        public uint FunnyVotes { get; set; }
        public bool IsEarlyAccess { get; set; }
        public bool IsPurchasedOnSteam { get; set; }

        public override string ToString() => $"Review[{ReviewId}] {(IsPositive ? "👍" : "👎")} by {AuthorName}";
    }

    
    /// Represents a Steam friend relationship
    
    public class SteamFriend
    {
        public string SteamId { get; set; }
        public string PersonaName { get; set; }
        public DateTime FriendsSince { get; set; }
        public string Relationship { get; set; } = "friend";
        public bool IsMutual { get; set; } = true;

        public override string ToString() => $"Friend[{SteamId}] {PersonaName}";
    }

    
    /// Crawling session statistics and metrics
    
    public class CrawlerStatistics
    {
        public DateTime StartTime { get; set; }
        public DateTime EndTime { get; set; }
        public uint RequestsMade { get; set; }
        public uint RequestsSuccessful { get; set; }
        public uint RequestsFailed { get; set; }
        public uint RequestsRateLimited { get; set; }
        public uint AppsCrawled { get; set; }
        public uint UsersCrawled { get; set; }
        public uint InventoryItemsFetched { get; set; }
        public uint MarketListingsFetched { get; set; }
        public ulong TotalBytesDownloaded { get; set; }
        public double AverageResponseTimeMs { get; set; }
        public List<string> Errors { get; set; } = new List<string>();

        public TimeSpan Duration => EndTime - StartTime;
        public double SuccessRate => RequestsMade > 0 
            ? (double)RequestsSuccessful / RequestsMade * 100.0 
            : 0.0;

        public override string ToString()
        {
            var sb = new StringBuilder();
            sb.AppendLine("═══════ Crawler Statistics ═══════");
            sb.AppendLine($"Duration: {Duration.TotalMinutes:F2} minutes");
            sb.AppendLine($"Requests: {RequestsMade} ({RequestsSuccessful} success, {RequestsFailed} failed, {RequestsRateLimited} rate-limited)");
            sb.AppendLine($"Success Rate: {SuccessRate:F1}%");
            sb.AppendLine($"Apps Crawled: {AppsCrawled}");
            sb.AppendLine($"Users Crawled: {UsersCrawled}");
            sb.AppendLine($"Inventory Items: {InventoryItemsFetched}");
            sb.AppendLine($"Market Listings: {MarketListingsFetched}");
            sb.AppendLine($"Data Downloaded: {FormatBytes(TotalBytesDownloaded)}");
            sb.AppendLine($"Avg Response Time: {AverageResponseTimeMs:F1}ms");
            sb.AppendLine($"Errors: {Errors.Count}");
            return sb.ToString();
        }

        private static string FormatBytes(ulong bytes)
        {
            string[] sizes = { "B", "KB", "MB", "GB", "TB" };
            double len = bytes;
            int order = 0;
            while (len >= 1024 && order < sizes.Length - 1)
            {
                order++;
                len /= 1024;
            }
            return $"{len:0.##} {sizes[order]}";
        }
    }

    #endregion

    // ============================================================================
    // SECTION 2: Rate Limiter & Request Manager
    // ============================================================================

    
    /// Token bucket rate limiter for respecting Steam API limits
    
    public class RateLimiter
    {
        private readonly double _maxTokens;
        private readonly double _refillRate; // tokens per second
        private double _currentTokens;
        private DateTime _lastRefill;
        private readonly object _lock = new object();

        
        /// Creates a new rate limiter
        
        /// <param name="maxRequests">Maximum burst requests</param>
        /// <param name="periodSeconds">Refill period in seconds</param>
        public RateLimiter(int maxRequests, double periodSeconds)
        {
            _maxTokens = maxRequests;
            _refillRate = maxRequests / periodSeconds;
            _currentTokens = maxRequests;
            _lastRefill = DateTime.UtcNow;
        }

        
        /// Wait until a token is available, then consume it
        
        public async Task WaitForTokenAsync(CancellationToken cancellationToken = default)
        {
            while (true)
            {
                cancellationToken.ThrowIfCancellationRequested();

                lock (_lock)
                {
                    RefillTokens();
                    if (_currentTokens >= 1)
                    {
                        _currentTokens -= 1;
                        return;
                    }
                }

                // Calculate wait time until next token
                double waitMs = (1.0 / _refillRate) * 1000.0;
                await Task.Delay((int)Math.Ceiling(waitMs), cancellationToken);
            }
        }

        
        /// Try to consume a token immediately
        
        public bool TryConsume()
        {
            lock (_lock)
            {
                RefillTokens();
                if (_currentTokens >= 1)
                {
                    _currentTokens -= 1;
                    return true;
                }
                return false;
            }
        }

        private void RefillTokens()
        {
            var now = DateTime.UtcNow;
            double elapsed = (now - _lastRefill).TotalSeconds;
            _currentTokens = Math.Min(_maxTokens, _currentTokens + elapsed * _refillRate);
            _lastRefill = now;
        }

        public double AvailableTokens
        {
            get
            {
                lock (_lock)
                {
                    RefillTokens();
                    return _currentTokens;
                }
            }
        }
    }

    // ============================================================================
    // SECTION 3: HTTP Client & Request Handler
    // ============================================================================

    
    /// Manages HTTP requests to Steam APIs with proper headers, cookies, and retry logic
    
    public class SteamHttpClient : IDisposable
    {
        private readonly HttpClient _httpClient;
        private readonly RateLimiter _rateLimiter;
        private readonly string _steamApiKey;
        private readonly CookieContainer _cookieContainer;
        private readonly ConcurrentDictionary<string, CacheEntry> _cache;
        private readonly CrawlerStatistics _statistics;
        private readonly TimeSpan _cacheDuration;
        private bool _disposed;

        private class CacheEntry
        {
            public string Response { get; set; }
            public DateTime CachedAt { get; set; }
        }

        public SteamHttpClient(string steamApiKey = "", 
                               int maxRequestsPerMinute = 200,
                               TimeSpan? cacheDuration = null)
        {
            _steamApiKey = steamApiKey;
            _cookieContainer = new CookieContainer();
            _cache = new ConcurrentDictionary<string, CacheEntry>();
            _statistics = new CrawlerStatistics();
            _cacheDuration = cacheDuration ?? TimeSpan.FromMinutes(5);
            _rateLimiter = new RateLimiter(maxRequestsPerMinute, 60.0);

            var handler = new HttpClientHandler
            {
                CookieContainer = _cookieContainer,
                AutomaticDecompression = DecompressionMethods.GZip | DecompressionMethods.Deflate,
                AllowAutoRedirect = true,
                MaxAutomaticRedirections = 5,
                UseCookies = true,
            };

            _httpClient = new HttpClient(handler)
            {
                Timeout = TimeSpan.FromSeconds(30),
                BaseAddress = new Uri("https://api.steampowered.com/")
            };

            // Set realistic browser headers
            _httpClient.DefaultRequestHeaders.Clear();
            _httpClient.DefaultRequestHeaders.Add("User-Agent", 
                "SteamBridge-Crawler/3.0 (Windows; .NET; en-US) " +
                "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
            _httpClient.DefaultRequestHeaders.Add("Accept", 
                "application/json, text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
            _httpClient.DefaultRequestHeaders.Add("Accept-Language", "en-US,en;q=0.9");
            _httpClient.DefaultRequestHeaders.Add("Accept-Encoding", "gzip, deflate");
            _httpClient.DefaultRequestHeaders.Add("Connection", "keep-alive");
        }

        
        /// Make a GET request to a Steam API endpoint with caching and retry logic
        
        public async Task<string> GetAsync(string url, bool useCache = true, 
                                           CancellationToken cancellationToken = default)
        {
            // Check cache first
            if (useCache && _cache.TryGetValue(url, out var cached))
            {
                if (DateTime.UtcNow - cached.CachedAt < _cacheDuration)
                {
                    return cached.Response;
                }
                _cache.TryRemove(url, out _);
            }

            int retryCount = 0;
            const int maxRetries = 3;

            while (retryCount < maxRetries)
            {
                try
                {
                    await _rateLimiter.WaitForTokenAsync(cancellationToken);
                    
                    var stopwatch = Stopwatch.StartNew();
                    Interlocked.Increment(ref Unsafe.As<uint, uint>(ref _statistics.RequestsMade));

                    using var response = await _httpClient.GetAsync(url, cancellationToken);
                    stopwatch.Stop();

                    // Update statistics
                    var currentStats = _statistics;
                    currentStats.AverageResponseTimeMs = 
                        (currentStats.AverageResponseTimeMs * (currentStats.RequestsMade - 1) + 
                         stopwatch.Elapsed.TotalMilliseconds) / currentStats.RequestsMade;

                    if (response.IsSuccessStatusCode)
                    {
                        Interlocked.Increment(ref Unsafe.As<uint, uint>(ref _statistics.RequestsSuccessful));
                        var content = await response.Content.ReadAsStringAsync();
                        
                        // Update cache
                        if (useCache)
                        {
                            _cache[url] = new CacheEntry 
                            { 
                                Response = content, 
                                CachedAt = DateTime.UtcNow 
                            };
                        }

                        // Track data downloaded
                        Interlocked.Add(ref Unsafe.As<ulong, ulong>(ref _statistics.TotalBytesDownloaded), 
                                       (ulong)content.Length);

                        return content;
                    }
                    else if ((int)response.StatusCode == 429) // Rate Limited
                    {
                        Interlocked.Increment(ref Unsafe.As<uint, uint>(ref _statistics.RequestsRateLimited));
                        var retryAfter = response.Headers.RetryAfter?.Delta ?? TimeSpan.FromSeconds(30);
                        await Task.Delay(retryAfter, cancellationToken);
                        retryCount++;
                    }
                    else if ((int)response.StatusCode >= 500) // Server Error
                    {
                        retryCount++;
                        await Task.Delay(TimeSpan.FromSeconds(Math.Pow(2, retryCount)), cancellationToken);
                    }
                    else
                    {
                        Interlocked.Increment(ref Unsafe.As<uint, uint>(ref _statistics.RequestsFailed));
                        _statistics.Errors.Add($"HTTP {(int)response.StatusCode}: {url}");
                        return null;
                    }
                }
                catch (TaskCanceledException)
                {
                    Interlocked.Increment(ref Unsafe.As<uint, uint>(ref _statistics.RequestsFailed));
                    _statistics.Errors.Add($"Timeout: {url}");
                    return null;
                }
                catch (HttpRequestException ex)
                {
                    Interlocked.Increment(ref Unsafe.As<uint, uint>(ref _statistics.RequestsFailed));
                    _statistics.Errors.Add($"Request Error: {ex.Message}");
                    retryCount++;
                    await Task.Delay(TimeSpan.FromSeconds(Math.Pow(2, retryCount)), cancellationToken);
                }
            }

            _statistics.Errors.Add($"Max retries exceeded: {url}");
            return null;
        }

        
        /// Make a POST request with JSON body
        
        public async Task<string> PostAsync(string url, string jsonBody, 
                                            CancellationToken cancellationToken = default)
        {
            await _rateLimiter.WaitForTokenAsync(cancellationToken);
            
            var content = new StringContent(jsonBody, Encoding.UTF8, "application/json");
            Interlocked.Increment(ref Unsafe.As<uint, uint>(ref _statistics.RequestsMade));

            var response = await _httpClient.PostAsync(url, content, cancellationToken);
            
            if (response.IsSuccessStatusCode)
            {
                Interlocked.Increment(ref Unsafe.As<uint, uint>(ref _statistics.RequestsSuccessful));
                return await response.Content.ReadAsStringAsync();
            }

            Interlocked.Increment(ref Unsafe.As<uint, uint>(ref _statistics.RequestsFailed));
            return null;
        }

        
        /// Clear the response cache
        
        public void ClearCache()
        {
            _cache.Clear();
        }

        public CrawlerStatistics Statistics => _statistics;
        public int CacheSize => _cache.Count;

        public void Dispose()
        {
            if (!_disposed)
            {
                _httpClient?.Dispose();
                _disposed = true;
            }
        }
    }

    // ============================================================================
    // SECTION 4: Main Steam Web Crawler
    // ============================================================================

    
    /// Main Steam web crawler that aggregates data from Steam platform
    /// Integrates with the Steam Bridge MetaVerse-BlockChain system
    
    public class SteamWebCrawler : IDisposable
    {
        private readonly SteamHttpClient _httpClient;
        private readonly string _steamApiKey;
        private readonly ConcurrentDictionary<uint, SteamApp> _appsCache;
        private readonly ConcurrentDictionary<string, SteamUser> _usersCache;
        private readonly ConcurrentDictionary<string, SteamInventoryItem> _inventoryCache;
        private readonly ConcurrentBag<SteamMarketListing> _marketListings;
        private readonly SemaphoreSlim _concurrencyLimiter;
        private readonly List<Func<Task>> _crawlTasks;
        private bool _disposed;
        private CancellationTokenSource _cts;

        // Steam API Endpoints
        private const string STEAM_API_BASE = "https://api.steampowered.com/";
        private const string STEAM_STORE_API = "https://store.steampowered.com/api/";
        private const string STEAM_COMMUNITY_API = "https://steamcommunity.com/";
        private const string STEAM_CDN = "https://cdn.cloudflare.steamstatic.com/";
        private const string STEAMSPY_API = "https://steamspy.com/api.php";

        public SteamWebCrawler(string steamApiKey = "", int maxConcurrency = 10,
                               int maxRequestsPerMinute = 200)
        {
            _steamApiKey = steamApiKey;
            _httpClient = new SteamHttpClient(steamApiKey, maxRequestsPerMinute);
            _appsCache = new ConcurrentDictionary<uint, SteamApp>();
            _usersCache = new ConcurrentDictionary<string, SteamUser>();
            _inventoryCache = new ConcurrentDictionary<string, SteamInventoryItem>();
            _marketListings = new ConcurrentBag<SteamMarketListing>();
            _concurrencyLimiter = new SemaphoreSlim(maxConcurrency);
            _crawlTasks = new List<Func<Task>>();
            _cts = new CancellationTokenSource();

            Console.WriteLine("[CRAWLER] Steam Web Crawler initialized");
            Console.WriteLine($"[CRAWLER] Max concurrency: {maxConcurrency}");
            Console.WriteLine($"[CRAWLER] Rate limit: {maxRequestsPerMinute} req/min");
        }

        
        /// Get list of all Steam apps from Steam API
        
        public async Task<List<SteamApp>> GetAppListAsync()
        {
            Console.WriteLine("[CRAWLER] Fetching Steam app list...");
            
            string url = $"{STEAM_API_BASE}ISteamApps/GetAppList/v2/?key={_steamApiKey}";
            var response = await _httpClient.GetAsync(url);
            
            if (response == null)
            {
                Console.WriteLine("[CRAWLER] Failed to fetch app list from Steam API, trying alternative...");
                // Fallback to SteamSpy
                url = $"{STEAMSPY_API}?request=all";
                response = await _httpClient.GetAsync(url);
            }

            if (response != null)
            {
                var apps = new List<SteamApp>();
                try
                {
                    using var doc = JsonDocument.Parse(response);
                    
                    if (doc.RootElement.TryGetProperty("applist", out var appList) &&
                        appList.TryGetProperty("apps", out var appsArray))
                    {
                        foreach (var app in appsArray.EnumerateArray())
                        {
                            if (app.TryGetProperty("appid", out var id) &&
                                app.TryGetProperty("name", out var name))
                            {
                                apps.Add(new SteamApp 
                                { 
                                    AppId = id.GetUInt32(), 
                                    Name = name.GetString() 
                                });
                            }
                        }
                    }
                }
                catch (JsonException ex)
                {
                    Console.WriteLine($"[CRAWLER] JSON parse error: {ex.Message}");
                }

                Console.WriteLine($"[CRAWLER] Found {apps.Count} apps on Steam");
                return apps;
            }

            return new List<SteamApp>();
        }

        
        /// Get detailed information about a specific Steam app
        
        public async Task<SteamApp> GetAppDetailsAsync(uint appId, 
                                                       CancellationToken cancellationToken = default)
        {
            // Check cache first
            if (_appsCache.TryGetValue(appId, out var cachedApp))
            {
                return cachedApp;
            }

            await _concurrencyLimiter.WaitAsync(cancellationToken);
            try
            {
                Console.WriteLine($"[CRAWLER] Fetching details for app {appId}...");

                // Get store details
                string storeUrl = $"{STEAM_STORE_API}appdetails?appids={appId}";
                var storeResponse = await _httpClient.GetAsync(storeUrl);

                // Get SteamSpy data for player counts
                string steamspyUrl = $"{STEAMSPY_API}?request=appdetails&appid={appId}";
                var steamspyResponse = await _httpClient.GetAsync(steamspyUrl);

                // Get achievement data
                string achievementsUrl = $"{STEAM_API_BASE}ISteamUserStats/GetGlobalAchievementPercentagesForApp/v2/" +
                                        $"?gameid={appId}&key={_steamApiKey}";
                var achievementsResponse = await _httpClient.GetAsync(achievementsUrl);

                // Parse store response
                var app = new SteamApp { AppId = appId };
                
                if (storeResponse != null)
                {
                    try
                    {
                        using var doc = JsonDocument.Parse(storeResponse);
                        if (doc.RootElement.TryGetProperty(appId.ToString(), out var appData) &&
                            appData.TryGetProperty("data", out var data))
                        {
                            app.Name = data.GetProperty("name").GetString();
                            app.Description = GetJsonPropertyString(data, "short_description");
                            app.Developer = GetJsonArrayFirstString(data, "developers");
                            app.Publisher = GetJsonArrayFirstString(data, "publishers");
                            
                            if (data.TryGetProperty("price_overview", out var price))
                            {
                                app.Price = GetJsonPropertyDecimal(price, "final") / 100m;
                                app.Currency = GetJsonPropertyString(price, "currency") ?? "USD";
                            }

                            if (data.TryGetProperty("categories", out var categories))
                            {
                                foreach (var cat in categories.EnumerateArray())
                                {
                                    app.Categories.Add(cat.GetProperty("description").GetString());
                                }
                            }

                            if (data.TryGetProperty("genres", out var genres))
                            {
                                foreach (var genre in genres.EnumerateArray())
                                {
                                    app.Genres.Add(genre.GetProperty("description").GetString());
                                }
                            }

                            if (data.TryGetProperty("release_date", out var releaseDate))
                            {
                                if (DateTime.TryParse(releaseDate.GetProperty("date").GetString(), 
                                                     out var parsedDate))
                                {
                                    app.ReleaseDate = parsedDate;
                                }
                            }

                            app.HeaderImageUrl = GetJsonPropertyString(data, "header_image");
                            app.SupportedLanguages = GetJsonArrayStrings(data, "supported_languages");
                            
                            if (data.TryGetProperty("pc_requirements", out var pcReq))
                            {
                                app.Requirements["pc_minimum"] = GetJsonPropertyString(pcReq, "minimum");
                            }

                            if (data.TryGetProperty("dlc", out var dlcs))
                            {
                                foreach (var dlc in dlcs.EnumerateArray())
                                {
                                    app.DLCs.Add(new SteamDLC { Id = dlc.GetUInt32() });
                                }
                            }
                        }
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"[CRAWLER] Error parsing store data for app {appId}: {ex.Message}");
                    }
                }

                // Parse SteamSpy response
                if (steamspyResponse != null)
                {
                    try
                    {
                        using var doc = JsonDocument.Parse(steamspyResponse);
                        var root = doc.RootElement;
                        
                        app.PositiveReviews = GetJsonPropertyUInt(root, "positive");
                        app.NegativeReviews = GetJsonPropertyUInt(root, "negative");
                        app.TotalReviews = app.PositiveReviews + app.NegativeReviews;
                        app.Rating = app.TotalReviews > 0 
                            ? (float)app.PositiveReviews / app.TotalReviews * 100f 
                            : 0f;
                        
                        app.PeakPlayers = GetJsonPropertyUInt(root, "ccu");
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"[CRAWLER] Error parsing SteamSpy data: {ex.Message}");
                    }
                }

                // Parse achievements
                if (achievementsResponse != null)
                {
                    try
                    {
                        using var doc = JsonDocument.Parse(achievementsResponse);
                        if (doc.RootElement.TryGetProperty("achievementpercentages", out var achList) &&
                            achList.TryGetProperty("achievements", out var achievements))
                        {
                            app.TotalAchievements = achievements.GetArrayLength();
                            foreach (var ach in achievements.EnumerateArray())
                            {
                                var name = ach.GetProperty("name").GetString();
                                var percent = (int)(ach.GetProperty("percent").GetDecimal() * 100);
                                app.Achievements[name] = percent;
                            }
                        }
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"[CRAWLER] Error parsing achievements: {ex.Message}");
                    }
                }

                app.LastUpdated = DateTime.UtcNow;
                _appsCache[appId] = app;
                Interlocked.Increment(ref Unsafe.As<uint, uint>(ref _httpClient.Statistics.AppsCrawled));
                
                Console.WriteLine($"[CRAWLER] Crawled app: {app.Name} (${app.Price}) - " +
                                 $"Rating: {app.Rating:F1}%");
                
                return app;
            }
            finally
            {
                _concurrencyLimiter.Release();
            }
        }

        
        /// Get Steam user profile information
        
        public async Task<SteamUser> GetUserProfileAsync(string steamId, 
                                                         CancellationToken cancellationToken = default)
        {
            if (_usersCache.TryGetValue(steamId, out var cachedUser))
            {
                return cachedUser;
            }

            await _concurrencyLimiter.WaitAsync(cancellationToken);
            try
            {
                Console.WriteLine($"[CRAWLER] Fetching profile for user {steamId}...");

                // Get player summary
                string summaryUrl = $"{STEAM_API_BASE}ISteamUser/GetPlayerSummaries/v2/" +
                                   $"?steamids={steamId}&key={_steamApiKey}";
                var summaryResponse = await _httpClient.GetAsync(summaryUrl);

                // Get owned games
                string gamesUrl = $"{STEAM_API_BASE}IPlayerService/GetOwnedGames/v1/" +
                                 $"?steamid={steamId}&key={_steamApiKey}&include_appinfo=1&include_played_free_games=1";
                var gamesResponse = await _httpClient.GetAsync(gamesUrl);

                // Get player level
                string levelUrl = $"{STEAM_API_BASE}IPlayerService/GetSteamLevel/v1/" +
                                 $"?steamid={steamId}&key={_steamApiKey}";
                var levelResponse = await _httpClient.GetAsync(levelUrl);

                var user = new SteamUser { SteamId = steamId };

                if (summaryResponse != null)
                {
                    try
                    {
                        using var doc = JsonDocument.Parse(summaryResponse);
                        if (doc.RootElement.TryGetProperty("response", out var response) &&
                            response.TryGetProperty("players", out var players) &&
                            players.GetArrayLength() > 0)
                        {
                            var player = players[0];
                            user.PersonaName = player.GetProperty("personaname").GetString();
                            user.ProfileUrl = player.GetProperty("profileurl").GetString();
                            user.AvatarUrl = player.GetProperty("avatarfull").GetString();
                            user.RealName = GetJsonPropertyString(player, "realname");
                            user.Location = GetJsonPropertyString(player, "loccountrycode");
                            
                            if (player.TryGetProperty("timecreated", out var timeCreated))
                            {
                                user.MemberSince = DateTimeOffset.FromUnixTimeSeconds(
                                    timeCreated.GetInt64()).DateTime;
                            }
                        }
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"[CRAWLER] Error parsing user summary: {ex.Message}");
                    }
                }

                // Parse games
                if (gamesResponse != null)
                {
                    try
                    {
                        using var doc = JsonDocument.Parse(gamesResponse);
                        if (doc.RootElement.TryGetProperty("response", out var response) &&
                            response.TryGetProperty("games", out var games))
                        {
                            user.GameCount = response.GetProperty("game_count").GetUInt32();
                            
                            foreach (var game in games.EnumerateArray())
                            {
                                var appId = game.GetProperty("appid").GetUInt32();
                                var playtime = game.GetProperty("playtime_forever").GetUInt32();
                                user.Playtime[appId] = playtime;
                                user.TotalPlaytimeHours += playtime / 60;
                            }
                        }
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"[CRAWLER] Error parsing user games: {ex.Message}");
                    }
                }

                // Parse level
                if (levelResponse != null)
                {
                    try
                    {
                        using var doc = JsonDocument.Parse(levelResponse);
                        if (doc.RootElement.TryGetProperty("response", out var response))
                        {
                            user.SteamLevel = response.GetProperty("player_level").GetUInt32();
                        }
                    }
                    catch { }
                }

                user.LastCrawled = DateTime.UtcNow;
                _usersCache[steamId] = user;
                Interlocked.Increment(ref Unsafe.As<uint, uint>(ref _httpClient.Statistics.UsersCrawled));
                
                Console.WriteLine($"[CRAWLER] Crawled user: {user.PersonaName} (Level {user.SteamLevel}, " +
                                 $"{user.GameCount} games)");
                
                return user;
            }
            finally
            {
                _concurrencyLimiter.Release();
            }
        }

        
        /// Get user's Steam inventory
        
        public async Task<List<SteamInventoryItem>> GetInventoryAsync(string steamId, uint appId,
                                                                       CancellationToken cancellationToken = default)
        {
            await _concurrencyLimiter.WaitAsync(cancellationToken);
            try
            {
                Console.WriteLine($"[CRAWLER] Fetching inventory for user {steamId}, app {appId}...");

                string url = $"{STEAM_COMMUNITY_API}inventory/{steamId}/{appId}/2?l=english&count=5000";
                var response = await _httpClient.GetAsync(url);

                var items = new List<SteamInventoryItem>();

                if (response != null)
                {
                    try
                    {
                        using var doc = JsonDocument.Parse(response);
                        if (doc.RootElement.TryGetProperty("descriptions", out var descriptions))
                        {
                            foreach (var desc in descriptions.EnumerateArray())
                            {
                                var item = new SteamInventoryItem
                                {
                                    AppId = appId,
                                    MarketHashName = desc.GetProperty("market_hash_name").GetString(),
                                    MarketName = desc.GetProperty("name").GetString(),
                                    Type = desc.GetProperty("type").GetString(),
                                    IconUrl = $"{STEAM_CDN}steamcommunity/public/images/apps/{appId}/" +
                                             $"{desc.GetProperty("icon_url").GetString()}",
                                    IsTradable = desc.GetProperty("tradable").GetInt32() == 1,
                                    IsMarketable = desc.GetProperty("marketable").GetInt32() == 1,
                                };

                                if (desc.TryGetProperty("tags", out var tags))
                                {
                                    foreach (var tag in tags.EnumerateArray())
                                    {
                                        item.Attributes[tag.GetProperty("category").GetString()] = 
                                            tag.GetProperty("internal_name").GetString();
                                        
                                        if (tag.GetProperty("category").GetString() == "Rarity")
                                        {
                                            item.Rarity = tag.GetProperty("internal_name").GetString();
                                        }
                                    }
                                }

                                items.Add(item);
                                _inventoryCache[item.MarketHashName] = item;
                                Interlocked.Increment(ref Unsafe.As<uint, uint>(
                                    ref _httpClient.Statistics.InventoryItemsFetched));
                            }
                        }
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"[CRAWLER] Error parsing inventory: {ex.Message}");
                    }
                }

                Console.WriteLine($"[CRAWLER] Found {items.Count} inventory items for user {steamId}");
                return items;
            }
            finally
            {
                _concurrencyLimiter.Release();
            }
        }

        
        /// Get Steam Community Market listings for an item
        
        public async Task<List<SteamMarketListing>> GetMarketListingsAsync(uint appId, string marketHashName,
                                                                            CancellationToken cancellationToken = default)
        {
            await _concurrencyLimiter.WaitAsync(cancellationToken);
            try
            {
                Console.WriteLine($"[CRAWLER] Fetching market listings for {marketHashName}...");

                string encodedName = HttpUtility.UrlEncode(marketHashName);
                string url = $"{STEAM_COMMUNITY_API}market/listings/{appId}/{encodedName}";
                var response = await _httpClient.GetAsync(url);

                var listings = new List<SteamMarketListing>();

                if (response != null)
                {
                    // Steam returns HTML with embedded JSON
                    var match = Regex.Match(response, @"var g_rgListingInfo\s*=\s*({.*?});", 
                                           RegexOptions.Singleline);
                    if (match.Success)
                    {
                        try
                        {
                            using var doc = JsonDocument.Parse(match.Groups[1].Value);
                            foreach (var listing in doc.RootElement.EnumerateObject())
                            {
                                var listingData = listing.Value;
                                var marketListing = new SteamMarketListing
                                {
                                    ListingId = listingData.GetProperty("listingid").GetString(),
                                    AppId = appId,
                                    MarketHashName = marketHashName,
                                    Price = listingData.GetProperty("price").GetInt32() / 100m,
                                    Quantity = listingData.GetProperty("quantity").GetUInt32(),
                                    ListedAt = DateTimeOffset.FromUnixTimeSeconds(
                                        listingData.GetProperty("listed_at").GetInt64()).DateTime,
                                    SellerSteamId = listingData.GetProperty("steamid").GetString(),
                                };
                                listings.Add(marketListing);
                                _marketListings.Add(marketListing);
                                Interlocked.Increment(ref Unsafe.As<uint, uint>(
                                    ref _httpClient.Statistics.MarketListingsFetched));
                            }
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine($"[CRAWLER] Error parsing market listings: {ex.Message}");
                        }
                    }
                }

                Console.WriteLine($"[CRAWLER] Found {listings.Count} market listings for {marketHashName}");
                return listings;
            }
            finally
            {
                _concurrencyLimiter.Release();
            }
        }

        
        /// Get price history for a Steam app
        
        public async Task<Dictionary<string, decimal>> GetPriceHistoryAsync(uint appId, 
                                                                             CancellationToken cancellationToken = default)
        {
            string url = $"{STEAM_STORE_API}appdetails?appids={appId}&cc=us&filters=price_overview";
            var response = await _httpClient.GetAsync(url);

            var priceHistory = new Dictionary<string, decimal>();

            if (response != null)
            {
                try
                {
                    using var doc = JsonDocument.Parse(response);
                    if (doc.RootElement.TryGetProperty(appId.ToString(), out var appData) &&
                        appData.TryGetProperty("data", out var data) &&
                        data.TryGetProperty("price_overview", out var price))
                    {
                        var today = DateTime.UtcNow.ToString("yyyy-MM-dd");
                        priceHistory[today] = price.GetProperty("final").GetDecimal() / 100m;
                    }
                }
                catch { }
            }

            return priceHistory;
        }

        
        /// Get current player count for a Steam app
        
        public async Task<uint> GetCurrentPlayerCountAsync(uint appId,
                                                           CancellationToken cancellationToken = default)
        {
            string url = $"{STEAM_API_BASE}ISteamUserStats/GetNumberOfCurrentPlayers/v1/" +
                        $"?appid={appId}&key={_steamApiKey}";
            var response = await _httpClient.GetAsync(url);

            if (response != null)
            {
                try
                {
                    using var doc = JsonDocument.Parse(response);
                    if (doc.RootElement.TryGetProperty("response", out var resp))
                    {
                        return resp.GetProperty("player_count").GetUInt32();
                    }
                }
                catch { }
            }

            return 0;
        }

        
        /// Get Steam reviews for an app
        
        public async Task<List<SteamReview>> GetReviewsAsync(uint appId, int limit = 100,
                                                             CancellationToken cancellationToken = default)
        {
            Console.WriteLine($"[CRAWLER] Fetching reviews for app {appId}...");

            string url = $"{STEAM_STORE_API}appreviews/{appId}?json=1&filter=all&language=english&" +
                        $"review_type=all&purchase_type=all&num_per_page={limit}";
            var response = await _httpClient.GetAsync(url);

            var reviews = new List<SteamReview>();

            if (response != null)
            {
                try
                {
                    using var doc = JsonDocument.Parse(response);
                    if (doc.RootElement.TryGetProperty("reviews", out var reviewsArray))
                    {
                        foreach (var review in reviewsArray.EnumerateArray())
                        {
                            reviews.Add(new SteamReview
                            {
                                ReviewId = review.GetProperty("recommendationid").GetString(),
                                AppId = appId,
                                AuthorSteamId = review.GetProperty("author").GetProperty("steamid").GetString(),
                                AuthorName = review.GetProperty("author").GetProperty("playername").GetString(),
                                Content = review.GetProperty("review").GetString(),
                                IsPositive = review.GetProperty("voted_up").GetBoolean(),
                                PlaytimeAtReview = review.GetProperty("author")
                                    .GetProperty("playtime_at_review").GetUInt32(),
                                CreatedAt = DateTimeOffset.FromUnixTimeSeconds(
                                    review.GetProperty("timestamp_created").GetInt64()).DateTime,
                                UpdatedAt = DateTimeOffset.FromUnixTimeSeconds(
                                    review.GetProperty("timestamp_updated").GetInt64()).DateTime,
                                HelpfulVotes = review.GetProperty("votes_up").GetUInt32(),
                                FunnyVotes = review.GetProperty("votes_funny").GetUInt32(),
                                IsEarlyAccess = review.GetProperty("early_access").GetBoolean(),
                                IsPurchasedOnSteam = review.GetProperty("steam_purchase").GetBoolean(),
                            });
                        }
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"[CRAWLER] Error parsing reviews: {ex.Message}");
                }
            }

            Console.WriteLine($"[CRAWLER] Found {reviews.Count} reviews for app {appId}");
            return reviews;
        }

        
        /// Crawl top apps and their details with parallel processing
        
        public async Task CrawlTopAppsAsync(int count = 100, CancellationToken cancellationToken = default)
        {
            Console.WriteLine($"\n[CRAWLER] Starting crawl of top {count} Steam apps...");
            _httpClient.Statistics.StartTime = DateTime.UtcNow;

            var apps = await GetAppListAsync();
            var topApps = apps.Take(count).ToList();

            var tasks = new List<Task>();
            foreach (var app in topApps)
            {
                tasks.Add(Task.Run(async () =>
                {
                    await GetAppDetailsAsync(app.AppId, cancellationToken);
                    await GetPriceHistoryAsync(app.AppId, cancellationToken);
                    var currentPlayers = await GetCurrentPlayerCountAsync(app.AppId, cancellationToken);
                    
                    if (_appsCache.TryGetValue(app.AppId, out var cachedApp))
                    {
                        cachedApp.CurrentPlayers = currentPlayers;
                    }
                }, cancellationToken));
            }

            await Task.WhenAll(tasks);
            _httpClient.Statistics.EndTime = DateTime.UtcNow;

            Console.WriteLine($"\n[CRAWLER] Crawl completed!");
            Console.WriteLine(_httpClient.Statistics.ToString());
        }

        
        /// Export crawled data to JSON files
        
        public async Task ExportDataAsync(string outputDir = "steam_data")
        {
            Directory.CreateDirectory(outputDir);
            Console.WriteLine($"\n[CRAWLER] Exporting data to {outputDir}...");

            var options = new JsonSerializerOptions 
            { 
                WriteIndented = true,
                DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull
            };

            // Export apps
            var appsJson = JsonSerializer.Serialize(_appsCache.Values.ToList(), options);
            await File.WriteAllTextAsync(Path.Combine(outputDir, "apps.json"), appsJson);
            Console.WriteLine($"[CRAWLER] Exported {_appsCache.Count} apps");

            // Export users
            var usersJson = JsonSerializer.Serialize(_usersCache.Values.ToList(), options);
            await File.WriteAllTextAsync(Path.Combine(outputDir, "users.json"), usersJson);
            Console.WriteLine($"[CRAWLER] Exported {_usersCache.Count} users");

            // Export inventory
            var inventoryJson = JsonSerializer.Serialize(_inventoryCache.Values.ToList(), options);
            await File.WriteAllTextAsync(Path.Combine(outputDir, "inventory.json"), inventoryJson);
            Console.WriteLine($"[CRAWLER] Exported {_inventoryCache.Count} inventory items");

            // Export market listings
            var marketJson = JsonSerializer.Serialize(_marketListings.ToList(), options);
            await File.WriteAllTextAsync(Path.Combine(outputDir, "market_listings.json"), marketJson);
            Console.WriteLine($"[CRAWLER] Exported {_marketListings.Count} market listings");

            // Export statistics
            var statsJson = JsonSerializer.Serialize(_httpClient.Statistics, options);
            await File.WriteAllTextAsync(Path.Combine(outputDir, "statistics.json"), statsJson);
            
            Console.WriteLine($"[CRAWLER] Data export complete!");
        }

        // Helper methods for safe JSON parsing
        private static string GetJsonPropertyString(JsonElement element, string propertyName)
        {
            if (element.TryGetProperty(propertyName, out var prop))
            {
                return prop.GetString();
            }
            return null;
        }

        private static decimal GetJsonPropertyDecimal(JsonElement element, string propertyName)
        {
            if (element.TryGetProperty(propertyName, out var prop))
            {
                return prop.GetDecimal();
            }
            return 0m;
        }

        private static uint GetJsonPropertyUInt(JsonElement element, string propertyName)
        {
            if (element.TryGetProperty(propertyName, out var prop))
            {
                return prop.GetUInt32();
            }
            return 0;
        }

        private static string GetJsonArrayFirstString(JsonElement element, string propertyName)
        {
            if (element.TryGetProperty(propertyName, out var prop) && 
                prop.ValueKind == JsonValueKind.Array && 
                prop.GetArrayLength() > 0)
            {
                return prop[0].GetString();
            }
            return null;
        }

        private static List<string> GetJsonArrayStrings(JsonElement element, string propertyName)
        {
            var list = new List<string>();
            if (element.TryGetProperty(propertyName, out var prop) && 
                prop.ValueKind == JsonValueKind.Array)
            {
                foreach (var item in prop.EnumerateArray())
                {
                    list.Add(item.GetString());
                }
            }
            return list;
        }

        // Public accessors
        public IReadOnlyDictionary<uint, SteamApp> CrawledApps => _appsCache;
        public IReadOnlyDictionary<string, SteamUser> CrawledUsers => _usersCache;
        public CrawlerStatistics Statistics => _httpClient.Statistics;

        public void CancelCrawl()
        {
            _cts?.Cancel();
        }

        public void Dispose()
        {
            if (!_disposed)
            {
                _cts?.Cancel();
                _cts?.Dispose();
                _httpClient?.Dispose();
                _concurrencyLimiter?.Dispose();
                _disposed = true;
            }
        }
    }

    // ============================================================================
    // SECTION 5: Integration with Steam Bridge MetaVerse-BlockChain
    // ============================================================================

    
    /// Bridges Steam crawl data to the MetaVerse-BlockChain system
    
    public class SteamDataBridge
    {
        private readonly SteamWebCrawler _crawler;

        public SteamDataBridge(SteamWebCrawler crawler)
        {
            _crawler = crawler;
        }

        
        /// Convert Steam inventory items to blockchain-trackable assets
        
        public List<SteamInventoryItem> GetBridgeableInventory(string steamId, uint appId)
        {
            // Items that can be bridged: tradable, marketable items with value
            var items = new List<SteamInventoryItem>();
            
            // This would query the crawler's cached data and filter for bridgeable items
            Console.WriteLine($"[BRIDGE] Identifying bridgeable items for user {steamId}, app {appId}");
            
            return items;
        }

        
        /// Create a blockchain representation of a Steam item
        
        public async Task<string> TokenizeSteamItemAsync(SteamInventoryItem item)
        {
            Console.WriteLine($"[BRIDGE] Tokenizing Steam item: {item.MarketName}");
            
            // This would create a token on the blockchain representing the Steam item
            // The token would track ownership, value, and provenance
            
            string tokenId = ComputeItemHash(item);
            
            Console.WriteLine($"[BRIDGE] Created token {tokenId} for {item.MarketName}");
            return tokenId;
        }

        private static string ComputeItemHash(SteamInventoryItem item)
        {
            using var sha256 = SHA256.Create();
            var input = $"{item.AppId}:{item.MarketHashName}:{item.Id}";
            var hash = sha256.ComputeHash(Encoding.UTF8.GetBytes(input));
            return BitConverter.ToString(hash).Replace("-", "").ToLowerInvariant();
        }
    }

    // ============================================================================
    // SECTION 6: Main Program
    // ============================================================================

    public class Program
    {
        public static async Task Main(string[] args)
        {
            Console.WriteLine("═══════════════════════════════════════════════════════");
            Console.WriteLine("  STEAM PLATFORM WEB CRAWLER v3.0");
            Console.WriteLine("  Part of Steam Bridge MetaVerse-BlockChain Ecosystem");
            Console.WriteLine("═══════════════════════════════════════════════════════\n");

            // Parse command line arguments
            string steamApiKey = "";
            int appCount = 20;
            int maxConcurrency = 5;
            
            for (int i = 0; i < args.Length; i++)
            {
                switch (args[i].ToLower())
                {
                    case "--key":
                        if (i + 1 < args.Length) steamApiKey = args[++i];
                        break;
                    case "--count":
                        if (i + 1 < args.Length) appCount = int.Parse(args[++i]);
                        break;
                    case "--concurrency":
                        if (i + 1 < args.Length) maxConcurrency = int.Parse(args[++i]);
                        break;
                }
            }

            if (string.IsNullOrEmpty(steamApiKey))
            {
                Console.WriteLine("[INFO] No Steam API key provided. Some features will be limited.");
                Console.WriteLine("[INFO] Get a key at: https://steamcommunity.com/dev/apikey");
                Console.WriteLine("[INFO] Usage: SteamWebCrawler.exe --key YOUR_API_KEY --count 50\n");
            }

            using var crawler = new SteamWebCrawler(steamApiKey, maxConcurrency);
            var bridge = new SteamDataBridge(crawler);

            try
            {
                // Demonstration crawl
                Console.WriteLine("[DEMO] Starting demonstration crawl...\n");

                // Crawl top apps
                await crawler.CrawlTopAppsAsync(appCount);

                // Example: Get specific app details
                var csgoApp = await crawler.GetAppDetailsAsync(730); // CS:GO
                if (csgoApp != null)
                {
                    Console.WriteLine($"\n[DETAIL] {csgoApp}");
                    Console.WriteLine($"  Developer: {csgoApp.Developer}");
                    Console.WriteLine($"  Achievements: {csgoApp.TotalAchievements}");
                    Console.WriteLine($"  Rating: {csgoApp.Rating:F1}% ({csgoApp.TotalReviews} reviews)");
                    Console.WriteLine($"  Peak Players: {csgoApp.PeakPlayers}");
                }

                // Example: Get reviews
                var reviews = await crawler.GetReviewsAsync(730, 20);
                if (reviews.Count > 0)
                {
                    Console.WriteLine($"\n[REVIEWS] Latest review for CS:GO:");
                    Console.WriteLine($"  {reviews[0]}");
                    Console.WriteLine($"  \"{reviews[0].Content.Substring(0, Math.Min(100, reviews[0].Content.Length))}...\"");
                }

                // Export data
                await crawler.ExportDataAsync();

                Console.WriteLine("\n═══════════════════════════════════════════════════════");
                Console.WriteLine("  Crawl completed successfully!");
                Console.WriteLine($"  Total data cache entries: {crawler.CrawledApps.Count}");
                Console.WriteLine("═══════════════════════════════════════════════════════");
            }
            catch (OperationCanceledException)
            {
                Console.WriteLine("\n[CRAWLER] Operation was cancelled.");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"\n[ERROR] {ex.Message}");
                Console.WriteLine(ex.StackTrace);
            }
        }
    }
}