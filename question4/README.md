# Emergency Medical Supply Routing System

## Introduction
This application calculates the absolute shortest route between medical facilities and regional supply hubs. It models physical locations as **vertices** and the traversing distances between them as **weighted edges**. The platform implements **Dijkstra’s algorithm** to efficiently isolate the minimum-distance path between a selected starting point and destination.

---

## Design and Implementation

### Network Architecture
The system features a small, predefined network consisting of six distinct locations:
* Central Warehouse
* Distribution Center
* District Hospital
* Health Center A
* Health Center B
* Health Center C

### Data Structure & Mechanics
* **Graph Layout**: A weighted **Adjacency Matrix** represents the structural distances between connected locations. 
* **Symmetry**: Because roads are modeled as bidirectional pathways, the resulting layout matrix is mathematically symmetric.
* **Path Reconstruction**: To trace and return the exact optimal path, the Dijkstra engine systematically tracks and maintains an estimated distance array, a visited-state tracking table, and a predecessor index log.
* **Data Guarding**: Full graph structures and individual edge weights are rigorously validated before executing any routing calculations.

### Core Capabilities
* Comprehensive display of all available tracking locations.
* Dynamic route optimization lookup.
* On-demand viewing of the total distance of the last successfully calculated path.

---

## Testing

The following scenarios were systematically executed to validate the routing engine's logic:

| Test Case | Expected Result |
| :--- | :--- |
| Central Warehouse to District Hospital Route | Identifies direct 14km path as the shortest route |
| Central Warehouse to Health Center C Route | Identifies a 17km path with the sequence: Central Warehouse → Distribution Center → Health Center A → Health Center C |
| Route from location to itself | Returns a distance of 0km instantly |
| Enter invalid location | The input is caught and rejected by the validator |
| Request route between valid locations | Path sequence and cumulative total distance are cleanly displayed |

---

## Complexity, Limitations, and Practical Application

### Algorithmic Complexity
For a network consisting of **V** locations:
* **Space Complexity**: Requires O(V²) memory space to hold the static adjacency layout grid.
* **Time Complexity**: The matrix-based implementation of Dijkstra’s algorithm runs in O(V²) time.
* **Auxiliary Memory**: Requires O(V) auxiliary space to handle the processing distance, visited-state tracking, and predecessor indexing arrays.

### System Limitations
* **Static Routing**: The system models fixed, unyielding distances. It does not factor in real-time traffic delays, emergency road closures, changing weather/travel conditions, or specialized vehicle size/weight restrictions.

### Practical Application Example
Given its small, predefined structure, this application is perfectly suited for demonstrating shortest-path algorithm mechanics and illustrating how critical medical supplies can be rapidly routed between warehouses, regional distribution centers, and frontline healthcare facilities during logistics planning.

