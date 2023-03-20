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

//�ɿ������� ���ж�λ
void XDemuxThread::Seek(double pos)
{
	//������
	Clear();
	mux.lock();
	bool status = this->isPause;
	mux.unlock();
	//ԭ��״̬��ʲô�ͼ�����ʲô��ԭ���ǲ����϶���������ͼ�������
	SetPause(status);
	mux.lock();
	if (demux)
		demux->Seek(pos);
	//ʵ��Ҫ��ʾ��λ��pts pos��һ�����λ�ã��൱�ڰٷֱ�
	long long seekPts = pos * demux->totalMs;
	while (!isExit)
	{
		AVPacket *pkt = demux->ReadVideo();
		if (!pkt)
			break;
		//if (pkt->stream_index == demux->audioStream)
		//{
		//	//��Ƶ����,�Ͷ���
		//	av_packet_free(&pkt);
		//	continue;
		//}
		//������뵽seekPts
		if (vt->RepaintPts(pkt, seekPts))
		{
			this->pts = seekPts;
			break;
		}
		//bool re = vt->decode->Send(pkt);
		////����ʧ��
		//if (!re)
		//	break;
		////����ɹ�������
		//AVFrame *frame = vt->decode->Recv();
		//if (!frame)
		//	continue;
		////����λ�� ��ʾ �˳�
		//if (frame->pts >= seekPts)
		//{
		//	//��ͣ״̬��ˢ�½�����λ�ã��ѵ�ǰ��pts��Ϊ֡���ڵ�λ��
		//	this->pts = frame->pts;
		//	vt->call->Repaint(frame);
		//	break;
		//}
		////�ͷŵ� �������ڴ�й©
		//av_frame_free(&frame);
	}

	mux.unlock();

	//Seekʱ����ͣ״̬
	if(!isPause)
		SetPause(false);
}

void XDemuxThread::SetPause(bool isPause)
{
	mux.lock();
	this->isPause = isPause;
	//Ҫ��at��vt����ͣ��
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
			//�����ͣ״̬����sleep
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

		//�������Ƶ�����ڣ���Ҫ������Ƶͬ��
		if (vt && at)
		{
			//��ȡ��Ƶpts��ʵ�ֻ���������
			pts = at->pts;
			//����Ƶͬ������
			vt->synpts = at->pts;
		}

		AVPacket *pkt = demux->Read();
		//û�ж�������
		if (!pkt)
		{
			mux.unlock();
			msleep(5);
			continue;
		}
		//�ж���������Ƶ
		if (demux->IsAudio(pkt))
		{
			if(at)
				at->Push(pkt);
		}
		//��������Ƶ
		else
		{
			if(vt)
				vt->Push(pkt);
		}
		mux.unlock();
		//��̫���ˣ�����һ����ͬ��
		msleep(1);
	}
}

//�ر��̣߳�������Դ
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
	//�򿪽��װ
	bool re = demux->Open(url);
	//ʧ�ܵĻ�
	if (!re)
	{
		cout << "demux->Open(url) failed!!!" << endl;
		return false;
	}
	//����Ƶ�������ʹ����߳�
	if (!vt->Open(demux->CopyVPara(), call, demux->width, demux->height))
	{
		re = false;
		cout << "vt->Open failed!!!" << endl;
	}
	//����Ƶ�������ʹ����߳�
	if (!at->Open(demux->CopyAPara(),demux->sampleRate, demux->channels))
	{
		re = false;
		cout << "at->Open failed!!!" << endl;
	}
	totalMs = demux->totalMs;
	mux.unlock();
	//ȫ���ɹ�
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
	//������ǰ�߳�
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