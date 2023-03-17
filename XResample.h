#pragma once
#include <mutex>
struct AVCodecParameters;
struct AVFrame;
struct SwrContext;
//��Ƶ�ز���
class XResample
{
public:
	//����������������һ�³��˲�����ʽ�����ΪS16
	virtual bool Open(AVCodecParameters *para);
	virtual void Close();

	//�����ز�����Ĵ�С,���ܳɹ�����ͷ�indata�ռ�	indata������	data�����
	virtual int Resample(AVFrame *indata, unsigned char *d);
	XResample();
	~XResample();
	//AV_SAMPLE_FMT_S16
	int outFormat = 1;
protected:
	std::mutex mux;
	SwrContext *actx = 0;
};

