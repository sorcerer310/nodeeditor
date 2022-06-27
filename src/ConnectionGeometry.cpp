#include "ConnectionGeometry.hpp"

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
        double yDistance = _in.y() - _out.y() + TURNING_LINE_PADDING;

        double vector = yDistance < 0 ? -1.0 : 1.0;

        verticalOffset = qMin(defaultOffset, std::abs(yDistance)) * vector;

        ratioX = 1.0;

    }

    horizontalOffset *= ratioX;

    //    qDebug() << "QPointF:" << _out.x() << ":" << _out.y();

    QPointF c1(_out.x() + horizontalOffset,
               _out.y() + verticalOffset);

    QPointF c2(_in.x() - horizontalOffset,
               _in.y() - verticalOffset);

    //控制显示矩形的大小
    if(c1.x()<=_points.at(0).x()+TURNING_LINE_PADDING)
        c1.setX(_points.at(0).x()+TURNING_LINE_PADDING);
    if(c2.x()>=_points.at(3).x()-TURNING_LINE_PADDING)
        c2.setX(_points.at(3).x()-TURNING_LINE_PADDING);

    return std::make_pair(c1,c2);

}

///当线段未被选中时执行该函数
QList<QPointF> ConnectionGeometry::connectionPoints(const int conIndex){
//    QList<QPointF> ConnectionGeometry::connectionPoints(){
    if(_selected) return _points;
    double xDistance = _out.x() - _in.x();

    int splitSpace = TURNING_LINE_PADDING*conIndex;
    //当输出点在输入点右侧（2拐点情况)
    if(xDistance<=0){
        if(abs(_out.x()-_in.x())<=splitSpace)
            splitSpace = abs(_out.x() - _in.x());
        //如果输出端点x小于当前折线点x坐标-20 或 输入端点x大于当前折线点x坐标+20 保持折线原来的x坐标
//        if(_in.x()<=_points[0].x()+splitSpace && _in.x() > _out.x() +splitSpace){
            if(_in.x()<=_points[0].x()+splitSpace ){
            _points[0] = QPointF(_in.x()-splitSpace,_out.y());
            _points[1] = QPointF(_in.x()-splitSpace,_in.y());
//        }else if(_out.x() >= _points[0].x()-splitSpace && _out.x()){
        }else if(_out.x() >= _points[0].x()-splitSpace ){
            _points[0] = QPointF(_out.x()+splitSpace,_out.y());
            _points[1] = QPointF(_out.x()+splitSpace,_in.y());
        }else{
            _points[0] = QPointF(_points[0].x(),_out.y());
            _points[1] = QPointF(_points[1].x(),_in.y());
        }
        _points[2] = QPointF(0,0);
        _points[3] = QPointF(0,0);
    }
    //当输出点在输入点左侧（4拐点情况）
    //TODO 下一步处理4拐点拖动时的问题
    else{
        if(abs(_in.x() - _out.x()) <= splitSpace)
            splitSpace = abs(_in.x() - _out.x());
        _points[0] = QPointF(_out.x()+splitSpace,_out.y());
        double yDistance = _in.y() - _out.y();
        _points[1] = QPointF(_out.x()+splitSpace,_out.y() +yDistance/2);
        _points[2] = QPointF(_in.x()-splitSpace,_in.y()-yDistance/2);
        _points[3] = QPointF(_in.x()-splitSpace,_in.y());
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
            if(pos.x()-_out.x()<=20) x = _out.x()+20;
            else if(pos.x() >= _in.x()-20) x = _in.x()-20;

            //            qDebug() << "pos.x:" << pos.x() << "_out.x:" << _out.x() << "_in.x:" << _in.x();
            _points[0].setX(x);
            _points[0].setY(_out.y());
            _points[1].setX( x);
            _points[1].setY(_in.y());

        }else if(xDistance>0){
            double horizontalOffset = 20;

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

