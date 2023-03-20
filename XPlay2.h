#pragma once
#include <QtWidgets/QWidget>
#include "ui_XPlay2.h"

class XPlay2 : public QWidget
{
	Q_OBJECT

public:
	XPlay2(QWidget *parent = Q_NULLPTR);
	~XPlay2();
	//��ʱ�� ��������ʾ ����ˢ�½���
	void timerEvent(QTimerEvent *e);

	//���ڳߴ�仯 ����resizeEvent����
	void resizeEvent(QResizeEvent *e);

	//˫��ȫ�� ����mouseDoubleClickEvent������ע�����صĺ��������ܴ����Ȼ������Ӧ��
	void mouseDoubleClickEvent(QMouseEvent *e);

	//��ʾ�Ƿ���ͣ:��ͣ״̬��ʾ���ţ�����״̬��ʾ��ͣ
	void SetPause(bool isPause);

//��ʾ�ǲۺ���
public slots:
	void OpenFile();
	void PlayOrPause();
	void SliderPress();
	void SliderRelease();

private:
	//�������Ƿ񱻰�ס��Ĭ��Ϊfalse��
	bool isSliderPress = false;
	Ui::XPlay2Class ui;
};
