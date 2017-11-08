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
    int getGlobalColorTableSize();
    //是否存在全局颜色表
    bool isGCT();
    void dump();
};

///////////////////////////////// Global color table
class GCT
{
private:
    char *gct;
    int count;
public :
    GCT(char*,int);
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
    int transparentColorIndex = 0;
public :
    GraphicControlExt(bitset<8>*,int,int);
    void dump();
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
    vector<unsigned int > bitmapArray;
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
    void seekIddsTo(unsigned long);
private:
    char *buffer;
    long gifSize;
    unsigned long seek ;
    //逻辑屏幕描述符
    char signature[4] = {0};
    char version[4] = {0};
    LSD *lsd;
    GCT *gct;
    AppExt *appExt;
    GraphicControlExt *currentGCE;
    ImageDescriptor *currentImageDescriptor;
    vector<ImageDataDes> idds;
    GraphicControlExt* getGce(ImageDataDes&);
    IddStruct* getIddStruct(ImageDataDes&);
};

#endif /* GIFParser_hpp */
