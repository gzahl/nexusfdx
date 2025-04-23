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

    TEST_ASSERT_NOT_NULL(state.message);
    TEST_ASSERT_EQUAL_STRING("BSP", state.message->values[0].label);
    TEST_ASSERT_EQUAL_STRING("kn", state.message->values[0].unit);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 4.00f, state.message->values[0].calculate(state.data));
}

// Test: Temperature reading of 16°C
void test_temp_16()
{
    reader.processMessage(0b00001000, 1); // Header = 8
    reader.processMessage(0b00010000);     // Data = 16h (16°C)
    MessageState state = reader.processMessage(0b00011000); // Checksum (XOR)

    TEST_ASSERT_TRUE(state.isComplete);
    TEST_ASSERT_EQUAL(8, state.fdxNr);
    TEST_ASSERT_EQUAL(3, state.expectedLength);
    TEST_ASSERT_EQUAL(16, state.data[0]);

    TEST_ASSERT_NOT_NULL(state.message);
    TEST_ASSERT_EQUAL_STRING("TMP", state.message->values[0].label);
    TEST_ASSERT_EQUAL_STRING("°C", state.message->values[0].unit);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 16.0f, state.message->values[0].calculate(state.data));
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

    TEST_ASSERT_NOT_NULL(state.message);
    TEST_ASSERT_EQUAL_STRING("BAT", state.message->values[0].label);
    TEST_ASSERT_EQUAL_STRING("V", state.message->values[0].unit);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 12.6f, state.message->values[0].calculate(state.data));
}

// Test: Depth reading of 2.40m
void test_depth_240()
{
    reader.processMessage(0b00000111, 1);  // Header = 7 
    reader.processMessage(0b11110000);     // Data[0] = F0 hex (240 decimal)
    reader.processMessage(0b00000000);     // Data[1] = High byte
    reader.processMessage(0b00000000);     // Data[2] = Alarm byte
    MessageState state = reader.processMessage(0b11110111); // Checksum

    TEST_ASSERT_TRUE(state.isComplete);
    TEST_ASSERT_EQUAL(7, state.fdxNr);
    TEST_ASSERT_EQUAL(5, state.expectedLength);
    TEST_ASSERT_EQUAL(0xF0, state.data[0]);
    TEST_ASSERT_EQUAL(0x00, state.data[1]);
    TEST_ASSERT_EQUAL(0x00, state.data[2]);

    TEST_ASSERT_NOT_NULL(state.message);
    TEST_ASSERT_EQUAL_STRING("DBT", state.message->values[0].label);
    TEST_ASSERT_EQUAL_STRING("m", state.message->values[0].unit);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.40f, state.message->values[0].calculate(state.data));
}

// Test: BSP reading of 2.51 kts
void test_bsp_251()
{
    reader.processMessage(0b00000000, 1);  // Header = 0
    reader.processMessage(0b11111011);      // Low byte = FB hex
    reader.processMessage(0b00000000);      // High byte
    MessageState state = reader.processMessage(0b11111011); // Checksum = 0 XOR 0 XOR FB

    TEST_ASSERT_TRUE(state.isComplete);
    TEST_ASSERT_EQUAL(0, state.fdxNr);
    TEST_ASSERT_EQUAL(4, state.expectedLength);
    TEST_ASSERT_EQUAL(0xFB, state.data[0]);
    TEST_ASSERT_EQUAL(0x00, state.data[1]);
    
    TEST_ASSERT_NOT_NULL(state.message);
    TEST_ASSERT_EQUAL_STRING("BSP", state.message->values[0].label);
    TEST_ASSERT_EQUAL_STRING("kn", state.message->values[0].unit);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.51f, state.message->values[0].calculate(state.data));
}

// Test: AWS and AWA reading
void test_aws_awa()
{
    reader.processMessage(0b00000001, 1);  // Header = 1
    reader.processMessage(0b01100100);     // AWS data byte 1 (100)
    reader.processMessage(0b00000000);     // AWS data byte 2 (0)
    reader.processMessage(0b11110100);     // AWA data byte 1 (244)
    reader.processMessage(0b00000001);     // AWA data byte 2 (1)
    MessageState state = reader.processMessage(0b10010000); // Checksum = 1 XOR 100 XOR 0 XOR 244 XOR 1

    TEST_ASSERT_TRUE(state.isComplete);
    TEST_ASSERT_EQUAL(1, state.fdxNr);
    TEST_ASSERT_EQUAL(6, state.expectedLength);

    TEST_ASSERT_NOT_NULL(state.message);
    TEST_ASSERT_EQUAL_STRING("AWS", state.message->values[0].label);
    TEST_ASSERT_EQUAL_STRING("m/s", state.message->values[0].unit);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, state.message->values[0].calculate(state.data));

    TEST_ASSERT_EQUAL_STRING("AWA", state.message->values[1].label);
    TEST_ASSERT_EQUAL_STRING("°", state.message->values[1].unit);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.74f, state.message->values[1].calculate(state.data));
}

// Test: TWS and TWA reading
void test_tws_twa()
{
    reader.processMessage(0b00010010, 1);  // Header = 18
    reader.processMessage(0b11001000);     // TWS data byte 1 (200)
    reader.processMessage(0b00000000);     // TWS data byte 2 (0)
    reader.processMessage(0b00000000);     // TWA data byte 1 (0)
    reader.processMessage(0b00000001);     // TWA data byte 2 (1)
    MessageState state = reader.processMessage(0b11011011); // Checksum = 18 XOR 200 XOR 0 XOR 0 XOR 1

    TEST_ASSERT_TRUE(state.isComplete);
    TEST_ASSERT_EQUAL(18, state.fdxNr);
    TEST_ASSERT_EQUAL(6, state.expectedLength);

    TEST_ASSERT_NOT_NULL(state.message);
    TEST_ASSERT_EQUAL_STRING("TWS", state.message->values[0].label);
    TEST_ASSERT_EQUAL_STRING("m/s", state.message->values[0].unit);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.0f, state.message->values[0].calculate(state.data));

    TEST_ASSERT_EQUAL_STRING("TWA", state.message->values[1].label);
    TEST_ASSERT_EQUAL_STRING("°", state.message->values[1].unit);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.41f, state.message->values[1].calculate(state.data));
}

// Test: SOG and COG reading
void test_sog_cog()
{
    reader.processMessage(0b00100001, 1);  // Header = 33
    reader.processMessage(0b01010110);     // SOG data byte 1 (86)
    reader.processMessage(0b00000000);     // SOG data byte 2 (0)
    reader.processMessage(0b10010000);     // COG data byte 1 (144)
    reader.processMessage(0b00000000);     // COG data byte 2 (0)
    MessageState state = reader.processMessage(0b11000011); // Checksum = 33 XOR 86 XOR 0 XOR 144 XOR 0

    TEST_ASSERT_TRUE(state.isComplete);
    TEST_ASSERT_EQUAL(33, state.fdxNr);
    TEST_ASSERT_EQUAL(6, state.expectedLength);

    TEST_ASSERT_NOT_NULL(state.message);
    TEST_ASSERT_EQUAL_STRING("SOG", state.message->values[0].label);
    TEST_ASSERT_EQUAL_STRING("kn", state.message->values[0].unit);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.86f, state.message->values[0].calculate(state.data));

    TEST_ASSERT_EQUAL_STRING("COG", state.message->values[1].label);
    TEST_ASSERT_EQUAL_STRING("°", state.message->values[1].unit);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.79f, state.message->values[1].calculate(state.data));
}

int main() {
    UNITY_BEGIN();
    
    RUN_TEST(test_valid_message_incomplete);
    RUN_TEST(test_valid_message_complete);
    RUN_TEST(test_unknown_fdxnr);
    RUN_TEST(test_bsp_400);
    RUN_TEST(test_temp_16);
    RUN_TEST(test_bat_126);
    RUN_TEST(test_depth_240);
    RUN_TEST(test_bsp_251);
    RUN_TEST(test_aws_awa);
    RUN_TEST(test_tws_twa);
    RUN_TEST(test_sog_cog);
    
    return UNITY_END();
}