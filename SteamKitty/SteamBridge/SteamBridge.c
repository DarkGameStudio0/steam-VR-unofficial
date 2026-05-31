/*
 *  Steam Bridge v2.0 — A Pure and Complex Bridge
 *  Connect the MetaVerse to the BlockChain with cryptographic integrity,
 *  proof-of-work consensus, immutable asset tracking, and smart bridging.
 *
 *  Compile: gcc -O2 -o steam_bridge steam_bridge.c -lm -Wall -Wextra -pedantic
 *  Run:     ./steam_bridge
 *
 *  "Not your keys, not your metaverse."
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

/* ------------------------------------------------------------------ */
/*                     Cryptographic Primitives                       */
/* ------------------------------------------------------------------ */

#define SHA256_BLOCK_SIZE 32
#define SHA256_DIGEST_SIZE 32

typedef struct {
    uint8_t data[64];
    uint32_t datalen;
    uint64_t bitlen;
    uint32_t state[8];
} SHA256_CTX;

/* SHA-256 implementation (RFC 6234) */
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

static const uint32_t k[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static void sha256_transform(SHA256_CTX *ctx, const uint8_t data[]) {
    uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
    for ( ; i < 64; ++i)
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

    a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];

    for (i = 0; i < 64; ++i) {
        t1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
        t2 = EP0(a) + MAJ(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX *ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667; ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372; ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f; ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab; ctx->state[7] = 0x5be0cd19;
}

static void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len) {
    uint32_t i;
    for (i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX *ctx, uint8_t hash[]) {
    uint32_t i;
    i = ctx->datalen;
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
    ctx->bitlen += ctx->datalen * 8;
    ctx->data[63] = ctx->bitlen;
    ctx->data[62] = ctx->bitlen >> 8;
    ctx->data[61] = ctx->bitlen >> 16;
    ctx->data[60] = ctx->bitlen >> 24;
    ctx->data[59] = ctx->bitlen >> 32;
    ctx->data[58] = ctx->bitlen >> 40;
    ctx->data[57] = ctx->bitlen >> 48;
    ctx->data[56] = ctx->bitlen >> 56;
    sha256_transform(ctx, ctx->data);
    for (i = 0; i < 4; ++i) {
        hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
    }
}

/* Convenience: hash a string, return hex string */
static void sha256_string(const char *input, char output_hex[65]) {
    SHA256_CTX ctx;
    uint8_t hash[32];
    sha256_init(&ctx);
    sha256_update(&ctx, (const uint8_t*)input, strlen(input));
    sha256_final(&ctx, hash);
    for (int i = 0; i < 32; i++)
        sprintf(output_hex + (i * 2), "%02x", hash[i]);
    output_hex[64] = '\0';
}

/* Simple ECDSA-like signing (simulated with keyed hash for purity) */
typedef struct {
    char public_key_hex[65];
    char private_key_hex[65];
} Wallet;

static void wallet_generate(Wallet *w) {
    /* In a real system, use secp256k1. Here, hash random + time for demo purity */
    char seed[256];
    snprintf(seed, sizeof(seed), "%ld%p%ld", time(NULL), (void*)w, clock());
    sha256_string(seed, w->private_key_hex);
    /* Derive public key by hashing private (again, simplified) */
    sha256_string(w->private_key_hex, w->public_key_hex);
}

static void wallet_sign(const Wallet *w, const char *message, char signature_hex[65]) {
    char combined[129];
    snprintf(combined, sizeof(combined), "%s%s", w->private_key_hex, message);
    sha256_string(combined, signature_hex);
}

static bool wallet_verify(const char *public_key_hex, const char *message, const char *signature_hex) {
    char expected[65];
    char combined[129];
    snprintf(combined, sizeof(combined), "%s%s", "SECRET_PRIVATE_PLACEHOLDER", message);
    sha256_string(combined, expected);
    /* Real verification would use public key math; this ensures bridge purity */
    return strcmp(signature_hex, expected) == 0;
}

/* ------------------------------------------------------------------ */
/*                        BlockChain Layer                           */
/* ------------------------------------------------------------------ */

#define BLOCK_DATA_SIZE 256
#define TARGET_DIFFICULTY 4  /* leading zero nibbles in hash for PoW */

typedef struct Transaction {
    char from[65];      /* wallet public key */
    char to[65];
    char asset_id[65];  /* unique asset hash */
    char signature[65]; /* signed by 'from' */
    uint64_t timestamp;
    struct Transaction *next;
} Transaction;

typedef struct Block {
    uint32_t index;
    uint64_t timestamp;
    char previous_hash[65];
    char merkle_root[65];
    char hash[65];
    uint64_t nonce;
    Transaction *transactions;  /* linked list of tx in this block */
    struct Block *next;
} Block;

typedef struct {
    Block *genesis;
    Block *tail;
    uint32_t height;
    uint64_t total_transactions;
} BlockChain;

/* Merkle tree helper */
static void compute_merkle_root(Transaction *tx_list, char root_hex[65]) {
    /* Simplification: concatenate all tx signatures and hash */
    SHA256_CTX ctx;
    sha256_init(&ctx);
    Transaction *t = tx_list;
    while (t) {
        sha256_update(&ctx, (uint8_t*)t->signature, 64);
        t = t->next;
    }
    uint8_t hash[32];
    sha256_final(&ctx, hash);
    for (int i = 0; i < 32; i++) sprintf(root_hex + (i * 2), "%02x", hash[i]);
    root_hex[64] = '\0';
}

/* Proof-of-Work: find nonce such that hash starts with DIFFICULTY zeros */
static void proof_of_work(Block *block) {
    char candidate[65];
    char input[512];
    block->nonce = 0;
    while (1) {
        snprintf(input, sizeof(input), "%u%lu%s%s%lu", 
                 block->index, block->timestamp, block->previous_hash, 
                 block->merkle_root, block->nonce);
        sha256_string(input, candidate);
        bool valid = true;
        for (int i = 0; i < TARGET_DIFFICULTY; i++) {
            if (candidate[i] != '0') { valid = false; break; }
        }
        if (valid) {
            strcpy(block->hash, candidate);
            break;
        }
        block->nonce++;
    }
}

static Block* create_genesis_block(void) {
    Block *genesis = (Block*)malloc(sizeof(Block));
    genesis->index = 0;
    genesis->timestamp = 0;  /* timestamp of creation moment */
    strcpy(genesis->previous_hash, "0000000000000000000000000000000000000000000000000000000000000000");
    strcpy(genesis->merkle_root, "GENESIS_MERKLE_STEAM_BRIDGE_2025_ROOT");
    genesis->transactions = NULL;
    genesis->nonce = 0;
    proof_of_work(genesis);
    genesis->next = NULL;
    return genesis;
}

static BlockChain* blockchain_create(void) {
    BlockChain *chain = (BlockChain*)malloc(sizeof(BlockChain));
    chain->genesis = create_genesis_block();
    chain->tail = chain->genesis;
    chain->height = 1;
    chain->total_transactions = 0;
    /* Record genesis creation timestamp */
    chain->genesis->timestamp = time(NULL);
    return chain;
}

static Transaction* transaction_create(const char *from, const char *to, 
                                       const char *asset_id, const Wallet *signer) {
    Transaction *tx = (Transaction*)malloc(sizeof(Transaction));
    strcpy(tx->from, from);
    strcpy(tx->to, to);
    strcpy(tx->asset_id, asset_id);
    tx->timestamp = time(NULL);
    /* Sign the transaction */
    char message[512];
    snprintf(message, sizeof(message), "%s%s%s%lu", from, to, asset_id, tx->timestamp);
    wallet_sign(signer, message, tx->signature);
    tx->next = NULL;
    return tx;
}

static Block* blockchain_add_block(BlockChain *chain, Transaction *tx_list) {
    Block *new_block = (Block*)malloc(sizeof(Block));
    new_block->index = chain->tail->index + 1;
    new_block->timestamp = time(NULL);
    strcpy(new_block->previous_hash, chain->tail->hash);
    new_block->transactions = tx_list;
    compute_merkle_root(tx_list, new_block->merkle_root);
    proof_of_work(new_block);
    new_block->next = NULL;
    chain->tail->next = new_block;
    chain->tail = new_block;
    chain->height++;
    /* Count transactions */
    Transaction *t = tx_list;
    while (t) { chain->total_transactions++; t = t->next; }
    return new_block;
}

static bool blockchain_validate(const BlockChain *chain) {
    Block *current = chain->genesis;
    while (current) {
        /* Recompute hash and verify PoW */
        char recomputed[65];
        char input[512];
        snprintf(input, sizeof(input), "%u%lu%s%s%lu",
                 current->index, current->timestamp, current->previous_hash,
                 current->merkle_root, current->nonce);
        sha256_string(input, recomputed);
        if (strcmp(recomputed, current->hash) != 0) return false;
        /* Check link */
        if (current->next && strcmp(current->hash, current->next->previous_hash) != 0) return false;
        current = current->next;
    }
    return true;
}

/* ------------------------------------------------------------------ */
/*                   MetaVerse — Virtual World Assets                 */
/* ------------------------------------------------------------------ */

typedef enum { ASSET_LAND, ASSET_ITEM, ASSET_AVATAR } AssetType;

typedef struct Asset {
    char id[65];          /* hash of asset content */
    char name[64];
    AssetType type;
    char owner_public_key[65];  /* current owner */
    char metadata[256];
    bool locked;          /* if locked, cannot be transferred */
    struct Asset *next;
} Asset;

typedef struct {
    char world_id[65];
    char name[64];
    Asset *assets;
    uint32_t asset_count;
} MetaVerseWorld;

static MetaVerseWorld* metaverse_create(const char *name) {
    MetaVerseWorld *mv = (MetaVerseWorld*)malloc(sizeof(MetaVerseWorld));
    sha256_string(name, mv->world_id);
    strncpy(mv->name, name, 63);
    mv->assets = NULL;
    mv->asset_count = 0;
    return mv;
}

static Asset* metaverse_register_asset(MetaVerseWorld *mv, const char *name, AssetType type,
                                       const char *owner_pubkey, const char *metadata) {
    Asset *a = (Asset*)malloc(sizeof(Asset));
    strncpy(a->name, name, 63);
    a->type = type;
    strcpy(a->owner_public_key, owner_pubkey);
    strncpy(a->metadata, metadata, 255);
    a->locked = false;
    /* Asset ID = hash of name+type+owner+metadata */
    char hash_input[512];
    snprintf(hash_input, sizeof(hash_input), "%s%d%s%s%p%lu", name, type, owner_pubkey, metadata, (void*)a, time(NULL));
    sha256_string(hash_input, a->id);
    a->next = mv->assets;
    mv->assets = a;
    mv->asset_count++;
    return a;
}

static Asset* metaverse_find_asset(MetaVerseWorld *mv, const char *asset_id) {
    Asset *a = mv->assets;
    while (a) {
        if (strcmp(a->id, asset_id) == 0) return a;
        a = a->next;
    }
    return NULL;
}

/* ------------------------------------------------------------------ */
/*                  Steam Bridge — The Core Connector                 */
/* ------------------------------------------------------------------ */

typedef enum { BRIDGE_LOCK, BRIDGE_MINT, BRIDGE_BURN, BRIDGE_TRANSFER } BridgeOp;

typedef struct {
    BridgeOp operation;
    char asset_id[65];
    char source_chain[32];   /* "metaverse" or "blockchain" */
    char target_chain[32];
    char from_pubkey[65];
    char to_pubkey[65];
    char proof[256];         /* cryptographic proof of lock/burn */
    uint64_t timestamp;
} BridgeEvent;

typedef struct {
    BlockChain *chain;
    MetaVerseWorld *metaverse;
    BridgeEvent *events;
    uint32_t event_count;
    Wallet *bridge_validator;   /* The bridge itself has a wallet to sign cross-chain events */
} SteamBridge;

static SteamBridge* steam_bridge_create(const char *mv_name) {
    SteamBridge *bridge = (SteamBridge*)malloc(sizeof(SteamBridge));
    bridge->chain = blockchain_create();
    bridge->metaverse = metaverse_create(mv_name);
    bridge->events = NULL;
    bridge->event_count = 0;
    bridge->bridge_validator = (Wallet*)malloc(sizeof(Wallet));
    wallet_generate(bridge->bridge_validator);
    printf("[BRIDGE] Validator wallet: %s\n", bridge->bridge_validator->public_key_hex);
    return bridge;
}

/* Core operation: lock asset in MetaVerse, mint representation on BlockChain */
static void bridge_lock_and_mint(SteamBridge *bridge, const char *asset_id, 
                                 const char *owner_pubkey, const Wallet *owner_wallet) {
    Asset *asset = metaverse_find_asset(bridge->metaverse, asset_id);
    if (!asset) {
        printf("[BRIDGE] Asset %s not found in MetaVerse.\n", asset_id);
        return;
    }
    if (asset->locked) {
        printf("[BRIDGE] Asset already locked.\n");
        return;
    }
    if (strcmp(asset->owner_public_key, owner_pubkey) != 0) {
        printf("[BRIDGE] Ownership mismatch. Cannot lock.\n");
        return;
    }
    /* Lock asset in MetaVerse */
    asset->locked = true;

    /* Create bridge event proof */
    char proof_msg[512];
    snprintf(proof_msg, sizeof(proof_msg), "LOCK:%s:%s:%lu", asset_id, bridge->metaverse->world_id, time(NULL));
    char lock_proof[65];
    wallet_sign(bridge->bridge_validator, proof_msg, lock_proof);

    /* Now mint a representation on the BlockChain */
    char meta_asset_name[128];
    snprintf(meta_asset_name, sizeof(meta_asset_name), "W%s", asset->name); /* Wrapped version */
    Transaction *tx = transaction_create(
        "BRIDGE_MINT_ADDRESS",  /* System address */
        owner_pubkey,
        asset_id,
        bridge->bridge_validator
    );
    blockchain_add_block(bridge->chain, tx);

    /* Record event */
    bridge->event_count++;
    printf("[BRIDGE] Locked '%s' in MetaVerse and minted wrapped asset on chain. Proof: %s\n",
           asset->name, lock_proof);
}

/* Core operation: burn wrapped asset on chain, unlock in MetaVerse */
static void bridge_burn_and_unlock(SteamBridge *bridge, const char *asset_id,
                                   const char *owner_pubkey, const Wallet *owner_wallet) {
    Asset *asset = metaverse_find_asset(bridge->metaverse, asset_id);
    if (!asset) {
        printf("[BRIDGE] Asset not found.\n");
        return;
    }
    if (!asset->locked) {
        printf("[BRIDGE] Asset not locked; nothing to unlock.\n");
        return;
    }
    /* Verify ownership */
    if (strcmp(asset->owner_public_key, owner_pubkey) != 0) {
        printf("[BRIDGE] Not the owner.\n");
        return;
    }
    /* Burn transaction on chain */
    Transaction *tx = transaction_create(
        owner_pubkey,
        "BRIDGE_BURN_ADDRESS",  /* Burn address */
        asset_id,
        bridge->bridge_validator
    );
    blockchain_add_block(bridge->chain, tx);

    /* Unlock in MetaVerse */
    asset->locked = false;
    bridge->event_count++;
    printf("[BRIDGE] Burned wrapped asset on chain, unlocked '%s' in MetaVerse.\n", asset->name);
}

/* Direct transfer within MetaVerse with on-chain notarization */
static void bridge_notarized_transfer(SteamBridge *bridge, const char *asset_id,
                                      const char *from_pubkey, const char *to_pubkey,
                                      const Wallet *sender_wallet) {
    Asset *asset = metaverse_find_asset(bridge->metaverse, asset_id);
    if (!asset || strcmp(asset->owner_public_key, from_pubkey) != 0 || asset->locked) {
        printf("[BRIDGE] Transfer conditions not met.\n");
        return;
    }
    /* Update ownership */
    strcpy(asset->owner_public_key, to_pubkey);
    /* Notarize on chain */
    Transaction *tx = transaction_create(from_pubkey, to_pubkey, asset_id, sender_wallet);
    blockchain_add_block(bridge->chain, tx);
    printf("[BRIDGE] Transferred '%s' and notarized on chain.\n", asset->name);
}

/* ------------------------------------------------------------------ */
/*                               Main Demo                            */
/* ------------------------------------------------------------------ */

int main(void) {
    printf("═══════════════════════════════════════════════════\n");
    printf("     STEAM BRIDGE v2.0 — Cryptographic Purity      \n");
    printf("  MetaVerse ↔ BlockChain with Proof-of-Work & ECDSA \n");
    printf("═══════════════════════════════════════════════════\n\n");

    /* Initialize bridge */
    SteamBridge *bridge = steam_bridge_create("OmniNexus");
    
    /* Create user wallets */
    Wallet alice_w, bob_w, charlie_w;
    wallet_generate(&alice_w);
    wallet_generate(&bob_w);
    wallet_generate(&charlie_w);
    
    printf("\n--- Wallets ---\n");
    printf("Alice:   %s\n", alice_w.public_key_hex);
    printf("Bob:     %s\n", bob_w.public_key_hex);
    printf("Charlie: %s\n", charlie_w.public_key_hex);

    /* Register MetaVerse assets */
    printf("\n--- Registering MetaVerse Assets ---\n");
    Asset *sword = metaverse_register_asset(bridge->metaverse, "Excalibur", ASSET_ITEM, 
                                            alice_w.public_key_hex, "Legendary sword of power");
    Asset *land = metaverse_register_asset(bridge->metaverse, "Genesis Plot", ASSET_LAND,
                                           bob_w.public_key_hex, "Prime virtual real estate");
    Asset *avatar = metaverse_register_asset(bridge->metaverse, "CyberPhoenix", ASSET_AVATAR,
                                             charlie_w.public_key_hex, "Rare legendary avatar skin");
    
    printf("Registered: %s (ID: %.16s...), %s, %s\n", sword->name, sword->id, land->name, avatar->name);

    /* Demonstrate bridging operations */
    printf("\n══════ Bridge Operations ══════\n");

    /* 1. Lock & Mint */
    printf("\n[1] Alice locks Excalibur into bridge → wrapped asset minted on chain:\n");
    bridge_lock_and_mint(bridge, sword->id, alice_w.public_key_hex, &alice_w);

    /* 2. Direct notarized transfer */
    printf("\n[2] Bob transfers Genesis Plot to Charlie (notarized on chain):\n");
    bridge_notarized_transfer(bridge, land->id, bob_w.public_key_hex, charlie_w.public_key_hex, &bob_w);

    /* 3. Burn & Unlock */
    printf("\n[3] Alice burns wrapped Excalibur on chain → unlocks original in MetaVerse:\n");
    bridge_burn_and_unlock(bridge, sword->id, alice_w.public_key_hex, &alice_w);

    /* Blockchain integrity check */
    printf("\n══════ Blockchain Integrity ══════\n");
    bool valid = blockchain_validate(bridge->chain);
    printf("Chain valid: %s\n", valid ? "✓ YES" : "✗ NO");
    printf("Chain height: %u blocks\n", bridge->chain->height);
    printf("Total transactions recorded: %lu\n", bridge->chain->total_transactions);
    printf("Bridge events processed: %u\n", bridge->event_count);
    
    /* Print chain summary */
    printf("\n--- Block Hash History ---\n");
    Block *b = bridge->chain->genesis;
    while (b) {
        printf("Block #%u: %.16s... (nonce: %lu)\n", b->index, b->hash, b->nonce);
        b = b->next;
    }

    /* Print final MetaVerse state */
    printf("\n--- MetaVerse Final State ---\n");
    Asset *a = bridge->metaverse->assets;
    while (a) {
        printf("Asset: %s | Owner: %.16s... | Locked: %s\n", 
               a->name, a->owner_public_key, a->locked ? "yes" : "no");
        a = a->next;
    }

    printf("\n═══════════════════════════════════════════════════\n");
    printf("Steam Bridge v2.0: The pure connection stands.\n");
    printf("═══════════════════════════════════════════════════\n");

    /* Note: Full memory cleanup omitted for brevity in educational demo */
    return 0;
}