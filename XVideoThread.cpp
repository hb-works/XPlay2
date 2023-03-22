#include "XVideoThread.h"
#include "XDecode.h"
#include <iostream>
using namespace std;
//��Ƶ�򿪣����ܳɹ��������ռ�
bool XVideoThread::Open(AVCodecParameters *para, IVideoCall *call, int width, int height)
{
	if (!para)
		return false;
	Clear();
	vmux.lock();
	//ͬ��ʱ��
	synpts = 0;
	//��ʼ����ʾ����XVideoWidget
	this->call = call;
	if (call)
	{
		call->Init(width, height);
	}
	vmux.unlock();
	//������Ƶ���������Ĭ�ϳɹ�
	int re = true;
	//���ʧ�ܣ�re = false
	if (!decode->Open(para))
	{
		re = false;
		cout << "video XDecode open failed!!!" << endl;
	}

	cout << "XVideoThread::Open success:" << re << endl;
	return re;
}

//ʵ����Ƶ��ͣ
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
		//�������ͣ״̬ ��sleep
		if (this->isPause)
		{
			vmux.unlock();
			msleep(5);
			continue;
		}
		//����Ƶͬ������Ƶ����Ƶ�죬��Ƶ����΢����ȵ���Ƶ
		//���õ�ͬ�������� ��Ƶȥͬ����Ƶ
		//����һ��ͬ��ʱ��synpts��ͬ��ʱ�� < ��ǰ��Ƶ���뵽��ʱ�䣬��Ƶ�����ٶȱ�ͬ��ʱ�䣨��XDemuxThread��������vt->synpts = at->pts;����
		//����˵��Ƶ�ܵñ���ƵҪ�죬��Ҫ��Ƶ���ȴ�����
		if (synpts > 0 && synpts < decode->pts)
		{
			vmux.unlock();
			msleep(1);
			continue;
		}
		//���ϴ���
		AVPacket *pkt = Pop();
		////û������
		//if (packs.empty() || !decode)
		//{
		//	vmux.unlock();
		//	msleep(1);
		//	continue;
		//}

		////������ ǰ����push_back�������Ƚ��ȳ����˴���ǰ������ݿ�ʼȡ
		//AVPacket *pkt = packs.front();
		//packs.pop_front();

		//����decode����
		bool re = decode->Send(pkt);
		//ʧ�ܵĻ�
		if (!re)
		{
			vmux.unlock();
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
			//�������� ��ʾ��Ƶ
			if (call)
			{
				call->Repaint(frame);
			}
		}
		vmux.unlock();
	}
}

//����pts��������յ��Ľ�������pts >= seekpts return true ������ʾ����
bool XVideoThread::RepaintPts(AVPacket *pkt, long long seekpts)
{
	vmux.lock();
	bool re = decode->Send(pkt);
	//true��ʾ��������
	if (!re)
	{
		vmux.unlock();
		return true;
	}
		
	//����ɹ�������
	AVFrame *frame = decode->Recv();
	if (!frame)
	{
		vmux.unlock();
		return false;
	}
	//����λ�� ��ʾ ���� ����true
	if (decode->pts >= seekpts)
	{
		//��ͣ״̬��ˢ�½�����λ�ã��ѵ�ǰ��pts��Ϊ֡���ڵ�λ��
		if (call)
			call->Repaint(frame);
		vmux.unlock();
		return true;
	}
	//�ͷŵ� �������ڴ�й©
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