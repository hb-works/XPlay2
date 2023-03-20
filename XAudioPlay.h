#pragma once
//重采样完后要进行音频播放
class XAudioPlay
{
public:
	int sampleRate = 44100;
	int sampleSize = 16;	//位数
	int channels = 2;
	//打开音频播放 定义为纯虚函数
	virtual bool Open() = 0;
	//关闭并清理
	virtual void Close() = 0;
	virtual void Clear() = 0;
	//返回减去缓冲中未播放的时间（ms）
	virtual long long GetNoPlayMs() = 0;

	//播放音频
	virtual bool Write(const unsigned char *data, int datasize) = 0;
	//判断是否有足够的空间写
	virtual int GetFree() = 0;

	//设置暂停
	virtual void SetPause(bool isPause) = 0;

	static XAudioPlay *Get();
	XAudioPlay();
	virtual ~XAudioPlay();
};

