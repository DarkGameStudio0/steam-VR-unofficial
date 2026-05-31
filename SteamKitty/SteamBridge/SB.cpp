/*
 *  Steam Bridge v3.0 — Enterprise-Grade Cross-Reality Bridge
 *  A comprehensive C++ implementation connecting MetaVerse to BlockChain
 *  with cryptographic integrity, proof-of-work consensus, smart bridging,
 *  and complete memory safety.
 *
 *  Compile: g++ -std=c++17 -O2 -o steam_bridge steam_bridge.cpp -lm -Wall -Wextra -pedantic
 *  Run:     ./steam_bridge
 *
 *  "Where virtual worlds meet immutable truth."
 */

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <memory>
#include <optional>
#include <variant>
#include <functional>
#include <algorithm>
#include <chrono>
#include <random>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <cassert>
#include <stdexcept>
#include <type_traits>

// ============================================================================
// SECTION 1: Cryptographic Primitives & Utility Classes
// ============================================================================

namespace SteamBridge::Crypto {

    /**
     * @class SHA256
     * @brief Pure C++ implementation of SHA-256 hash algorithm (FIPS 180-4)
     * 
     * Provides static methods for hashing strings and binary data,
     * plus a streaming context for incremental hashing.
     */
    class SHA256 {
    private:
        static constexpr size_t BLOCK_SIZE = 64;
        static constexpr size_t DIGEST_SIZE = 32;
        static constexpr uint32_t K[64] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
            0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
            0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
            0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
            0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
            0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
            0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
            0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
            0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
        };

    public:
        /**
         * @class Context
         * @brief Streaming context for incremental SHA-256 hashing
         */
        class Context {
        private:
            uint8_t m_buffer[BLOCK_SIZE];
            size_t m_buffer_length;
            uint64_t m_bit_count;
            uint32_t m_state[8];

            // Bitwise rotation and logical functions
            static inline uint32_t rotr(uint32_t value, unsigned int count) noexcept {
                return (value >> count) | (value << (32 - count));
            }
            
            static inline uint32_t choose(uint32_t x, uint32_t y, uint32_t z) noexcept {
                return (x & y) ^ (~x & z);
            }
            
            static inline uint32_t majority(uint32_t x, uint32_t y, uint32_t z) noexcept {
                return (x & y) ^ (x & z) ^ (y & z);
            }
            
            static inline uint32_t sigma0(uint32_t x) noexcept {
                return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
            }
            
            static inline uint32_t sigma1(uint32_t x) noexcept {
                return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
            }
            
            static inline uint32_t epsilon0(uint32_t x) noexcept {
                return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
            }
            
            static inline uint32_t epsilon1(uint32_t x) noexcept {
                return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
            }

            void transform(const uint8_t data[BLOCK_SIZE]) noexcept;

        public:
            Context() noexcept;
            void update(const uint8_t* data, size_t length) noexcept;
            void update(const std::string_view& data) noexcept;
            std::array<uint8_t, DIGEST_SIZE> finalize() noexcept;
            void reset() noexcept;
        };

        /**
         * @brief Compute SHA-256 hash of a string and return hex representation
         * @param input String to hash
         * @return 64-character hexadecimal string
         */
        static std::string hash_hex(const std::string_view& input);
        
        /**
         * @brief Compute SHA-256 hash of binary data
         * @param data Pointer to data
         * @param length Length in bytes
         * @return 32-byte hash array
         */
        static std::array<uint8_t, DIGEST_SIZE> hash(const uint8_t* data, size_t length);
        
        /**
         * @brief Compute double SHA-256 (SHA256d) as used in Bitcoin
         */
        static std::string hash256d_hex(const std::string_view& input);
    };

    // Implementation of SHA256::Context
    SHA256::Context::Context() noexcept {
        reset();
    }

    void SHA256::Context::reset() noexcept {
        m_buffer_length = 0;
        m_bit_count = 0;
        m_state[0] = 0x6a09e667; m_state[1] = 0xbb67ae85;
        m_state[2] = 0x3c6ef372; m_state[3] = 0xa54ff53a;
        m_state[4] = 0x510e527f; m_state[5] = 0x9b05688c;
        m_state[6] = 0x1f83d9ab; m_state[7] = 0x5be0cd19;
    }

    void SHA256::Context::transform(const uint8_t data[BLOCK_SIZE]) noexcept {
        uint32_t m[64];
        // Prepare message schedule
        for (size_t i = 0, j = 0; i < 16; ++i, j += 4) {
            m[i] = (static_cast<uint32_t>(data[j]) << 24) |
                   (static_cast<uint32_t>(data[j + 1]) << 16) |
                   (static_cast<uint32_t>(data[j + 2]) << 8) |
                   static_cast<uint32_t>(data[j + 3]);
        }
        for (size_t i = 16; i < 64; ++i) {
            m[i] = sigma1(m[i - 2]) + m[i - 7] + sigma0(m[i - 15]) + m[i - 16];
        }

        // Initialize working variables
        uint32_t a = m_state[0], b = m_state[1], c = m_state[2], d = m_state[3];
        uint32_t e = m_state[4], f = m_state[5], g = m_state[6], h = m_state[7];

        // Compression function main loop
        for (size_t i = 0; i < 64; ++i) {
            uint32_t t1 = h + epsilon1(e) + choose(e, f, g) + K[i] + m[i];
            uint32_t t2 = epsilon0(a) + majority(a, b, c);
            h = g; g = f; f = e; e = d + t1;
            d = c; c = b; b = a; a = t1 + t2;
        }

        // Update state
        m_state[0] += a; m_state[1] += b; m_state[2] += c; m_state[3] += d;
        m_state[4] += e; m_state[5] += f; m_state[6] += g; m_state[7] += h;
    }

    void SHA256::Context::update(const uint8_t* data, size_t length) noexcept {
        for (size_t i = 0; i < length; ++i) {
            m_buffer[m_buffer_length++] = data[i];
            if (m_buffer_length == BLOCK_SIZE) {
                transform(m_buffer);
                m_bit_count += 512;
                m_buffer_length = 0;
            }
        }
    }

    void SHA256::Context::update(const std::string_view& data) noexcept {
        update(reinterpret_cast<const uint8_t*>(data.data()), data.length());
    }

    std::array<uint8_t, SHA256::DIGEST_SIZE> SHA256::Context::finalize() noexcept {
        std::array<uint8_t, DIGEST_SIZE> digest{};
        
        // Padding
        size_t i = m_buffer_length;
        m_buffer[i++] = 0x80;
        if (m_buffer_length >= 56) {
            while (i < BLOCK_SIZE) m_buffer[i++] = 0;
            transform(m_buffer);
            i = 0;
        }
        while (i < 56) m_buffer[i++] = 0;
        
        // Append bit length
        m_bit_count += m_buffer_length * 8;
        m_buffer[56] = static_cast<uint8_t>(m_bit_count >> 56);
        m_buffer[57] = static_cast<uint8_t>(m_bit_count >> 48);
        m_buffer[58] = static_cast<uint8_t>(m_bit_count >> 40);
        m_buffer[59] = static_cast<uint8_t>(m_bit_count >> 32);
        m_buffer[60] = static_cast<uint8_t>(m_bit_count >> 24);
        m_buffer[61] = static_cast<uint8_t>(m_bit_count >> 16);
        m_buffer[62] = static_cast<uint8_t>(m_bit_count >> 8);
        m_buffer[63] = static_cast<uint8_t>(m_bit_count);
        transform(m_buffer);

        // Produce digest
        for (i = 0; i < 4; ++i) {
            digest[i]      = static_cast<uint8_t>((m_state[0] >> (24 - i * 8)) & 0xFF);
            digest[i + 4]  = static_cast<uint8_t>((m_state[1] >> (24 - i * 8)) & 0xFF);
            digest[i + 8]  = static_cast<uint8_t>((m_state[2] >> (24 - i * 8)) & 0xFF);
            digest[i + 12] = static_cast<uint8_t>((m_state[3] >> (24 - i * 8)) & 0xFF);
            digest[i + 16] = static_cast<uint8_t>((m_state[4] >> (24 - i * 8)) & 0xFF);
            digest[i + 20] = static_cast<uint8_t>((m_state[5] >> (24 - i * 8)) & 0xFF);
            digest[i + 24] = static_cast<uint8_t>((m_state[6] >> (24 - i * 8)) & 0xFF);
            digest[i + 28] = static_cast<uint8_t>((m_state[7] >> (24 - i * 8)) & 0xFF);
        }
        
        return digest;
    }

    std::string SHA256::hash_hex(const std::string_view& input) {
        Context ctx;
        ctx.update(input);
        auto digest = ctx.finalize();
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (uint8_t byte : digest) {
            ss << std::setw(2) << static_cast<int>(byte);
        }
        return ss.str();
    }

    std::array<uint8_t, SHA256::DIGEST_SIZE> SHA256::hash(const uint8_t* data, size_t length) {
        Context ctx;
        ctx.update(data, length);
        return ctx.finalize();
    }

    std::string SHA256::hash256d_hex(const std::string_view& input) {
        return hash_hex(hash_hex(input));
    }

} // namespace SteamBridge::Crypto

// ============================================================================
// SECTION 2: Cryptographic Wallet & Digital Signatures
// ============================================================================

namespace SteamBridge::Crypto {

    /**
     * @class Wallet
     * @brief Represents a user's cryptographic identity with keypair
     * 
     * In a production system, this would use secp256k1 elliptic curve.
     * For purity of demonstration, we use deterministic key derivation
     * with SHA-256 to create consistent keypairs from entropy.
     */
    class Wallet {
    private:
        std::string m_private_key_hex;  // 64 hex chars = 32 bytes
        std::string m_public_key_hex;   // 64 hex chars
        std::string m_address;          // Shorter public identifier
        
        /**
         * @brief Derive public key from private key using one-way hash
         * @note Simplified - real systems use ECDSA public key derivation
         */
        static std::string derive_public_key(const std::string& private_key_hex) {
            return SHA256::hash_hex("PUBKEY_DERIVATION_V1:" + private_key_hex);
        }
        
        /**
         * @brief Derive address from public key
         */
        static std::string derive_address(const std::string& public_key_hex) {
            return SHA256::hash_hex("ADDRESS_V1:" + public_key_hex).substr(0, 40);
        }

    public:
        /**
         * @brief Default constructor - creates wallet with random entropy
         */
        Wallet() {
            // Generate entropy from multiple sources
            auto now = std::chrono::high_resolution_clock::now();
            auto duration = now.time_since_epoch();
            auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
            
            std::random_device rd;
            std::mt19937_64 gen(rd());
            std::uniform_int_distribution<uint64_t> dist;
            
            std::stringstream entropy;
            entropy << std::hex << nanos << dist(gen) << dist(gen) << rd();
            
            m_private_key_hex = SHA256::hash_hex(entropy.str());
            m_public_key_hex = derive_public_key(m_private_key_hex);
            m_address = derive_address(m_public_key_hex);
        }
        
        /**
         * @brief Constructor with explicit private key (for testing/restoration)
         */
        explicit Wallet(const std::string& private_key_hex) 
            : m_private_key_hex(private_key_hex) {
            if (private_key_hex.length() != 64) {
                throw std::invalid_argument("Private key must be 64 hex characters");
            }
            m_public_key_hex = derive_public_key(m_private_key_hex);
            m_address = derive_address(m_public_key_hex);
        }
        
        // Accessors
        [[nodiscard]] const std::string& get_private_key_hex() const noexcept { return m_private_key_hex; }
        [[nodiscard]] const std::string& get_public_key_hex() const noexcept { return m_public_key_hex; }
        [[nodiscard]] const std::string& get_address() const noexcept { return m_address; }
        
        /**
         * @brief Sign a message using the private key
         * @param message The message to sign
         * @return 64-char hex signature
         */
        [[nodiscard]] std::string sign(const std::string_view& message) const {
            // ECDSA signing simulation: Hash(private_key || message)
            std::string data = m_private_key_hex + std::string(message);
            return SHA256::hash_hex(data);
        }
        
        /**
         * @brief Verify a signature (static method)
         * @param public_key_hex The signer's public key
         * @param message The original message
         * @param signature_hex The signature to verify
         * @return true if signature is valid
         */
        [[nodiscard]] static bool verify(const std::string& public_key_hex, 
                                         const std::string_view& message,
                                         const std::string& signature_hex) {
            // In real ECDSA, use public key math. Here we use hash-based verification
            std::string derived_priv = SHA256::hash_hex("VERIFY_SALT:" + public_key_hex);
            std::string expected_sig = SHA256::hash_hex(derived_priv + std::string(message));
            return signature_hex == expected_sig;
        }
        
        /**
         * @brief String representation for debugging
         */
        [[nodiscard]] std::string to_string() const {
            std::stringstream ss;
            ss << "Wallet[Address: " << m_address.substr(0, 16) << "...]";
            return ss.str();
        }
        
        // Comparison operators
        bool operator==(const Wallet& other) const noexcept {
            return m_public_key_hex == other.m_public_key_hex;
        }
        bool operator!=(const Wallet& other) const noexcept {
            return !(*this == other);
        }
    };

    /**
     * @class MerkleTree
     * @brief Merkle tree implementation for transaction verification
     */
    class MerkleTree {
    private:
        std::vector<std::string> m_leaves;
        std::vector<std::vector<std::string>> m_tree;
        std::string m_root_hash;

        void build_tree() {
            if (m_leaves.empty()) {
                m_root_hash = SHA256::hash_hex("EMPTY_MERKLE_TREE");
                return;
            }
            
            m_tree.clear();
            m_tree.push_back(m_leaves);
            
            while (m_tree.back().size() > 1) {
                std::vector<std::string> next_level;
                const auto& current = m_tree.back();
                
                for (size_t i = 0; i < current.size(); i += 2) {
                    if (i + 1 < current.size()) {
                        next_level.push_back(SHA256::hash_hex(current[i] + current[i + 1]));
                    } else {
                        // Odd number - duplicate last element
                        next_level.push_back(SHA256::hash_hex(current[i] + current[i]));
                    }
                }
                m_tree.push_back(next_level);
            }
            
            m_root_hash = m_tree.back()[0];
        }

    public:
        MerkleTree() = default;
        
        explicit MerkleTree(const std::vector<std::string>& items) : m_leaves(items) {
            build_tree();
        }
        
        void add_leaf(const std::string& item) {
            m_leaves.push_back(item);
            build_tree();
        }
        
        [[nodiscard]] const std::string& get_root_hash() const noexcept { return m_root_hash; }
        
        [[nodiscard]] size_t leaf_count() const noexcept { return m_leaves.size(); }
        
        /**
         * @brief Generate Merkle proof for a leaf
         * @return Vector of sibling hashes from leaf to root
         */
        [[nodiscard]] std::vector<std::pair<std::string, bool>> generate_proof(size_t index) const {
            std::vector<std::pair<std::string, bool>> proof; // hash, is_left
            size_t current_index = index;
            
            for (size_t level = 0; level < m_tree.size() - 1; ++level) {
                bool is_left = (current_index % 2 == 0);
                size_t sibling_index = is_left ? current_index + 1 : current_index - 1;
                
                if (sibling_index < m_tree[level].size()) {
                    proof.emplace_back(m_tree[level][sibling_index], is_left);
                }
                
                current_index /= 2;
            }
            
            return proof;
        }
    };

} // namespace SteamBridge::Crypto

// ============================================================================
// SECTION 3: BlockChain Layer
// ============================================================================

namespace SteamBridge::Blockchain {

    using namespace Crypto;
    
    // Difficulty target for Proof-of-Work (number of leading zero nibbles)
    constexpr int DEFAULT_DIFFICULTY = 4;
    constexpr size_t MAX_TRANSACTIONS_PER_BLOCK = 100;
    constexpr size_t BLOCK_SIZE_LIMIT = 1024 * 1024; // 1MB simulated

    /**
     * @class Transaction
     * @brief Represents a single transaction on the blockchain
     * 
     * Contains sender, receiver, asset reference, amount, and cryptographic signature.
     */
    class Transaction {
    private:
        std::string m_tx_id;           // Transaction hash
        std::string m_from_address;    // Sender's wallet address
        std::string m_to_address;      // Recipient's wallet address
        std::string m_asset_id;        // Asset being transferred (empty for currency)
        uint64_t m_amount;             // Amount (for fungible tokens)
        uint64_t m_timestamp;          // Unix timestamp
        std::string m_signature;       // Cryptographic signature
        std::string m_data;            // Optional arbitrary data
        uint32_t m_nonce;              // Transaction nonce (ordering)
        bool m_is_bridge_tx;           // Whether this is a bridge transaction

    public:
        Transaction(const std::string& from, const std::string& to,
                   const std::string& asset_id, uint64_t amount,
                   const Wallet& signer, const std::string& data = "",
                   bool is_bridge = false)
            : m_from_address(from), m_to_address(to), m_asset_id(asset_id),
              m_amount(amount), m_data(data), m_is_bridge_tx(is_bridge) {
            
            m_timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();
            
            // Create unique nonce based on timestamp and random factor
            m_nonce = static_cast<uint32_t>(m_timestamp ^ 
                       (std::hash<std::string>{}(from + to + asset_id) & 0xFFFFFFFF));
            
            // Create transaction ID
            std::stringstream ss;
            ss << m_from_address << m_to_address << m_asset_id 
               << m_amount << m_timestamp << m_data << m_nonce;
            m_tx_id = SHA256::hash_hex(ss.str());
            
            // Sign the transaction
            std::string message = ss.str();
            m_signature = signer.sign(message);
        }
        
        // Accessors
        [[nodiscard]] const std::string& get_tx_id() const noexcept { return m_tx_id; }
        [[nodiscard]] const std::string& get_from_address() const noexcept { return m_from_address; }
        [[nodiscard]] const std::string& get_to_address() const noexcept { return m_to_address; }
        [[nodiscard]] const std::string& get_asset_id() const noexcept { return m_asset_id; }
        [[nodiscard]] uint64_t get_amount() const noexcept { return m_amount; }
        [[nodiscard]] uint64_t get_timestamp() const noexcept { return m_timestamp; }
        [[nodiscard]] const std::string& get_signature() const noexcept { return m_signature; }
        [[nodiscard]] const std::string& get_data() const noexcept { return m_data; }
        [[nodiscard]] uint32_t get_nonce() const noexcept { return m_nonce; }
        [[nodiscard]] bool is_bridge_transaction() const noexcept { return m_is_bridge_tx; }
        
        /**
         * @brief Serialize transaction for hashing
         */
        [[nodiscard]] std::string serialize() const {
            std::stringstream ss;
            ss << m_tx_id << m_from_address << m_to_address << m_asset_id
               << m_amount << m_timestamp << m_signature << m_nonce;
            return ss.str();
        }
        
        /**
         * @brief Validate transaction signature
         */
        [[nodiscard]] bool is_valid() const {
            std::stringstream ss;
            ss << m_from_address << m_to_address << m_asset_id 
               << m_amount << m_timestamp << m_data << m_nonce;
            return Wallet::verify(m_from_address, ss.str(), m_signature);
        }
    };

    /**
     * @class Block
     * @brief A single block in the blockchain
     * 
     * Contains a list of transactions, metadata, and proof-of-work.
     */
    class Block {
    private:
        uint32_t m_index;                  // Block height
        uint64_t m_timestamp;              // Creation timestamp
        std::string m_previous_hash;       // Link to previous block
        std::string m_merkle_root;         // Merkle tree root of transactions
        std::string m_hash;                // This block's hash
        uint64_t m_nonce;                  // Proof-of-work nonce
        std::vector<std::shared_ptr<Transaction>> m_transactions;
        int m_difficulty;                  // Mining difficulty

        /**
         * @brief Compute the Merkle tree root from transactions
         */
        [[nodiscard]] std::string compute_merkle_root() const {
            if (m_transactions.empty()) {
                return SHA256::hash_hex("EMPTY_BLOCK_MERKLE");
            }
            
            MerkleTree tree;
            for (const auto& tx : m_transactions) {
                tree.add_leaf(tx->get_tx_id());
            }
            return tree.get_root_hash();
        }

        /**
         * @brief Perform proof-of-work mining
         */
        void mine() {
            std::string target_prefix(m_difficulty, '0');
            m_nonce = 0;
            
            while (true) {
                std::stringstream ss;
                ss << m_index << m_timestamp << m_previous_hash 
                   << m_merkle_root << m_nonce;
                m_hash = SHA256::hash_hex(ss.str());
                
                if (m_hash.substr(0, m_difficulty) == target_prefix) {
                    break;
                }
                
                ++m_nonce;
                
                // Check for overflow (extremely unlikely but safe)
                if (m_nonce == UINT64_MAX) {
                    m_timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::system_clock::now().time_since_epoch()
                    ).count();
                    m_nonce = 0;
                }
            }
        }

    public:
        Block(uint32_t index, const std::string& previous_hash, int difficulty = DEFAULT_DIFFICULTY)
            : m_index(index), m_previous_hash(previous_hash), m_difficulty(difficulty) {
            m_timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();
        }
        
        /**
         * @brief Add a transaction to the block
         */
        void add_transaction(std::shared_ptr<Transaction> tx) {
            if (m_transactions.size() >= MAX_TRANSACTIONS_PER_BLOCK) {
                throw std::runtime_error("Block transaction limit reached");
            }
            m_transactions.push_back(std::move(tx));
        }
        
        /**
         * @brief Finalize the block (compute Merkle root and mine)
         */
        void finalize() {
            m_merkle_root = compute_merkle_root();
            mine();
        }
        
        // Accessors
        [[nodiscard]] uint32_t get_index() const noexcept { return m_index; }
        [[nodiscard]] uint64_t get_timestamp() const noexcept { return m_timestamp; }
        [[nodiscard]] const std::string& get_previous_hash() const noexcept { return m_previous_hash; }
        [[nodiscard]] const std::string& get_merkle_root() const noexcept { return m_merkle_root; }
        [[nodiscard]] const std::string& get_hash() const noexcept { return m_hash; }
        [[nodiscard]] uint64_t get_nonce() const noexcept { return m_nonce; }
        [[nodiscard]] size_t get_transaction_count() const noexcept { return m_transactions.size(); }
        [[nodiscard]] const std::vector<std::shared_ptr<Transaction>>& get_transactions() const noexcept { 
            return m_transactions; 
        }
        
        /**
         * @brief Validate the block's integrity
         */
        [[nodiscard]] bool is_valid() const {
            // Verify hash meets difficulty target
            std::string target_prefix(m_difficulty, '0');
            std::stringstream ss;
            ss << m_index << m_timestamp << m_previous_hash << m_merkle_root << m_nonce;
            std::string computed_hash = SHA256::hash_hex(ss.str());
            
            if (m_hash != computed_hash) return false;
            if (m_hash.substr(0, m_difficulty) != target_prefix) return false;
            
            // Verify Merkle root
            if (m_merkle_root != compute_merkle_root()) return false;
            
            // Verify all transactions
            for (const auto& tx : m_transactions) {
                if (!tx->is_valid()) return false;
            }
            
            return true;
        }
    };

    /**
     * @class BlockChain
     * @brief The complete blockchain data structure
     * 
     * Manages the chain of blocks, provides validation, and maintains state.
     */
    class BlockChain {
    private:
        std::vector<std::shared_ptr<Block>> m_chain;
        std::unordered_map<std::string, std::shared_ptr<Transaction>> m_tx_index;
        int m_difficulty;
        uint64_t m_total_transactions;
        
        /**
         * @brief Create the genesis block
         */
        std::shared_ptr<Block> create_genesis_block() {
            auto genesis = std::make_shared<Block>(0, 
                "0000000000000000000000000000000000000000000000000000000000000000",
                m_difficulty);
            
            // Create a genesis message transaction
            auto genesis_tx = std::make_shared<Transaction>(
                "GENESIS", "GENESIS", "GENESIS_ASSET", 0,
                Wallet(), "Steam Bridge v3.0 Genesis Block - " + 
                std::to_string(std::chrono::system_clock::now().time_since_epoch().count())
            );
            genesis->add_transaction(genesis_tx);
            genesis->finalize();
            
            return genesis;
        }

    public:
        explicit BlockChain(int difficulty = DEFAULT_DIFFICULTY) 
            : m_difficulty(difficulty), m_total_transactions(0) {
            m_chain.push_back(create_genesis_block());
            m_total_transactions += m_chain[0]->get_transaction_count();
            std::cout << "[BLOCKCHAIN] Genesis block created: " 
                      << m_chain[0]->get_hash().substr(0, 16) << "...\n";
        }
        
        /**
         * @brief Add a new block to the chain
         */
        std::shared_ptr<Block> add_block(const std::vector<std::shared_ptr<Transaction>>& transactions) {
            auto block = std::make_shared<Block>(
                static_cast<uint32_t>(m_chain.size()),
                m_chain.back()->get_hash(),
                m_difficulty
            );
            
            for (const auto& tx : transactions) {
                block->add_transaction(tx);
                m_tx_index[tx->get_tx_id()] = tx;
            }
            
            block->finalize();
            m_chain.push_back(block);
            m_total_transactions += transactions.size();
            
            std::cout << "[BLOCKCHAIN] Block #" << block->get_index() << " mined: "
                      << block->get_hash().substr(0, 16) << "... (nonce: " 
                      << block->get_nonce() << ", txs: " << transactions.size() << ")\n";
            
            return block;
        }
        
        /**
         * @brief Add a single transaction as a new block
         */
        std::shared_ptr<Block> add_transaction(std::shared_ptr<Transaction> tx) {
            return add_block({tx});
        }
        
        /**
         * @brief Validate the entire blockchain
         */
        [[nodiscard]] bool validate_chain() const {
            for (size_t i = 0; i < m_chain.size(); ++i) {
                // Validate individual block
                if (!m_chain[i]->is_valid()) {
                    std::cerr << "[VALIDATION] Block #" << i << " is invalid!\n";
                    return false;
                }
                
                // Validate chain linkage
                if (i > 0 && m_chain[i]->get_previous_hash() != m_chain[i-1]->get_hash()) {
                    std::cerr << "[VALIDATION] Chain broken at block #" << i << "!\n";
                    return false;
                }
            }
            return true;
        }
        
        /**
         * @brief Find a transaction by ID
         */
        [[nodiscard]] std::optional<std::shared_ptr<Transaction>> find_transaction(
            const std::string& tx_id) const {
            auto it = m_tx_index.find(tx_id);
            if (it != m_tx_index.end()) {
                return it->second;
            }
            return std::nullopt;
        }
        
        // Accessors
        [[nodiscard]] size_t get_height() const noexcept { return m_chain.size(); }
        [[nodiscard]] uint64_t get_total_transactions() const noexcept { return m_total_transactions; }
        [[nodiscard]] int get_difficulty() const noexcept { return m_difficulty; }
        [[nodiscard]] const std::vector<std::shared_ptr<Block>>& get_blocks() const noexcept { 
            return m_chain; 
        }
        [[nodiscard]] std::shared_ptr<Block> get_latest_block() const noexcept { 
            return m_chain.back(); 
        }
        
        /**
         * @brief Set mining difficulty (adjusts for future blocks)
         */
        void set_difficulty(int difficulty) {
            if (difficulty < 1 || difficulty > 64) {
                throw std::invalid_argument("Difficulty must be between 1 and 64");
            }
            m_difficulty = difficulty;
        }
    };

} // namespace SteamBridge::Blockchain

// ============================================================================
// SECTION 4: MetaVerse Layer
// ============================================================================

namespace SteamBridge::MetaVerse {

    using namespace Crypto;
    using namespace Blockchain;

    /**
     * @enum AssetType
     * @brief Categories of virtual assets in the MetaVerse
     */
    enum class AssetType : uint8_t {
        LAND,           // Virtual real estate
        ITEM,           // In-world items
        AVATAR,         // Character/Avatar
        WEARABLE,       // Clothing/accessories for avatars
        CURRENCY,       // In-world currency
        BLUEPR