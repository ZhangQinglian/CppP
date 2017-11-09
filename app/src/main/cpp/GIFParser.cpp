//
//  GIFParser.cpp
//  cpp_p
//
//  Created by Scott Zhang on 2017/11/7.
//  Copyright © 2017年 Scott Zhang. All rights reserved.
//

#include "GIFParser.hpp"
#include <android/log.h>
#include <list>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,"gif",__VA_ARGS__)
GIFParser::GIFParser(char *buffer,long size)
{
    this->gifSize = size;
    this->buffer = buffer;
    this->seek = 0;
}

void GIFParser::init()
{
    //signature
    memcpy(this->signature, this->buffer + seek, 3);
    seek+=3;
    //version
    memcpy(this->version, this->buffer + seek, 3);
    seek+=3;
    LOGD("signature : %s\n",getSignature());
    LOGD("version : %s\n",getVersion());
    char *lsd = new char[7];
    memcpy(lsd, this->buffer + seek, 7);
    seek += 7;
    // LSD
    LSD *lsdObject = new LSD(lsd);
    this->lsd = lsdObject;
    this->lsd->dump();
    if(this->lsd->isGCT()){
        unsigned char *gctBuffer = new unsigned char[this->lsd->getGlobalColorTableSize()*3] ;
        memcpy(gctBuffer, this->buffer + seek, this->lsd->getGlobalColorTableSize()*3);
        seek += this->lsd->getGlobalColorTableSize()*3;
        GCT *gctObject = new GCT(gctBuffer,this->lsd->getGlobalColorTableSize()*3);
        this->gct = gctObject;
        this->gct->dump();
    }

    bool end = false;
    char introducer;
    unsigned int terminator = 0;
    unsigned int extensionLabel = 0;
    unsigned long gceStart = 0;
    unsigned long gceStop = 0;
    unsigned long imageDataStart = 0;
    unsigned long imageDataStop = 0;
    while (!end) {

        memcpy(&introducer, this->buffer+seek, 1);
        seek += 1;
        LOGD("   -introducer : %c\n",introducer);
        if(introducer == 0x21){
            LOGD("       -this block is an extension\n");
            memcpy(&extensionLabel, this->buffer+seek, 1);
            seek += 1;
            LOGD("       -extension label = %d\n",extensionLabel);
            if(extensionLabel == 0xFF){ // Application extension
                LOGD("           -Applicaiont extension \n");
                int blockSize = 0;
                memcpy(&blockSize, this->buffer+seek, 1);
                seek += 1;
                
                AppExt appExtObject;
                char identifier[9] = {0};
                char authenticationCode[4] = {0};
                memcpy(identifier, this->buffer + seek, 8);
                seek += 8;
                memcpy(authenticationCode, this->buffer + seek, 3);
                seek += 3;
                
                
                int subBlockSize = 0;
                
                list<unsigned char> subblockContent;
                do{
                    memcpy(&subBlockSize, this->buffer + seek, 1);
                    seek += 1;
                    if(subBlockSize != 0){
                        char subBlockBuffer[subBlockSize];
                        memcpy(subBlockBuffer, buffer + seek, subBlockSize);
                        seek += subBlockSize;
                        for(int i = 0;i<subBlockSize;i++){
                            subblockContent.push_back(subBlockBuffer[i]);
                        }
                        DataSubBlocks dataSubBlocksObject(subBlockBuffer,subBlockSize);
                    }else{
                        seek -= 1;
                    }
                }while (subBlockSize == 255);

                appExtObject.setIdentifier(identifier);
                appExtObject.setAuthenticationCode(authenticationCode);
                //this->appExt->setSubData(&dataSubBlocksObject);
                appExtObject.dump();
                //LOGD("            -sub block size : %ld\n",subblockContent.size());
                unsigned char subBlockStr[subblockContent.size()];
                for(int i = 0;i<subblockContent.size();i++){
                    subBlockStr[i] = subblockContent.front();
                    subblockContent.pop_front();
                }
                //LOGD("            -sub block str : %s\n",subBlockStr);

                
                memcpy(&terminator, this->buffer + seek, 1);
                seek += 1;
                LOGD("           -terminator : %d\n",terminator);
                continue;
            }
            if(extensionLabel == 0xF9) //  Graphic control extension
            {

                LOGD( "           -Graphic control extension \n");
                LOGD("--------------------------- graphic control extensin start index %ld\n",seek);
                gceStart = seek;
                int blockSize = 0;
                memcpy(&blockSize, this->buffer + seek, 1);
                seek += 1;
                //LOGD("           -block size %d\n",blockSize);
                bitset<8> compressByte;
                memcpy(&compressByte, this->buffer + seek, 1);
                seek += 1;
                
                unsigned int delayTime = 0;
                memcpy(&delayTime, this->buffer + seek , 2);
                seek += 2;
                
                unsigned int transParentColorIndex = 0;
                memcpy(&transParentColorIndex, this->buffer + seek, 1);
                seek += 1;
                
                GraphicControlExt gceObject(&compressByte,delayTime,transParentColorIndex);
                gceObject.dump();
                memcpy(&terminator, this->buffer + seek, 1);
                seek += 1;
                LOGD("--------------------------- graphic control extensin stop index %ld\n",seek);
                gceStop = seek;
                LOGD("           -terminator : %d\n",terminator);
                continue;
            }
        }
        if(introducer == 0x2c) // image descriptor
        {
            LOGD("       -this block is an image descriptor\n");
            LOGD("--------------------------- image descriptor start index %ld\n",seek);
            imageDataStart = seek;
            unsigned int left = 0;
            unsigned int top = 0;
            unsigned int w = 0;
            unsigned int h = 0;
            bitset<8> compressByte;
            memcpy(&left, buffer + seek, 2);
            seek += 2;
            memcpy(&top, this->buffer + seek, 2);
            seek += 2;
            memcpy(&w, this->buffer + seek, 2);
            seek += 2;
            memcpy(&h, this->buffer + seek, 2);
            seek += 2;
            memcpy(&compressByte, this->buffer + seek, 1);
            seek += 1;
            ImageDescriptor imageDesObject(left,top,w,h,&compressByte);
            imageDesObject.dump();
            //不需要本地颜色表
            if(!imageDesObject.needLocalColorTable())
            {
                LOGD("               -loading image data\n");
                unsigned int miniCodeSize = 0;
                memcpy(&miniCodeSize, this->buffer + seek, 1);
                seek += 1;
                LOGD("               -mini code size : %d\n",miniCodeSize);

                int subBlockSize = 0;
                int totalSize = 0;
                do{
                    memcpy(&subBlockSize, this->buffer + seek, 1);
                    //LOGD("               -block size : %d\n",subBlockSize);
                    seek += 1;
                    if(subBlockSize != 0)
                    {
                        char subBlockBuffer[subBlockSize];
                        memcpy(subBlockBuffer, buffer + seek, subBlockSize);
                        seek += subBlockSize;
                        DataSubBlocks dataSubBlocksObject(subBlockBuffer,subBlockSize);
                        totalSize += subBlockSize;
                    }else{
                        seek -= 1;
                    }

                }while (subBlockSize == 255) ;
                //LOGD("               -total size : %d\n",totalSize);
                memcpy(&terminator, this->buffer + seek, 1);
                seek += 1;
                LOGD("--------------------------- image descriptor stop index %ld\n",seek);
                imageDataStop = seek;
                this->putIdds(gceStart,gceStop,imageDataStart,imageDataStop);
                LOGD("               -terminator : %d\n",terminator);
                continue;
            }
            
        }
        end = true;
    }
    
}
char* GIFParser::getSignature()
{
    return this->signature;
}

char* GIFParser::getVersion()
{
    return this->version;
}

GIFParser::~GIFParser()
{
    delete [] this->buffer;
    delete this->gct;
    delete this->lsd;
}

void GIFParser::putIdds(unsigned long gceStart, unsigned long gceStop, unsigned long imageDataStart,
                        unsigned long imageDataStop)
{
    if(gceStart != 0 && gceStop != 0 && imageDataStart != 0 && imageDataStop != 0){
        ImageDataDes imageDataDes(gceStart,gceStop,imageDataStart,imageDataStop);
        this->idds.push_back(imageDataDes);
    }
}

unsigned long GIFParser::getIddSize()
{
    return idds.size();
}
int GIFParser::getW() {
    return this->lsd->getScreenWidth();
}
int GIFParser::getH() {
    return this->lsd->getScreenHeigth();
}
OneFrame* GIFParser::seekIddsTo(unsigned long position)
{
    LOGD("seek idd to : %ld\n",position );
    ImageDataDes idd = this->idds.at(position);
    GraphicControlExt *gce = getGce(idd);
    IddStruct *iddStruct = getIddStruct(idd);
    OneFrame *oneFrame = new OneFrame;
    oneFrame->iddStruct = iddStruct;
    oneFrame->graphicControlExt = gce;

    return oneFrame;
}
GraphicControlExt* GIFParser::getGce(ImageDataDes &idd) {
    LOGD( "           -Graphic control extension \n");
    unsigned long vSeek = idd.getGceStart();
    int blockSize = 0;
    memcpy(&blockSize, this->buffer + vSeek, 1);
    vSeek += 1;
    //LOGD("           -block size %d\n",blockSize);
    bitset<8> compressByte;
    memcpy(&compressByte, this->buffer + vSeek, 1);
    vSeek += 1;

    unsigned int delayTime = 0;
    memcpy(&delayTime, this->buffer + vSeek , 2);
    vSeek += 2;

    unsigned int transParentColorIndex = 0;
    memcpy(&transParentColorIndex, this->buffer + vSeek, 1);
    vSeek += 1;

    GraphicControlExt *gceObject = new GraphicControlExt(&compressByte,delayTime,transParentColorIndex);
    unsigned int terminator = 0;
    memcpy(&terminator, this->buffer + vSeek, 1);
    vSeek += 1;
    LOGD("           -terminator : %d\n",terminator);
    return gceObject;
}

IddStruct* GIFParser::getIddStruct(ImageDataDes &imageDataDes) {
    LOGD("       -this block is an image descriptor\n");
    unsigned long vSeek = imageDataDes.getImageDataStart();
    unsigned int left = 0;
    unsigned int top = 0;
    unsigned int w = 0;
    unsigned int h = 0;
    struct IddStruct *iddStruct = new IddStruct;
    bitset<8> compressByte;
    memcpy(&left, buffer + vSeek, 2);
    vSeek += 2;
    memcpy(&top, this->buffer + vSeek, 2);
    vSeek += 2;
    memcpy(&w, this->buffer + vSeek, 2);
    vSeek += 2;
    memcpy(&h, this->buffer + vSeek, 2);
    vSeek += 2;
    memcpy(&compressByte, this->buffer + vSeek, 1);
    vSeek += 1;
    ImageDescriptor *imageDesObject = new ImageDescriptor(left,top,w,h,&compressByte);
    iddStruct->imageDescriptor = imageDesObject;
    //不需要本地颜色表
    if(!iddStruct->imageDescriptor->needLocalColorTable())
    {
        LOGD("               -loading image data\n");
        unsigned int miniCodeSize = 0;
        memcpy(&miniCodeSize, this->buffer + vSeek, 1);
        vSeek += 1;
        LOGD("               -mini code size : %d\n",miniCodeSize);

        unsigned int subBlockSize = 0;
        int totalSize = 0;
        vector<unsigned int> *bitmapArray = new vector<unsigned int>;
        do{
            memcpy(&subBlockSize, this->buffer + vSeek, 1);
            //LOGD("               -block size : %d\n",subBlockSize);
            vSeek += 1;
            if(subBlockSize != 0)
            {
                unsigned char subBlockBuffer[subBlockSize];
                memcpy(subBlockBuffer, buffer + vSeek, subBlockSize);
                vSeek += subBlockSize;
                for(int i = 0;i<subBlockSize;i++)
                {
                    unsigned  int index = subBlockBuffer[i];
                    unsigned int color = this->gct->getColorIntAt(index);
                    bitset<24> bColor(color);
                    bitmapArray->push_back(color);
                    LOGD("index %d \n " , index);
                    //LOGD("index %d , color table index : %s\n " , index,bColor.to_string().c_str());
                }
                totalSize += subBlockSize;
            }else{
                vSeek -= 1;
            }

        }while (subBlockSize == 255) ;
        iddStruct->bitmapArray = bitmapArray;

        LOGD("               -total size : %ld\n",iddStruct->bitmapArray->size());
        unsigned int terminator = 0;
        memcpy(&terminator, this->buffer + vSeek, 1);
        vSeek += 1;
        LOGD("               -terminator : %d\n",terminator);
    }
    return iddStruct;
}
///////////////////// LSD ////////////////////////////
LSD::LSD(char *lsd)
{
    this->lsdBuffer = lsd;
}
LSD::~LSD() {
    delete[] lsdBuffer;
}
int LSD::getScreenWidth()
{
    int w = 0;
    memcpy(&w, lsdBuffer, 2);
    return w;
}
int LSD::getScreenHeigth()
{
    int h = 0;
    memcpy(&h, lsdBuffer+2, 2);
    return h;
}
bitset<8> LSD::getCompressByte()
{
    bitset<8> compress;
    memcpy(&compress, lsdBuffer+4, 1);
    return compress;
}
int LSD::getBgColorIndex()
{
    int bgColorIndex;
    memcpy(&bgColorIndex, lsdBuffer+5, 1);
    return bgColorIndex;
}
int LSD::getRadio()
{
    int radio;
    memcpy(&radio, lsdBuffer+6, 1);
    return radio;
}
void LSD::dump()
{
    LOGD("screen w : %d\n",getScreenWidth());
    LOGD("screen h : %d\n",getScreenHeigth());
    LOGD("lsd compress byte : %s\n",getCompressByte().to_string().c_str());
    LOGD("lsd bg color index : %d\n",getBgColorIndex());
    LOGD("lsd radio : %d\n",getRadio());
    LOGD("lsd is GCT here : %d\n",isGCT());
    LOGD("lsd global color table size : %d\n",getGlobalColorTableSize());
}
size_t LSD::getGlobalColorTableSize()
{
    bitset<8> byte = getCompressByte();
    size_t count = byte.to_ulong() & 7;
    return (size_t)(1<<(count + 1));
}
bool LSD::isGCT()
{
    bitset<8> byte = getCompressByte();
    return (byte.to_ulong() & (1<<7)) == (1<<7);
}

/////////////////////////////////// GCT
GCT::GCT(unsigned char *buffer,size_t count)
{
    this->gct = buffer;
    this->count = count;
}
GCT::~GCT() {
    delete[] this->gct;
}
void GCT::dump()
{
    LOGD(" global color table size : %ld\n",count);
    int index = 0;
    for(int i = 0;i<=this->count - 3;i+=3){
        unsigned int r = this->gct[i] << 16;
        unsigned int g = this->gct[i+1] << 8;
        unsigned int b = this->gct[i+2] ;
        bitset<24> color(r|g|b);
        LOGD("gct index : %d = %s",index,color.to_string().c_str());
        index ++ ;
    }
}
unsigned int GCT::getColorIntAt(unsigned int position) {
    //LOGD(" try to get color at : %d\n",position);
    unsigned int r = this->gct[position * 3] << 16;
    unsigned int g = this->gct[position*3+1] << 8;
    unsigned int b = this->gct[position*3+2] ;
    //LOGD("r = %d ,g = %d ,b = %d",r,g,b);
    bitset<24> color(r|g|b);
    //LOGD("gct index : %d = %s",position,color.to_string().c_str());
    return (r|g|b);
}
///////////////////////////////////// AppExt
void AppExt::setIdentifier(char *identifer)
{
    this->appIdentifier = identifer;
}
void AppExt::setAuthenticationCode(char *authenticationCode)
{
    this->appAuthenticationCode = authenticationCode;
}
void AppExt::setSubData(DataSubBlocks * subBlocks){
    this->subBlocks = subBlocks;
}
void AppExt::dump()
{
    cout << "app extension idenfifier : " << this->appIdentifier << endl;
    cout << "app extension authenticatin code : " << this->appAuthenticationCode << endl;
    //this->subBlocks->dump();
}
///////////////////////////////////// Data sub-blocks
DataSubBlocks::DataSubBlocks(char* data,int size)
{
    this->data = data;
    this->size = size;
}
void DataSubBlocks::dump()
{
    cout << "data sub blocks size : " << this->size << endl;
}
///////////////////////////////////// Graphic control extension
GraphicControlExt::GraphicControlExt(bitset<8>* compressByte,int delayTime ,int transparentColorIndex)
{
    this->compressByte = compressByte;
    this->delayTime = delayTime;
    this->transparentColorIndex = transparentColorIndex;
}
void GraphicControlExt::dump()
{
    LOGD("GraphicControlExt compress byte : %s\n",this->compressByte->to_string().c_str());
    LOGD("GraphicControlExt delay time : %d\n",delayTime);
    LOGD("GraphicControlExt transparent color index : %d\n",transparentColorIndex);
}
///////////////////////////////////// Image desciptor
ImageDescriptor::ImageDescriptor(unsigned int left,unsigned int top,unsigned int w,unsigned int h,bitset<8>* compressByte)
{
    this->left = left;
    this->top = top;
    this->w = w;
    this->h = h;
    this->compressByte = compressByte;
}
bool ImageDescriptor::needLocalColorTable()
{
    return (*(this->compressByte)).to_ulong() & (1<<7);
}
unsigned int ImageDescriptor::getTop() {
    return this->top;
}
unsigned int ImageDescriptor::getLeft() {
    return this->left;
}
unsigned int ImageDescriptor::getW() {
    return this->w;
}
unsigned int ImageDescriptor::getH() {
    return this->h;
}
void ImageDescriptor::dump()
{
    LOGD("Image descriptor left : %d\n",this->left);
    LOGD("Image descriptor top : %d\n",this->top);
    LOGD("Image descriptor w : %d\n",this->w);
    LOGD("Image descriptor h : %d\n",this->h);
    LOGD("Image descriptor compressByte : %s\n",this->compressByte->to_string().c_str());
    LOGD("Image descriptor need local color table : %d\n",needLocalColorTable());
}
////////////////////////////////////// Image data descriptor
ImageDataDes::ImageDataDes(unsigned long gceStart, unsigned long gceStop, unsigned long imageDataStart, unsigned long imageDataStop)
{
    this->gceStart = gceStart;
    this->gceStop = gceStop;
    this->imageDataStart = imageDataStart;
    this->imageDataStop = imageDataStop;
}
void ImageDataDes::dump()
{
    LOGD("gce start %ld\n",this->gceStart);
    LOGD("gce stop %ld\n",this->gceStop);
    LOGD("image data start %ld\n",this->imageDataStart);
    LOGD("image data stop %ld\n",this->imageDataStop);
}
unsigned long ImageDataDes::getGceStart() {
    return this->gceStart;
}
unsigned long ImageDataDes::getGceStop() {
    return this->gceStop;
}
unsigned long ImageDataDes::getImageDataStart() {
    return this->imageDataStart;
}
unsigned long ImageDataDes::getImageDataStop() {
    return this->imageDataStop;
}