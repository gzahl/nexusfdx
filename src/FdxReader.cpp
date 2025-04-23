#include "FdxReader.h"
#include "FdxUtils.h"
#include "FdxMessages.h"

FdxReader::FdxReader() : state() {
}

const FdxMessage* FdxReader::findMessage(uint8_t fdxNr) const {
    auto it = FdxMessages::messageMap.find(fdxNr);
    return it != FdxMessages::messageMap.end() ? &it->second : nullptr;
}

MessageState FdxReader::processMessage(int msg8, int parity) {
    const bool hasParity = (parity == 1);

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
    const FdxMessage* message = findMessage(fdxNr);
    
    if (message) {
        state = MessageState(fdxNr, message->len);
        state.message = message;
        return state;
    }

    state = MessageState();
    state.fdxNr = fdxNr;
    state.markInvalid();
    return state;
}

void FdxReader::processNewSender(int msg8) {
    state.reset();
    printf("New Sender ID: %d\n", msg8 & 0x7F);
}

void FdxReader::processData(int msg8) {
    if (!state.isValid() || !state.addData(msg8)) return;
    
    if (state.hasExpectedLength()) {
        state.isComplete = true;
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