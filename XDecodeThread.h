#pragma once
//����XAudioThread �� XVideoThread�еĹ��ܣ���Ϊ���ǵĻ���
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
	//������߳���ΰ�����ӹ���
	virtual void Push(AVPacket *pkt);

	//�������
	virtual void Clear();

	//������Դ��ֹͣ�߳�
	virtual void Close();

	//ȡ��һ֡���ݣ�����ջ����������ݷ���NULL
	virtual AVPacket *Pop();
	//������
	int maxList = 100;
	//��isExit�������߳��˳�������һֱ���������Ҫ�˳���isExit��Ϊtrue
	bool isExit = false;

	
protected:
	XDecode *decode = 0;
	//�൱�������ߺ�������ģʽ��Push(AVPacket *pkt)�������˶�����
	std::list <AVPacket *> packs;
	std::mutex mux;
	
};