#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gain_control.h"
#include "noise_suppression.h"
#include "ns_core.h"
#include "front.h"

ZeusFront::ZeusFront()
{
    sampleHz = 16000;
    frameSh    = 160;
    nBands     = 1;
    frameMs    = 10;
    agc        = NULL;
    agcIn      = NULL;
    agcOut     = NULL;
    ns         = NULL;
    nsIn       = NULL;
    nsOut      = NULL;
    temp       = NULL;
}

ZeusFront::~ZeusFront()
{
    if(agc != NULL)
    {
        WebRtcAgc_Free(agc);
        agc = NULL;
    }
    if(ns != NULL)
    {
        WebRtcNs_Free((NsHandle*)ns);
        ns = NULL;
    }
    if(temp)
    {
        free(temp);
        temp = NULL;
    }
    FreeBuffer((void**)agcIn);
    FreeBuffer((void**)agcOut);
    FreeBuffer((void**)nsIn);
    FreeBuffer((void**)nsOut);
}


/*
 *input:
 *    sampleHz 采样率，只支持8k/16k/32k/48k
 *    bit      位数，只支持16位
 *    agcDb    增益倍数，0~31，越大声音越大，一般设为20
 *    agcDbfs  相对于full scale的db，0~31，越小声音越大，一般设为3
 *output:
 *    int      0-success，1-fail
 */
int ZeusFront::FrontInit(int sampleHzIn, int bit, int agcDb, int agcDbfs, int nsMode)
{
    if(nBands < 1 || nBands > 2)
    {
        fprintf(stderr, "[FrontInit]: only support nBands 1 or 2\n");
        return -1;
    }
    if(bit != 16)
    {
        fprintf(stderr, "[FrontInit]: only support 16bit pcm\n");
        return -1;

    }
    sampleHz = sampleHzIn;
    switch(sampleHz)
    {
        case 8000:
        case 16000:
        case 32000:
        case 48000:
            frameSh = sampleHz * ((float)frameMs / 1000);
            break;
        default:
            fprintf(stderr, "[FrontInit]: only support sampleHz 8k 16k 32k 48k\n");
            return -1;
    }
    if(AgcInit(agcDb, agcDbfs) != 0)
    {
        return -1;
    }
    if(NsInit(nsMode) != 0)
    {
        return -1;
    }
    return 0;
}


int ZeusFront::AgcInit(int agcDb, int agcDbfs)
{
    int iBand, status;
    if(agcDbfs < 0 || agcDbfs > 31 || agcDb < 0 || agcDb > 31)
    {
        fprintf(stderr, "[AgcInit]: agcDb and agcDbfs should be 0~31\n");
        return -1;
    }
    agc = WebRtcAgc_Create();
    int minLevel = 0;
    int maxLevel = 255;
    int agcMode  = kAgcModeFixedDigital;
    status = WebRtcAgc_Init(agc, minLevel, maxLevel, agcMode, sampleHz);
    if(status != 0)
    {
        fprintf(stderr, "[AgcInit]: failed in WebRtcAgc_Init\n");
        return -1;
    }
    WebRtcAgcConfig agcConfig;
    agcConfig.limiterEnable = 1;
    agcConfig.compressionGaindB = agcDb;   //在Fixed模式下，越大声音越大
    agcConfig.targetLevelDbfs = agcDbfs;   //dbfs表示相对于full scale的下降值，0表示full scale，越小声音越大
    status = WebRtcAgc_set_config(agc, agcConfig);
    if(status != 0)
    {
        fprintf(stderr, "[AgcInit]: failed in WebRtcAgc_set_config\n");
        return -1;
    }
    agcIn  = (short**)malloc(nBands*sizeof(short*));
	agcOut = (short**)malloc(nBands*sizeof(short*));
    for(iBand = 0; iBand < nBands; iBand++)
    {
	    agcIn [iBand] = (short*)malloc(frameSh*sizeof(short));
    	agcOut[iBand] = (short*)malloc(frameSh*sizeof(short));
    }
    return 0;
}


int ZeusFront::NsInit(int nsMode)
{
    int status, iBand;
    if(nsMode < 0 || nsMode > 3)
    {
        fprintf(stderr, "[NsInit]: only support nsMode 0 1 2 3\n");
        return -1;
    }
	ns = WebRtcNs_Create();
	status = WebRtcNs_Init((NsHandle*)ns, sampleHz);
	if(status != 0)
    {
		fprintf(stderr, "[NsInit]: failed in WebRtcNs_Init\n");
		return -1;
	}
	status = WebRtcNs_set_policy((NsHandle*)ns, nsMode);
	if(status != 0)
    {
		fprintf(stderr, "[NsInit]: failed in WebRtcNs_set_policy\n");
		return -1;
	}
    temp  = (short *)malloc(BUFFERSIZE*sizeof(short));
    nsIn  = (float**)malloc(nBands*sizeof(float*));
	nsOut = (float**)malloc(nBands*sizeof(float*));
    for(iBand = 0; iBand < nBands; iBand++)
    {
	    nsIn[iBand]  = (float*)malloc(frameSh*sizeof(float));
    	nsOut[iBand] = (float*)malloc(frameSh*sizeof(float));
    }
    return 0;
}


/*
 *input:
 *    input    输入语音数据
 *    pcmLen   输入语音数据包含short的个数
 *output:
 *    output   首先经过增益agc，然后经过ns，得到的语音数据
 *    int      0-success，1-fail
 */

int ZeusFront::FrontProc(short *input, short *output, int pcmLen)
{
    if(input == NULL || output == NULL || pcmLen < 0 || pcmLen > BUFFERSIZE)
    {
		fprintf(stderr, "[FrontProc]: failed in input params, atmost 1M short\n");
		return -1;
    }
    ResetBuffer();
    if(AgcProc(input, pcmLen) != 0)
    {
		fprintf(stderr, "[FrontProc]: failed in AgcProc\n");
		return -1;
    }
    if(NsProc(output, pcmLen) != 0)
    {
		fprintf(stderr, "[FrontProc]: failed in NsProc\n");
		return -1;
    }
    return 0;
}

int ZeusFront::AgcProc(short *input, int pcmLen)
{
    int iFrame, status;
    int nFrames = pcmLen / frameSh;               //帧数
    int leftLen = pcmLen % frameSh;               //最后一帧大小
    int onceLen = frameSh;                        //一帧大小
    nFrames = (leftLen > 0) ? nFrames+1 : nFrames;
    int micLevelIn  = 0;                          //麦克风输入级别，将上一次的输出级别作为本次的输入级别
    int micLevelOut = 0;                          //麦克风的输出级别
    uint8_t saturationWarning;                    //是否有溢出发生，增益放大以后的最大值超过了65536
    int echo = 0;                                 //增益放大是否考虑回声影响
    for(iFrame = 0; iFrame < nFrames; iFrame++)
    {
        if(iFrame == nFrames-1 && leftLen != 0)
        {
            onceLen = leftLen;
        }
        memcpy(agcIn[0], input+iFrame*frameSh, onceLen*sizeof(short));
        status = WebRtcAgc_Process(agc, agcIn, nBands, frameSh, agcOut, micLevelIn, &micLevelOut, echo, &saturationWarning);
        if(status != 0)
        {
		    fprintf(stderr, "[AgcProc]: failed in WebRtcAgc_Process\n");
		    return -1;
        }
        if(saturationWarning != 0)
        {
            fprintf(stdout, "[AgcProc]: saturationWarning occured\n");
        }
        //fprintf(stdout, "%d\n", agcOut[0][iFrame*frameSh]);
        memcpy(temp+iFrame*frameSh, agcOut[0], onceLen*sizeof(short));
        micLevelIn = micLevelOut;
    }
    return 0;
}

int ZeusFront::NsProc(short *output, int pcmLen)
{
    int iFrame, iShort;
    int nFrames = pcmLen / frameSh;               //帧数
    int leftLen = pcmLen % frameSh;               //最后一帧大小
    int onceLen = frameSh;                        //一帧大小
    nFrames = (leftLen > 0) ? nFrames+1 : nFrames;
    for(iFrame = 0; iFrame < nFrames; iFrame++)
    {
        if(iFrame == nFrames-1 && leftLen != 0)
        {
            onceLen = leftLen;
        }
        for(iShort = 0; iShort < onceLen; iShort++)
        {
            nsIn[0][iShort] = (float)temp[iFrame*frameSh+iShort];
        }
        WebRtcNs_AnalyzeCore((NoiseSuppressionC*)ns, nsIn[0]);
        WebRtcNs_Process((NsHandle*)ns, nsIn, nBands, nsOut);
        for(iShort = 0; iShort < onceLen; iShort++)
        {
            output[iFrame*frameSh+iShort] = (short)nsOut[0][iShort];
        }
    }
    return 0;

}


void ZeusFront::ResetBuffer()
{
    int iBand;
    for(iBand = 0; iBand < nBands; iBand++)
    {
        memset(agcIn [iBand], 0, frameSh*sizeof(short));
        memset(agcOut[iBand], 0, frameSh*sizeof(short));
        memset(nsIn  [iBand], 0, frameSh*sizeof(float));
        memset(nsOut [iBand], 0, frameSh*sizeof(float));
    }
    memset(temp, 0, BUFFERSIZE*sizeof(short));
}

void ZeusFront::FreeBuffer(void **buffer)
{
    if(buffer != NULL)
    {
        for(int iBand = 0; iBand < nBands; iBand++)
        {
            if(buffer[iBand] != NULL)
            {
                free(buffer[iBand]);
                buffer[iBand] = NULL;
            }
        }
        free(buffer);
        buffer = NULL;
    }
}
    
