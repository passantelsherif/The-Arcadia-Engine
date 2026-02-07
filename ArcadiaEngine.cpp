#include "ArcadiaEngine.h"
#include <algorithm>
#include <queue>
#include <numeric>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <set>

using namespace std;

// =========================================================
// PART A: DATA STRUCTURES (Concrete Implementations)
// =========================================================
// --- 1. PlayerTable (Double Hashing) ---
class ConcretePlayerTable : public PlayerTable {
private:
    // Define data structures
    enum State{
        EMPTY, 
        OCCUPIED,
        DELETED
    };

    struct Element{
        int playerID;
        string name;
        State state;
    };
    
    vector<Element> hashTable;
    int size;
    int occupiedCells;
public:
    ConcretePlayerTable() {
        // Initialize hash table info
        size = 101;
        hashTable.resize(size);
        occupiedCells = 0;

        // Initialize all slots to empty
        for (int i = 0; i < hashTable.size(); i++) {
            hashTable[i].state = EMPTY;
        }
    }

    // First hash function (division) as the slides
    int h1(int playerID){
        return playerID % hashTable.size();
    }

    // Second hash function as the slides
    int h2(int playerID){
        int P = findPrevPrime(hashTable.size() - 1);  // Prime smaller than hash table size
        int result = P - (playerID % P);
        return (result == 0) ? 1 : result; // Ensure h2 never returns 0
    }

    void insert(int playerID, string name) override {
        if (occupiedCells >= size) {
            throw "Table is full";
        }

        int h1Key = h1(playerID);
        int h2Key = h2(playerID);
        int i = 0;
        int firstDeletedIndex = -1;

        while (i < hashTable.size()) {
            int currentIndex = (h1Key + i * h2Key) % hashTable.size();

            if (hashTable[currentIndex].state == EMPTY) {
                // Found empty slot
                int insertIndex = (firstDeletedIndex != -1) ? firstDeletedIndex : currentIndex;

                hashTable[insertIndex].playerID = playerID;
                hashTable[insertIndex].name = name;
                hashTable[insertIndex].state = OCCUPIED;
                occupiedCells++;
                return;

            } else if (hashTable[currentIndex].state == DELETED) {
                // Found tombstone - remember first one
                if (firstDeletedIndex == -1) {
                    firstDeletedIndex = currentIndex;
                }
            } else if (hashTable[currentIndex].state == OCCUPIED) {
                // Check if same playerID exists
                if (hashTable[currentIndex].playerID == playerID) {
                    // Update existing player
                    hashTable[currentIndex].name = name; // update
                    return;
                }
            }

            i++;
        }

        // If we get here and found a deleted slot, use it
        if (firstDeletedIndex != -1) {
            hashTable[firstDeletedIndex].playerID = playerID;
            hashTable[firstDeletedIndex].name = name;
            hashTable[firstDeletedIndex].state = OCCUPIED;
            occupiedCells++;
            return;
        }

        // Signal full
        throw "Table is full";
    }

    string search(int playerID) override {
        int h1Key = h1(playerID);
        int h2Key = h2(playerID);
        int i = 0;
        while (i < hashTable.size()) {
            int currentIndex = (h1Key + i * h2Key) % hashTable.size();
            
            if (hashTable[currentIndex].state == EMPTY) {
                // Player not found
                return "";
            } 
            else if (hashTable[currentIndex].state == OCCUPIED) {
                if (hashTable[currentIndex].playerID == playerID) {
                    // Player found
                    return hashTable[currentIndex].name;
                }
            }
            // Continue probing for DELETED or different OCCUPIED
            i++;
        }
        
        // Searched entire table and didn't found the player
        return "";
    }

    // ------ Helper Functions -----------
    bool isPrime(int n){
        if (n <= 1) return false;
        
        // Check divisors from 2 to sqrt(n)
        for (int i = 2; i * i <= n; i++) {
            if (n % i == 0) return false;
        }

        return true;
    }

    int findNextPrime(int n){
        if (n <= 2) return 2;
        while (!isPrime(n)) {
            n++;
        }
        return n;
    }

    int findPrevPrime(int n){
        if (n <= 2) return 2;
        while (!isPrime(n)) {
            n--;
        }
        return n;
    }
    // -----------------------------------
};

// --- 2. Leaderboard (Skip List) ---
class ConcreteLeaderboard : public Leaderboard {
private:
    struct Node{
        int id;
        int score;
        vector<Node*> forward;

        Node(int playerID, int playerScore, int level){
            id = playerID;
            score = playerScore;
            forward.resize(level, nullptr);
        }
    };

    int maxLevel;
    int highLevel;
    Node* head;

    // To randomly decide how tall the new node will be
    int randomGen(){
        int lvl = 1;
        while( (rand() % 2) && lvl < maxLevel )
        {
            lvl++;
        }
        return lvl;
    }

public:
    // Initialize skip list
    ConcreteLeaderboard() {
        maxLevel = 32;
        highLevel = 1;
        head = new Node(-1,-1,maxLevel);
    }

    // Implement skip list insertion
    void addScore(int playerID, int score) override {
        vector<Node*> update(maxLevel, nullptr); //keeps track of last node at each level before the insertion point.
        Node* current = head;
        for(int i= highLevel-1; i>= 0; i--)
        {
              while(current->forward[i] != nullptr && 
                  (current->forward[i]->score > score ||  (current->forward[i]->score == score && current->forward[i]->id < playerID)))
            {
                // For descending: higher score first, or same score with smaller ID first
                current = current->forward[i];
            }
            update[i] = current;
        }

        // Generate random num of levels for the new node
        int k = randomGen(); 

        // If the new node is taller than the current tallest node
        if(k > highLevel)
        {
            for(int i =highLevel; i<k; i++)
            {
                update[i] = head; // The new node will be first at these levels
            }
            highLevel = k;
        }

        Node* newNode = new Node(playerID, score, k);

        // Insert the new node in k levels
        for(int i=0; i<k; i++)
        {
            newNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newNode;
        }
    }

    // Implement skip list deletion
    void removePlayer(int playerID) override {
        // Find the node using linear search at level 0
        Node* target = nullptr;
        Node* currentLinear = head->forward[0];
        while(currentLinear != nullptr && currentLinear->id != playerID) {
            currentLinear = currentLinear->forward[0];
        }
        target = currentLinear;
        
        if(target == nullptr) return;

        // store the nodes that before the target node 
        vector<Node*> update(maxLevel,nullptr);
        Node* current = head;

        for(int i= highLevel-1; i>= 0; i--)
        {
            while(current->forward[i] != nullptr && 
                 (current->forward[i]->score > target->score ||  (current->forward[i]->score == target->score && current->forward[i]->id < target->id))) 
            {
                current = current->forward[i];
            }
            update[i] = current;
        }

        if(update[0]->forward[0] != target)
        {
            // Player not found
            return;
        }

        // Reconnect links at each level
        for(int i=0; i< target->forward.size(); i++)
        {
            update[i]->forward[i] = target->forward[i];
        }

        // Delete the node
        delete target;

        // check if the top level is empty after deletion
        while(highLevel > 1 && head->forward[highLevel-1] == nullptr)
        {
            highLevel--;
        }
    }

    // Return top N player IDs in descending score order
    vector<int> getTopN(int n) override {
        vector<int> top_players;

        Node* temp = head->forward[0]; // level 0 contains all nodes so we don't need to traverse upper levels
        for(int i=0; i<n && temp!=nullptr ; i++)
        {
            top_players.push_back(temp->id);
            temp = temp->forward[0];
        }
        return top_players;
    }
};

// --- 3. AuctionTree (Red-Black Tree) ---
class ConcreteAuctionTree : public AuctionTree {
private:
    // Define your Red-Black Tree node structure
    // Each node needs: id, price, color, left, right, parent pointers
    enum Color{
        black,
        red
    };

    struct RedBlackNode{
        int ID;
        double price;
        Color color;         // color of node
        RedBlackNode* left;  // left child 
        RedBlackNode* right; // right child
        RedBlackNode* p;     // parent node

        RedBlackNode(int itemID, int price){
            this->ID = itemID;
            this->price = price;
        };
    };

    RedBlackNode* NIL;  // null pointer always black
    RedBlackNode* root; // root of the tree always black
public:
    ConcreteAuctionTree() {
        // Initialize your Red-Black Tree
        NIL = new RedBlackNode(NULL, NULL);
        NIL->color = black;
        NIL->left = NIL->right = NIL->p = NIL;
        root = NIL;
    }

    void insertItem(int itemID, int price) override {
        // Implement Red-Black Tree insertion

        // initialize node with given parameters
        RedBlackNode* x = new RedBlackNode(itemID, price);
        x->left = x->right = NIL;
        x->p = NIL;
        
        // Insert node using BST insertion
        root = bstInsert(root, x);
        
        // fix RB properties
        x->color = red;
        while(x != root && x->p->color == red){
            // if the node's parent is left therefore the x is in the left subtree
            if (x->p == x->p->p->left){
                RedBlackNode* y = x->p->p->right; // Uncle node
                if(y->color == red){  // Case 1 uncle is red
                    y->color = black;
                    x->p->color = black;
                    x->p->p->color = red;
                    x = x->p->p;
                } else { // Case 2 and Case 3 uncle is black (rotate)
                    // x is right child of parent (left rotate then right rotate)
                    if(x == x->p->right){
                        x = x->p;
                        leftRotate(x);
                    }
                    x->p->color = black;
                    x->p->p->color = red;
                    rightRotate(x->p->p);
                }
            } else {
                // if the node's parent is right therefore the x is in the right subtree
                RedBlackNode* y = x->p->p->left; // Uncle node
                if(y->color == red){  // Case 1 uncle is red
                    y->color = black;
                    x->p->color = black;
                    x->p->p->color = red;
                    x = x->p->p;
                } else { // Case 2 and Case 3 uncle is black (rotate)
                    // x is right child of parent (right rotate then left rotate)
                    if(x == x->p->left){
                        x = x->p;
                        rightRotate(x);
                    }
                    x->p->color = black;
                    x->p->p->color = red;
                    leftRotate(x->p->p);
                }
            }
        }
        root->color = black;
    }

    void deleteItem(int itemID) override {
        // Find the node to delete
        RedBlackNode* z = search(root, itemID);
        if (z == NIL) return; // Node not found
        
        RedBlackNode* x;
        RedBlackNode* y;
        
        // Find node y which is going to be removed
        if (z->left == NIL || z->right == NIL) {
            y = z;
        } else {
            y = successor(z);
            z->ID = y->ID;
            z->price = y->price;
        }
        
        // Find child x of y
        if (y->right != NIL) {
            x = y->right;
        } else {
            x = y->left;
        }
        
        // Note x might be NIL; create a pretend node
        if (x == NIL) {
            x = new RedBlackNode(0, 0);
            x->color = black;
            x->left = x->right = NIL;
        }
        
        // Detach x from y
        x->p = y->p;
        
        if (y->p == NIL) {
            root = x;
        } else {
            if (y == y->p->left) {
                y->p->left = x;
            } else {
                y->p->right = x;
            }
        }
        
        // Fix tree if removed node was black
        if (y->color == black) {
            while (x != root && x->color == black) {
                if (x == x->p->left) {
                    RedBlackNode* w = x->p->right;
                    
                    if (w->color == red) {
                        w->color = black;
                        x->p->color = red;
                        leftRotate(x->p);
                        w = x->p->right;
                    }
                    
                    if (w->left->color == black && w->right->color == black) {
                        w->color = red;
                        x = x->p;
                    } else {
                        if (w->right->color == black) {
                            w->left->color = black;
                            w->color = red;
                            rightRotate(w);
                            w = x->p->right;
                        }
                        w->color = x->p->color;
                        x->p->color = black;
                        w->right->color = black;
                        leftRotate(x->p);
                        x = root;
                    }
                } else {
                    RedBlackNode* w = x->p->left;
                    
                    if (w->color == red) {
                        w->color = black;
                        x->p->color = red;
                        rightRotate(x->p);
                        w = x->p->left;
                    }
                    
                    if (w->right->color == black && w->left->color == black) {
                        w->color = red;
                        x = x->p;
                    } else {
                        if (w->left->color == black) {
                            w->right->color = black;
                            w->color = red;
                            leftRotate(w);
                            w = x->p->left;
                        }
                        w->color = x->p->color;
                        x->p->color = black;
                        w->left->color = black;
                        rightRotate(x->p);
                        x = root;
                    }
                }
            }
            x->color = black;
        }
        
        // x is a pretend node (check if ID is 0)
        if (x->ID == 0) {
            if (x == x->p->left) {
                x->p->left = NIL;
            } else {
                x->p->right = NIL;
            }
            delete x;
        }
    }

    // --------- HELPER FUNCTIONS --------------
    // search for a node by ID
    RedBlackNode* search(RedBlackNode* node, int itemID) {
        if (node == NIL || node->ID == itemID) {
            return node;
        }
        if (itemID < node->ID) {
            return search(node->left, itemID);
        }
        return search(node->right, itemID);
    }
    
    // left rotate
    void leftRotate(RedBlackNode* x) {
        RedBlackNode* y = x->right;
        x->right = y->left;
        
        if (y->left != NIL) {
            y->left->p = x;
        }
        
        y->p = x->p;
        
        if (x->p == NIL) {
            root = y;
        } else if (x == x->p->left) {
            x->p->left = y;
        } else {
            x->p->right = y;
        }
        
        y->left = x;
        x->p = y;
    }
    
    // right rotate
    void rightRotate(RedBlackNode* y) {
        RedBlackNode* x = y->left;
        y->left = x->right;
        
        if (x->right != NIL) {
            x->right->p = y;
        }
        
        x->p = y->p;
        
        if (y->p == NIL) {
            root = x;
        } else if (y == y->p->left) {
            y->p->left = x;
        } else {
            y->p->right = x;
        }
        
        x->right = y;
        y->p = x;
    }

    // BST Insert
    RedBlackNode* bstInsert(RedBlackNode* node, RedBlackNode* newNode) {
        // If tree is empty or reached leaf position
        if (node == NIL) {
            return newNode;
        }
        
        // Recursively find position and insert
        if (newNode->price < node->price || (newNode->price == node->price && newNode->ID < node->ID)) {
            node->left = bstInsert(node->left, newNode);
            node->left->p = node;
        } else {
            node->right = bstInsert(node->right, newNode);
            node->right->p = node;
        }
        
        return node;
    }
    
    // find successor
    RedBlackNode* successor(RedBlackNode* z) {
        RedBlackNode* y = z->right;
        while (y->left != NIL) {
            y = y->left;
        }
        return y;
    }
    // ------------- END OF HELPER -------------------
};

// =========================================================
// PART B: INVENTORY SYSTEM (Dynamic Programming)
// =========================================================
int InventorySystem::optimizeLootSplit(int n, vector<int>& coins) {
    // TODO: Implement partition problem using DP
    // Goal: Minimize |sum(subset1) - sum(subset2)|
    // Hint: Use subset sum DP to find closest sum to total/2
    int totalSum = 0;
    for (int x : coins)
        totalSum += x;

    int halfCoins = totalSum / 2;

    vector<vector<int>> dp(n + 1, vector<int>(halfCoins + 1, 0));

    // Base cases
    dp[0][0] = 1;
    for (int i = 1; i <= n; i++)
        dp[i][0] = 1;


    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= halfCoins; j++) {
            if (j < coins[i - 1]) {
                dp[i][j] = dp[i - 1][j];
            } else {
                if (dp[i - 1][j] == 1 ||
                    dp[i - 1][j - coins[i - 1]] == 1)
                    dp[i][j] = 1;
                else
                    dp[i][j] = 0;
            }
        }
    }

    // Find the best sum near to half of total
    int best = 0;
    for (int i = halfCoins; i >= 0; i--) {
        if (dp[n][i] == 1) {             // to check if sum is possible using all coins
            best = i;
            break;
        }
    }

    return totalSum - 2 * best;
}

int InventorySystem::maximizeCarryValue(int capacity, vector<pair<int, int>>& items) {
    // TODO: Implement 0/1 Knapsack using DP
    // items = {weight, value} pairs
    // Return maximum value achievable within capacity

    if (items.empty() || capacity == 0) {
        return 0;
    }

    int n = items.size();

    vector<vector<int>> dp(n + 1, vector<int>(capacity + 1, 0));

    for (int i = 1; i <= n; i++) {
        int weight = items[i-1].first;
        int value = items[i-1].second;

        for (int W = 0; W <= capacity; W++) {
            if (weight > W) {
                // can not take it so become similar to the  cell above it
                dp[i][W] = dp[i-1][W];
            } else {
                // take the maximum [ the cell above it OR  (value + dp[W- weight] ) ]
                dp[i][W] = max(dp[i-1][W], value + dp[i-1][W - weight]);
            }
        }
    }

    return dp[n][capacity];
}

long long InventorySystem::countStringPossibilities(string s) {
    // TODO: Implement string decoding DP
    // Rules: "uu" can be decoded as "w" or "uu"
    //        "nn" can be decoded as "m" or "nn"
    // Count total possible decodings

    const int MOD = 1000000007;
    int n = s.length();

    for (char &character : s) {
        character = tolower(character);

    }

    //invalid input (keyboard is broken , cannot product 'w' pr 'm')
    for (char character : s) {
        if (character == 'w' || character == 'm') {
            return 0;
        }
    }

    //initialize all array element = 0 as initial values
    vector<long long> dp(n + 1, 0);

    //index 0 , 1 its values always 1
    dp[0] = 1;                   // this handles empty inputs (there is one way : do nothing)
    dp[1] = 1;                   // handle one character which has only one way : it self only

    for (int i = 2; i <= n; i++) {
        dp[i] = dp[i - 1];

        if ((s[i - 2] == 'u' && s[i - 1] == 'u') ||
            (s[i - 2] == 'n' && s[i - 1] == 'n')) {
            dp[i] = (dp[i] + dp[i - 2]) % MOD;
        }
    }

    return dp[n];
}

// =========================================================
// PART C: WORLD NAVIGATOR (Graphs)
// =========================================================
bool WorldNavigator::pathExists(int n, vector<vector<int>>& edges, int source, int dest) {
    // Implement path existence check using BFS or DFS
    if (source == dest)
        return true;
    // Build pairs list
    vector<vector<int>> pairs(n);
    for (auto& e : edges) {
        int u = e[0];
        int v = e[1];
        // make all pairs
        pairs[u].push_back(v);
        pairs[v].push_back(u);  // bidirectional
    }
    // BFS
    vector visited(n, false);
    queue<int> q;

    visited[source] = true;
    q.push(source);

    while (!q.empty())
    {
        int u = q.front();
        q.pop();
        for (auto& v : pairs[u])
        {
            if (!visited[v])
            {
                if (v == dest)
                    return true;
                q.push(v);
                visited[v] = true;
            }
        }
    }
    // edges are bidirectional
    return false;
}

struct Edge {
    int to;
    long long goldCost;
    long long silverCost;
};

long long WorldNavigator::minBribeCost(int n, int m, long long goldRate, long long silverRate, vector<vector<int>>& roadData) {
    // Implement Minimum Spanning Tree (Kruskal's or Prim's)
    // roadData[i] = {u, v, goldCost, silverCost}
    // Build adjacency list
    vector<vector<Edge>> adj(n);
    for (auto &r : roadData) {
        int u = r[0];
        int v = r[1];
        long long goldCost = r[2];
        long long silverCost = r[3];
        // bidirectional
        adj[u].push_back({v, goldCost, silverCost});
        adj[v].push_back({u, goldCost, silverCost});
    }

    // Min-heap: (cost, vertex)
    using P = pair<long long, int>; // cost, vertex
    priority_queue<P, vector<P>, greater<P>> pq;

    vector inMST(n, false);
    vector minCost(n, LLONG_MAX);
    vector<long long> minGold(n, 0);
    vector<long long> minSilver(n, 0);

    minCost[0] = 0;
    pq.push({0, 0});

    long long totalGold = 0, totalSilver = 0;
    int count = 0;

    while (!pq.empty()) {
        auto [cost, u] = pq.top();
        pq.pop();
        if (inMST[u])
            continue;

        inMST[u] = true;
        totalGold += minGold[u];
        totalSilver += minSilver[u];
        count++;

        for (auto &e : adj[u]) {
            int v = e.to;
            long long edgeCost = e.goldCost * goldRate + e.silverCost * silverRate;

            if (!inMST[v] && edgeCost < minCost[v]) {
                minCost[v] = edgeCost;
                minGold[v] = e.goldCost;
                minSilver[v] = e.silverCost;
                pq.push({edgeCost, v});
            }
        }
    }

    if (count != n)
        return -1;
    return totalGold * goldRate + totalSilver * silverRate;
    // Total cost = goldCost * goldRate + silverCost * silverRate
    // Return -1 if graph cannot be fully connected
}

string decimalToBinary(long long num){
    if (num == 0)
        return "0";
    string bits = "";
    while (num > 0){
        int remainder = num % 2;
        bits = (remainder ? '1' : '0') + bits;
        num = num / 2;
    }
    return bits;
}

string binaryAdd(string a, string b){
    int n = a.size(), m = b.size();
    // Pad shorter string with zeros
    if (n < m) a = string(m-n, '0') + a;
    if (m < n) b = string(n-m, '0') + b;

    string result = "";
    int carry = 0;
    for (int i = a.size()-1; i >= 0; i--){
        int bitA = a[i]-'0';
        int bitB = b[i]-'0';
        int sum = bitA + bitB + carry;
        result = char((sum % 2) + '0') + result;
        carry = sum / 2;
    }
    if (carry) result = '1' + result;
    return result;
}

string WorldNavigator::sumMinDistancesBinary(int n, vector<vector<int>>& roads) {
    // Implement All-Pairs Shortest Path (Floyd-Warshall)
    // Sum all shortest distances between unique pairs (i < j)
    long long dist[n][n];
    string sumBinary = "0";
    for (int i = 0 ; i < n; i++)
    {
        for (int j = 0 ; j < n; j++)
        {
            if (i == j)
                dist[i][j] = 0;
            else
                dist [i][j] = LLONG_MAX;
        }
    }
    for (auto road : roads)
    {
        int u = road[0];
        int v = road[1];
        int distance = road[2];
        dist[u][v] = distance;
        dist[v][u] = distance;
    }
    for (int k = 0 ; k < n; k++)
    {
        for (int i = 0 ; i < n; i++)
        {
            for (int j = 0 ; j < n ; j++)
            {
                if (dist[i][k] != LLONG_MAX && dist[k][j] != LLONG_MAX)
                {
                    dist[i][j] = min(dist[i][j] , dist[i][k] + dist[k][j]);
                }
            }
        }
    }
    for (int i = 0 ; i < n; i++)
    {
        for (int j = 0 ; j < i; j++)
        {
            if (dist[i][j] != LLONG_MAX )
            {
                string bin = decimalToBinary(dist[i][j]);
                sumBinary = binaryAdd(sumBinary, bin);
            }
        }
    }
    // Return the sum as a binary string
    // Hint: Handle large numbers carefully
    return sumBinary;
}

// =========================================================
// PART D: SERVER KERNEL (Greedy)
// =========================================================
int ServerKernel::minIntervals(vector<char>& tasks, int n) {
    // Implement task scheduler with cooling time
    // Same task must wait 'n' intervals before running again
    // Return minimum total intervals needed (including idle time)
    // Hint: Use greedy approach with frequency counting
    priority_queue<int> maxHeap;
    queue<pair<int, int>> q;
    vector<int> freq(26, 0);
    // Create a frequency array for each character by its Ascii code
    for (char c : tasks) {
        freq[toupper(c) - 'A']++;
    }
    for (int f : freq) {
        if (f > 0) maxHeap.push(f);
    }
    int time = 0;
    while (!maxHeap.empty() || !q.empty()) {
        time++;
        if (!maxHeap.empty()) {
            int remainingCount = maxHeap.top() - 1;
            maxHeap.pop();
            if (remainingCount > 0) {
                q.push({remainingCount, time + n});
            }
        }
        if (!q.empty() && q.front().second == time) {
            maxHeap.push(q.front().first);
            q.pop();
        }
    }
    return time;
}

// =========================================================
// FACTORY FUNCTIONS (Required for Testing)
// =========================================================
extern "C" {
    PlayerTable* createPlayerTable() { 
        return new ConcretePlayerTable(); 
    }

    Leaderboard* createLeaderboard() { 
        return new ConcreteLeaderboard(); 
    }

    AuctionTree* createAuctionTree() { 
        return new ConcreteAuctionTree(); 
    }
}
