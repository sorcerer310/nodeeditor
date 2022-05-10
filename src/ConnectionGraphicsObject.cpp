#include "ConnectionGraphicsObject.hpp"

#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QGraphicsBlurEffect>
#include <QtWidgets/QStyleOptionGraphicsItem>
#include <QtWidgets/QGraphicsView>

#include "FlowScene.hpp"

#include "Connection.hpp"
#include "ConnectionGeometry.hpp"
#include "ConnectionPainter.hpp"
#include "ConnectionState.hpp"
#include "ConnectionBlurEffect.hpp"

#include "NodeGraphicsObject.hpp"

#include "NodeConnectionInteraction.hpp"

#include "Node.hpp"

using QtNodes::ConnectionGraphicsObject;
using QtNodes::Connection;
using QtNodes::FlowScene;

ConnectionGraphicsObject::
ConnectionGraphicsObject(FlowScene &scene,
                         Connection &connection)
    : _scene(scene)
    , _connection(connection)
{
    _scene.addItem(this);

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    setAcceptHoverEvents(true);

    //   addGraphicsEffect();

    setZValue(-1.0);

}


ConnectionGraphicsObject::
~ConnectionGraphicsObject()
{
    _scene.removeItem(this);
}


QtNodes::Connection&
ConnectionGraphicsObject::
connection()
{
    return _connection;
}


QRectF
ConnectionGraphicsObject::
boundingRect() const
{
    return _connection.connectionGeometry().boundingRect();
}


QPainterPath
ConnectionGraphicsObject::
shape() const
{
#ifdef DEBUG_DRAWING

    //QPainterPath path;

    //path.addRect(boundingRect());
    //return path;

#else
    auto const &geom =
            _connection.connectionGeometry();

    return ConnectionPainter::getPainterStroke(geom);

#endif
}


void
ConnectionGraphicsObject::
setGeometryChanged()
{
    prepareGeometryChange();
}


///该move函数是node在移动时会被调用，用于移动连接线
void ConnectionGraphicsObject::move()
{
    for(PortType portType: { PortType::In, PortType::Out } )
    {
        if (auto node = _connection.getNode(portType))
        {
            auto const &nodeGraphics = node->nodeGraphicsObject();

            auto const &nodeGeom = node->nodeGeometry();

            QPointF scenePos =
                    nodeGeom.portScenePosition(_connection.getPortIndex(portType),
                                               portType,
                                               nodeGraphics.sceneTransform());

            QTransform sceneTransform = this->sceneTransform();

            QPointF connectionPos = sceneTransform.inverted().map(scenePos);

            _connection.connectionGeometry().setEndPoint(portType,connectionPos);

            //测试代码,当node移动时重新计算绘制连接线
               _connection.connectionGeometry().connectionPoints();
            //测试代码
            _connection.getConnectionGraphicsObject().setGeometryChanged();
            _connection.getConnectionGraphicsObject().update();
        }
    }

}

void ConnectionGraphicsObject::lock(bool locked)
{
    setFlag(QGraphicsItem::ItemIsMovable, !locked);
    setFlag(QGraphicsItem::ItemIsFocusable, !locked);
    setFlag(QGraphicsItem::ItemIsSelectable, !locked);
}


void
ConnectionGraphicsObject::
paint(QPainter* painter,
      QStyleOptionGraphicsItem const* option,
      QWidget*)
{

    ///覆盖过期线段
    painter->setClipRect(option->exposedRect);
    ConnectionPainter::paint(painter,_connection);
}


void
ConnectionGraphicsObject::
mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mousePressEvent(event);
    //鼠标选中线段，设置图形的selected为true
    _connection.connectionGeometry().setSelected(true);
    //event->ignore();
}


void
ConnectionGraphicsObject::
mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    prepareGeometryChange();

    auto view = static_cast<QGraphicsView*>(event->widget());
    auto node = locateNodeAt(event->scenePos(),
                             _scene,
                             view->transform());

    auto &state = _connection.connectionState();

    state.interactWithNode(node);


    if (node)
    {
        node->reactToPossibleConnection(state.requiredPort(),
                                        _connection.dataType(oppositePort(state.requiredPort())),
                                        event->scenePos());
        //        node->nodeGeometry().portScenePosition()
    }

    //-------------------

    QPointF offset = event->pos() - event->lastPos();

    auto requiredPort = _connection.requiredPort();

    //    _connection.getPortIndex()

    if (requiredPort != PortType::None)
    {
        _connection.connectionGeometry().moveEndPoint(requiredPort, offset);
    }
    else{
        if(_connection.getConnectionGraphicsObject().isSelected()){

            //计算out port与in port位置与node上下沿的距离
            Node *nout = _connection.getNode(PortType::Out);
            QPointF posout = nout->nodeGeometry().portScenePosition( _connection.getPortIndex(PortType::Out),PortType::Out);
            double hout = nout->nodeGeometry().height();
            std::pair<double,double> out_distance_pair = std::make_pair(posout.y(),hout-posout.y());

            Node *nin = _connection.getNode(PortType::In);
            QPointF posin = nin->nodeGeometry().portScenePosition(_connection.getPortIndex(PortType::In),PortType::In);
            double hin = nin->nodeGeometry().height();
            std::pair<double,double> in_distance_pair = std::make_pair(posin.y(),hin-posin.y());
            //调用connectionPorints更新线段位置
            _connection.connectionGeometry().connectionPoints(event->pos(),in_distance_pair,out_distance_pair);
        }

    }


    update();
    event->accept();
}


void
ConnectionGraphicsObject::
mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    //鼠标在线段上释放设置selected为false
    _connection.connectionGeometry().setSelected(false);
    ungrabMouse();
    event->accept();

    auto node = locateNodeAt(event->scenePos(), _scene,
                             _scene.views()[0]->transform());

    NodeConnectionInteraction interaction(*node, _connection, _scene);

    if (node && interaction.tryConnect())
    {
        node->resetReactionToConnection();
    }

    if (_connection.connectionState().requiresPort())
    {
        _scene.deleteConnection(_connection);
    }
}


void
ConnectionGraphicsObject::
hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    _connection.connectionGeometry().setHovered(true);
    update();
    _scene.connectionHovered(connection(), event->screenPos());
    event->accept();
}


void
ConnectionGraphicsObject::
hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    _connection.connectionGeometry().setHovered(false);

    update();
    _scene.connectionHoverLeft(connection());
    event->accept();
}


void
ConnectionGraphicsObject::
addGraphicsEffect()
{
    auto effect = new QGraphicsBlurEffect;

    effect->setBlurRadius(5);
    setGraphicsEffect(effect);

    //auto effect = new QGraphicsDropShadowEffect;
    //auto effect = new ConnectionBlurEffect(this);
    //effect->setOffset(4, 4);
    //effect->setColor(QColor(Qt::gray).darker(800));
}
