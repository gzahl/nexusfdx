#ifndef MESSAGE_PROCESSOR_H
#define MESSAGE_PROCESSOR_H

#include <cstdio>
#include <functional>
#include <cstdint>

enum class MessageValidity {
    Valid,
    Unknown,
    Invalid
};

struct MessageState {
    static constexpr uint8_t MAX_DATA_SIZE = 24;

    MessageState() = default;
    explicit MessageState(uint8_t fdxNr, uint8_t expectedLength) 
        : fdxNr(fdxNr), expectedLength(expectedLength) {}

    int fdxNr = 0;
    int expectedLength = 0;
    int currentSize = 0;
    int data[MAX_DATA_SIZE] = {0};
    MessageValidity validity = MessageValidity::Valid;
    bool isComplete = false;

    bool isValid() const { return validity == MessageValidity::Valid; }
    void markInvalid() { validity = MessageValidity::Invalid; }
    
    bool addData(uint8_t byte) {
        if (currentSize >= MAX_DATA_SIZE) return false;
        data[currentSize++] = byte;
        return true;
    }
    
    bool hasExpectedLength() const { 
        return currentSize == expectedLength - 1; 
    }
};

struct FdxMessage {
    int nr;
    int len;
    const char *label;
    const char *unit;
    std::function<float(const int *)> calculate;
};

class MessageProcessor {
public:
    MessageProcessor();
    MessageState processMessage(int msg9_Rx, int parity = 0);

private:
    static const int PARITY_MASK = 0x100;
    static const int BIT7_MASK = 0x80;

    MessageState state;
    
    bool isHeaderByte(int msg8, bool hasParity) const;
    bool isNewSenderByte(int msg8) const;
    MessageState processHeader(int msg8);
    void processNewSender(int msg8);
    void processData(int msg8);
    void handleFDXData();

    static const FdxMessage FdxMessages[];
    static const int FdxMessagesSize;

    const FdxMessage* findMessage(uint8_t fdxNr) const;
};

#endif