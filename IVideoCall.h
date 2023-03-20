#pragma once
struct AVFrame;
//接口类 连接XVideoThread和XVideoWidget
class IVideoCall
{
public:
	virtual void Init(int width, int height) = 0;
	virtual void Repaint(AVFrame *frame) = 0;
};

