#include "XAudioPlay.h"
#include <QAudioFormat>
#include <QAudioOutput>
#include <mutex>
//跟Qt有关的都写到这个类中
class CXAudioPlay :public XAudioPlay
{
public:
	QAudioOutput *output = NULL;
	QIODevice *io = NULL;
	std::mutex mux;

	virtual long long GetNoPlayMs()
	{
		mux.lock();
		if (!output)
		{
			mux.unlock();
			return 0;
		}

		long long pts = 0;
		//还未播放的音频的字节数
		double size = output->bufferSize() - output->bytesFree();
		//一秒音频的字节大小
		double secSize = sampleRate * (sampleSize / 8) * channels;
		if (secSize <= 0)
		{
			pts = 0;
		}
		else
		{
			//未播放的时间(ms)
			pts = (size / secSize) * 1000;
		}
		mux.unlock();
		return pts;
	}

	void Clear()
	{
		mux.lock();
		if (io)
		{
			io->reset();	
		}
		mux.unlock();
	}

	virtual void Close()
	{
		//要加锁，因为打开和关闭有可能不在一个线程
		mux.lock();
		if (io)
		{
			io->close();
			io = NULL;
		}
		if (output)
		{
			output->stop();
			delete output;
			output = 0;
		}
		mux.unlock();
	}

	virtual bool Open()
	{
		Close();//每次打开前都先关闭，再重新打开
		QAudioFormat fmt;
		fmt.setSampleRate(sampleRate);
		fmt.setSampleSize(sampleSize);
		fmt.setChannelCount(channels);
		fmt.setCodec("audio/pcm");
		fmt.setByteOrder(QAudioFormat::LittleEndian);
		fmt.setSampleType(QAudioFormat::UnSignedInt);
		mux.lock();
		output = new QAudioOutput(fmt);
		io = output->start();//开始播放音频数据
		mux.unlock();
		if(io)	
			return true;
		return false;
	}

	void SetPause(bool isPause)
	{
		mux.lock();
		if (!output)
		{
			mux.unlock();
			return;
		}
		if (isPause)
		{
			//把output挂起
			output->suspend();
		}
		else
		{
			//恢复
			output->resume();
		}
		mux.unlock();
	}

	//播放音频
	virtual bool Write(const unsigned char *data, int datasize)
	{
		if (!data || data <= 0)
			return false;
		mux.lock();
		if (!output || !io)
		{
			//未初始化好
			mux.unlock();
			return 0;
		}
		//写入数据
		int size = io->write((char *)data, datasize);
		mux.unlock();
		//如果只是写了一部分
		if (datasize != size)
			return false;
		return true;
	}
	//判断是否有足够的空间写
	virtual int GetFree()
	{
		mux.lock();
		if (!output)
		{
			//未初始化好
			mux.unlock();
			return 0;
		}
		//获取到free的空间大小
		int free = output->bytesFree();
		mux.unlock();
		return free;
	}
};

XAudioPlay *XAudioPlay::Get()
{
	static CXAudioPlay play;
	return &play;
}

XAudioPlay::XAudioPlay()
{

}

XAudioPlay::~XAudioPlay()
{

}