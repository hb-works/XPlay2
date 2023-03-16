#include "XDemux.h"
#include <iostream>
using namespace std;

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "swresample.lib")

static double r2d(AVRational r)
{
	return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}
bool XDemux::Open(const char *url)
{
	//open֮ǰ��close,Close()��λ��Ҫע�� ��Ҫ���������棬��Ȼ��������ˣ���ΪClose����Ҳ����
	Close();

	//��������
	AVDictionary *opts = NULL;
	//����rtsp����tcpЭ���
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);

	//������ʱʱ��
	av_dict_set(&opts, "max_delay", "500", 0);

	mux.lock();
	int re = avformat_open_input(
		&ic,
		url,
		0,		//0	��ʾ�Զ�ѡ������
		&opts	//�������ã� ����rtsp����ʱʱ��
	);
	if (re != 0)
	{
		mux.unlock();
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		cout << "open" << url << "failed!:" << buf << endl;
		return false;
	}
	cout << "open" << url << "success!" << endl;


	//��ȡ����Ϣ
	re = avformat_find_stream_info(ic, 0);
	//��ȡ��ʱ�� ����
	int totalMs = ic->duration / (AV_TIME_BASE / 1000);
	cout << "��ʱ����" << totalMs << "����" << endl;
	//��ӡ��Ƶ������ϸ��Ϣ
	av_dump_format(ic, 0, url, 0);

	///��ȡ��Ƶ��
	videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	AVStream *as = ic->streams[videoStream];
	cout << "=======================================" << endl;
	cout << videoStream << "��Ƶ��Ϣ" << endl;
	//��������п���û�� �ȽϿɿ����ǽ�����AVFamily
	//���
	cout << "width = " << as->codecpar->width << endl;
	//�߶�
	cout << "height = " << as->codecpar->height << endl;
	//֡�� fps AVRational
	cout << "video fps = " << r2d(as->avg_frame_rate) << endl;

	///��ȡ��Ƶ��
	audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	as = ic->streams[audioStream];
	cout << "========================================" << endl;
	cout << audioStream << "��Ƶ��Ϣ" << endl;
	//������
	cout << "sample_rate = " << as->codecpar->sample_rate << endl;
	// AVSampleFormat
	cout << "format = " << as->codecpar->format << endl;
	//ͨ����
	cout << "channels = " << as->codecpar->channels << endl;
	cout << "codec_id = " << as->codecpar->codec_id << endl;
	cout << "audio fps = " << r2d(as->avg_frame_rate) << endl;
	//һ֡���ݣ��� ��ͨ��������
	cout << "frame_size = " << as->codec->frame_size << endl;
	// 1024 * 2 * 2 = 4096  fps = sample_rate/frame_size
	mux.unlock();

	return true;
}

//�ռ���Ҫ�������ͷţ��ͷ�AVPacket����ռ䣬�����ݿռ� av_packet_free
AVPacket *XDemux::Read()
{
	//����ʱ��Ҳ��Ҫ��������Ϊ����ʱ��ǰ���ic���ܻ�down��
	mux.lock();

	//ic��û������,Ҳ����˵�ļ���û�򿪾͵���Read()
	if (!ic)	//�ݴ�
	{
		mux.unlock();
		return 0;
	}
	//����Ѿ����� ��Ҫ��ȡ����
	AVPacket *pkt = av_packet_alloc();
	//��ȡһ֡��������һ֡�Ŀռ�
	int re = av_read_frame(ic, pkt);
	//�п���ʧ��
	if (re != 0)
	{
		mux.unlock();
		//ע�⣺������ɹ�Ҫ�ǵ��ͷ�pkt�ռ䣬��Ȼ������ڴ�й©
		av_packet_free(&pkt);
		return 0;
	}
	//ptsת��Ϊ����
	//����stream_index����������Ƶ������Ƶ
	pkt->pts = pkt->pts * (r2d(ic->streams[pkt->stream_index]->time_base) * 1000);
	pkt->dts = pkt->dts * (r2d(ic->streams[pkt->stream_index]->time_base) * 1000);
	
	mux.unlock(); //�ͷ���
	cout << pkt->pts << " " << flush;
	return pkt;
}

//��ȡ��Ƶ���� ���صĿռ���Ҫ����(����avcodec_parameters_free��������)����Ϊ���Ǹ�����һ�ݵ�
AVCodecParameters *XDemux::CopyVPara()
{
	mux.lock();
	if (!ic)	//�ݴ�
	{
		mux.unlock();
		return NULL;
	}
	//����һ����Ƶ������Ϣ
	AVCodecParameters *pa = avcodec_parameters_alloc();
	avcodec_parameters_copy(pa, ic->streams[videoStream]->codecpar);
	mux.unlock();
	return pa;
}

//��ȡ��Ƶ���� ���صĿռ���Ҫ����(����avcodec_parameters_free��������)����Ϊ���Ǹ�����һ�ݵ�
AVCodecParameters *XDemux::CopyAPara()
{
	mux.lock();
	if (!ic)	//�ݴ�
	{
		mux.unlock();
		return NULL;
	}
	//����һ����Ƶ������Ϣ
	AVCodecParameters *pa = avcodec_parameters_alloc();
	avcodec_parameters_copy(pa, ic->streams[audioStream]->codecpar);
	mux.unlock();
	return pa;
}

//��ն�ȡ����
void XDemux::Clear()
{
	mux.lock();
	if (!ic)
	{
		mux.unlock();
		return;
	}
	//�����ȡ����
	avformat_flush(ic);
	mux.unlock();
}

void XDemux::Close()
{
	mux.lock();
	if (!ic)
	{
		mux.unlock();
		return;
	}
	avformat_close_input(&ic);
	//ý����ʱ����ʼ��
	totalMs = 0;
	mux.unlock();
}

//seek λ�� pos��0.0~1.0֮�� ���ưٷֱ�
bool XDemux::Seek(double pos)
{
	mux.lock();
	if (!ic)	//�ݴ�
	{
		mux.unlock();
		return false;
	}
	//�����ȡ���� seek���µ�λ�����������ǰ�Ļ��������⣬���ܳ���ճ������
	avformat_flush(ic);
	long long seekPos = 0;
	seekPos = ic->streams[videoStream]->duration * pos;
	//int ms = 3000; //����λ�� ����ʱ�������������ת��
	//long long pos = (double)ms / (double)1000 * r2d(ic->streams[pkt->stream_index]->time_base);
	//av_seek_frame �൱��������������������λ���ؼ�֡��AVSEEK_FLAG_BACKWARD�����ң��������֡��Σ�գ�������ǹؼ�֡�ǽ��벻������
	//��û�ж�λ������Ҫ������λ�ã�ֻ�Ƕ�λ������Ҫ��λ�õ�ǰһ���ؼ�֡λ��
	int re = av_seek_frame(ic, videoStream, seekPos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
	mux.unlock();
	
	//re>=0���ǳɹ�
	if (re < 0)
	{
		return false;
	}
	return true;

}

XDemux::XDemux()
{
	static bool isFirst = true;
	//���� ��ֹ���̴߳�������ʱ������
	static std::mutex dmux;
	dmux.lock();
	if (isFirst)	//����ǵ�һ�γ�ʼ��
	{
		//��ʼ����װ��
		av_register_all();

		//��ʼ������� (���Դ�rtsp rtmp http Э�����ý����Ƶ)
		avformat_network_init();
		isFirst = false;
	}
	dmux.unlock();
}

XDemux::~XDemux()
{

}