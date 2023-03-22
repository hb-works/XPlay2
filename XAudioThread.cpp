#include "XAudioThread.h"
#include "XDecode.h"
#include "XAudioPlay.h"
#include "XResample.h"
#include <iostream>
using namespace std;
//完成整个音频线程的资源清理
void XAudioThread::Clear()
{
	//调用基类清理
	XDecodeThread::Clear();
	mux.lock();
	//清理音频
	if (ap)
		ap->Clear();
	mux.unlock();
}

//停止线程，清理资源
void XAudioThread::Close()
{
	XDecodeThread::Close();
	if (res)
	{
		res->Close();
		amux.lock();
		delete res;
		res = NULL;
		amux.unlock();
	}
	if (ap)
	{
		ap->Close();
		amux.lock();
		ap = NULL;
		amux.unlock();
	}
}

bool XAudioThread::Open(AVCodecParameters *para, int sampleRate, int channels)
{
	if (!para)
		return false;
	//每次打开前先清理一遍，确保打开正确
	Clear();
	amux.lock();
	pts = 0;
	//不需要创建了 在基类中创建了
	//if (!decode)
	//	decode = new XDecode();
	
	bool re = true;
	//音频重采样
	if (!res->Open(para, false))
	{
		//重采样失败
		re = false;
		cout << "XResample open failed!!!" << endl;
	}
	//记录采样率和通道数
	//重采样成功
	ap->sampleRate = sampleRate;
	ap->channels = channels;
	re = true;
	//打开音频数据
	if (!ap->Open())
	{
		//打开音频数据失败
		re = false;
		cout << "XAudioPlay open failed!!!" << endl;
	}
	//打开音频数据成功
	re = true;
	//解码音频数据
	if (!decode->Open(para))
	{
		//解码音频数据失败
		re = false;
		cout << "audio XDecode open failed!!!" << endl;
	}
	//重采样+解码+播放音频都成功 == XAudioThread成功，解锁，返回结果
	amux.unlock();
	cout << "XAudioThread::Open success:" << re << endl;
	return re;
}

//实现音频暂停
void XAudioThread::SetPause(bool isPause)
{
	//amux.lock();
	this->isPause = isPause;
	if (ap)
		ap->SetPause(isPause);
	//amux.unlock();
}

void XAudioThread::run()
{
	unsigned char *pcm = new unsigned char[1024 * 1024 * 10];
	while (!isExit)
	{
		amux.lock();
		if (this->isPause)
		{
			amux.unlock();
			msleep(5);
			continue;
		}
		////没有数据
		//if (packs.empty() || !decode || !res || !ap)
		//{
		//	amux.unlock();
		//	msleep(1);
		//	continue;
		//}
		////有数据 前面用push_back，队列先进先出，此处从前面的数据开始取
		//AVPacket *pkt = packs.front();
		//packs.pop_front();
		
		AVPacket *pkt = Pop();
		//调用decode解码
		bool re = decode->Send(pkt);
		//失败的话
		if (!re)
		{
			amux.unlock();
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
			//减去缓冲中未播放的时间
			pts = decode->pts - ap->GetNoPlayMs();
			//cout << "audio pts " << pts << endl;

			//读到数据就进行重采样,pcm存放重采样数据
			int size = res->Resample(frame, pcm);
			//播放音频
			while (!isExit)
			{
				if (size <= 0)
					break;
				//缓冲未播完，空间不够 或 暂停状态就先不要写
				if (ap->GetFree() < size || isPause)
				{
					msleep(1);
					continue;
				}
				ap->Write(pcm, size);
				break;
			}
		}	
		amux.unlock();
	}
	//函数结束记得释放空间，以免造成内存泄漏
	delete pcm;
}

XAudioThread::XAudioThread()
{
	if (!ap)
		ap = XAudioPlay::Get();
	if (!res)
		res = new XResample();
}

XAudioThread::~XAudioThread()
{

}