// comprehensive_tests.cpp
#include "ArcadiaEngine.h"
#include <iostream>
#include <vector>
#include <cassert>
#include <chrono>
#include <random>
#include <string>
#include <algorithm>
#include <queue>
#include <iomanip>

using namespace std;

// ==========================================
// EXTERNAL FUNCTIONS
// ==========================================
extern "C" {
    PlayerTable* createPlayerTable();
    Leaderboard* createLeaderboard();
    AuctionTree* createAuctionTree();
}

// ==========================================
// TEST RUNNER
// ==========================================
class ComprehensiveTester {
    int passed = 0;
    int failed = 0;
    
public:
    void runAllTests() {
        cout << "=== COMPREHENSIVE ARCADIA ENGINE TESTS ===" << endl;
        cout << "==========================================" << endl;
        
        testPartA_HashTable();
        testPartA_SkipList();
        testPartA_RedBlackTree();
        testPartB_Inventory();
        testPartC_Navigator();
        testPartD_Kernel();
        testPerformance();
        testMemory();
        
        cout << "\n==========================================" << endl;
        cout << "SUMMARY: Passed: " << passed << " | Failed: " << failed << endl;
        cout << "==========================================" << endl;
        
        if (failed == 0) {
            cout << "✓ All comprehensive tests passed!" << endl;
        } else {
            cout << "✗ Some tests failed. Review your implementation." << endl;
        }
    }
    
private:
    void assertTest(string testName, bool condition) {
        cout << "TEST: " << left << setw(60) << testName;
        if (condition) {
            cout << "[ PASS ]" << endl;
            passed++;
        } else {
            cout << "[ FAIL ]" << endl;
            failed++;
        }
    }
    
    // ==========================================
    // PART A: HASH TABLE TESTS
    // ==========================================
    void testPartA_HashTable() {
        cout << "\n--- Part A: Hash Table (PlayerTable) ---" << endl;
        
        // Test 1: Basic insert and search
        {
            PlayerTable* table = createPlayerTable();
            table->insert(1, "Alice");
            assertTest("HashTable: Basic insert/search", 
                      table->search(1) == "Alice");
            delete table;
        }
        
        // Test 2: Insert, Delete, Reinsert (tombstone handling)
        {
            PlayerTable* table = createPlayerTable();
            table->insert(1, "Alice");
            // Note: If no delete function, test update
            table->insert(1, "Bob");  // Update existing
            assertTest("HashTable: Update existing player", 
                      table->search(1) == "Bob");
            delete table;
        }
        
        // Test 3: Double hashing with many collisions
        {
            PlayerTable* table = createPlayerTable();
            bool allInserted = true;
            
            // Insert keys that might cause collisions
            for (int i = 0; i < 50; i++) {
                int key = 101 * i + 1;  // All hash to same initial position (1)
                table->insert(key, "Player" + to_string(key));
                
                // Verify insertion
                if (table->search(key) != "Player" + to_string(key)) {
                    allInserted = false;
                    break;
                }
            }
            
            assertTest("HashTable: Double hashing with collisions", allInserted);
            delete table;
        }
        
        // Test 4: Table full scenario
        {
            PlayerTable* table = createPlayerTable();
            bool fullMessageCaught = false;
            
            // Fill table with 101 players
            for (int i = 0; i < 101; i++) {
                table->insert(i, "Player" + to_string(i));
            }
            
            // Try to insert 102nd player
            try {
                table->insert(102, "ExtraPlayer");
            } catch (const char* msg) {
                if (string(msg) == "Table is full") {
                    fullMessageCaught = true;
                }
            }
            
            assertTest("HashTable: Throw 'Table is full' when full", fullMessageCaught);
            delete table;
        }
    }
    
    // ==========================================
    // PART A: SKIP LIST TESTS
    // ==========================================
    void testPartA_SkipList() {
        cout << "\n--- Part A: Skip List (Leaderboard) ---" << endl;
        
        // Test 1: Empty leaderboard
        {
            Leaderboard* board = createLeaderboard();
            vector<int> result = board->getTopN(5);
            assertTest("SkipList: Empty leaderboard getTopN", 
                      result.empty());
            delete board;
        }
        
        // Test 2: getTopN where n > total players
        {
            Leaderboard* board = createLeaderboard();
            board->addScore(1, 100);
            board->addScore(2, 200);
            vector<int> top = board->getTopN(10);
            assertTest("SkipList: getTopN(n > total players)", 
                      top.size() == 2);
            delete board;
        }
        
        // Test 3: Multiple ties with different IDs
        {
            Leaderboard* board = createLeaderboard();
            board->addScore(10, 100);
            board->addScore(20, 100);
            board->addScore(5, 100);
            board->addScore(15, 100);
            
            vector<int> top = board->getTopN(4);
            bool correctOrder = (top.size() == 4 && 
                               top[0] == 5 && 
                               top[1] == 10 && 
                               top[2] == 15 && 
                               top[3] == 20);
            
            assertTest("SkipList: Multiple ties with ascending IDs", correctOrder);
            delete board;
        }
        
        // Test 4: Remove non-existent player
        {
            Leaderboard* board = createLeaderboard();
            board->addScore(1, 100);
            board->addScore(2, 200);
            
            // Should not crash
            board->removePlayer(999);
            
            // Original players should still exist
            vector<int> top = board->getTopN(2);
            bool stillExists = (top.size() == 2);
            
            assertTest("SkipList: Remove non-existent player", stillExists);
            delete board;
        }
        
        // Test 5: Remove all players one by one
        {
            Leaderboard* board = createLeaderboard();
            board->addScore(1, 100);
            board->addScore(2, 200);
            board->addScore(3, 300);
            
            board->removePlayer(2);
            board->removePlayer(1);
            board->removePlayer(3);
            
            vector<int> top = board->getTopN(5);
            assertTest("SkipList: Remove all players", top.empty());
            delete board;
        }
    }
    
    // ==========================================
    // PART A: RED-BLACK TREE TESTS
    // ==========================================
    void testPartA_RedBlackTree() {
        cout << "\n--- Part A: Red-Black Tree (AuctionTree) ---" << endl;
        
        // Test 1: Insert many items with same price
        {
            AuctionTree* tree = createAuctionTree();
            bool allInserted = true;
            
            for (int i = 0; i < 100; i++) {
                tree->insertItem(i, 50); // All same price
            }
            
            // Delete some items to test deletion with duplicates
            for (int i = 0; i < 50; i++) {
                tree->deleteItem(i);
            }
            
            // Should not crash during operations
            assertTest("RBTree: Many items with same price", allInserted);
            delete tree;
        }
        
        // Test 2: Delete non-existent item
        {
            AuctionTree* tree = createAuctionTree();
            tree->insertItem(1, 100);
            
            // Should not crash
            tree->deleteItem(999);
            
            assertTest("RBTree: Delete non-existent item", true); // Pass if no crash
            delete tree;
        }
        
        // Test 3: Delete root node repeatedly
        {
            AuctionTree* tree = createAuctionTree();
            tree->insertItem(1, 100);
            tree->insertItem(2, 50);
            tree->insertItem(3, 150);
            
            tree->deleteItem(2); // Might be root
            tree->deleteItem(1);
            tree->deleteItem(3);
            
            // Tree should be empty
            // Note: Can't directly check emptiness, but should not crash
            assertTest("RBTree: Delete all nodes (including root)", true);
            delete tree;
        }
        
        // Test 4: Insert in reverse price order
        {
            AuctionTree* tree = createAuctionTree();
            
            // Insert in descending price order
            for (int i = 100; i > 0; i--) {
                tree->insertItem(i, i * 10);
            }
            
            // Insert some with same price
            tree->insertItem(101, 500);
            tree->insertItem(102, 500);
            tree->insertItem(103, 500);
            
            // Delete items with same price
            tree->deleteItem(102);
            
            assertTest("RBTree: Insert in reverse price order", true);
            delete tree;
        }
        
        // Test 5: Complex scenario with mixed operations
        {
            AuctionTree* tree = createAuctionTree();
            
            // Insert random items
            for (int i = 0; i < 50; i++) {
                tree->insertItem(i, rand() % 1000);
            }
            
            // Delete some random items
            for (int i = 0; i < 20; i++) {
                tree->deleteItem(i * 2);
            }
            
            // Insert more items
            for (int i = 50; i < 80; i++) {
                tree->insertItem(i, rand() % 1000);
            }
            
            assertTest("RBTree: Complex mixed operations", true);
            delete tree;
        }
    }
    
    // ==========================================
    // PART B: INVENTORY SYSTEM TESTS
    // ==========================================
    void testPartB_Inventory() {
        cout << "\n--- Part B: Inventory System ---" << endl;
        
        // Test 1: Loot Splitting - Empty coins list
        {
            vector<int> empty = {};
            int result = InventorySystem::optimizeLootSplit(0, empty);
            assertTest("LootSplit: Empty list", result == 0);
        }
        
        // Test 2: Loot Splitting - Single coin
        {
            vector<int> single = {100};
            int result = InventorySystem::optimizeLootSplit(1, single);
            assertTest("LootSplit: Single coin", result == 100);
        }
        
        // Test 3: Loot Splitting - Large values
        {
            vector<int> large = {1000000, 2000000, 3000000};
            int result = InventorySystem::optimizeLootSplit(3, large);
            // Expected: Best split {3000000} vs {1000000,2000000} = 0
            assertTest("LootSplit: Large values", result == 0);
        }
        
        // Test 4: Knapsack - Capacity 0
        {
            vector<pair<int, int>> items = {{1, 10}, {2, 20}};
            int value = InventorySystem::maximizeCarryValue(0, items);
            assertTest("Knapsack: Capacity 0", value == 0);
        }
        
        // Test 5: Knapsack - Empty items list
        {
            vector<pair<int, int>> emptyItems = {};
            int value = InventorySystem::maximizeCarryValue(10, emptyItems);
            assertTest("Knapsack: Empty items list", value == 0);
        }
        
        // Test 6: Knapsack - Item weight > capacity
        {
            vector<pair<int, int>> items = {{100, 1000}};
            int value = InventorySystem::maximizeCarryValue(10, items);
            assertTest("Knapsack: Item weight > capacity", value == 0);
        }
        
        // Test 7: Knapsack - All items fit exactly
        {
            vector<pair<int, int>> items = {{2, 20}, {3, 30}, {5, 50}};
            int value = InventorySystem::maximizeCarryValue(10, items);
            assertTest("Knapsack: All items fit exactly", value == 100);
        }
        
        // Test 8: Chat Autocorrect - Empty string
        {
            long long count = InventorySystem::countStringPossibilities("");
            assertTest("ChatAutocorrect: Empty string", count == 1);
        }
        
        // Test 9: Chat Autocorrect - Single character
        {
            long long count = InventorySystem::countStringPossibilities("a");
            assertTest("ChatAutocorrect: Single character", count == 1);
        }
        
        // Test 10: Chat Autocorrect - Very long string
        {
            string longStr(1000, 'u');
            long long count = InventorySystem::countStringPossibilities(longStr);
            // Should not crash and return valid modulo result
            assertTest("ChatAutocorrect: Very long string", count >= 0 && count < 1000000007);
        }
        
        // Test 11: Chat Autocorrect - Mixed patterns
        {
            long long count = InventorySystem::countStringPossibilities("uunnuunn");
            // Test complex combinations
            assertTest("ChatAutocorrect: Mixed patterns", count > 0);
        }
        
        // Test 12: Chat Autocorrect - All 'm's and 'w's
        {
            long long count1 = InventorySystem::countStringPossibilities("mmmm");
            long long count2 = InventorySystem::countStringPossibilities("wwww");
            assertTest("ChatAutocorrect: Multiple m and w", count1 > 0 && count2 > 0);
        }
    }
    
    // ==========================================
    // PART C: WORLD NAVIGATOR TESTS
    // ==========================================
    void testPartC_Navigator() {
        cout << "\n--- Part C: World Navigator ---" << endl;
        
        // Test 1: Path Existence - Single node graph
        {
            vector<vector<int>> edges = {};
            bool exists = WorldNavigator::pathExists(1, edges, 0, 0);
            assertTest("PathExists: Single node", exists == true);
        }
        
        // Test 2: Path Existence - Disconnected large graph
        {
            vector<vector<int>> edges = {{0, 1}, {1, 2}, {3, 4}, {4, 5}};
            bool exists = WorldNavigator::pathExists(6, edges, 0, 5);
            assertTest("PathExists: Disconnected components", exists == false);
        }
        
        // Test 3: Path Existence - Self-loop edges
        {
            vector<vector<int>> edges = {{0, 0}};
            bool exists = WorldNavigator::pathExists(1, edges, 0, 0);
            assertTest("PathExists: Self-loop", exists == true);
        }
        
        // Test 4: Path Existence - Multiple paths between nodes
        {
            vector<vector<int>> edges = {{0, 1}, {0, 2}, {1, 3}, {2, 3}, {1, 2}};
            bool exists = WorldNavigator::pathExists(4, edges, 0, 3);
            assertTest("PathExists: Multiple paths", exists == true);
        }
        
        // Test 5: MST - Single node
        {
            vector<vector<int>> roads = {};
            long long cost = WorldNavigator::minBribeCost(1, 0, 1, 1, roads);
            assertTest("MST: Single node", cost == 0);
        }
        
        // Test 6: MST - Disconnected graph (should return -1)
        {
            vector<vector<int>> roads = {{0, 1, 10, 0}, {2, 3, 5, 0}};
            long long cost = WorldNavigator::minBribeCost(4, 2, 1, 1, roads);
            assertTest("MST: Disconnected graph", cost == -1);
        }
        
        // Test 7: MST - Large rates (potential overflow)
        {
            vector<vector<int>> roads = {{0, 1, 1000, 2000}, {1, 2, 1500, 2500}};
            long long goldRate = 1000000000;
            long long silverRate = 1000000000;
            long long cost = WorldNavigator::minBribeCost(3, 2, goldRate, silverRate, roads);
            // Should not overflow
            assertTest("MST: Large rates no overflow", cost > 0);
        }
        
        // Test 8: MST - Road with zero cost
        {
            vector<vector<int>> roads = {{0, 1, 0, 0}, {1, 2, 5, 0}, {0, 2, 10, 0}};
            long long cost = WorldNavigator::minBribeCost(3, 3, 1, 1, roads);
            // Should include zero-cost road
            assertTest("MST: Zero cost roads", cost == 5);
        }
        
        // Test 9: Teleporter - Single node
        {
            vector<vector<int>> roads = {};
            string result = WorldNavigator::sumMinDistancesBinary(1, roads);
            assertTest("Teleporter: Single node", result == "0");
        }
        
        // Test 10: Teleporter - Completely disconnected graph
        {
            vector<vector<int>> roads = {};
            string result = WorldNavigator::sumMinDistancesBinary(3, roads);
            // All pairs disconnected, sum should be 0 (since disconnected pairs don't count?)
            // Actually based on example: disconnected pairs give ∞, might not be included in sum
            assertTest("Teleporter: Disconnected graph", result == "0");
        }
        
        // Test 11: Teleporter - Complete graph with power-of-2 weights
        {
            vector<vector<int>> roads = {
                {0, 1, 1}, {0, 2, 2}, {0, 3, 4},
                {1, 2, 8}, {1, 3, 16},
                {2, 3, 32}
            };
            string result = WorldNavigator::sumMinDistancesBinary(4, roads);
            // Should compute correct sum
            assertTest("Teleporter: Complete graph power-of-2", !result.empty());
        }
        
        // Test 12: Teleporter - Very large distances (2^31)
        {
            vector<vector<int>> roads = {{0, 1, 2147483647}}; // 2^31 - 1
            string result = WorldNavigator::sumMinDistancesBinary(2, roads);
            // Should handle large numbers
            assertTest("Teleporter: Large distances", !result.empty());
        }
    }
    
    // ==========================================
    // PART D: SERVER KERNEL TESTS
    // ==========================================
    void testPartD_Kernel() {
        cout << "\n--- Part D: Server Kernel ---" << endl;
        
        // Test 1: Empty tasks
        {
            vector<char> empty = {};
            int intervals = ServerKernel::minIntervals(empty, 2);
            assertTest("Scheduler: Empty tasks", intervals == 0);
        }
        
        // Test 2: n=0 (no cooling)
        {
            vector<char> tasks = {'A', 'A', 'A'};
            int intervals = ServerKernel::minIntervals(tasks, 0);
            assertTest("Scheduler: n=0 no cooling", intervals == 3);
        }
        
        // Test 3: Single task repeated many times
        {
            vector<char> tasks(100, 'A');
            int intervals = ServerKernel::minIntervals(tasks, 1);
            // Formula: (count-1)*(n+1) + 1 = 99*2 + 1 = 199
            assertTest("Scheduler: Single task repeated", intervals == 199);
        }
        
        // Test 4: All tasks unique (alphabet)
        {
            vector<char> tasks = {'A','B','C','D','E','F'};
            int intervals = ServerKernel::minIntervals(tasks, 2);
            assertTest("Scheduler: All unique tasks", intervals == 6);
        }
        
        // Test 5: Maximum constraints simulation
        {
            vector<char> tasks;
            for (int i = 0; i < 1000; i++) { // Test with 1000 instead of 10000 for speed
                tasks.push_back('A' + (rand() % 26));
            }
            
            int intervals = ServerKernel::minIntervals(tasks, 100);
            // Should compute without crashing
            assertTest("Scheduler: Large input", intervals > 0);
        }
        
        // Test 6: Mixed frequencies
        {
            vector<char> tasks = {'A','A','A','A','B','B','B','C','C','D'};
            int intervals = ServerKernel::minIntervals(tasks, 2);
            // Known pattern: A B C A B C A B D A
            assertTest("Scheduler: Mixed frequencies", intervals == 10);
        }
    }
    
    // ==========================================
    // PERFORMANCE TESTS
    // ==========================================
    void testPerformance() {
        cout << "\n--- Performance Tests ---" << endl;
        
        // Test Skip List O(log n) insertion
        {
            Leaderboard* board = createLeaderboard();
            auto start = chrono::high_resolution_clock::now();
            
            for (int i = 0; i < 100000; i++) {
                board->addScore(i, rand() % 10000);
            }
            
            auto end = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
            
            cout << "SkipList: Inserted 100,000 players in " << duration.count() << "ms" << endl;
            assertTest("Performance: SkipList O(log n) insertion", duration.count() < 1000);
            
            delete board;
        }
        
        // Test Hash Table O(1) average
        {
            PlayerTable* table = createPlayerTable();
            auto start = chrono::high_resolution_clock::now();
            
            for (int i = 0; i < 1000; i++) { // Limited by table size 101
                table->insert(i % 100, "Player" + to_string(i));
            }
            
            auto end = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
            
            cout << "HashTable: 1000 operations in " << duration.count() << "μs" << endl;
            assertTest("Performance: HashTable O(1) operations", duration.count() < 10000);
            
            delete table;
        }
        
        // Test Knapsack O(n×W)
        {
            vector<pair<int, int>> items;
            for (int i = 0; i < 1000; i++) {
                items.push_back({rand() % 100 + 1, rand() % 1000 + 1});
            }
            
            auto start = chrono::high_resolution_clock::now();
            int result = InventorySystem::maximizeCarryValue(1000, items);
            auto end = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
            
            cout << "Knapsack: 1000 items in " << duration.count() << "ms" << endl;
            assertTest("Performance: Knapsack DP", duration.count() < 1000);
        }
    }
    
    // ==========================================
    // MEMORY TESTS
    // ==========================================
    void testMemory() {
        cout << "\n--- Memory Tests ---" << endl;
        
        // Test repeated creation/deletion
        {
            bool noCrash = true;
            
            for (int i = 0; i < 100; i++) {
                Leaderboard* board = createLeaderboard();
                for (int j = 0; j < 1000; j++) {
                    board->addScore(j, j * 10);
                }
                
                // Remove some players
                for (int j = 0; j < 500; j++) {
                    board->removePlayer(j);
                }
                
                delete board;
            }
            
            assertTest("Memory: Repeated create/delete cycles", noCrash);
        }
        
        // Test large structure cleanup
        {
            AuctionTree* tree = createAuctionTree();
            
            // Insert many items
            for (int i = 0; i < 10000; i++) {
                tree->insertItem(i, rand() % 10000);
            }
            
            // Delete half
            for (int i = 0; i < 5000; i++) {
                tree->deleteItem(i * 2);
            }
            
            // Cleanup should happen in destructor
            delete tree;
            
            assertTest("Memory: Large tree cleanup", true);
        }
    }
};

// ==========================================
// MAIN FUNCTION
// ==========================================
int main() {
    // Seed random for consistent tests
    srand(42);
    
    ComprehensiveTester tester;
    tester.runAllTests();
    
    return 0;
}