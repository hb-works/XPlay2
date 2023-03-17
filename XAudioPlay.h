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
	//关闭
	virtual void Close() = 0;

	//播放音频
	virtual bool Write(const unsigned char *data, int datasize) = 0;
	//判断是否有足够的空间写
	virtual int GetFree() = 0;

	static XAudioPlay *Get();
	XAudioPlay();
	virtual ~XAudioPlay();
};

