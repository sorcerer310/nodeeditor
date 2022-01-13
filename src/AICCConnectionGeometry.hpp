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
    //start is output point,end is input point
    std::list<QPointF> connectionPoints(QPointF start,QPointF end){

    }

private:
    std::list<QPointF> _tpoints;


private:

    double _lineWidth;
    bool _hovered;

};
}

