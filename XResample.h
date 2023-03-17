#pragma once
#include <mutex>
struct AVCodecParameters;
struct AVFrame;
struct SwrContext;
//音频重采样
class XResample
{
public:
	//输出参数和输入参数一致除了采样格式，输出为S16
	virtual bool Open(AVCodecParameters *para);
	virtual void Close();

	//返回重采样后的大小,不管成功与否都释放indata空间	indata：输入	data：输出
	virtual int Resample(AVFrame *indata, unsigned char *d);
	XResample();
	~XResample();
	//AV_SAMPLE_FMT_S16
	int outFormat = 1;
protected:
	std::mutex mux;
	SwrContext *actx = 0;
};

