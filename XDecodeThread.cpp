#include "XDecodeThread.h"
#include "XDecode.h"

//������Դ��ֹͣ�߳�
void XDecodeThread::Close()
{
	Clear();
	
	isExit = true;
	wait();
	//�ȴ��߳��˳�����ܽ���decode������Ϊ�߳����п��ܷ���decode
	decode->Close();
	mux.lock();
	delete decode;
	decode = NULL;
	mux.unlock();
}

//������У��ڻ���ά�����еĴ���
void XDecodeThread::Clear()
{
	mux.lock();
	decode->Clear();
	//�������
	while (!packs.empty())
	{
		AVPacket *pkt = packs.front();
		XFreePacket(&pkt);
		packs.pop_front();
	}
	mux.unlock();
}

//ȡ��һ֡���ݣ�����ջ����������ݷ���NULL
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

//������߳���ΰ�����ӹ���
void XDecodeThread::Push(AVPacket *pkt)
{
	if (!pkt)
		return;
	//���� isExit = false�����˳�
	while (!isExit)
	{
		mux.lock();
		//����û�г���maxListʱ����ӽ�������
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
	//�򿪽�����
	if (!decode)
		decode = new XDecode();
}

XDecodeThread::~XDecodeThread()
{
	//�ȴ��߳��˳�
	isExit = true;
	wait();
}