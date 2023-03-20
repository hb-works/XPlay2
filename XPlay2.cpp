#include "XPlay2.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include "XDemuxThread.h"
static XDemuxThread dt;
XPlay2::XPlay2(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	//dt Start
	dt.Start();
	//��ʱ������ ��������ʾ
	startTimer(40);
}

XPlay2::~XPlay2()
{
	//������������Close()
	dt.Close();
}

//��ס������
void XPlay2::SliderPress()
{
	isSliderPress = true;
}

//�ɿ������� ��seek����
void XPlay2::SliderRelease()
{
	isSliderPress = false;
	double pos = 0.0;
	pos = (double)ui.playPos->value() / (double)ui.playPos->maximum();
	dt.Seek(pos);
}

//��ʱ�� ��������ʾ
void XPlay2::timerEvent(QTimerEvent *e)
{
	//�������������ס ��ֱ��return����Ҫ�ٳ���ˢ�µ�����λ����
	if (isSliderPress)
		return;
	long long total = dt.totalMs;
	if (total > 0)
	{
		//�õ�λ������
		double pos = (double)dt.pts / (double)total;
		//��ԭ��Ƶ�ֳ���1000�ݣ�maximum==999����*��Ӧ�ı����Ϳ��Եõ����ŵ�λ��
		int v = ui.playPos->maximum() * pos;
		//��ʾ��������λ��
		ui.playPos->setValue(v);
	}
}

//˫��ȫ��
void XPlay2::mouseDoubleClickEvent(QMouseEvent *e)
{
	//�����ȫ�����ٴ�˫���ͱ��Normal״̬
	if (isFullScreen())
		this->showNormal();
	else
		this->showFullScreen();
}

//���ڳߴ�仯 ����reSizeEvent����
void XPlay2::resizeEvent(QResizeEvent *e)
{
	ui.playPos->move(50, this->height() - 100);
	ui.playPos->resize(this->width() - 100, ui.playPos->height());
	ui.openFile->move(100, this->height() - 150);
	ui.isPlay->move(ui.openFile->x() + ui.openFile->width() + 10, ui.openFile->y());
	//��video��size��Ϊ����ǰ����һ���Ϳ�����
	ui.openGLWidget->resize(this->size());
}

//����or��ͣ
void XPlay2::PlayOrPause()
{
	//��ͣ�󻹻��������һС�λ�������е����ݣ��������û���⡣
	//��ͣ�ڽ������Ƶ����ҲҪ������ͣ
	bool isPause = !dt.isPause;
	SetPause(isPause);
	dt.SetPause(isPause);
}

//��ʾ�Ƿ���ͣ:��ͣ״̬��ʾ���ţ�����״̬��ʾ��ͣ
void XPlay2::SetPause(bool isPause)
{
	if (isPause)
		ui.isPlay->setText(QString::fromLocal8Bit("�� ��"));
	else
		ui.isPlay->setText(QString::fromLocal8Bit("�� ͣ"));
}

//���ⲿ����Ƶ�ļ�
void XPlay2::OpenFile()
{
	//���û�ѡ���ļ�
	QString name = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("ѡ����Ƶ�ļ�"));
	//���û��ѡ���ļ� ��ֱ��return
	if (name.isEmpty())
		return;
	this->setWindowTitle(name);
	//toLocal8Bit() ת��unicode
	if (!dt.Open(name.toLocal8Bit(), ui.openGLWidget))
	{
		QMessageBox::information(0, "error", "open file failed!!!");
		return;
	}
	//�򿪳ɹ�
	SetPause(dt.isPause);

	//���ļ�

}
