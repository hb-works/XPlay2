#pragma once
#include <list>
#include <mutex>
#include <QThread>
#include "IVideoCall.h"
#include "XDecodeThread.h"
struct AVPacket;
struct AVCodecParameters;
class XDecode;
//负责解码和显示视频
class XVideoThread : public XDecodeThread
{
public:
	//解码pts，如果接收到的解码数据pts >= seekpts return true 并且显示画面
	virtual bool RepaintPts(AVPacket *pkt, long long seekpts);

	//视频打开，不管成功与否都清理空间
	virtual bool Open(AVCodecParameters *para, IVideoCall *call, int width, int height);
	void run();

	XVideoThread();
	virtual ~XVideoThread();

	//同步时间，由外部传入
	long long synpts = 0;

	void SetPause(bool isPause);
	//设置一个是否暂停的状态
	bool isPause = false;

	
protected:
	std::mutex vmux;
	IVideoCall *call = 0;
};

