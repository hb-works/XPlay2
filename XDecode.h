#pragma once
#include <iostream>
#include <mutex>
struct AVCodecParameters;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
class XDecode
{
public:
	//����һ����Ա��ȷ����ǰ������Ƶ������Ƶ
	bool isAudio = false;

	//�򿪽�����,���ܳɹ�����ͷ�para�ռ�
	virtual bool Open(AVCodecParameters *para);

	//���͵������̣߳����ܳɹ�����ͷ�pkt�ռ䣨�����ý�����ݣ�
	virtual bool Send(AVPacket *pkt);
	
	//��ȡ���������ݣ�һ��send������Ҫ���recv����ȡ�����е�����Send NULL��Recv���
	//ÿ�θ���һ�ݣ��ɵ������ͷ� av_frame_free  �ռ��ͷ���C++�������������
	//�ռ���ص�����Ҫ��ϸ��������Ƶ�ܶ�ʱ�䶼�������ڴ�й©��
	virtual AVFrame *Recv();

	virtual void Clear();
	virtual void Close();


	XDecode();
	virtual ~XDecode();

protected:
	AVCodecContext *codec = 0;
	std::mutex mux;
};

