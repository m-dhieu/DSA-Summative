# Network Device Command Registry

## Introduction
This application systematically stores, searches, displays, counts, and manages network device configuration commands. It organizes command strings utilizing a **Binary Search Tree (BST)**, demonstrating how hierarchical, tree-based data structures inherently support ordered data management and optimized lookup capabilities.

---

## Design and Implementation

### Data Structure & Tree Mechanics
* **Node Properties**: Commands are stored inside dynamically allocated tree nodes. Each node retains a dedicated command string alongside pointers to its `left` and `right` children.
* **Ordering Logic**: The BST arranges commands utilizing standard string ordering rules, facilitating highly efficient lookup times when the tree layout remains reasonably balanced.

### Core Capabilities
* Standard commands registration and status verification.
* In-order sorted display and cumulative node counting.
* Full memory cleanup (clearing the registry) and data serialization.
* Strict enforcement of a **50-command maximum capacity limit**.

### Error Mitigation & Security
* **Input Conditioning**: The system screens for, intercepts, and handles invalid, duplicate, overlong, or excess array entries.
* **Spelling Suggestions**: The application integrates the **Levenshtein edit-distance algorithm** to suggest alternative options when an unknown command is supplied. This metric calculates the minimum structural insertions, deletions, or substitutions needed to convert one string into another. Matches are filtered to an edit distance of **two or less**.
* **Audit Trail**: Unrecognized commands are caught and recorded sequentially in `audit.log`.
* **Atomic Persistence**: Data saves to `commands.txt` by writing to a temporary file before triggering a filesystem replacement to protect against partial file corruptions.

---

## Testing

The following scenarios were systematically executed to validate registry stability:

| Test Case | Expected Result |
| :--- | :--- |
| Register valid command | Command is successfully inserted into the BST |
| Register duplicate command | Duplicate entry is rejected immediately |
| Verify registered command | System confirms that the command exists in the tree |
| Verify unknown command with similar spelling | Closest match suggestion is displayed if within an edit distance of \(\le 2\) |
| Display and count commands | Commands are outputted in alphabetical (in-order) sequence with an accurate total count |
| Save and reload registry | Saved commands are pulled back into memory seamlessly after restarting the application |

---

## Limitations and Practical Application

### Known Constraints
* **Tree Imbalance**: In scenarios where the BST becomes severely unbalanced, lookup, insertion, and deletion metrics degrade from an ideal logarithmic scale down to linear time complexity, approaching \(O(n)\) instead of \(O(\log n)\).
* **Log Contamination**: The `audit.log` parser structure could break or misalign if the initial user input contains the exact internal delimiter characters utilized by the log file generator.

### Practical Application Example
Network administrators can deploy this platform to maintain a strict, verified lookup repository of approved core router and switch operations:

```text
Approved Command Base Examples:
  ├── show-interfaces
  ├── check-connection
  └── configure-dns

Usage Scenario:
If a technician inputs a misspelled variant of a command during high-stress 
network troubleshooting, the underlying Levenshtein engine automatically 
intercepts it, checks the threshold, and prints out the closest safe 
registered alternative—drastically reducing manual configuration errors.
```

