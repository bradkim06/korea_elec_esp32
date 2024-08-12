#include <stdint.h>

class LeadOff {
   private:
    int send_msg = 0;
    int checkCapOpenTime = 1, checkFallDownTime = 5;

   public:
    void LeadOffData(uint8_t raw_data[], int protocol_mode);
    int getIsCapOpen();
    bool getIsFallDown();
    int getMsgSignal();
    void setMsgSignal(int send_msg);
    void setTime(int checkCapOpenTime, int checkFallDownTime);
};