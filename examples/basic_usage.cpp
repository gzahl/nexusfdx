#include <FdxReader.h>
#include <iostream>

/**
 * This example demonstrates how to use the NexusFDX FdxReader
 * to decode messages from a Nexus/NX2 network.
 * 
 * The FdxReader handles 9-bit messages where the 9th bit (parity)
 * is used to mark the first byte of a message. Each message consists of:
 * - Header byte (with parity bit set)
 * - Data bytes
 * - Checksum byte
 */

// Helper function to print message details
void printMessageDetails(const MessageState& state) {
    std::cout << "Message FDX Nr: " << state.fdxNr << "\n";
    std::cout << "Message complete: " << (state.isComplete ? "yes" : "no") << "\n";
    std::cout << "Message valid: " << (state.isValid() ? "yes" : "no") << "\n";
    if (state.isComplete && state.isValid()) {
        std::cout << "Data bytes: ";
        for (int i = 0; i < state.currentSize; i++) {
            printf("0x%02X ", state.data[i]);
        }
        std::cout << "\n";
    }
    std::cout << "-------------------\n";
}

int main() {
    // Create a message processor instance
    FdxReader reader;
    MessageState state;

    // Example 1: Process a BSP (Boat Speed) message (FDX Nr 0)
    // Format: [Header][Data1][Data2][Checksum]
    // Example speed: 4.00 knots = 400 (144 + 256 * 1)
    std::cout << "Example 1: Processing BSP message (4.00 knots)\n";
    reader.processMessage(0b00000000, 1);  // Header with parity=1
    reader.processMessage(0b10010000);     // Data byte 1 (0x90 = 144)
    reader.processMessage(0b00000001);     // Data byte 2 (0x01)
    state = reader.processMessage(0b10010001);     // Checksum
    printMessageDetails(state);

    // Example 2: Process a Temperature message (FDX Nr 8)
    // Format: [Header][Data][Checksum]
    // Example: 22°C = 0x16
    std::cout << "\nExample 2: Processing Temperature message (22°C)\n";
    reader.processMessage(0b00001000, 1);  // Header (FDX 8) with parity
    reader.processMessage(0b00010110);     // Data (0x16 = 22°C)
    state = reader.processMessage(0b00011110);     // Checksum
    printMessageDetails(state);

    // Example 3: Process an invalid message
    std::cout << "\nExample 3: Processing invalid message\n";
    state = reader.processMessage(0b01111111, 1);  // Unknown FDX number (127)
    printMessageDetails(state);

    return 0;
}