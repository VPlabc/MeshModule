#include <Arduino.h>

#pragma pack(push, 1)
struct HistoryRecord {
    uint32_t timestamp;  // 4 bytes (Unix timestamp)
    uint16_t user_id;    // 2 bytes (tối đa 65,535 user)
    uint8_t action_type; // 1 byte (0: login, 1: logout, etc.)
    uint8_t reserved;    // 1 byte (dự phòng)
};
#pragma pack(pop)

#define MAX_HISTORY 2000 // 2000 bản ghi = 16KB

class HistoryStorage {
private:
    HistoryRecord records[MAX_HISTORY];
    int head = 0;
    int count = 0;
    
public:
    void addRecord(uint16_t user_id, uint8_t action) {
        records[head] = {
            .timestamp = (uint32_t)time(nullptr),
            .user_id = user_id,
            .action_type = action
        };
        
        head = (head + 1) % MAX_HISTORY;
        if(count < MAX_HISTORY) count++;
    }

    void getBinaryData(uint8_t* buffer) {
        int start = (count == MAX_HISTORY) ? head : 0;
        for(int i = 0; i < count; i++) {
            int idx = (start + i) % MAX_HISTORY;
            memcpy(buffer + (i * sizeof(HistoryRecord)), 
                   &records[idx], 
                   sizeof(HistoryRecord));
        }
    }
};


