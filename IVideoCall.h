#pragma once
struct AVFrame;
//�ӿ��� ����XVideoThread��XVideoWidget
class IVideoCall
{
public:
	virtual void Init(int width, int height) = 0;
	virtual void Repaint(AVFrame *frame) = 0;
};

