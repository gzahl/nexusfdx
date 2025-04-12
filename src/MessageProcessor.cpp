#include "MessageProcessor.h"


// Define the lookup table for FDX messages
const FdxMessage MessageProcessor::FdxMessages[] = {
    {0, 4, "BSP", "kn", [](const int *data)
     { return float((data[1] + 256 * data[2])) / 100; }},
    {1, 6, "AWS", "m/s", [](const int *data)
     { return float((data[1] + 256 * data[2])) / 100; }},
    {2, 5, "HDC", "°", [](const int *data)
     { return float((data[1] + 256 * data[2])) / 182.044; }},
    {4, 5, "Unknown", "", nullptr},
    {7, 5, "DBT", "m", [](const int *data)
     { return float((data[1] + 256 * data[2])) / 100; }},
    {8, 3, "TMP", "°C", [](const int *data)
     { return float(data[1]); }},
    {9, 3, "BAT", "V", [](const int *data)
     { return float(data[1]) / 10; }},
    {17, 4, "VMG", "kn", nullptr},
    {18, 6, "TWS", "m/s", nullptr},
    {32, 10, "Lat/Long", "", nullptr},
    {33, 6, "SOG/COG", "kn/°", nullptr},
    {34, 9, "BTW/CTS/DTW", "°/°/nm", nullptr},
    {35, 7, "BOD/XTE", "°/nm", nullptr},
    {98, 4, "TWD", "°", nullptr}};
const int MessageProcessor::FdxMessagesSize = sizeof(FdxMessages) / sizeof(FdxMessages[0]);

const FdxMessage* MessageProcessor::findMessage(uint8_t fdxNr) const {
    for (int i = 0; i < FdxMessagesSize; ++i) {
        if (FdxMessages[i].nr == fdxNr) {
            return &FdxMessages[i];
        }
    }
    return nullptr;
}

MessageProcessor::MessageProcessor() : state() {}

MessageState MessageProcessor::processMessage(int msg8, int parity) {
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

bool MessageProcessor::isHeaderByte(int msg8, bool hasParity) const {
    return hasParity && !(msg8 & BIT7_MASK);
}

bool MessageProcessor::isNewSenderByte(int msg8) const {
    return msg8 & BIT7_MASK;
}

MessageState MessageProcessor::processHeader(int msg8) {
    state = MessageState();
    state.fdxNr = msg8 & 0x7F;

    const FdxMessage* msg = findMessage(state.fdxNr);
    if (msg) {
        state = MessageState(state.fdxNr, msg->len);
        return state;
    }

    state.markInvalid();
    return state;
}

void MessageProcessor::processNewSender(int msg8) {
    state = MessageState();
    printf("New Sender ID: %d\n", msg8 & 0x7F);
}

void MessageProcessor::processData(int msg8) {
    if (!state.isValid() || !state.addData(msg8)) return;
    
    if (state.hasExpectedLength()) {
        state.isComplete = true;
        handleFDXData();
    }
}

void MessageProcessor::handleFDXData() {
    const FdxMessage* msg = findMessage(state.fdxNr);
    if (!msg || !msg->calculate) {
        printf("Unknown or unsupported FDXnr: %d\n", state.fdxNr);
        return;
    }

    float value = msg->calculate(state.data);
    printf("%s: %.2f %s\n", msg->label, value, msg->unit);
}