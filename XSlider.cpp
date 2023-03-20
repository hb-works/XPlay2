#include "XSlider.h"

//重载鼠标点击事件，实现点击滑动条跳转到相应的位置
void XSlider::mousePressEvent(QMouseEvent *e)
{
	//pos为百分比
	double pos = (double)e->pos().x() / (double)width();
	//百分比 * 最大值 = 实际位置
	setValue(pos * this->maximum());

	//原有事件要继续处理 这样处理有问题:会点不到滑动条最后的位置
	//QSlider::mousePressEvent(e);

	//解决：松开事件得到处理
	QSlider::sliderReleased();
}

XSlider::XSlider(QWidget *parent)
	: QSlider(parent)
{
}

XSlider::~XSlider()
{
}
