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
	//定时器启动 滑动条显示
	startTimer(40);
}

XPlay2::~XPlay2()
{
	//在析构函数中Close()
	dt.Close();
}

//按住滑动条
void XPlay2::SliderPress()
{
	isSliderPress = true;
}

//松开滑动条 做seek操作
void XPlay2::SliderRelease()
{
	isSliderPress = false;
	double pos = 0.0;
	pos = (double)ui.playPos->value() / (double)ui.playPos->maximum();
	dt.Seek(pos);
}

//定时器 滑动条显示
void XPlay2::timerEvent(QTimerEvent *e)
{
	//如果滑动条被按住 就直接return，不要再持续刷新到播放位置了
	if (isSliderPress)
		return;
	long long total = dt.totalMs;
	if (total > 0)
	{
		//得到位置所在
		double pos = (double)dt.pts / (double)total;
		//把原视频分成了1000份（maximum==999），*相应的倍数就可以得到播放的位置
		int v = ui.playPos->maximum() * pos;
		//显示进度条的位置
		ui.playPos->setValue(v);
	}
}

//双击全屏
void XPlay2::mouseDoubleClickEvent(QMouseEvent *e)
{
	//如果是全屏，再次双击就变回Normal状态
	if (isFullScreen())
		this->showNormal();
	else
		this->showFullScreen();
}

//窗口尺寸变化 重载reSizeEvent函数
void XPlay2::resizeEvent(QResizeEvent *e)
{
	ui.playPos->move(50, this->height() - 100);
	ui.playPos->resize(this->width() - 100, ui.playPos->height());
	ui.openFile->move(100, this->height() - 150);
	ui.isPlay->move(ui.openFile->x() + ui.openFile->width() + 10, ui.openFile->y());
	//把video的size设为跟当前窗口一样就可以了
	ui.openGLWidget->resize(this->size());
}

//播放or暂停
void XPlay2::PlayOrPause()
{
	//暂停后还会继续播放一小段缓冲队列中的内容，缓冲队列没问题。
	//暂停在解码和音频播放也要做到暂停
	bool isPause = !dt.isPause;
	SetPause(isPause);
	dt.SetPause(isPause);
}

//显示是否暂停:暂停状态显示播放，播放状态显示暂停
void XPlay2::SetPause(bool isPause)
{
	if (isPause)
		ui.isPlay->setText(QString::fromLocal8Bit("播 放"));
	else
		ui.isPlay->setText(QString::fromLocal8Bit("暂 停"));
}

//打开外部音视频文件
void XPlay2::OpenFile()
{
	//让用户选择文件
	QString name = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择视频文件"));
	//如果没有选择文件 就直接return
	if (name.isEmpty())
		return;
	this->setWindowTitle(name);
	//toLocal8Bit() 转成unicode
	if (!dt.Open(name.toLocal8Bit(), ui.openGLWidget))
	{
		QMessageBox::information(0, "error", "open file failed!!!");
		return;
	}
	//打开成功
	SetPause(dt.isPause);

	//打开文件

}
