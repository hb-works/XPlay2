#pragma once
#include <list>
#include <mutex>
#include <QThread>
#include "IVideoCall.h"
#include "XDecodeThread.h"
struct AVPacket;
struct AVCodecParameters;
class XDecode;
//����������ʾ��Ƶ
class XVideoThread : public XDecodeThread
{
public:
	//����pts��������յ��Ľ�������pts >= seekpts return true ������ʾ����
	virtual bool RepaintPts(AVPacket *pkt, long long seekpts);

	//��Ƶ�򿪣����ܳɹ��������ռ�
	virtual bool Open(AVCodecParameters *para, IVideoCall *call, int width, int height);
	void run();

	XVideoThread();
	virtual ~XVideoThread();

	//ͬ��ʱ�䣬���ⲿ����
	long long synpts = 0;

	void SetPause(bool isPause);
	//����һ���Ƿ���ͣ��״̬
	bool isPause = false;

	
protected:
	std::mutex vmux;
	IVideoCall *call = 0;
};

