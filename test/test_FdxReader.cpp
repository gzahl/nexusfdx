#include <unity.h>
#include "FdxReader.h"

// Global instance of FdxReader for testing
FdxReader reader;

void setUp(void) {
    // Reset reader state before each test
    reader = FdxReader();
}

void tearDown(void) {
    // No cleanup needed
}

// Test: Valid Message - Incomplete
void test_valid_message_incomplete()
{
    int msg9_Rx = 0b10000000; // Example message with incomplete data
    MessageState state = reader.processMessage(msg9_Rx);

    TEST_ASSERT_FALSE(state.isComplete); // Message should be incomplete
}

// Test: Valid Message - Complete
void test_valid_message_complete()
{
    // Message with FDXnr=0 (BSP) which expects 4 bytes total
    reader.processMessage(0b00000000, 1); // Header with parity=1
    reader.processMessage(0b00000001);    // Data byte 1
    reader.processMessage(0b00000010);    // Data byte 2
    MessageState state = reader.processMessage(0b00000011); // Checksum

    TEST_ASSERT_TRUE(state.isComplete); 
    TEST_ASSERT_EQUAL(0, state.fdxNr);
    TEST_ASSERT_EQUAL(4, state.expectedLength);
}

// Test: Unknown FDXnr
void test_unknown_fdxnr()
{
    int msg9_Rx = 0b01111111; // FDXnr 127
    MessageState state = reader.processMessage(msg9_Rx, 1);

    TEST_ASSERT_FALSE(state.isValid());
    TEST_ASSERT_EQUAL(127, state.fdxNr);
}

// Test: BSP according to bitorder document
void test_bsp_400()
{
    // Example 4.00 kts, *100 -> 400 = 144 + 256 * 1
    reader.processMessage(0b00000000, 1);
    reader.processMessage(0b10010000); // Data byte 1
    reader.processMessage(0b00000001); // Data byte 2
    MessageState state = reader.processMessage(0b10010001); // Checksum

    TEST_ASSERT_TRUE(state.isComplete);
    TEST_ASSERT_EQUAL(0, state.fdxNr);
    TEST_ASSERT_EQUAL(4, state.expectedLength);
    TEST_ASSERT_EQUAL(144, state.data[0]);
    TEST_ASSERT_EQUAL(1, state.data[1]);
}

// Test: Temperature reading of 22°C
void test_temp_22()
{
    reader.processMessage(0b00001000, 1); // Header = 8
    reader.processMessage(0b00010000);     // Data = 16h (22°C)
    MessageState state = reader.processMessage(0b00011000); // Checksum (XOR)

    TEST_ASSERT_TRUE(state.isComplete);
    TEST_ASSERT_EQUAL(8, state.fdxNr);
    TEST_ASSERT_EQUAL(3, state.expectedLength);
    TEST_ASSERT_EQUAL(16, state.data[0]);
}

// Test: Battery voltage reading of 12.6V
void test_bat_126()
{
    reader.processMessage(0b00001001, 1);  // Header = 9
    reader.processMessage(0b01111110);      // Data = 7E hex (12.6V * 10)
    MessageState state = reader.processMessage(0b01110111); // Checksum (XOR)

    TEST_ASSERT_TRUE(state.isComplete);
    TEST_ASSERT_EQUAL(9, state.fdxNr);
    TEST_ASSERT_EQUAL(3, state.expectedLength);
    TEST_ASSERT_EQUAL(0x7E, state.data[0]);
}

// Test: Depth reading of 2.40m
void test_depth_240()
{
    reader.processMessage(0b00000111, 1);  // Header = 7 
    reader.processMessage(0b00000000);     // Data[0] = High byte
    reader.processMessage(0b11110000);     // Data[1] = F0 hex (240 decimal)
    reader.processMessage(0b00000000);     // Data[2] = Alarm byte
    MessageState state = reader.processMessage(0b11110111); // Checksum

    TEST_ASSERT_TRUE(state.isComplete);
    TEST_ASSERT_EQUAL(7, state.fdxNr);
    TEST_ASSERT_EQUAL(5, state.expectedLength);
    TEST_ASSERT_EQUAL(0x00, state.data[0]);
    TEST_ASSERT_EQUAL(0xF0, state.data[1]);
    TEST_ASSERT_EQUAL(0x00, state.data[2]);
}

// Test: BSP reading of 2.51 kts
void test_bsp_251()
{
    reader.processMessage(0b00000000, 1);  // Header = 0
    reader.processMessage(0b00000000);      // High byte
    reader.processMessage(0b11111011);      // Low byte = FB hex
    MessageState state = reader.processMessage(0b00000000); // Checksum

    TEST_ASSERT_TRUE(state.isComplete);
    TEST_ASSERT_EQUAL(0, state.fdxNr);
    TEST_ASSERT_EQUAL(4, state.expectedLength);
    TEST_ASSERT_EQUAL(0x00, state.data[0]);
    TEST_ASSERT_EQUAL(0xFB, state.data[1]);
}

int main() {
    UNITY_BEGIN();
    
    RUN_TEST(test_valid_message_incomplete);
    RUN_TEST(test_valid_message_complete);
    RUN_TEST(test_unknown_fdxnr);
    RUN_TEST(test_bsp_400);
    RUN_TEST(test_temp_22);
    RUN_TEST(test_bat_126);
    RUN_TEST(test_depth_240);
    RUN_TEST(test_bsp_251);
    
    return UNITY_END();
}