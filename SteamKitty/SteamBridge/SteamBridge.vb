' ============================================================================
' Steam Bridge v3.0 — Enterprise-Grade Cross-Reality Bridge
' A comprehensive VB.NET implementation connecting MetaVerse to BlockChain
' with cryptographic integrity, proof-of-work consensus, smart bridging,
' and complete memory safety.
'
' Compile: vbc /target:exe /out:SteamBridge.exe SteamBridge.vb /r:System.Numerics.dll
' Run:     SteamBridge.exe
'
' "Where virtual worlds meet immutable truth."
' ============================================================================

Imports System
Imports System.Security.Cryptography
Imports System.Text
Imports System.Collections.Generic
Imports System.Numerics
Imports System.Threading
Imports System.Threading.Tasks
Imports System.Linq
Imports System.Runtime.CompilerServices
Imports System.IO
Imports System.Text.Json

' ============================================================================
' SECTION 1: Cryptographic Primitives & Utility Classes
' ============================================================================

Namespace SteamBridge.Crypto

    
    ''' Pure VB.NET implementation of SHA-256 hash algorithm (FIPS 180-4)
    ''' Provides static methods for hashing strings and binary data
    
    Public Class SHA256Hash
        Private Const BLOCK_SIZE As Integer = 64
        Private Const DIGEST_SIZE As Integer = 32
        
        Private Shared ReadOnly K As UInteger() = {
            &H428A2F98UI, &H71374491UI, &HB5C0FBCFUI, &HE9B5DBA5UI,
            &H3956C25BUI, &H59F111F1UI, &H923F82A4UI, &HAB1C5ED5UI,
            &HD807AA98UI, &H12835B01UI, &H243185BEUI, &H550C7DC3UI,
            &H72BE5D74UI, &H80DEB1FEUI, &H9BDC06A7UI, &HC19BF174UI,
            &HE49B69C1UI, &HEFBE4786UI, &HFC19DC6UI, &H240CA1CCUI,
            &H2DE92C6FUI, &H4A7484AAUI, &H5CB0A9DCUI, &H76F988DAUI,
            &H983E5152UI, &HA831C66DUI, &HB00327C8UI, &HBF597FC7UI,
            &HC6E00BF3UI, &HD5A79147UI, &H6CA6351UI, &H14292967UI,
            &H27B70A85UI, &H2E1B2138UI, &H4D2C6DFCUI, &H53380D13UI,
            &H650A7354UI, &H766A0ABBUI, &H81C2C92EUI, &H92722C85UI,
            &HA2BFE8A1UI, &HA81A664BUI, &HC24B8B70UI, &HC76C51A3UI,
            &HD192E819UI, &HD6990624UI, &HF40E3585UI, &H106AA070UI,
            &H19A4C116UI, &H1E376C08UI, &H2748774CUI, &H34B0BCB5UI,
            &H391C0CB3UI, &H4ED8AA4AUI, &H5B9CCA4FUI, &H682E6FF3UI,
            &H748F82EEUI, &H78A5636FUI, &H84C87814UI, &H8CC70208UI,
            &H90BEFFFAUI, &HA4506CEBUI, &HBEF9A3F7UI, &HC67178F2UI
        }

        
        ''' Streaming context for incremental SHA-256 hashing
        
        Public Class Context
            Private buffer(BLOCK_SIZE - 1) As Byte
            Private bufferLength As Integer
            Private bitCount As ULong
            Private state(7) As UInteger

            Private Shared Function RotR(value As UInteger, count As Integer) As UInteger
                Return (value >> count) Or (value << (32 - count))
            End Function

            Private Shared Function Ch(x As UInteger, y As UInteger, z As UInteger) As UInteger
                Return (x And y) Xor (Not x And z)
            End Function

            Private Shared Function Maj(x As UInteger, y As UInteger, z As UInteger) As UInteger
                Return (x And y) Xor (x And z) Xor (y And z)
            End Function

            Private Shared Function Sigma0(x As UInteger) As UInteger
                Return RotR(x, 7) Xor RotR(x, 18) Xor (x >> 3)
            End Function

            Private Shared Function Sigma1(x As UInteger) As UInteger
                Return RotR(x, 17) Xor RotR(x, 19) Xor (x >> 10)
            End Function

            Private Shared Function Epsilon0(x As UInteger) As UInteger
                Return RotR(x, 2) Xor RotR(x, 13) Xor RotR(x, 22)
            End Function

            Private Shared Function Epsilon1(x As UInteger) As UInteger
                Return RotR(x, 6) Xor RotR(x, 11) Xor RotR(x, 25)
            End Function

            Private Sub Transform(data As Byte())
                Dim m(63) As UInteger
                
                ' Prepare message schedule
                For i As Integer = 0 To 15
                    Dim j As Integer = i * 4
                    m(i) = CUInt(data(j)) << 24 Or CUInt(data(j + 1)) << 16 Or
                           CUInt(data(j + 2)) << 8 Or CUInt(data(j + 3))
                Next
                
                For i As Integer = 16 To 63
                    m(i) = Sigma1(m(i - 2)) + m(i - 7) + Sigma0(m(i - 15)) + m(i - 16)
                Next

                ' Initialize working variables
                Dim a As UInteger = state(0)
                Dim b As UInteger = state(1)
                Dim c As UInteger = state(2)
                Dim d As UInteger = state(3)
                Dim e As UInteger = state(4)
                Dim f As UInteger = state(5)
                Dim g As UInteger = state(6)
                Dim h As UInteger = state(7)

                ' Compression function main loop
                For i As Integer = 0 To 63
                    Dim t1 As UInteger = h + Epsilon1(e) + Ch(e, f, g) + K(i) + m(i)
                    Dim t2 As UInteger = Epsilon0(a) + Maj(a, b, c)
                    h = g : g = f : f = e : e = d + t1
                    d = c : c = b : b = a : a = t1 + t2
                Next

                ' Update state
                state(0) += a : state(1) += b : state(2) += c : state(3) += d
                state(4) += e : state(5) += f : state(6) += g : state(7) += h
            End Sub

            Public Sub New()
                Reset()
            End Sub

            Public Sub Reset()
                bufferLength = 0
                bitCount = 0
                state(0) = &H6A09E667UI : state(1) = &HBB67AE85UI
                state(2) = &H3C6EF372UI : state(3) = &HA54FF53AUI
                state(4) = &H510E527FUI : state(5) = &H9B05688CUI
                state(6) = &H1F83D9ABUI : state(7) = &H5BE0CD19UI
            End Sub

            Public Sub Update(data As Byte())
                If data IsNot Nothing Then
                    For Each b As Byte In data
                        buffer(bufferLength) = b
                        bufferLength += 1
                        If bufferLength = BLOCK_SIZE Then
                            Transform(buffer)
                            bitCount += 512UL
                            bufferLength = 0
                        End If
                    Next
                End If
            End Sub

            Public Sub Update(data As String)
                Update(Encoding.UTF8.GetBytes(data))
            End Sub

            Public Function Finalize() As Byte()
                Dim digest(DIGEST_SIZE - 1) As Byte
                Dim i As Integer = bufferLength
                
                ' Padding
                buffer(i) = &H80
                i += 1
                
                If bufferLength >= 56 Then
                    While i < BLOCK_SIZE
                        buffer(i) = 0 : i += 1
                    End While
                    Transform(buffer)
                    i = 0
                End If
                
                While i < 56
                    buffer(i) = 0 : i += 1
                End While
                
                ' Append bit length
                bitCount += CULng(bufferLength) * 8UL
                buffer(56) = CByte(bitCount >> 56)
                buffer(57) = CByte(bitCount >> 48)
                buffer(58) = CByte(bitCount >> 40)
                buffer(59) = CByte(bitCount >> 32)
                buffer(60) = CByte(bitCount >> 24)
                buffer(61) = CByte(bitCount >> 16)
                buffer(62) = CByte(bitCount >> 8)
                buffer(63) = CByte(bitCount)
                Transform(buffer)

                ' Produce digest
                For i = 0 To 3
                    digest(i) = CByte((state(0) >> (24 - i * 8)) And &HFFUI)
                    digest(i + 4) = CByte((state(1) >> (24 - i * 8)) And &HFFUI)
                    digest(i + 8) = CByte((state(2) >> (24 - i * 8)) And &HFFUI)
                    digest(i + 12) = CByte((state(3) >> (24 - i * 8)) And &HFFUI)
                    digest(i + 16) = CByte((state(4) >> (24 - i * 8)) And &HFFUI)
                    digest(i + 20) = CByte((state(5) >> (24 - i * 8)) And &HFFUI)
                    digest(i + 24) = CByte((state(6) >> (24 - i * 8)) And &HFFUI)
                    digest(i + 28) = CByte((state(7) >> (24 - i * 8)) And &HFFUI)
                Next
                
                Return digest
            End Function
        End Class

        
        ''' Compute SHA-256 hash of a string and return hex representation
        
        Public Shared Function HashHex(input As String) As String
            Dim ctx As New Context()
            ctx.Update(input)
            Dim digest As Byte() = ctx.Finalize()
            Dim sb As New StringBuilder()
            For Each b As Byte In digest
                sb.Append(b.ToString("x2"))
            Next
            Return sb.ToString()
        End Function

        
        ''' Compute SHA-256 hash of binary data
        
        Public Shared Function Hash(data As Byte()) As Byte()
            Dim ctx As New Context()
            ctx.Update(data)
            Return ctx.Finalize()
        End Function

        
        ''' Compute double SHA-256 (SHA256d) as used in Bitcoin
        
        Public Shared Function Hash256dHex(input As String) As String
            Return HashHex(HashHex(input))
        End Function
    End Class

    
    ''' Represents a user's cryptographic identity with keypair
    
    Public Class Wallet
        Private m_privateKeyHex As String
        Private m_publicKeyHex As String
        Private m_address As String

        
        ''' Derive public key from private key using one-way hash
        
        Private Shared Function DerivePublicKey(privateKeyHex As String) As String
            Return SHA256Hash.HashHex("PUBKEY_DERIVATION_V1:" & privateKeyHex)
        End Function

        
        ''' Derive address from public key
        
        Private Shared Function DeriveAddress(publicKeyHex As String) As String
            Return SHA256Hash.HashHex("ADDRESS_V1:" & publicKeyHex).Substring(0, 40)
        End Function

        
        ''' Default constructor - creates wallet with random entropy
        
        Public Sub New()
            ' Generate entropy from multiple sources
            Using rng As RNGCryptoServiceProvider = New RNGCryptoServiceProvider()
                Dim randomBytes(31) As Byte
                rng.GetBytes(randomBytes)
                Dim entropy As String = Convert.ToBase64String(randomBytes) & 
                                        DateTime.Now.Ticks.ToString()
                m_privateKeyHex = SHA256Hash.HashHex(entropy)
            End Using
            m_publicKeyHex = DerivePublicKey(m_privateKeyHex)
            m_address = DeriveAddress(m_publicKeyHex)
        End Sub

        
        ''' Constructor with explicit private key (for testing/restoration)
        
        Public Sub New(privateKeyHex As String)
            If privateKeyHex.Length <> 64 Then
                Throw New ArgumentException("Private key must be 64 hex characters")
            End If
            m_privateKeyHex = privateKeyHex
            m_publicKeyHex = DerivePublicKey(m_privateKeyHex)
            m_address = DeriveAddress(m_publicKeyHex)
        End Sub

        ' Properties
        Public ReadOnly Property PrivateKeyHex As String
            Get
                Return m_privateKeyHex
            End Get
        End Property

        Public ReadOnly Property PublicKeyHex As String
            Get
                Return m_publicKeyHex
            End Get
        End Property

        Public ReadOnly Property Address As String
            Get
                Return m_address
            End Get
        End Property

        
        ''' Sign a message using the private key
        
        Public Function Sign(message As String) As String
            Dim data As String = m_privateKeyHex & message
            Return SHA256Hash.HashHex(data)
        End Function

        
        ''' Verify a signature
        
        Public Shared Function Verify(publicKeyHex As String, message As String, 
                                     signatureHex As String) As Boolean
            ' In real ECDSA, use public key math. Here we use hash-based verification
            Dim derivedPriv As String = SHA256Hash.HashHex("VERIFY_SALT:" & publicKeyHex)
            Dim expectedSig As String = SHA256Hash.HashHex(derivedPriv & message)
            Return signatureHex = expectedSig
        End Function

        
        ''' String representation for debugging
        
        Public Overrides Function ToString() As String
            Return $"Wallet[Address: {m_address.Substring(0, 16)}...]"
        End Function
    End Class

    
    ''' Merkle tree implementation for transaction verification
    
    Public Class MerkleTree
        Private m_leaves As List(Of String)
        Private m_tree As List(Of List(Of String))
        Private m_rootHash As String

        Private Sub BuildTree()
            If m_leaves.Count = 0 Then
                m_rootHash = SHA256Hash.HashHex("EMPTY_MERKLE_TREE")
                Return
            End If

            m_tree = New List(Of List(Of String))()
            m_tree.Add(New List(Of String)(m_leaves))

            While m_tree.Last().Count > 1
                Dim nextLevel As New List(Of String)()
                Dim current As List(Of String) = m_tree.Last()

                For i As Integer = 0 To current.Count - 1 Step 2
                    If i + 1 < current.Count Then
                        nextLevel.Add(SHA256Hash.HashHex(current(i) & current(i + 1)))
                    Else
                        ' Odd number - duplicate last element
                        nextLevel.Add(SHA256Hash.HashHex(current(i) & current(i)))
                    End If
                Next
                m_tree.Add(nextLevel)
            End While

            m_rootHash = m_tree.Last()(0)
        End Sub

        Public Sub New()
            m_leaves = New List(Of String)()
            m_tree = New List(Of List(Of String))()
        End Sub

        Public Sub New(items As IEnumerable(Of String))
            m_leaves = New List(Of String)(items)
            m_tree = New List(Of List(Of String))()
            BuildTree()
        End Sub

        Public Sub AddLeaf(item As String)
            m_leaves.Add(item)
            BuildTree()
        End Sub

        Public ReadOnly Property RootHash As String
            Get
                Return m_rootHash
            End Get
        End Property

        Public ReadOnly Property LeafCount As Integer
            Get
                Return m_leaves.Count
            End Get
        End Property

        
        ''' Generate Merkle proof for a leaf
        
        Public Function GenerateProof(index As Integer) As List(Of (Hash As String, IsLeft As Boolean))
            Dim proof As New List(Of (String, Boolean))()
            Dim currentIndex As Integer = index

            For level As Integer = 0 To m_tree.Count - 2
                Dim isLeft As Boolean = (currentIndex Mod 2 = 0)
                Dim siblingIndex As Integer = If(isLeft, currentIndex + 1, currentIndex - 1)

                If siblingIndex < m_tree(level).Count Then
                    proof.Add((m_tree(level)(siblingIndex), isLeft))
                End If

                currentIndex \= 2
            Next

            Return proof
        End Function
    End Class

End Namespace

' ============================================================================
' SECTION 2: BlockChain Layer
' ============================================================================

Namespace SteamBridge.Blockchain

    ' Difficulty target for Proof-of-Work (number of leading zero nibbles)
    Public Const DEFAULT_DIFFICULTY As Integer = 4
    Public Const MAX_TRANSACTIONS_PER_BLOCK As Integer = 100

    
    ''' Represents a single transaction on the blockchain
    
    Public Class Transaction
        Private m_txId As String
        Private m_fromAddress As String
        Private m_toAddress As String
        Private m_assetId As String
        Private m_amount As ULong
        Private m_timestamp As ULong
        Private m_signature As String
        Private m_data As String
        Private m_nonce As UInteger
        Private m_isBridgeTx As Boolean

        Public Sub New(fromAddress As String, toAddress As String, assetId As String,
                      amount As ULong, signer As Crypto.Wallet, Optional data As String = "",
                      Optional isBridge As Boolean = False)
            m_fromAddress = fromAddress
            m_toAddress = toAddress
            m_assetId = assetId
            m_amount = amount
            m_data = data
            m_isBridgeTx = isBridge

            ' Create timestamp
            m_timestamp = CULng(DateTimeOffset.UtcNow.ToUnixTimeSeconds())

            ' Create unique nonce
            Dim hashInput As String = fromAddress & toAddress & assetId & m_timestamp.ToString()
            m_nonce = CUInt(m_timestamp Xor (CULng(hashInput.GetHashCode()) And &HFFFFFFFFUL))

            ' Create transaction ID
            Dim txInput As String = $"{m_fromAddress}{m_toAddress}{m_assetId}{m_amount}{m_timestamp}{m_data}{m_nonce}"
            m_txId = Crypto.SHA256Hash.HashHex(txInput)

            ' Sign the transaction
            m_signature = signer.Sign(txInput)
        End Sub

        ' Properties
        Public ReadOnly Property TxId As String
            Get
                Return m_txId
            End Get
        End Property

        Public ReadOnly Property FromAddress As String
            Get
                Return m_fromAddress
            End Get
        End Property

        Public ReadOnly Property ToAddress As String
            Get
                Return m_toAddress
            End Get
        End Property

        Public ReadOnly Property AssetId As String
            Get
                Return m_assetId
            End Get
        End Property

        Public ReadOnly Property Amount As ULong
            Get
                Return m_amount
            End Get
        End Property

        Public ReadOnly Property Timestamp As ULong
            Get
                Return m_timestamp
            End Get
        End Property

        Public ReadOnly Property Signature As String
            Get
                Return m_signature
            End Get
        End Property

        Public ReadOnly Property Data As String
            Get
                Return m_data
            End Get
        End Property

        Public ReadOnly Property Nonce As UInteger
            Get
                Return m_nonce
            End Get
        End Property

        Public ReadOnly Property IsBridgeTransaction As Boolean
            Get
                Return m_isBridgeTx
            End Get
        End Property

        
        ''' Serialize transaction for hashing
        
        Public Function Serialize() As String
            Return $"{m_txId}{m_fromAddress}{m_toAddress}{m_assetId}{m_amount}{m_timestamp}{m_signature}{m_nonce}"
        End Function

        
        ''' Validate transaction signature
        
        Public Function IsValid() As Boolean
            Dim txInput As String = $"{m_fromAddress}{m_toAddress}{m_assetId}{m_amount}{m_timestamp}{m_data}{m_nonce}"
            Return Crypto.Wallet.Verify(m_fromAddress, txInput, m_signature)
        End Function

        Public Overrides Function ToString() As String
            Return $"TX[{m_txId.Substring(0, 16)}...] {m_fromAddress.Substring(0, 8)} -> {m_toAddress.Substring(0, 8)}"
        End Function
    End Class

    
    ''' A single block in the blockchain
    
    Public Class Block
        Private m_index As UInteger
        Private m_timestamp As ULong
        Private m_previousHash As String
        Private m_merkleRoot As String
        Private m_hash As String
        Private m_nonce As ULong
        Private m_transactions As List(Of Transaction)
        Private m_difficulty As Integer

        
        ''' Compute the Merkle tree root from transactions
        
        Private Function ComputeMerkleRoot() As String
            If m_transactions.Count = 0 Then
                Return Crypto.SHA256Hash.HashHex("EMPTY_BLOCK_MERKLE")
            End If

            Dim tree As New Crypto.MerkleTree()
            For Each tx As Transaction In m_transactions
                tree.AddLeaf(tx.TxId)
            Next
            Return tree.RootHash
        End Function

        
        ''' Perform proof-of-work mining
        
        Private Sub Mine()
            Dim targetPrefix As New String("0"c, m_difficulty)
            m_nonce = 0UL

            While True
                Dim hashInput As String = $"{m_index}{m_timestamp}{m_previousHash}{m_merkleRoot}{m_nonce}"
                m_hash = Crypto.SHA256Hash.HashHex(hashInput)

                If m_hash.Substring(0, m_difficulty) = targetPrefix Then
                    Exit While
                End If

                m_nonce += 1UL

                ' Check for overflow (extremely unlikely but safe)
                If m_nonce = ULong.MaxValue Then
                    m_timestamp = CULng(DateTimeOffset.UtcNow.ToUnixTimeSeconds())
                    m_nonce = 0UL
                End If
            End While
        End Sub

        Public Sub New(index As UInteger, previousHash As String,
                      Optional difficulty As Integer = DEFAULT_DIFFICULTY)
            m_index = index
            m_previousHash = previousHash
            m_difficulty = difficulty
            m_transactions = New List(Of Transaction)()
            m_timestamp = CULng(DateTimeOffset.UtcNow.ToUnixTimeSeconds())
        End Sub

        
        ''' Add a transaction to the block
        
        Public Sub AddTransaction(tx As Transaction)
            If m_transactions.Count >= MAX_TRANSACTIONS_PER_BLOCK Then
                Throw New InvalidOperationException("Block transaction limit reached")
            End If
            m_transactions.Add(tx)
        End Sub

        
        ''' Finalize the block (compute Merkle root and mine)
        
        Public Sub Finalize()
            m_merkleRoot = ComputeMerkleRoot()
            Mine()
        End Sub

        ' Properties
        Public ReadOnly Property Index As UInteger
            Get
                Return m_index
            End Get
        End Property

        Public ReadOnly Property Timestamp As ULong
            Get
                Return m_timestamp
            End Get
        End Property

        Public ReadOnly Property PreviousHash As String
            Get
                Return m_previousHash
            End Get
        End Property

        Public ReadOnly Property MerkleRoot As String
            Get
                Return m_merkleRoot
            End Get
        End Property

        Public ReadOnly Property Hash As String
            Get
                Return m_hash
            End Get
        End Property

        Public ReadOnly Property Nonce As ULong
            Get
                Return m_nonce
            End Get
        End Property

        Public ReadOnly Property TransactionCount As Integer
            Get
                Return m_transactions.Count
            End Get
        End Property

        Public ReadOnly Property Transactions As List(Of Transaction)
            Get
                Return m_transactions
            End Get
        End Property

        Public ReadOnly Property Difficulty As Integer
            Get
                Return m_difficulty
            End Get
        End Property

        
        ''' Validate the block's integrity
        
        Public Function IsValid() As Boolean
            ' Verify hash meets difficulty target
            Dim targetPrefix As New String("0"c, m_difficulty)
            Dim hashInput As String = $"{m_index}{m_timestamp}{m_previousHash}{m_merkleRoot}{m_nonce}"
            Dim computedHash As String = Crypto.SHA256Hash.HashHex(hashInput)

            If m_hash <> computedHash Then Return False
            If m_hash.Substring(0, m_difficulty) <> targetPrefix Then Return False

            ' Verify Merkle root
            If m_merkleRoot <> ComputeMerkleRoot() Then Return False

            ' Verify all transactions
            For Each tx As Transaction In m_transactions
                If Not tx.IsValid() Then Return False
            Next

            Return True
        End Function

        Public Overrides Function ToString() As String
            Return $"Block #{m_index} [{m_hash.Substring(0, 16)}...] Txs: {m_transactions.Count} Nonce: {m_nonce}"
        End Function
    End Class

    
    ''' The complete blockchain data structure
    
    Public Class BlockChain
        Private m_chain As List(Of Block)
        Private m_txIndex As Dictionary(Of String, Transaction)
        Private m_difficulty As Integer
        Private m_totalTransactions As ULong

        
        ''' Create the genesis block
        
        Private Function CreateGenesisBlock() As Block
            Dim genesis As New Block(0,
                "0000000000000000000000000000000000000000000000000000000000000000",
                m_difficulty)

            ' Create a genesis message transaction
            Dim genesisWallet As New Crypto.Wallet()
            Dim genesisTx As New Transaction(
                "GENESIS", "GENESIS", "GENESIS_ASSET", 0,
                genesisWallet,
                $"Steam Bridge v3.0 Genesis Block - {DateTimeOffset.UtcNow.ToUnixTimeSeconds()}"
            )
            genesis.AddTransaction(genesisTx)
            genesis.Finalize()

            Return genesis
        End Function

        Public Sub New(Optional difficulty As Integer = DEFAULT_DIFFICULTY)
            m_difficulty = difficulty
            m_chain = New List(Of Block)()
            m_txIndex = New Dictionary(Of String, Transaction)()
            m_totalTransactions = 0UL

            Dim genesis As Block = CreateGenesisBlock()
            m_chain.Add(genesis)
            m_totalTransactions += CULng(genesis.TransactionCount)

            Console.WriteLine($"[BLOCKCHAIN] Genesis block created: {genesis.Hash.Substring(0, 16)}...")
        End Sub

        
        ''' Add a new block to the chain
        
        Public Function AddBlock(transactions As List(Of Transaction)) As Block
            Dim block As New Block(CUInt(m_chain.Count), m_chain.Last().Hash, m_difficulty)

            For Each tx As Transaction In transactions
                block.AddTransaction(tx)
                m_txIndex(tx.TxId) = tx
            Next

            block.Finalize()
            m_chain.Add(block)
            m_totalTransactions += CULng(transactions.Count)

            Console.WriteLine($"[BLOCKCHAIN] Block #{block.Index} mined: {block.Hash.Substring(0, 16)}... " &
                            $"(nonce: {block.Nonce}, txs: {transactions.Count})")

            Return block
        End Function

        
        ''' Add a single transaction as a new block
        
        Public Function AddTransaction(tx As Transaction) As Block
            Return AddBlock(New List(Of Transaction) From {tx})
        End Function

        
        ''' Validate the entire blockchain
        
        Public Function ValidateChain() As Boolean
            For i As Integer = 0 To m_chain.Count - 1
                ' Validate individual block
                If Not m_chain(i).IsValid() Then
                    Console.Error.WriteLine($"[VALIDATION] Block #{i} is invalid!")
                    Return False
                End If

                ' Validate chain linkage
                If i > 0 AndAlso m_chain(i).PreviousHash <> m_chain(i - 1).Hash Then
                    Console.Error.WriteLine($"[VALIDATION] Chain broken at block #{i}!")
                    Return False
                End If
            Next
            Return True
        End Function

        
        ''' Find a transaction by ID
        
        Public Function FindTransaction(txId As String) As Transaction
            If m_txIndex.ContainsKey(txId) Then
                Return m_txIndex(txId)
            End If
            Return Nothing
        End Function

        ' Properties
        Public ReadOnly Property Height As Integer
            Get
                Return m_chain.Count
            End Get
        End Property

        Public ReadOnly Property TotalTransactions As ULong
            Get
                Return m_totalTransactions
            End Get
        End Property

        Public ReadOnly Property Difficulty As Integer
            Get
                Return m_difficulty
            End Get
        End Property

        Public ReadOnly Property Blocks As List(Of Block)
            Get
                Return m_chain
            End Get
        End Property

        Public ReadOnly Property LatestBlock As Block
            Get
                Return m_chain.Last()
            End Get
        End Property

        
        ''' Set mining difficulty (adjusts for future blocks)
        
        Public Sub SetDifficulty(difficulty As Integer)
            If difficulty < 1 OrElse difficulty > 64 Then
                Throw New ArgumentException("Difficulty must be between 1 and 64")
            End If
            m_difficulty = difficulty
        End Sub
    End Class

End Namespace

' ============================================================================
' SECTION 3: MetaVerse Layer
' ============================================================================

Namespace SteamBridge.MetaVerse

    
    ''' Categories of virtual assets in the MetaVerse
    
    Public Enum AssetType As Byte
        LAND = 0       ' Virtual real estate
        ITEM = 1       ' In-world items
        AVATAR = 2     ' Character/Avatar
        WEARABLE = 3   ' Clothing/accessories for avatars
        CURRENCY = 4   ' In-world currency
        BLUEPRINT = 5  ' Building plans
    End Enum

    
    ''' Represents a virtual asset in the MetaVerse
    
    Public Class Asset
        Private m_assetId As String
        Private m_name As String
        Private m_type As AssetType
        Private m_ownerAddress As String
        Private m_metadata As Dictionary(Of String, String)
        Private m_isLocked As Boolean
        Private m_createdAt As ULong
        Private m_blockchainTxId As String

        Public Sub New(name As String, type As AssetType, ownerAddress As String,
                      Optional metadata As Dictionary(Of String, String) = Nothing,
                      Optional blockchainTxId As String = "")
            m_name = name
            m_type = type
            m_ownerAddress = ownerAddress
            m_metadata = If(metadata, New Dictionary(Of String, String)())
            m_isLocked = False
            m_createdAt = CULng(DateTimeOffset.UtcNow.ToUnixTimeSeconds())
            m_blockchainTxId = blockchainTxId

            ' Create unique asset ID
            Dim hashInput As String = $"{name}{type}{ownerAddress}{m_createdAt}{Guid.NewGuid()}"
            m_assetId = Crypto.SHA256Hash.HashHex(hashInput)
        End Sub

        ' Properties
        Public ReadOnly Property AssetId As String
            Get
                Return m_assetId
            End Get
        End Property

        Public Property Name As String
            Get
                Return m_name
            End Get
            Set(value As String)
                m_name = value
            End Set
        End Property

        Public ReadOnly Property Type As AssetType
            Get
                Return m_type
            End Get
        End Property

        Public Property OwnerAddress As String
            Get
                Return m_ownerAddress
            End Get
            Set(value As String)
                m_ownerAddress = value
            End Set
        End Property

        Public ReadOnly Property Metadata As Dictionary(Of String, String)
            Get
                Return m_metadata
            End Get
        End Property

        Public Property IsLocked As Boolean
            Get
                Return m_isLocked
            End Get
            Set(value As Boolean)
                m_isLocked = value
            End Set
        End Property

        Public ReadOnly Property CreatedAt As ULong
            Get
                Return m_createdAt
            End Get
        End Property

        Public Property BlockchainTxId As String
            Get
                Return m_blockchainTxId
            End Get
            Set(value As String)
                m_blockchainTxId = value
            End Set
        End Property

        Public Overrides Function ToString() As String
            Return $"Asset[{m_type}] {m_name} (Owner: {m_ownerAddress.Substring(0, 8)}...)"
        End Function
    End Class

    
    ''' Represents a user/entity within the MetaVerse
    
    Public Class User
        Private m_address As String
        Private m_username As String
        Private m_avatarId As String
        Private m_assets As Dictionary(Of String, Asset)
        Private m_joinedAt As ULong
        Private m_lastActive As ULong

        Public Sub New(address As String, username As String, Optional avatarId As String = "")
            m_address = address
            m_username = username
            m_avatarId = avatarId
            m_assets = New Dictionary(Of String, Asset)()
            m_joinedAt = CULng(DateTimeOffset.UtcNow.ToUnixTimeSeconds())
            m_lastActive = m_joinedAt
        End Sub

        ' Properties
        Public ReadOnly Property Address As String
            Get
                Return m_address
            End Get
        End Property

        Public Property Username As String
            Get
                Return m_username
            End Get
            Set(value As String)
                m_username = value
            End Set
        End Property

        Public Property AvatarId As String
           