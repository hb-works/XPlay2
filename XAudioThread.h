#pragma once
#include <QThread>
#include <mutex>
#include <iostream>
#include "XDecodeThread.h"
struct AVCodecParameters;
class XAudioPlay;
class XResample;
//�̳��߳�QThread
class XAudioThread:public XDecodeThread
{
public:
	//��ǰ��Ƶ���ŵ�pts
	long long pts = 0;
	//��Ƶ�򿪣����ܳɹ��������ռ�
	virtual bool Open(AVCodecParameters *para, int sampleRate, int channels);
	//ֹͣ�̣߳�������Դ
	virtual void Close();
	virtual void Clear();

	void run();
	XAudioThread();
	virtual ~XAudioThread();

	void SetPause(bool isPause);
	//����һ���Ƿ���ͣ��״̬
	bool isPause = false;
protected:
	std::mutex amux;
	XAudioPlay *ap = 0;
	XResample *res = 0;
};

