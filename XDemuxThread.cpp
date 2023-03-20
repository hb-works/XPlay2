#include "XDemuxThread.h"
#include "XDemux.h"
#include "XVideoThread.h"
#include "XAudioThread.h"
#include "XDecode.h"
#include <iostream>
extern "C"
{
#include <libavformat/avformat.h>
}

using namespace std;

void XDemuxThread::Clear()
{
	mux.lock();
	if (demux)
		demux->Clear();
	if (vt)
		vt->Clear();
	if (at)
		at->Clear();
	mux.unlock();
}

//松开滑动条 进行定位
void XDemuxThread::Seek(double pos)
{
	//清理缓冲
	Clear();
	mux.lock();
	bool status = this->isPause;
	mux.unlock();
	//原本状态是什么就继续是什么，原本是播放拖动滑动条后就继续播放
	SetPause(status);
	mux.lock();
	if (demux)
		demux->Seek(pos);
	//实际要显示的位置pts pos是一个相对位置，相当于百分比
	long long seekPts = pos * demux->totalMs;
	while (!isExit)
	{
		AVPacket *pkt = demux->ReadVideo();
		if (!pkt)
			break;
		//if (pkt->stream_index == demux->audioStream)
		//{
		//	//音频数据,就丢弃
		//	av_packet_free(&pkt);
		//	continue;
		//}
		//如果解码到seekPts
		if (vt->RepaintPts(pkt, seekPts))
		{
			this->pts = seekPts;
			break;
		}
		//bool re = vt->decode->Send(pkt);
		////解码失败
		//if (!re)
		//	break;
		////解码成功，接收
		//AVFrame *frame = vt->decode->Recv();
		//if (!frame)
		//	continue;
		////到达位置 显示 退出
		//if (frame->pts >= seekPts)
		//{
		//	//暂停状态下刷新进度条位置，把当前的pts设为帧所在的位置
		//	this->pts = frame->pts;
		//	vt->call->Repaint(frame);
		//	break;
		//}
		////释放掉 不能有内存泄漏
		//av_frame_free(&frame);
	}

	mux.unlock();

	//Seek时是暂停状态
	if(!isPause)
		SetPause(false);
}

void XDemuxThread::SetPause(bool isPause)
{
	mux.lock();
	this->isPause = isPause;
	//要把at和vt都暂停了
	if (at)
		at->SetPause(isPause);
	if(vt)
		vt->SetPause(isPause);
	mux.unlock();
}

void XDemuxThread::run()
{
	while (!isExit)
	{
		mux.lock();
		if (isPause)
		{
			//如果暂停状态，就sleep
			mux.unlock();
			msleep(5);
			continue;
		}
		if (!demux)
		{
			mux.unlock();
			msleep(5);
			continue;
		}

		//如果音视频都存在，就要做音视频同步
		if (vt && at)
		{
			//获取音频pts，实现滑动条操作
			pts = at->pts;
			//音视频同步操作
			vt->synpts = at->pts;
		}

		AVPacket *pkt = demux->Read();
		//没有读到数据
		if (!pkt)
		{
			mux.unlock();
			msleep(5);
			continue;
		}
		//判断数据是音频
		if (demux->IsAudio(pkt))
		{
			if(at)
				at->Push(pkt);
		}
		//数据是视频
		else
		{
			if(vt)
				vt->Push(pkt);
		}
		mux.unlock();
		//读太快了，放慢一点做同步
		msleep(1);
	}
}

//关闭线程，清理资源
void XDemuxThread::Close()
{
	isExit = true;
	wait();
	if (vt)
		vt->Close();
	if (at)
		at->Close();
	mux.lock();
	delete vt;
	delete at;
	vt = NULL;
	at = NULL;
	mux.unlock();
}

bool XDemuxThread::Open(const char *url, IVideoCall *call)
{
	if (url == 0 || url[0] == '\0')
		return false;
	mux.lock();
	if (!demux)
		demux = new XDemux();
	if (!vt)
		vt = new XVideoThread();
	if (!at)
		at = new XAudioThread();
	//打开解封装
	bool re = demux->Open(url);
	//失败的话
	if (!re)
	{
		cout << "demux->Open(url) failed!!!" << endl;
		return false;
	}
	//打开视频解码器和处理线程
	if (!vt->Open(demux->CopyVPara(), call, demux->width, demux->height))
	{
		re = false;
		cout << "vt->Open failed!!!" << endl;
	}
	//打开音频解码器和处理线程
	if (!at->Open(demux->CopyAPara(),demux->sampleRate, demux->channels))
	{
		re = false;
		cout << "at->Open failed!!!" << endl;
	}
	totalMs = demux->totalMs;
	mux.unlock();
	//全部成功
	cout << "XDemuxThread::Open " << re << endl;
	return re;
}

void XDemuxThread::Start()
{
	mux.lock();
	if (!demux)
		demux = new XDemux();
	if (!vt)
		vt = new XVideoThread();
	if (!at)
		at = new XAudioThread();
	//启动当前线程
	QThread::start();
	if (vt)
		vt->start();
	if (at)
		at->start();
	mux.unlock();
}

XDemuxThread::XDemuxThread()
{

}

XDemuxThread::~XDemuxThread()
{
	isExit = true;
	wait();
}