#pragma once
#include <QThread>
#include <mutex>
#include "IVideoCall.h"
class XDemux;
class XVideoThread;
class XAudioThread;

class XDemuxThread : public QThread
{
public:
	//�������󲢴�
	virtual bool Open(const char *url, IVideoCall *call);
	//���������߳�
	virtual void Start();

	//�ر��̣߳�������Դ
	virtual void Close();
	virtual void Clear();

	//�ɿ������� ���ж�λ
	virtual void Seek(double pos);

	void run();
	XDemuxThread();
	virtual ~XDemuxThread();
	bool isExit = false;

	//��ʱ�� ������ʵ����Ҫ demux�������ţ����������������ų�ȥ���Ա�ʵ�ֻ�����
	long long pts = 0;
	long long totalMs = 0;
	void SetPause(bool isPause);
	//����һ���Ƿ���ͣ��״̬
	bool isPause = false;
protected:
	std::mutex mux;
	XDemux *demux = 0;
	XVideoThread *vt = 0;
	XAudioThread *at = 0;
};

