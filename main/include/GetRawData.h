#include <stdint.h>
#define EEG_MAX_CHANNEL 3
class GetRawData {
   private:
    int SEND_PERIOD = 5;
    // msg Siganl value
    int msgSig = 0;

   public:
    void DataSort(uint8_t raw_data[], int protocol_mode);
    int getEegCh1Data();
    int getEegCh2Data();
    int getPpgData();
    int batteryData();
    double* getAccData();
    double* getGyroData();
    int* getTempData();
    int getPriodMsgSignal();
    void setMsgPriod(int sec);
    void setPriodMsgSignal(int msg);
};