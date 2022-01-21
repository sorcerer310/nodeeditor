﻿#include "ConnectionGeometry.hpp"

#include <cmath>

#include "StyleCollection.hpp"

using QtNodes::ConnectionGeometry;
using QtNodes::PortType;


ConnectionGeometry::
ConnectionGeometry()
    : _in(0, 0)
    , _out(0, 0)
    ,_points({QPointF(),QPointF(),QPointF(),QPointF()})
    //, _animationPhase(0)
    , _lineWidth(3.0)
    , _hovered(false)

{ }

QPointF const&
ConnectionGeometry::
getEndPoint(PortType portType) const
{
    Q_ASSERT(portType != PortType::None);

    return (portType == PortType::Out ?
                _out :
                _in);
}


void
ConnectionGeometry::
setEndPoint(PortType portType, QPointF const& point)
{
    switch (portType)
    {
    case PortType::Out:
        _out = point;
        break;

    case PortType::In:
        _in = point;
        break;

    default:
        break;
    }
}


void
ConnectionGeometry::
moveEndPoint(PortType portType, QPointF const &offset)
{
    switch (portType)
    {
    case PortType::Out:
        _out += offset;
        break;

    case PortType::In:
        _in += offset;
        break;

    default:
        break;
    }
}


QRectF
ConnectionGeometry::
boundingRect() const
{
    auto points = pointsC1C2();

    QRectF basicRect = QRectF(_out, _in).normalized();
    QRectF c1c2Rect = QRectF(points.first, points.second).normalized();

    auto const &connectionStyle =
            StyleCollection::connectionStyle();

    float const diam = connectionStyle.pointDiameter();

    QRectF commonRect = basicRect.united(c1c2Rect);

    QPointF const cornerOffset(diam, diam);

    commonRect.setTopLeft(commonRect.topLeft() - cornerOffset);
    commonRect.setBottomRight(commonRect.bottomRight() + 2 * cornerOffset);

    return commonRect;
}


std::pair<QPointF, QPointF>
ConnectionGeometry::
pointsC1C2() const
{
    const double defaultOffset = 99999;
    //const double defaultOffset = 0;

    double xDistance = _in.x() - _out.x();

    double horizontalOffset = qMin(defaultOffset, std::abs(xDistance));

    double verticalOffset = 0;

    double ratioX = 0.5;

    if (xDistance <= 0)
    {
        double yDistance = _in.y() - _out.y() + 20;

        double vector = yDistance < 0 ? -1.0 : 1.0;

        verticalOffset = qMin(defaultOffset, std::abs(yDistance)) * vector;

        ratioX = 1.0;
    }

    horizontalOffset *= ratioX;

    QPointF c1(_out.x() + horizontalOffset,
               _out.y() + verticalOffset);

    QPointF c2(_in.x() - horizontalOffset,
               _in.y() - verticalOffset);

    return std::make_pair(c1, c2);
}

///当线段未被选中时执行该函数
//QList<QPointF> ConnectionGeometry::connectionPoints(std::pair<double,double> portInDistancePair,std::pair<double,double> portOutDistancePair){
//QList<QPointF> ConnectionGeometry::connectionPoints(){
QList<QPointF> ConnectionGeometry::connectionPoints(){

    if(_selected) return _points;
    double xDistance = _out.x() - _in.x();
    //当输出点在输入点右侧（2拐点情况)
    if(xDistance<=0){
        double horizontalOffset = std::abs(xDistance);

        _points[0] = QPointF(_out.x()+horizontalOffset/2,_out.y());
        _points[1] = QPointF(_in.x()-horizontalOffset/2,_in.y());
    }
    //当输出点在输入点左侧（4拐点情况）
    else{
        double horizontalOffset = 20;


        _points[0] = QPointF(_out.x()+horizontalOffset,_out.y());
        //        //out port 在下方的情况
        //        if(_out.y() >=_in.y()){
        //            double yDistance = _in.y()- _out.y() - portOutDistancePair.first - portInDistancePair.second;
        //            _points[1] = QPointF(_out.x()+horizontalOffset,_out.y() - portOutDistancePair.first - yDistance/2);
        //            _points[2] = QPointF(_in.x()-horizontalOffset,_out.y() - portOutDistancePair.first - yDistance/2);
        //        }
        //        //out port 在上方的情况
        //        else{
        //            double yDistance =  _out.y()+_in.y()-portOutDistancePair.second - portInDistancePair.first ;
        //            _points[1] = QPointF(_out.x()+horizontalOffset,_out.y()+portOutDistancePair.second +yDistance/2);
        //            _points[2] = QPointF(_in.x()-horizontalOffset,_out.y()+portOutDistancePair.second+yDistance/2);
        //        }

        double yDistance = _in.y() - _out.y();
        _points[1] = QPointF(_out.x()+horizontalOffset,_out.y() +yDistance/2);
        _points[2] = QPointF(_in.x()-horizontalOffset,_in.y()-yDistance/2);
        _points[3] = QPointF(_in.x()-horizontalOffset,_in.y());
    }
    return _points;
}

///当线段选中时执行该绘制函数
QList<QPointF> ConnectionGeometry::connectionPoints(QPointF pos,std::pair<double,double> portInDistancePair,std::pair<double,double> portOutDistancePair)
{
    //未选择时普通的连接点计算
    if(_selected){

        double xDistance = _out.x() - _in.x();
        if(xDistance<=0){

            double x = pos.x();
            if(pos.x()<=20) x = _out.x()+20;
            else if(pos.x() >= _in.x()-20) x = _in.x()-20;

            _points[0].setX(x);
            _points[0].setY(_out.y());
            _points[1].setX( x);
            _points[1].setY(_in.y());

        }else if(xDistance>0){
            double horizontalOffset = 20;

            //            qDebug() << portInDistancePair << portOutDistancePair;

            double y = pos.y();
            //分两种情况：
            //当out port在下方
            if(_out.y() >= _in.y()){
                if(pos.y()<=_in.y() + portInDistancePair.second+20) y = _in.y() +portInDistancePair.second+20;
                else if(pos.y() >= _out.y() - portOutDistancePair.first-20) y = _out.y() - portOutDistancePair.first-20;
            }else{
                if(pos.y()<=_out.y() + portOutDistancePair.second+20) y = _out.y() + portOutDistancePair.second+20;
                else if(pos.y()>=_in.y() - portInDistancePair.first-20) y = _in.y() - portInDistancePair.first-20;
            }

            _points[0].setX(_out.x()+horizontalOffset);
            _points[0].setY(_out.y());
            _points[1].setX(_out.x()+horizontalOffset);
            _points[1].setY(y);
            _points[2].setX(_in.x()-horizontalOffset);
            _points[2].setY(y);
            _points[3].setX(_in.x()-horizontalOffset);
            _points[3].setY(_in.y());

        }
    }

    return _points;
}

QList<QPointF> ConnectionGeometry::getPoints(){
    return _points;
}

std::pair<double,double> ConnectionGeometry::getOutPort2NodeSizeDistance(){

//    _connection.connectionGeometry()
//    Node *nout = _connection->getNode(PortType::Out);
//    QPointF posout = nout->nodeGeometry().portScenePosition( _connection.getPortIndex(PortType::Out),PortType::Out);
//    double hout = nout->nodeGeometry().height();
//    std::pair<double,double> out_distance_pair = std::make_pair(posout.y(),hout-posout.y());
    return std::make_pair(0,0);
}

std::pair<double,double> ConnectionGeometry::getInPort2NodeSizeDistance(){
    return std::make_pair(0,0);
}


















