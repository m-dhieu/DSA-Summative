# Embedded Firmware Log Compression Utility

## Introduction
This application compresses and decompresses device log files utilizing **lossless Huffman coding**. Huffman coding is a foundational compression algorithm that optimally assigns shorter binary codes to highly frequent symbols and longer codes to less frequent ones. This makes the platform ideally suited for demonstrating how constrained embedded systems can drastically reduce the physical storage footprint required for diagnostic logs.

---

## Design and Implementation

### Tree Construction & Coding Mechanics
* **Frequency Mapping**: The utility instantiates a comprehensive frequency table containing 256 entries to map out every possible distinct byte value.
* **Min-Heap Pipeline**: A min-heap structure is leveraged to dynamically construct the Huffman tree by repeatedly extracting and merging the two active nodes possessing the lowest relative frequencies.
* **Code Generation**: The application traverses the finalized binary tree topology to generate variable-length bit configurations for each tracked symbol.

### Compression Architecture
During compression, the engine reads the source log file, aggregates symbol frequencies, constructs the tree, and writes out the encoded bitstream to `device_log.huff`. 

The structured archive header layout is strictly defined as follows:
* **Magic Identifier**: A 4-byte `HUF1` signature block at the front.
* **Size Tracker**: An 8-byte field recording the exact original file size.
* **Frequency Table**: A 256-entry array allocating 8 bytes per entry.
* **Header Overhead**: The fixed header consumes exactly **2,060 bytes** of space before the compressed payload stream begins.

### Decompression & Data Integrity
During decompression, the engine parses and validates the archive header, recreates the exact Huffman tree layout, and decodes the payload to `device_log_restored.txt`. 

To completely insulate the platform against data corruption or truncated archives, it performs the following routines:
* **Payload Verification**: The decoded output is cross-checked against the expected original file size metrics and symbol distribution frequencies.
* **Byte-for-Byte Validation**: A direct byte-level verification check is triggered against the source file.
* **Memory Safety**: All dynamically allocated memory structures for tree nodes and processing buffers are explicitly freed to prevent leaks.

---

## Testing

The following scenarios were systematically tested to confirm the stability of the compression engine:

| Test Case | Expected Result |
| :--- | :--- |
| Compress normal device log | Huffman archive is generated and compression statistics display accurately |
| Decompress valid archive | The restored log file matches the original source file exactly |
| Compress file containing one repeated character | System gracefully handles single-symbol edge-case tree structures |
| Use missing input file | File access constraint is handled safely without crashing |
| Decompress truncated or corrupted archive | Defective file states are intercepted and a clean failure is reported |
| Verify successfully restored file | Byte-for-byte validation confirms files are identical |

---

## Complexity, Limitations, and Practical Application

### Algorithmic Complexity
Where **n** represents the input file size and **k** equals the count of unique symbols (k ≤ 256):
* **Tree Building**: Constructing the Huffman tree layout runs in O(k log k) time via the tracking min-heap structure.
* **Data Processing**: Reading, encoding, and decoding execution metrics scale proportionally to file size, with minor processing variances depending on final bitstring lengths.

### System Limitations
* **Memory Scaling**: The present architecture buffers operational datasets inside memory, causing its RAM footprint to expand relative to file size.
* **Header Overhead**: The mandatory 2,060-byte static header can outscale and negate the compression benefits when processing small log targets.
* **Tool Incompatibility**: The engine outputs a proprietary custom archive format, making it incompatible with standard consumer decompression suites.
* **Partial Output**: Critical runtime decompression aborts may leave orphaned, partially written files behind in the system directory.

### Practical Application Example
This utility is an exceptional choice for firmware diagnostics, low-level telemetry logging, and archival configurations where maintaining absolute bit-level data integrity is required:

```text
[SCENARIO: Flash Storage Optimization]
A remote temperature sensor captures thousands of repetitive status updates. 
Using this Huffman coding tool, it can heavily compress the text streams down 
prior to writing them to scarce physical flash memory chips or transferring 
them over narrow bandwidth channels to a technician's terminal.
```

