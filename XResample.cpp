#include "XResample.h"
#include <iostream>
extern "C" {
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>
}
#pragma comment(lib, "swresample.lib")

using namespace std;

void XResample::Close()
{
	mux.lock();
	if (!actx) 
	{
		swr_free(&actx);
		mux.unlock();
	}

	mux.unlock();
}

//����������������һ�³��˲�����ʽ�����ΪS16
bool XResample::Open(AVCodecParameters *para, bool isClearPara)
{
	if (!para)
		return false;
	mux.lock();
	//��Ƶ�ز��� aac�����������32λ�Ĳ�֧�֣�Ҫת��Ϊ16λ��64λ��
	//��������
	//if (!actx)
	//	actx = swr_alloc();
	//���÷����� ���actxΪ�գ�swr_alloc_set_opts�����ռ�
	actx = swr_alloc_set_opts(
		actx,								//��Ƶ�ز���������
		av_get_default_channel_layout(2),	//�����ʽ��2ͨ��
		(AVSampleFormat)outFormat,					//���������ʽ S16:16λ�������ֽڱ�ʾ
		para->sample_rate,					//�����ʣ�һ���ӵ���Ƶ��������
		av_get_default_channel_layout(para->channels),	//�����ʽ
		(AVSampleFormat)para->format,
		para->sample_rate,
		0, 0
	);
	if(isClearPara)
		//�����˾��ͷŵ�
		avcodec_parameters_free(&para);
	//swr_init��ʼ��
	int re = swr_init(actx);
	mux.unlock();
	//swr_init��ʼ��ʧ��
	if (re != 0)
	{
		//��buffer����Ŵ�����Ϣ
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		cout << "swr_init failed!:" << buf << endl;
		return false;
	}
	//unsigned char *pcm = NULL;
	return true;
}

//�����ز�����Ĵ�С,���ܳɹ�����ͷ�indata�ռ�
int XResample::Resample(AVFrame *indata, unsigned char *d)
{
	if (!indata)
		return 0;
	if (!d) 
	{
		av_frame_free(&indata);
		return 0;
	}
	uint8_t *data[2] = { 0 };
	data[0] = d;
	int re = swr_convert(actx,					//������
		data, indata->nb_samples,			//���, ������ ���������������жϣ��Է���ʽ���ԣ�
		(const uint8_t**)indata->data, indata->nb_samples		//����, ������
	);
	int outSize = re * indata->channels * av_get_bytes_per_sample((AVSampleFormat)outFormat);
	//����Ͻ��ͷŵ� ��Ȼ������ڴ�й¶
	av_frame_free(&indata);
	if (re <= 0)
		return re;
	return outSize;
}

XResample::XResample()
{

}

XResample::~XResample()
{

}