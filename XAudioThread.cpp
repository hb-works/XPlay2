#include "XAudioThread.h"
#include "XDecode.h"
#include "XAudioPlay.h"
#include "XResample.h"
#include <iostream>
using namespace std;
//���������Ƶ�̵߳���Դ����
void XAudioThread::Clear()
{
	//���û�������
	XDecodeThread::Clear();
	mux.lock();
	//������Ƶ
	if (ap)
		ap->Clear();
	mux.unlock();
}

//ֹͣ�̣߳�������Դ
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
	Clear();
	amux.lock();
	pts = 0;
	//����Ҫ������ �ڻ����д�����
	//if (!decode)
	//	decode = new XDecode();
	
	bool re = true;
	if (!res->Open(para, false))
	{
		re = false;
		cout << "XResample open failed!!!" << endl;
	}
	ap->sampleRate = sampleRate;
	ap->channels = channels;
	re = true;
	if (!ap->Open())
	{
		re = false;
		cout << "XAudioPlay open failed!!!" << endl;
	}
	re = true;
	if (!decode->Open(para))
	{
		re = false;
		cout << "audio XDecode open failed!!!" << endl;
	}
	amux.unlock();
	cout << "XAudioThread::Open success:" << re << endl;
	return re;
}

//ʵ����Ƶ��ͣ
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
		////û������
		//if (packs.empty() || !decode || !res || !ap)
		//{
		//	amux.unlock();
		//	msleep(1);
		//	continue;
		//}
		////������ ǰ����push_back�������Ƚ��ȳ����˴���ǰ������ݿ�ʼȡ
		//AVPacket *pkt = packs.front();
		//packs.pop_front();
		
		AVPacket *pkt = Pop();
		//����decode����
		bool re = decode->Send(pkt);
		//ʧ�ܵĻ�
		if (!re)
		{
			amux.unlock();
			msleep(1);
			continue;
		}
		//�ɹ��Ļ��ͽ������ݣ��п���һ��Send���Recv�����ԼӸ�whileѭ��
		while (!isExit)
		{
			AVFrame *frame = decode->Recv();
			//û������ֱ��break;
			if (!frame)
				break;
			//��ȥ������δ���ŵ�ʱ��
			pts = decode->pts - ap->GetNoPlayMs();
			//cout << "audio pts " << pts << endl;

			//�������ݾͽ����ز���,pcm����ز�������
			int size = res->Resample(frame, pcm);
			//������Ƶ
			while (!isExit)
			{
				if (size <= 0)
					break;
				//����δ���꣬�ռ䲻�� �� ��ͣ״̬���Ȳ�Ҫд
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
	//���������ǵ��ͷſռ䣬��������ڴ�й©
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