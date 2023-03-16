#pragma once
#include <mutex>
struct AVFormatContext;
struct AVPacket;
struct AVCodecParameters;
class XDemux
{
public:
	//��ý���ļ�����ý�� rtmp http rstp
	virtual bool Open(const char *url);

	//�ռ���Ҫ�������ͷţ��ͷ�AVPacket����ռ䣬�����ݿռ� av_packet_free
	virtual AVPacket *Read();

	//��ȡ��Ƶ���� ���صĿռ���Ҫ����(����avcodec_parameters_free��������)����Ϊ���Ǹ�����һ�ݵ�
	virtual AVCodecParameters *CopyVPara();

	//��ȡ��Ƶ���� ���صĿռ���Ҫ����(����avcodec_parameters_free��������)����Ϊ���Ǹ�����һ�ݵ�
	virtual AVCodecParameters *CopyAPara();

	//seek λ�� pos��0.0~1.0֮�� ���ưٷֱ�
	virtual bool Seek(double pos);

	//��ն�ȡ����
	virtual void Clear();
	virtual void Close();

	XDemux();
	virtual ~XDemux();

	//ý����ʱ�� �����룩
	int totalMs = 0;

protected:
	//��һ�����������ԭ���Ǿ��������������ͷ�
	std::mutex mux;
	//���װ������
	AVFormatContext *ic = NULL;
	//����Ƶ��������ȡʱ��������Ƶ
	int videoStream = 0;
	int audioStream = 1;
};
