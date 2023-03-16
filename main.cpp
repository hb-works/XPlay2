#include "XPlay2.h"
#include <QtWidgets/QApplication>
#include <iostream>
#include "XDemux.h"
using namespace std;
//v1080.mp4
int main(int argc, char *argv[])
{
	//����XDemux
	XDemux demux;

	//���������
	char *url = "rtmp://live.hkstv.hk.lxdns.com/live/hks";
	//�����ļ�
	url = "v1080.mp4";
	cout << "demux.Open = " << demux.Open(url) << endl;
	demux.Read();
	demux.Clear();
	demux.Close();
	cout << "CopyVPara = " << demux.CopyVPara() << endl;
	cout << "CopyAPara = " << demux.CopyAPara() << endl;
	cout << "seek = " << demux.Seek(0.95) << endl;

	for (;;)
	{
		AVPacket *pkt = demux.Read();
		if (!pkt)
			break;
	}

	QApplication a(argc, argv);
	XPlay2 w;
	w.show();
	return a.exec();
}
