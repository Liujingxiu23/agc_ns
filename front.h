#ifndef _FRONT_H_
#define _FRONT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define BUFFERSIZE  (1024*1024)
class ZeusFront{
    public:
        ZeusFront();
        ~ZeusFront();
        int FrontInit(int sampleHz, int bit, int agcDb, int agcDbfs, int nsMode);
        int FrontProc(short *input, short *output, int pcmLen);
    private:
        int AgcInit(int agcDb, int agcDbfs);
        int AgcProc(short *input, int pcmLen);
        int NsInit(int nsMode);
        int NsProc(short *output, int pcmLen);
        void ResetBuffer();
        void FreeBuffer(void **buffer);
        int sampleHz;   //语音采样率
        int frameMs;    //一帧语音长度，以毫秒为单位
        int frameSh;    //一帧short的个数， 默认一帧10ms
        int nBands;     //num of band, including high and low, we only use low band
        // agc part
        void* agc;      //agc instance
        short **agcIn;  //agc input[band][data]
        short **agcOut; //agc output[band][data]
        // ns part
        void* ns;       //ns instance
        float **nsIn;   //ns input[band][data]
        float **nsOut;  //ns output[band][data]
        // temp result
        short *temp;    //temp: used to store result from last step
};

#ifdef __cplusplus
}
#endif

#endif
