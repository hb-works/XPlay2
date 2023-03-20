#include "XDecode.h"
#include <iostream>
extern "C"
{
	#include "libavcodec/avcodec.h"
}
using namespace std;

void XFreePacket(AVPacket **pkt)
{
	if (!pkt || !(*pkt))
		return;
	av_packet_free(pkt);
}

void XFreeFrame(AVFrame **frame)
{
	if (!frame || !(*frame))
		return;
	av_frame_free(frame);
}

//�رպ�����
void XDecode::Clear()
{
	mux.lock();
	//������뻺��
	if (codec)
	{
		avcodec_flush_buffers(codec);
	}
	mux.unlock();
}
void XDecode::Close()
{
	mux.lock();
	if (codec)
	{
		avcodec_close(codec);
		avcodec_free_context(&codec);
	}
	pts = 0;
	mux.unlock();
}

//�򿪽�����
bool XDecode::Open(AVCodecParameters *para)
{
	if (!para)
	{
		//�������Ϊ�� ֱ�ӷ���false
		return false;
	}
	//��ǰ�ȹر�
	Close();
	///��������
	///�ҵ�������
	AVCodec *vcodec = avcodec_find_decoder(para->codec_id);
	//����ط��п����Ҳ��������������ԼӸ��ж�
	if (!vcodec)
	{
		//ʧ��ҲҪ�ͷ�para����Ȼ������ڴ�й©
		avcodec_parameters_free(&para);
		cout << "can't find the codec id " << para->codec_id << endl;
		return false;
	}
	//�ҵ�������
	cout << "find the AVCodec " << para->codec_id << endl;

	mux.lock();

	///����������������
	codec = avcodec_alloc_context3(vcodec);

	///���ý����������Ĳ���
	avcodec_parameters_to_context(codec, para);
	avcodec_parameters_free(&para);

	//���߳̽���
	codec->thread_count = 8;

	///�򿪽�����������
	int re = avcodec_open2(codec, 0, 0);
	//���re��=0����ʧ����
	if (re != 0)
	{
		//���ʧ�� �ͷſռ�
		avcodec_free_context(&codec);
		mux.unlock();
		//��buffer����Ŵ�����Ϣ
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		cout << "avcodec_open2 failed!:" << buf << endl;
		return false;
	}
	//�򿪳ɹ�
	mux.unlock();
	cout << "avcodec_open2 success!" << endl;
	avcodec_parameters_free(&para);
	return true;
}

//���͵������̣߳����ܳɹ�����ͷ�pkt�ռ䣨�����ý�����ݣ�
bool XDecode::Send(AVPacket *pkt)
{
	//���pkt�����ڻ���pkt��sizeС�ڵ���0����pkt��data�����ڣ�ֱ�ӷ���false
	if (!pkt || pkt->size <= 0 || !pkt->data)
		return false;
	mux.lock();
	//���������û�� ���� ��ʧ�� ֱ�ӽ�������false
	if (!codec)
	{
		mux.unlock();
		return false;
	}
	int re = avcodec_send_packet(codec, pkt);
	
	mux.unlock();
	av_packet_free(&pkt);
	if (re != 0)
		return false;
	return true;
}

//��ȡ���������ݣ�һ��send������Ҫ���recv����ȡ�����е�����Send NULL��Recv���
//ÿ�θ���һ�ݣ��ɵ������ͷ� av_frame_free  �ռ��ͷ���C++�������������
//�ռ���ص�����Ҫ��ϸ��������Ƶ�ܶ�ʱ�䶼�������ڴ�й©��
AVFrame *XDecode::Recv()
{
	mux.lock();
	//���������û�� ���� ��ʧ�� ֱ�ӽ�������0
	if (!codec)
	{
		mux.unlock();
		return 0;
	}
	//����һ���ռ������ս�������
	AVFrame *frame = av_frame_alloc();
	//������д�뵽frame�У��ɹ�����0
	int re = avcodec_receive_frame(codec, frame);
	mux.unlock();
	if (re != 0) 
	{
		//ʧ����Ҫ�ѿռ��ͷŵ���Ȼ�󷵻�NULL
		av_frame_free(&frame);
		return NULL;
	}
	//cout << "[" << frame->linesize[0] << "] " << flush;
	pts = frame->pts;
	//�ɹ��˾Ͱѽ��յ������ݷ���
	return frame;
}

XDecode::XDecode()
{
	
}

XDecode::~XDecode()
{

}