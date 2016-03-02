/*
 * =====================================================================================
 *
 *       Filename:  interface.c
 *
 *    Description:  interface for libso
 *
 *        Version:  1.0
 *        Created:  2015年12月03日 18时56分26秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "front.h"
#include "interface.h"

void *createFront()
{
    ZeusFront *front = new ZeusFront();
    return front;
}

int initialFront(void *front, int sampleHz, int bit, int agcDb, int agcDbfs, int nsMode)
{
    if(front == NULL)
    {
        fprintf(stderr, "[initialFront]: input front pointer is null\n");
        return -1;
    }
    ZeusFront *inst = (ZeusFront*)front;
    int status = 0;
    status = inst->FrontInit(sampleHz, bit, agcDb, agcDbfs, nsMode);
    if(status != 0)
    {
        fprintf(stderr, "[initialFront]: failed to init\n");
        return -1;
    }
    return 0;
}

int processFront(void *front, short *input, short *output, int pcmLen)
{
    if(front == NULL)
    {
        fprintf(stderr, "[processFront]: input front pointer is null\n");
        return -1;
    }
    ZeusFront *inst = (ZeusFront*)front;
    int status = inst->FrontProc(input, output, pcmLen);
    if(status != 0)
    {
        fprintf(stderr, "[processFront]: failed to init\n");
        return -1;
    }
    return 0;
}

int destroyFront(void *&front)
{
    if(front == NULL)
    {
        fprintf(stderr, "[destroyFront]: input front pointer is null\n");
        return -1;
    }
    ZeusFront *inst = (ZeusFront*)front;
    delete inst;
    front = NULL;
    return 0;
}
