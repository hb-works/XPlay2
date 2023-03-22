#include "XVideoThread.h"
#include "XDecode.h"
#include <iostream>
using namespace std;
//视频打开，不管成功与否都清理空间
bool XVideoThread::Open(AVCodecParameters *para, IVideoCall *call, int width, int height)
{
	if (!para)
		return false;
	Clear();
	vmux.lock();
	//同步时间
	synpts = 0;
	//初始化显示窗口XVideoWidget
	this->call = call;
	if (call)
	{
		call->Init(width, height);
	}
	vmux.unlock();
	//进行视频解码操作，默认成功
	int re = true;
	//如果失败，re = false
	if (!decode->Open(para))
	{
		re = false;
		cout << "video XDecode open failed!!!" << endl;
	}

	cout << "XVideoThread::Open success:" << re << endl;
	return re;
}

//实现视频暂停
void XVideoThread::SetPause(bool isPause)
{
	vmux.lock();
	this->isPause = isPause;
	vmux.unlock();
}

void XVideoThread::run()
{
	while (!isExit)
	{
		vmux.lock();
		//如果是暂停状态 就sleep
		if (this->isPause)
		{
			vmux.unlock();
			msleep(5);
			continue;
		}
		//音视频同步，视频比音频快，视频就稍微慢点等等音频
		//采用的同步策略是 视频去同步音频
		//设置一个同步时间synpts，同步时间 < 当前视频解码到的时间，视频解码速度比同步时间（在XDemuxThread中设置了vt->synpts = at->pts;）大，
		//就是说视频跑得比音频要快，就要视频做等待操作
		if (synpts > 0 && synpts < decode->pts)
		{
			vmux.unlock();
			msleep(1);
			continue;
		}
		//整合代码
		AVPacket *pkt = Pop();
		////没有数据
		//if (packs.empty() || !decode)
		//{
		//	vmux.unlock();
		//	msleep(1);
		//	continue;
		//}

		////有数据 前面用push_back，队列先进先出，此处从前面的数据开始取
		//AVPacket *pkt = packs.front();
		//packs.pop_front();

		//调用decode解码
		bool re = decode->Send(pkt);
		//失败的话
		if (!re)
		{
			vmux.unlock();
			msleep(1);
			continue;
		}
		//成功的话就接受数据，有可能一次Send多次Recv，所以加个while循环
		while (!isExit)
		{
			AVFrame *frame = decode->Recv();
			//没读到就直接break;
			if (!frame)
				break;
			//读到数据 显示视频
			if (call)
			{
				call->Repaint(frame);
			}
		}
		vmux.unlock();
	}
}

//解码pts，如果接收到的解码数据pts >= seekpts return true 并且显示画面
bool XVideoThread::RepaintPts(AVPacket *pkt, long long seekpts)
{
	vmux.lock();
	bool re = decode->Send(pkt);
	//true表示结束解码
	if (!re)
	{
		vmux.unlock();
		return true;
	}
		
	//解码成功，接收
	AVFrame *frame = decode->Recv();
	if (!frame)
	{
		vmux.unlock();
		return false;
	}
	//到达位置 显示 解锁 返回true
	if (decode->pts >= seekpts)
	{
		//暂停状态下刷新进度条位置，把当前的pts设为帧所在的位置
		if (call)
			call->Repaint(frame);
		vmux.unlock();
		return true;
	}
	//释放掉 不能有内存泄漏
	XFreeFrame(&frame);
	vmux.unlock();
	return false;
}

XVideoThread::XVideoThread()
{

}

XVideoThread::~XVideoThread()
{
	
}