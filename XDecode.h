#pragma once
#include <iostream>
#include <mutex>
struct AVCodecParameters;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
class XDecode
{
public:
	//增加一个成员，确定当前的是音频还是视频
	bool isAudio = false;

	//打开解码器,不管成功与否都释放para空间
	virtual bool Open(AVCodecParameters *para);

	//发送到解码线程，不管成功与否都释放pkt空间（对象和媒体内容）
	virtual bool Send(AVPacket *pkt);
	
	//获取解码后的数据，一次send可能需要多次recv，获取缓冲中的数据Send NULL再Recv多次
	//每次复制一份，由调用者释放 av_frame_free  空间释放是C++编程中最大的问题
	//空间相关的问题要仔细，做音视频很多时间都花在找内存泄漏上
	virtual AVFrame *Recv();

	virtual void Clear();
	virtual void Close();


	XDecode();
	virtual ~XDecode();

protected:
	AVCodecContext *codec = 0;
	std::mutex mux;
};

