#pragma once

#include "PortType.hpp"

#include <QtCore/QPointF>
#include <QtCore/QRectF>

#include <iostream>
#include <list>

namespace QtNodes
{

class AICCConnectionGeometry
{
public:
    AICCConnectionGeometry();
    ///start 为输出点坐标，end为输入点坐标，坐标系为scene坐标系
    std::list<QPointF> connectionPoints(QPointF start,QPointF end){

    }

private:
    std::list<QPointF> _tpoints;


private:

    double _lineWidth;
    bool _hovered;

};
}

