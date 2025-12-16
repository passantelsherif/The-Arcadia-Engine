# Arcadia Engine

## Overview
This project involves implementing the backend engine for an MMORPG called "Arcadia Engine," as part of the CS321 - Algorithms Analysis and Design course at Cairo University. The engine is divided into four subsystems, each focusing on specific algorithms and data structures.

## Project Structure
The engine consists of the following subsystems:

1. **The Registry** – Handles player lookups and leaderboards using hashing and skip lists.
2. **The Inventory System** – Manages loot distribution and storage using dynamic programming.
3. **The Navigator** – Manages travel routes and safe paths using graph algorithms.
4. **The Kernel** – Manages server CPU tasks using greedy algorithms.

## Implementation Guidelines

### File Structure
- **ArcadiaEngine.h** – **Do not modify** this file under any circumstances. Changes will result in a grade of zero.
- **ArcadiaEngine.cpp** – Implement all required functions and helper structures here.
- **main_test_student.cpp** – Provided for basic testing. Passing this does not guarantee full marks.

### Part A: The Registry (Data Structures)
1. **Player Lookup (Hash Table)**
   - Fixed size: 101
   - Collision resolution: Double Hashing
   - No dynamic resizing allowed
   - Throw "Table is full" if full after probing

2. **Live Leaderboard (Skip List)**
   - Sorted by score (descending)
   - Tie-breaking: Player ID (ascending)
   - `removePlayer` may use \(O(N)\) scan + \(O(\log N)\) deletion
   - `getTopN` must be efficient

3. **Auction House (Red-Black Tree)**
   - Ordered by price
   - Duplicate prices allowed; use composite key (price, ID)
   - No STL containers allowed for internal mapping
   - `deleteItem` may use \(O(N)\) traversal + \(O(\log N)\) deletion

### Part B: The Inventory System (Dynamic Programming)
1. **Loot Splitting (Partition Problem)** – Minimize difference between two sums.
2. **Inventory Packer (Knapsack)** – Maximize value within weight capacity.
3. **Chat Autocorrect (String DP)** – Count possible original strings modulo \(10^9 + 7\).

### Part C: The Navigator (Graph Algorithms)
1. **Safe Passage (Path Existence)** – Check connectivity between two cities.
2. **The Bribe (MST Variant)** – Minimum cost to connect all cities.
3. **The Teleporter Network (All-Pairs Shortest Path)** – Sum of shortest distances in binary.

### Part D: The Kernel (Greedy Algorithms)
- **Job Scheduler** – Minimum CPU intervals with cooling time constraints.

## Expected Complexities
- Hash table: \(O(1)\) average
- Skip list: \(O(\log n)\)
- Red-Black tree: \(O(\log n)\)
- Knapsack: \(O(n \times W)\)
- Partition: \(O(n \times \text{sum})\)
- Path existence: \(O(V + E)\)
- MST: \(O(E \log V)\)
- APSP: \(O(V^3)\)
- Scheduler: \(O(n \log n)\)DATA STRUCTURES
