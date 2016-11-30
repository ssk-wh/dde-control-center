#include "monitor.h"

Monitor::Monitor(QObject *parent)
    : QObject(parent)
{

}

void Monitor::setX(const int x)
{
    if (m_x == x)
        return;

    m_x = x;

    emit xChanged(m_x);
}

void Monitor::setY(const int y)
{
    if (m_y == y)
        return;

    m_y = y;

    emit yChanged(m_y);
}

void Monitor::setW(const int w)
{
    if (m_w == w)
        return;

    m_w = w;

    emit wChanged(m_w);
}

void Monitor::setH(const int h)
{
    if (m_h == h)
        return;

    m_h = h;

    emit hChanged(m_h);
}

void Monitor::setRotate(const quint16 rotate)
{
    if (m_rotate == rotate)
        return;

    m_rotate = rotate;

    emit rotateChanged(m_rotate);
}

void Monitor::setName(const QString &name)
{
    m_name = name;
}

void Monitor::setRotateList(const QList<quint16> &rotateList)
{
    m_rotateList = rotateList;
}

void Monitor::setCurrentMode(const Resolution &resolution)
{
    if (m_currentMode == resolution)
        return;

    m_currentMode = resolution;

    emit currentModeChanged(m_currentMode);
}

void Monitor::setModeList(const ResolutionList &modeList)
{
    m_modeList.clear();

    // NOTE: ignore resolution if below 800x600
    for (auto m : modeList)
        if (m.width() >= 800 && m.height() >= 600)
            m_modeList.append(m);
}
