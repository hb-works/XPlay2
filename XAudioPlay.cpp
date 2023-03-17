#include "XAudioPlay.h"
#include <QAudioFormat>
#include <QAudioOutput>
#include <mutex>
//��Qt�йصĶ�д���������
class CXAudioPlay :public XAudioPlay
{
public:
	QAudioOutput *output = NULL;
	QIODevice *io = NULL;
	std::mutex mux;
	virtual void Close()
	{
		//Ҫ��������Ϊ�򿪺͹ر��п��ܲ���һ���߳�
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
		Close();//ÿ�δ�ǰ���ȹرգ������´�
		QAudioFormat fmt;
		fmt.setSampleRate(sampleRate);
		fmt.setSampleSize(sampleSize);
		fmt.setChannelCount(channels);
		fmt.setCodec("audio/pcm");
		fmt.setByteOrder(QAudioFormat::LittleEndian);
		fmt.setSampleType(QAudioFormat::UnSignedInt);
		mux.lock();
		output = new QAudioOutput(fmt);
		io = output->start();//��ʼ������Ƶ����
		mux.unlock();
		if(io)	
			return true;
		return false;
	}

	//������Ƶ
	virtual bool Write(const unsigned char *data, int datasize)
	{
		if (!data || data <= 0)
			return false;
		mux.lock();
		if (!output || !io)
		{
			//δ��ʼ����
			mux.unlock();
			return 0;
		}
		//д������
		int size = io->write((char *)data, datasize);
		mux.unlock();
		//���ֻ��д��һ����
		if (datasize != size)
			return false;
		return true;
	}
	//�ж��Ƿ����㹻�Ŀռ�д
	virtual int GetFree()
	{
		mux.lock();
		if (!output)
		{
			//δ��ʼ����
			mux.unlock();
			return 0;
		}
		//��ȡ��free�Ŀռ��С
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