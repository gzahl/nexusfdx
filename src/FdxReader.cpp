#include "FdxReader.h"

// Define the lookup table for FDX messages
const FdxMessage FdxReader::FdxMessages[] = {
    {0, 4, {
        {"BSP", "kn", [](const int *data) { 
            return float((data[0] + 256 * data[1])) / 100; 
        }}
    }},
    {1, 6, {
        {"AWS", "m/s", [](const int *data) { 
            return float((data[0] + 256 * data[1])) / 100; 
        }},
        {"AWA", "°", [](const int *data) {
            float awa = float((data[2] + 256 * data[3])) * FdxReader::DEGREES_SCALE;
            return awa > 180 ? -(360 - awa) : awa;
        }}
    }},
    {2, 5, {
        {"HDC", "°", [](const int *data) { 
            return float((data[0] + 256 * data[1])) * FdxReader::DEGREES_SCALE; 
        }}
    }},
    {7, 5, {
        {"DBT", "m", [](const int *data) { 
            return float((data[0] + 256 * data[1])) / 100; 
        }}
    }},
    {8, 3, {
        {"TMP", "°C", [](const int *data) { 
            return float(data[0]); 
        }}
    }},
    {9, 3, {
        {"BAT", "V", [](const int *data) { 
            return float(data[0]) / 10; 
        }}
    }},
    {17, 4, {
        {"VMG", "kn", [](const int *data) {
            int vmg_100 = data[0] + 256 * data[1];
            if (vmg_100 > 32767) {
                return -float((vmg_100 ^ 65535) + 1) / 100;
            }
            return float(vmg_100 + 1) / 100;
        }}
    }},
    {18, 6, {
        {"TWS", "m/s", [](const int *data) { 
            return float((data[0] + 256 * data[1])) / 100; 
        }},
        {"TWA", "°", [](const int *data) {
            float twa = float((data[2] + 256 * data[3])) * FdxReader::DEGREES_SCALE;
            return twa > 180 ? -(360 - twa) : twa;
        }}
    }},
    {33, 6, {
        {"SOG", "kn", [](const int *data) { 
            return float((data[0] + 256 * data[1])) / 100; 
        }},
        {"COG", "°", [](const int *data) { 
            return float((data[2] + 256 * data[3])) * FdxReader::DEGREES_SCALE; 
        }}
    }},
    {34, 9, {
        {"BTW", "°", [](const int *data) { 
            return float((data[0] + 256 * data[1])) * FdxReader::DEGREES_SCALE; 
        }},
        {"DTW", "nm", [](const int *data) { 
            return float((data[4] + 256 * data[5] + 256 * 256 * data[6])) / 1000; 
        }}
    }},
    {98, 4, {
        {"TWD", "°", [](const int *data) { 
            return float((data[0] + 256 * data[1])) * FdxReader::DEGREES_SCALE; 
        }}
    }}
};

const int FdxReader::FdxMessagesSize = sizeof(FdxMessages) / sizeof(FdxMessages[0]);

FdxReader::FdxReader() : state() {}

const FdxMessage* FdxReader::findMessage(uint8_t fdxNr) const {
    for (int i = 0; i < FdxMessagesSize; ++i) {
        if (FdxMessages[i].nr == fdxNr) {
            return &FdxMessages[i];
        }
    }
    return nullptr;
}

MessageState FdxReader::processMessage(int msg8, int parity) {
    const bool hasParity = (parity == 1);

    printf("processMessage: msg8=%d, parity=%d\n", msg8, parity);

    if (isHeaderByte(msg8, hasParity)) {
        return processHeader(msg8);
    }

    // Not a header byte - process as data if we have an active state
    if (state.expectedLength > 0) {
        processData(msg8);
        
        if (state.hasExpectedLength()) {
            MessageState result = state;
            state = MessageState();
            result.isComplete = true;
            return result;
        }
    }

    return state;
}

bool FdxReader::isHeaderByte(int msg8, bool hasParity) const {
    return hasParity && !(msg8 & BIT7_MASK);
}

bool FdxReader::isNewSenderByte(int msg8) const {
    return msg8 & BIT7_MASK;
}

MessageState FdxReader::processHeader(int msg8) {
    uint8_t fdxNr = msg8 & 0x7F;
    
    // We'll look up the actual message later, for now just check if we know the length
    for (int i = 0; i < FdxMessagesSize; ++i) {
        if (FdxMessages[i].nr == fdxNr) {
            state = MessageState(fdxNr, FdxMessages[i].len);
            return state;
        }
    }

    state = MessageState();
    state.fdxNr = fdxNr;
    state.markInvalid();
    return state;
}

void FdxReader::processNewSender(int msg8) {
    state = MessageState();
    printf("New Sender ID: %d\n", msg8 & 0x7F);
}

void FdxReader::processData(int msg8) {
    if (!state.isValid() || !state.addData(msg8)) return;
    
    if (state.hasExpectedLength()) {
        state.isComplete = true;
        // Look up the message definition now that we have all data
        state.message = findMessage(state.fdxNr);
        handleFDXData();
    }
}

void FdxReader::handleFDXData() {
    if (!state.message || state.message->values.empty()) {
        printf("Unknown or unsupported FDXnr: %d\n", state.fdxNr);
        return;
    }

    for (const auto& value : state.message->values) {
        if (value.calculate) {
            float result = value.calculate(state.data);
            printf("%s: %.2f %s\n", value.label, result, value.unit);
        }
    }
}