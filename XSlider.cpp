#include "XSlider.h"

//����������¼���ʵ�ֵ����������ת����Ӧ��λ��
void XSlider::mousePressEvent(QMouseEvent *e)
{
	//posΪ�ٷֱ�
	double pos = (double)e->pos().x() / (double)width();
	//�ٷֱ� * ���ֵ = ʵ��λ��
	setValue(pos * this->maximum());

	//ԭ���¼�Ҫ�������� ��������������:��㲻������������λ��
	//QSlider::mousePressEvent(e);

	//������ɿ��¼��õ�����
	QSlider::sliderReleased();
}

XSlider::XSlider(QWidget *parent)
	: QSlider(parent)
{
}

XSlider::~XSlider()
{
}
