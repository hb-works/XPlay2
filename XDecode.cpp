#include "XDecode.h"
#include <iostream>
extern "C"
{
	#include "libavcodec/avcodec.h"
}
using namespace std;

void XFreePacket(AVPacket **pkt)
{
	if (!pkt || !(*pkt))
		return;
	av_packet_free(pkt);
}

void XFreeFrame(AVFrame **frame)
{
	if (!frame || !(*frame))
		return;
	av_frame_free(frame);
}

//关闭和清理
void XDecode::Clear()
{
	mux.lock();
	//清理解码缓冲
	if (codec)
	{
		avcodec_flush_buffers(codec);
	}
	mux.unlock();
}
void XDecode::Close()
{
	mux.lock();
	if (codec)
	{
		avcodec_close(codec);
		avcodec_free_context(&codec);
	}
	pts = 0;
	mux.unlock();
}

//打开解码器
bool XDecode::Open(AVCodecParameters *para)
{
	if (!para)
	{
		//如果参数为空 直接返回false
		return false;
	}
	//打开前先关闭
	Close();
	///解码器打开
	///找到解码器
	AVCodec *vcodec = avcodec_find_decoder(para->codec_id);
	//这个地方有可能找不到解码器，所以加个判断
	if (!vcodec)
	{
		//失败也要释放para，不然会造成内存泄漏
		avcodec_parameters_free(&para);
		cout << "can't find the codec id " << para->codec_id << endl;
		return false;
	}
	//找到解码器
	cout << "find the AVCodec " << para->codec_id << endl;

	mux.lock();

	///创建解码器上下文
	codec = avcodec_alloc_context3(vcodec);

	///配置解码器上下文参数
	avcodec_parameters_to_context(codec, para);
	avcodec_parameters_free(&para);

	//八线程解码
	codec->thread_count = 8;

	///打开解码器上下文
	int re = avcodec_open2(codec, 0, 0);
	//如果re！=0就是失败了
	if (re != 0)
	{
		//如果失败 释放空间
		avcodec_free_context(&codec);
		mux.unlock();
		//用buffer来存放错误信息
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		cout << "avcodec_open2 failed!:" << buf << endl;
		return false;
	}
	//打开成功
	mux.unlock();
	cout << "avcodec_open2 success!" << endl;
	avcodec_parameters_free(&para);
	return true;
}

//发送到解码线程，不管成功与否都释放pkt空间（对象和媒体内容）
bool XDecode::Send(AVPacket *pkt)
{
	//如果pkt不存在或者pkt的size小于等于0或者pkt的data不存在，直接返回false
	if (!pkt || pkt->size <= 0 || !pkt->data)
		return false;
	mux.lock();
	//如果解码器没打开 或者 打开失败 直接解锁返回false
	if (!codec)
	{
		mux.unlock();
		return false;
	}
	int re = avcodec_send_packet(codec, pkt);
	
	mux.unlock();
	av_packet_free(&pkt);
	if (re != 0)
		return false;
	return true;
}

//获取解码后的数据，一次send可能需要多次recv，获取缓冲中的数据Send NULL再Recv多次
//每次复制一份，由调用者释放 av_frame_free  空间释放是C++编程中最大的问题
//空间相关的问题要仔细，做音视频很多时间都花在找内存泄漏上
AVFrame *XDecode::Recv()
{
	mux.lock();
	//如果解码器没打开 或者 打开失败 直接解锁返回0
	if (!codec)
	{
		mux.unlock();
		return 0;
	}
	//创建一个空间来接收解码数据
	AVFrame *frame = av_frame_alloc();
	//把内容写入到frame中，成功返回0
	int re = avcodec_receive_frame(codec, frame);
	mux.unlock();
	if (re != 0) 
	{
		//失败了要把空间释放掉，然后返回NULL
		av_frame_free(&frame);
		return NULL;
	}
	//cout << "[" << frame->linesize[0] << "] " << flush;
	pts = frame->pts;
	//成功了就把接收到的数据返回
	return frame;
}

XDecode::XDecode()
{
	
}

XDecode::~XDecode()
{

}