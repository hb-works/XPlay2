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
	//open之前先close,Close()的位置要注意 不要放在锁里面，不然造成死锁了，因为Close里面也有锁
	Close();

	//参数设置
	AVDictionary *opts = NULL;
	//设置rtsp流以tcp协议打开
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);

	//网络延时时间
	av_dict_set(&opts, "max_delay", "500", 0);

	mux.lock();
	int re = avformat_open_input(
		&ic,
		url,
		0,		//0	表示自动选择解封器
		&opts	//参数设置， 比如rtsp的延时时间
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


	//获取流信息
	re = avformat_find_stream_info(ic, 0);
	//获取总时长 毫秒
	int totalMs = ic->duration / (AV_TIME_BASE / 1000);
	cout << "总时长：" << totalMs << "毫秒" << endl;
	//打印视频流的详细信息
	av_dump_format(ic, 0, url, 0);

	///获取视频流
	videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	AVStream *as = ic->streams[videoStream];
	cout << "=======================================" << endl;
	cout << videoStream << "视频信息" << endl;
	//宽高数据有可能没有 比较可靠的是解码后的AVFamily
	//宽度
	cout << "width = " << as->codecpar->width << endl;
	//高度
	cout << "height = " << as->codecpar->height << endl;
	//帧率 fps AVRational
	cout << "video fps = " << r2d(as->avg_frame_rate) << endl;

	///获取音频流
	audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	as = ic->streams[audioStream];
	cout << "========================================" << endl;
	cout << audioStream << "音频信息" << endl;
	//采样率
	cout << "sample_rate = " << as->codecpar->sample_rate << endl;
	// AVSampleFormat
	cout << "format = " << as->codecpar->format << endl;
	//通道数
	cout << "channels = " << as->codecpar->channels << endl;
	cout << "codec_id = " << as->codecpar->codec_id << endl;
	cout << "audio fps = " << r2d(as->avg_frame_rate) << endl;
	//一帧数据？？ 单通道样本数
	cout << "frame_size = " << as->codec->frame_size << endl;
	// 1024 * 2 * 2 = 4096  fps = sample_rate/frame_size
	mux.unlock();

	return true;
}

//空间需要调用者释放，释放AVPacket对象空间，和数据空间 av_packet_free
AVPacket *XDemux::Read()
{
	//读的时候也需要加锁，因为读的时候，前面的ic可能会down掉
	mux.lock();

	//ic还没创建好,也就是说文件还没打开就调用Read()
	if (!ic)	//容错
	{
		mux.unlock();
		return 0;
	}
	//如果已经打开了 就要读取数据
	AVPacket *pkt = av_packet_alloc();
	//读取一帧并分配这一帧的空间
	int re = av_read_frame(ic, pkt);
	//有可能失败
	if (re != 0)
	{
		mux.unlock();
		//注意：如果不成功要记得释放pkt空间，不然会造成内存泄漏
		av_packet_free(&pkt);
		return 0;
	}
	//pts转换为毫秒
	//根据stream_index来区分是音频还是视频
	pkt->pts = pkt->pts * (r2d(ic->streams[pkt->stream_index]->time_base) * 1000);
	pkt->dts = pkt->dts * (r2d(ic->streams[pkt->stream_index]->time_base) * 1000);
	
	mux.unlock(); //释放锁
	cout << pkt->pts << " " << flush;
	return pkt;
}

//获取视频参数 返回的空间需要清理(调用avcodec_parameters_free进行清理)，因为这是复制了一份的
AVCodecParameters *XDemux::CopyVPara()
{
	mux.lock();
	if (!ic)	//容错
	{
		mux.unlock();
		return NULL;
	}
	//拷贝一份视频参数信息
	AVCodecParameters *pa = avcodec_parameters_alloc();
	avcodec_parameters_copy(pa, ic->streams[videoStream]->codecpar);
	mux.unlock();
	return pa;
}

//获取音频参数 返回的空间需要清理(调用avcodec_parameters_free进行清理)，因为这是复制了一份的
AVCodecParameters *XDemux::CopyAPara()
{
	mux.lock();
	if (!ic)	//容错
	{
		mux.unlock();
		return NULL;
	}
	//拷贝一份音频参数信息
	AVCodecParameters *pa = avcodec_parameters_alloc();
	avcodec_parameters_copy(pa, ic->streams[audioStream]->codecpar);
	mux.unlock();
	return pa;
}

//清空读取缓存
void XDemux::Clear()
{
	mux.lock();
	if (!ic)
	{
		mux.unlock();
		return;
	}
	//清理读取缓冲
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
	//媒体总时长初始化
	totalMs = 0;
	mux.unlock();
}

//seek 位置 pos：0.0~1.0之间 类似百分比
bool XDemux::Seek(double pos)
{
	mux.lock();
	if (!ic)	//容错
	{
		mux.unlock();
		return false;
	}
	//清理读取缓冲 seek到新的位置如果还有以前的缓冲会出问题，可能出现粘包现象
	avformat_flush(ic);
	long long seekPos = 0;
	seekPos = ic->streams[videoStream]->duration * pos;
	//int ms = 3000; //三秒位置 根据时间基数（分数）转换
	//long long pos = (double)ms / (double)1000 * r2d(ic->streams[pkt->stream_index]->time_base);
	//av_seek_frame 相当于拖拉条：满足条件后定位到关键帧，AVSEEK_FLAG_BACKWARD往后找；如果任意帧很危险，如果不是关键帧是解码不出来的
	//并没有定位到你想要的真正位置，只是定位到你想要的位置的前一个关键帧位置
	int re = av_seek_frame(ic, videoStream, seekPos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
	mux.unlock();
	
	//re>=0都是成功
	if (re < 0)
	{
		return false;
	}
	return true;

}

XDemux::XDemux()
{
	static bool isFirst = true;
	//加锁 防止多线程创建对象时出问题
	static std::mutex dmux;
	dmux.lock();
	if (isFirst)	//如果是第一次初始化
	{
		//初始化封装库
		av_register_all();

		//初始化网络库 (可以打开rtsp rtmp http 协议的流媒体视频)
		avformat_network_init();
		isFirst = false;
	}
	dmux.unlock();
}

XDemux::~XDemux()
{

}