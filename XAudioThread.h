#pragma once
#include <QThread>
#include <mutex>
#include <iostream>
#include "XDecodeThread.h"
struct AVCodecParameters;
class XAudioPlay;
class XResample;
//继承线程QThread
class XAudioThread:public XDecodeThread
{
public:
	//当前音频播放的pts
	long long pts = 0;
	//音频打开，不管成功与否都清理空间
	virtual bool Open(AVCodecParameters *para, int sampleRate, int channels);
	//停止线程，清理资源
	virtual void Close();
	virtual void Clear();

	void run();
	XAudioThread();
	virtual ~XAudioThread();

	void SetPause(bool isPause);
	//设置一个是否暂停的状态
	bool isPause = false;
protected:
	std::mutex amux;
	XAudioPlay *ap = 0;
	XResample *res = 0;
};

