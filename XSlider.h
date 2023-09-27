#pragma once

#include <QObject>
#include <QMouseEvent>
#include <QSlider>
class XSlider : public QSlider
{
	Q_OBJECT

public:
	XSlider(QWidget *parent = NULL);
	~XSlider();
	//重载鼠标点击事件函数，实现点击滑动条跳转到相应的位置
	void mousePressEvent(QMouseEvent *e);
};
