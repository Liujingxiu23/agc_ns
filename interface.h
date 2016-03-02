#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 *function:
 *    创建一个语音预处理实体
 *input:
 *    空
 *output:
 *    pointer  后续操作需要传入该指针
 */
void *createFront();

/*
 *function:
 *    初始化语音预处理模块
 *input:
 *    front    createFront产生的指针
 *    sampleHz 采样率，只支持8k/16k/32k/48k
 *    bit      位数，只支持16位
 *    agcDb    增益倍数，0~31，越大声音越大，一般设为20
 *    agcDbfs  相对于full scale的db，0~31，越小声音越大，一般设为3
 *    ndMode   噪音消除模式，0 1 2 3，一般设置成2
 *output:
 *    int      0 success，-1 fail
 */
int initialFront(void *front, int sampleHz, int bit, int agcDb, int agcDbfs, int nsMode);

/*
 *function:
 *    使用语音预处理模块对语音进行处理
 *input:
 *    front    createFront产生的指针
 *    input    输入语音数据
 *    output   输入语音数据(必须预先分配好空间)
 *    pcmLen   输入语音数据包含short的个数
 *output:
 *    output   首先经过增益agc，然后经过ns，得到的语音数据
 *    int      0 success，-1 fail
 */
int processFront(void *front, short *input, short *output, int pcmLen);

/*
 *function:
 *    释放预处理空间
 *input:
 *    front    createFront产生的指针
 *output:
 *    int      0 success，-1 fail
 */
int destroyFront(void *&front);

#ifdef __cplusplus
}
#endif

#endif
