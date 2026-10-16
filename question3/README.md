# Server Dependency Network Analyzer

## Introduction
This application systematically models structural relationships between distinct servers within a computer network. It represents servers as **vertices** and directed dependencies as **edges**. Graphs are ideally suited for representing network structures because they mathematically model distinct, independent entities alongside the custom relational connections between them.

---

## Design and Implementation

### Data Structure & Memory Architecture
* **Entity Storage**: Server names are held inside a dynamically allocated tracking array.
* **Relationship Layout**: Inter-server dependencies are represented via a dynamically allocated **Adjacency Matrix**, where individual entry states indicate whether one server relies on another.
* **Directional Logic**: Dependencies are explicitly directed. A relationship mapped from Server A to Server B does not inherently imply or create a reverse dependency from Server B to Server A.

### Operations & Capabilities
* Adding and removing servers dynamically.
* Instantiating and deleting specific dependency configurations.
* Displaying the structural adjacency matrix visually.
* Querying incoming and outgoing dependencies per vertex.

### Validation & Memory Safety
* **Input Hardening**: All server names undergo formal normalization and validation checks prior to executing matrix calculations.
* **Transactional Allocations**: When a new server is added, the application allocates separate replacement storage for the server array and the matrix before committing the structural change. This transactional approach prevents the active graph from being left in a corrupted or partially updated state if an allocation fails.
* **Matrix Compacting**: Removing a target server cleanly deletes its associated entry in the server array while shifting and compacting rows/columns in the adjacency matrix to maintain structural continuity.

---

## Testing

The following scenarios were systematically executed to validate network analyzer performance:

| Test Case | Expected Result |
| :--- | :--- |
| Display initial graph | Seeded baseline servers and dependencies render accurately in the matrix |
| Add server | The server is successfully appended and the matrix structure expands cleanly |
| Create dependency | The corresponding directed relationship is precisely recorded in the intersecting cell |
| Delete dependency | The specified edge is removed without altering unrelated matrix states |
| Delete server | The target server and all associated incoming/outgoing dependency edges are purged |
| Enter an invalid/duplicate server name | The system intercepts the input and the operation is rejected |

---

## Complexity, Limitations, and Practical Application

### Algorithmic Complexity
For a graph containing **V** servers:
* **Space Complexity**: Requires O(V²) memory footprint to hold the structural grid.
* **Edge Verification**: Finding whether an edge exists operates at an optimal O(1) lookup time.
* **Dependency Queries**: Mapping all incoming or outgoing connections takes linear O(V) time.
* **Vertex Mutability**: Adding or removing a server scales at O(V²) work because the underlying matrix must be reallocated, copied, or structurally compacted.

### System Limitations
* **Memory Inefficiency**: The adjacency matrix architecture consumes significant, rigid memory reserves when representing large, sparsely connected networks where most cells remain zero.

### Practical Application Example
This analytical system can be deployed by IT infrastructure teams managing complex, multi-tiered application environments (such as an online shopping platform):

```text
[DEPENDENCY CHAIN EXAMPLE]
 Web Server ──(depends on)──> Application Server ──(depends on)──> Database Server
```

If the primary database server experiences an unscheduled outage, the analyzer instantly empowers administrators to identify every upstream service affected by that failure. This clear visual mapping supports maintenance scheduling, disaster recovery drills, and risk assessments prior to server upgrades.

