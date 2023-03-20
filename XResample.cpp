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

//输出参数和输入参数一致除了采样格式，输出为S16
bool XResample::Open(AVCodecParameters *para, bool isClearPara)
{
	if (!para)
		return false;
	mux.lock();
	//音频重采样 aac解码出来的是32位的不支持，要转换为16位或64位的
	//设置属性
	//if (!actx)
	//	actx = swr_alloc();
	//不用分配了 如果actx为空，swr_alloc_set_opts会分配空间
	actx = swr_alloc_set_opts(
		actx,								//音频重采样上下文
		av_get_default_channel_layout(2),	//输出格式，2通道
		(AVSampleFormat)outFormat,					//输出样本格式 S16:16位，两个字节表示
		para->sample_rate,					//采样率：一秒钟的音频样本数量
		av_get_default_channel_layout(para->channels),	//输入格式
		(AVSampleFormat)para->format,
		para->sample_rate,
		0, 0
	);
	if(isClearPara)
		//用完了就释放掉
		avcodec_parameters_free(&para);
	//swr_init初始化
	int re = swr_init(actx);
	mux.unlock();
	//swr_init初始化失败
	if (re != 0)
	{
		//用buffer来存放错误信息
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		cout << "swr_init failed!:" << buf << endl;
		return false;
	}
	//unsigned char *pcm = NULL;
	return true;
}

//返回重采样后的大小,不管成功与否都释放indata空间
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
	int re = swr_convert(actx,					//上下文
		data, indata->nb_samples,			//输出, 样本数 （可以在这做个判断，以防格式不对）
		(const uint8_t**)indata->data, indata->nb_samples		//输入, 样本数
	);
	int outSize = re * indata->channels * av_get_bytes_per_sample((AVSampleFormat)outFormat);
	//用完赶紧释放掉 不然会造成内存泄露
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