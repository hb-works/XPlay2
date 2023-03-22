#include "XDecodeThread.h"
#include "XDecode.h"

//清理资源，停止线程
void XDecodeThread::Close()
{
	Clear();
	
	isExit = true;
	wait();
	//等待线程退出后才能进行decode清理，因为线程中有可能访问decode
	decode->Close();
	mux.lock();
	delete decode;
	decode = NULL;
	mux.unlock();
}

//清理队列，在基类维护队列的处理
void XDecodeThread::Clear()
{
	mux.lock();
	decode->Clear();
	//清理队列
	while (!packs.empty())
	{
		AVPacket *pkt = packs.front();
		XFreePacket(&pkt);
		packs.pop_front();
	}
	mux.unlock();
}

//取出一帧数据，并出栈，如果无数据返回NULL
AVPacket *XDecodeThread::Pop()
{
	mux.lock();
	if (packs.empty())
	{
		mux.unlock();
		return NULL;
	}
	AVPacket *pkt = packs.front();
	packs.pop_front();
	mux.unlock();
	return pkt;
}

//另外的线程如何把任务加过来
void XDecodeThread::Push(AVPacket *pkt)
{
	if (!pkt)
		return;
	//阻塞 isExit = false代表不退出
	while (!isExit)
	{
		mux.lock();
		//容量没有超过maxList时才添加进队列中
		if (packs.size() < maxList)
		{
			packs.push_back(pkt);
			mux.unlock();
			break;
		}
		mux.unlock();
		msleep(1);
	}
}

XDecodeThread::XDecodeThread()
{
	//打开解码器
	if (!decode)
		decode = new XDecode();
}

XDecodeThread::~XDecodeThread()
{
	//等待线程退出
	isExit = true;
	wait();
}