#pragma once
//整合XAudioThread 和 XVideoThread中的功能，作为它们的基类
#include <list>
#include <mutex>
#include <QThread>
struct AVPacket;
class XDecode;
class XDecodeThread : public QThread
{
public:
	XDecodeThread();
	virtual ~XDecodeThread();
	//另外的线程如何把任务加过来
	virtual void Push(AVPacket *pkt);

	//清理队列
	virtual void Clear();

	//清理资源，停止线程
	virtual void Close();

	//取出一帧数据，并出栈，如果无数据返回NULL
	virtual AVPacket *Pop();
	//最大队列
	int maxList = 100;
	//用isExit来控制线程退出，避免一直堵塞，如果要退出把isExit设为true
	bool isExit = false;

	
protected:
	XDecode *decode = 0;
	//相当于生产者和消费者模式，Push(AVPacket *pkt)生产到此队列中
	std::list <AVPacket *> packs;
	std::mutex mux;
	
};