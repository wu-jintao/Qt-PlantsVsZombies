//
// Created by sun on 9/2/16.
//

#include "Timer.h"

//QTimer    定时器类
Timer::Timer(QObject *parent, int timeout, std::function<void(void)> functor) : QTimer(parent)
{
    setInterval(timeout); //设置时间间隔
    if (timeout < 50)
        setTimerType(Qt::PreciseTimer); //高精度
    setSingleShot(true); //接口实现单次定时器
    connect(this, &Timer::timeout, [this, functor] { functor(); deleteLater(); }); //connect函数
}


//QTimeLine类提供用于控制动画的时间表
TimeLine::TimeLine(QObject *parent, int duration, int interval, std::function<void(qreal)> onChanged, std::function<void(void)> onFinished, CurveShape shape)
        : QTimeLine(duration, parent)
{
    if (duration == 0) {
        int i = 1;
        ++i;
    }
    setUpdateInterval(interval); //用于控制更新动画的时间间隔
    setCurveShape(shape); //用于控制间隔输出数值的一个变化规律
    connect(this, &TimeLine::valueChanged, onChanged);
    connect(this, &TimeLine::finished, [this, onFinished] { onFinished(); deleteLater(); });
}
