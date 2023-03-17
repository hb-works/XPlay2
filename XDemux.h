#pragma once
#include <mutex>
struct AVFormatContext;
struct AVPacket;
struct AVCodecParameters;
class XDemux
{
public:
	//打开媒体文件或流媒体 rtmp http rstp
	virtual bool Open(const char *url);

	//空间需要调用者释放，释放AVPacket对象空间，和数据空间 av_packet_free
	virtual AVPacket *Read();

	//判断是音频还是视频
	virtual bool IsAudio(AVPacket *pkt);

	//获取视频参数 返回的空间需要清理(调用avcodec_parameters_free进行清理)，因为这是复制了一份的
	virtual AVCodecParameters *CopyVPara();

	//获取音频参数 返回的空间需要清理(调用avcodec_parameters_free进行清理)，因为这是复制了一份的
	virtual AVCodecParameters *CopyAPara();

	//seek 位置 pos：0.0~1.0之间 类似百分比
	virtual bool Seek(double pos);

	//清空读取缓存
	virtual void Clear();
	virtual void Close();

	XDemux();
	virtual ~XDemux();

	//媒体总时长 （毫秒）
	int totalMs = 0;

	int width = 0;
	int height = 0;
	int sampleRate = 0;
	int channels = 0;

protected:
	//加一个互斥变量，原则是尽晚加锁，尽早释放
	std::mutex mux;
	//解封装上下文
	AVFormatContext *ic = NULL;
	//音视频索引，读取时区分音视频
	int videoStream = 0;
	int audioStream = 1;
};

