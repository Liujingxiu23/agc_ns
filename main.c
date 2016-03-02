/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2015年12月03日 16时29分36秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "front.h"
#define MAXSTRLEN 1024
#if 0
int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "Usage: <in_filelist>\n");
        return -1;
    }
	char *fileIn  = argv[1];
    ZeusFront front;
    int status;
    status = front.FrontInit(16000, 16, 20, 3, 2);
    if(status != 0)
    {
        fprintf(stderr, "failed to init\n");
        return -1;
    }
    FILE *listFp  = fopen(fileIn,"r");
    if(listFp == NULL)
    {
        fprintf(stderr, "failed to open pcm\n");
        return -1;
    }
    char pcmFile[MAXSTRLEN];
    char newPcmFile[MAXSTRLEN];
    int  tempSize = 5*1024;
    short *in  = (short*)calloc(tempSize, sizeof(short));
    short *out = (short*)calloc(tempSize, sizeof(short));
    while ((fgets (pcmFile, MAXSTRLEN, listFp)) != NULL)
    {
        if(0 == strcmp(pcmFile, "\n"))
        {
            memset(pcmFile, 0, MAXSTRLEN);
            continue;
        }
        int len = strlen(pcmFile);
        if(('\n' == pcmFile[len-1]) && (len > 1))
        {
            pcmFile[len-1] = '\0';
            strcpy(newPcmFile, pcmFile);
            strcat(newPcmFile, ".new");
        }
        FILE *inFp  = fopen(pcmFile,"r");
        FILE *outFp = fopen(newPcmFile,"w");
        if(inFp == NULL || outFp == NULL)
        {
            fprintf(stderr, "failed to open pcm\n");
            return -1;
        }
        int pcmLen = tempSize;
        while(pcmLen > 0)
        {
            pcmLen = fread(in, sizeof(short), tempSize, inFp);
            front.FrontProc(in, out, pcmLen);
            pcmLen = fwrite(out, sizeof(short), pcmLen, outFp);
        }
        fclose(inFp);
        fclose(outFp);
        memset(pcmFile, 0, MAXSTRLEN);
        memset(newPcmFile, 0, MAXSTRLEN);
    }
    fclose(listFp);
    free(in);
    free(out);
    return 0;
}

#else

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        fprintf(stderr, "Usage: <in> <out>\n");
        return -1;
    }
	char *fileIn  = argv[1];
	char *fileOut = argv[2];
    ZeusFront front;
    int status;
    status = front.FrontInit(16000, 16, 20, 3, 3);
    if(status != 0)
    {
        fprintf(stderr, "failed to init\n");
        return -1;
    }
	FILE *inFp  = fopen(fileIn,"r");
    FILE *outFp = fopen(fileOut,"w");
    if(inFp == NULL || outFp == NULL)
    {
        fprintf(stderr, "failed to open pcm\n");
        return -1;
    }
    int tempSize = 5*1024;
    short *in  = (short*)calloc(tempSize, sizeof(short));
    short *out = (short*)calloc(tempSize, sizeof(short));
    int pcmLen = tempSize;
    while(pcmLen > 0)
    {
        pcmLen = fread(in, sizeof(short), tempSize, inFp);
        front.FrontProc(in, out, pcmLen);
        pcmLen = fwrite(out, sizeof(short), pcmLen, outFp);
    }
    fclose(inFp);
    fclose(outFp);
    free(in);
    free(out);
    return 0;
}
#endif
