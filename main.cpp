#include "XPlay2.h"
#include "XResample.h"
#include <QtWidgets/QApplication>
#include <iostream>
#include "XDemux.h"
#include "XDecode.h"
#include <QThread>
#include "XAudioPlay.h"
#include "XAudioThread.h"
#include "XVideoThread.h"
using namespace std;

class TestThread : public QThread
{
public:
	XAudioThread at;
	XVideoThread vt;
	void Init()
	{
		//��������ӣ��������Ѿ��޷����ˣ�
        const char *url = "rtmp://live.hkstv.hk.lxdns.com/live/hks";
		//�����ļ�
		url = "v1080.mp4";
		cout << "demux.Open = " << demux.Open(url) << endl;
		//cout << "CopyVPara = " << demux.CopyVPara() << endl;
		//cout << "CopyAPara = " << demux.CopyAPara() << endl;
		//cout << "seek = " << demux.Seek(0.95) << endl;

		//cout << "vdecode.Open() = " << vdecode.Open(demux.CopyVPara()) << endl;
		//vdecode.Clear();
		//vdecode.Close();
		//cout << "adecode.Open() = " << adecode.Open(demux.CopyAPara()) << endl;
		//cout << "resample.Open = " << resample.Open(demux.CopyAPara());
		//XAudioPlay::Get()->channels = demux.channels;
		//XAudioPlay::Get()->sampleRate = demux.sampleRate;

		//cout << "XAudioPlay::Get()->Open() = " << XAudioPlay::Get()->Open() << endl;

		cout << "at.Open = " << at.Open(demux.CopyAPara(), demux.sampleRate, demux.channels);
		cout << "vt.Open = " << vt.Open(demux.CopyVPara(), video, demux.width, demux.height);
		at.start();
		vt.start();
	}
	
	void run()
	{
		for (;;)
		{
			AVPacket *pkt = demux.Read();
			//��Ƶ
			if (demux.IsAudio(pkt))
			{
				at.Push(pkt);
				//adecode.Send(pkt);
				//AVFrame *frame = adecode.Recv();
				////len:�ز����Ŀռ��С
				//int len = resample.Resample(frame, pcm);
				//cout << "Resample: " << len << " ";
				//while (len > 0)
				//{
				//	if (XAudioPlay::Get()->GetFree() >= len)
				//	{
				//		//�ռ��㹻��д��
				//		XAudioPlay::Get()->Write(pcm, len);
				//		break;
				//	}
				//	//ǰ�����Ƶû�������ȴ�
				//	msleep(1);
				//}
				//cout << "Audio:" << frame << endl;
			}
			//��Ƶ
			else
			{
				vt.Push(pkt);
				//vdecode.Send(pkt);
				//AVFrame *frame = vdecode.Recv();
				//video->Repaint(frame);
				//msleep(40);
				//cout << "Video:" << frame << endl;
			}
			if (!pkt)
				break;
		}		
	}
	//����XDemux
	XDemux demux;
	//�������
	//XDecode adecode;
	//XDecode vdecode;
	//XResample resample;
	XVideoWidget *video = 0;

	unsigned char *pcm = new unsigned char[1024 * 1024];
};
#include "XDemuxThread.h"
//v1080.mp4
int main(int argc, char *argv[])
{
	
	//TestThread tt;
	QApplication a(argc, argv);
	XPlay2 w;
	w.show();

	//��ʼ��gl����
	//w.ui.openGLWidget->Init(tt.demux.width, tt.demux.height);
	//tt.video = w.ui.openGLWidget;
	//tt.Init();
	//tt.start();
	//XDemuxThread dt;
	//char *url = "rtmp://live.hkstv.hk.lxdns.com/live/hks";
	//url = "v1080.mp4";
	//dt.Open(url, w.ui.openGLWidget);
	//dt.Start();

	return a.exec();
}
