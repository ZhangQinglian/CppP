//
//  GIFParser.hpp
//  cpp_p
//
//  Created by Scott Zhang on 2017/11/7.
//  Copyright © 2017年 Scott Zhang. All rights reserved.
//

#ifndef GIFParser_hpp
#define GIFParser_hpp

#include <iostream>
#include <bitset>
#include <vector>
#include <jni.h>
using namespace std;

//////////////////////////////// Logical Screen Descriptor
class LSD {
private:
    char *lsdBuffer;
public:
    LSD(char*);
    ~LSD();
    //图像宽
    int getScreenWidth();
    //图像高度
    int getScreenHeigth();
    //压缩字节
    bitset<8> getCompressByte();
    //背景色索引
    int getBgColorIndex();
    //图像宽高比
    int getRadio();
    //获得全局颜色表的长度
    size_t getGlobalColorTableSize();
    //是否存在全局颜色表
    bool isGCT();
    void dump();
};

///////////////////////////////// Global color table
class GCT
{
private:
    unsigned char *gct;
    size_t count;
public :
    GCT(unsigned char*,size_t);
    ~GCT();
    void dump();
    unsigned int getColorIntAt(unsigned int);
};
///////////////////////////////// Data Sub-blocks
class DataSubBlocks
{
    private :
    int size;
    char *data;
public:
    DataSubBlocks(char*,int);
    void dump();
};
///////////////////////////////// Application Extension
class AppExt
{
private:
    char *appIdentifier;
    char *appAuthenticationCode;
    DataSubBlocks *subBlocks;
public:
    void setIdentifier(char*);
    void setAuthenticationCode(char*);
    void setSubData(DataSubBlocks*);
    void dump();
};
///////////////////////////////// Graphic control extension
class GraphicControlExt
{
private:
    bitset<8> *compressByte;
    int delayTime = 0;
    unsigned int transparentColorIndex = 0;
public :
    GraphicControlExt(bitset<8>*,int,unsigned int);
    unsigned int getTransparentInde();
    bool shouldSkip();
    void dump();
    void release();
};
///////////////////////////////// Image Descriptor
class ImageDescriptor
{
private:
    unsigned int left = 0;
    unsigned int top = 0;
    unsigned int w = 0;
    unsigned int h = 0;
    bitset<8> *compressByte;
public:
    ImageDescriptor(unsigned int,unsigned int,unsigned int,unsigned int,bitset<8>*);
    bool needLocalColorTable();
    void dump();
    unsigned int getLeft();
    unsigned int getTop();
    unsigned int getW();
    unsigned int getH();
    void release();
};
///////////////////////////////// Image Data Descriptor (本程序自定义，并非gif协议中的类型)
class ImageDataDes
{
private:
    unsigned long gceStart = 0;
    unsigned long gceStop = 0;
    unsigned long imageDataStart = 0;
    unsigned long imageDataStop = 0;
public:
    ImageDataDes(unsigned long, unsigned long, unsigned long, unsigned long);
    void dump();
    unsigned long getGceStart();
    unsigned long getGceStop();
    unsigned long getImageDataStart();
    unsigned long getImageDataStop();
};
////////////////////////////// idd struct
struct IddStruct{
    ImageDescriptor *imageDescriptor;
    vector<unsigned int > *bitmapArray;
};
////////////////////////////// Frame
struct OneFrame{
    GraphicControlExt *graphicControlExt;
    IddStruct *iddStruct;
};
///////////////////////////////// Parser
class GIFParser
{
public:
    GIFParser(char *buffer,long size);
    ~GIFParser();
    void init();
    char* getSignature();
    char* getVersion();
    void putIdds(unsigned long, unsigned long, unsigned long, unsigned long);
    unsigned long getIddSize();
    OneFrame* seekIddsTo(unsigned long);
    int getW();
    int getH();
    int parseBinary (char const * const);
    GCT *getGCT();
private:
    char *buffer;
    long gifSize;
    unsigned long seek ;
    //逻辑屏幕描述符
    char signature[4] = {0};
    char version[4] = {0};
    LSD *lsd;
    GCT *gct;
    vector<ImageDataDes> idds;
    GraphicControlExt* getGce(ImageDataDes&);
    IddStruct* getIddStruct(ImageDataDes&);
};

#endif /* GIFParser_hpp */
