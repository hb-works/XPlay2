#pragma once
#include <QtWidgets/QWidget>
#include "ui_XPlay2.h"

class XPlay2 : public QWidget
{
	Q_OBJECT

public:
	XPlay2(QWidget *parent = Q_NULLPTR);
	~XPlay2();
	//定时器 滑动条显示 持续刷新界面
	void timerEvent(QTimerEvent *e);

	//窗口尺寸变化 重载resizeEvent函数
	void resizeEvent(QResizeEvent *e);

	//双击全屏 重载mouseDoubleClickEvent函数（注意重载的函数名不能打错，不然不能响应）
	void mouseDoubleClickEvent(QMouseEvent *e);

	//显示是否暂停:暂停状态显示播放，播放状态显示暂停
	void SetPause(bool isPause);

//表示是槽函数
public slots:
	void OpenFile();
	void PlayOrPause();
	void SliderPress();
	void SliderRelease();

private:
	//滑动条是否被按住，默认为false；
	bool isSliderPress = false;
	Ui::XPlay2Class ui;
};
