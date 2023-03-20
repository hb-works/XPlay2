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
	//创建对象并打开
	virtual bool Open(const char *url, IVideoCall *call);
	//启动所有线程
	virtual void Start();

	//关闭线程，清理资源
	virtual void Close();
	virtual void Clear();

	//松开滑动条 进行定位
	virtual void Seek(double pos);

	void run();
	XDemuxThread();
	virtual ~XDemuxThread();
	bool isExit = false;

	//定时器 滑动条实现需要 demux本身不开放，把这两个变量开放出去，以便实现滑动条
	long long pts = 0;
	long long totalMs = 0;
	void SetPause(bool isPause);
	//设置一个是否暂停的状态
	bool isPause = false;
protected:
	std::mutex mux;
	XDemux *demux = 0;
	XVideoThread *vt = 0;
	XAudioThread *at = 0;
};

