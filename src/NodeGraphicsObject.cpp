#include "NodeGraphicsObject.hpp"

#include <iostream>
#include <cstdlib>


#include <QtWidgets/QtWidgets>
#include <QtWidgets/QGraphicsEffect>

#include "ConnectionGraphicsObject.hpp"
#include "ConnectionState.hpp"

#include "FlowScene.hpp"
#include <FlowView.hpp>
#include "NodePainter.hpp"

#include "Node.hpp"
#include "NodeDataModel.hpp"
#include "NodeConnectionInteraction.hpp"

#include "StyleCollection.hpp"
#include <ctime>

using QtNodes::NodeGraphicsObject;
using QtNodes::Node;
using QtNodes::FlowScene;
//using QtNodes::FlowView;

NodeGraphicsObject::
NodeGraphicsObject(FlowScene &scene,
                   Node& node)
  : _scene(scene)
  , _node(node)
  , _locked(false)
  , _proxyWidget(nullptr)
{
  _scene.addItem(this);

  setFlag(QGraphicsItem::ItemDoesntPropagateOpacityToChildren, true);
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);




  auto const &nodeStyle = node.nodeDataModel()->nodeStyle();

  {
    auto effect = new QGraphicsDropShadowEffect;
    effect->setOffset(4, 4);
    effect->setBlurRadius(20);
    effect->setColor(nodeStyle.ShadowColor);

    setGraphicsEffect(effect);
  }

  setOpacity(nodeStyle.Opacity);

  setAcceptHoverEvents(true);

  setZValue(0);

  embedQWidget();

  // connect to the move signals to emit the move signals in FlowScene
  auto onMoveSlot = [this] {
    _scene.nodeMoved(_node, pos());
  };
  connect(this, &QGraphicsObject::xChanged, this, onMoveSlot);
  connect(this, &QGraphicsObject::yChanged, this, onMoveSlot);

  //gzl
  connect(&scene, &FlowScene::scale_param_intern1, this, &NodeGraphicsObject::reset_cache_mode);

}

//gzl
void
NodeGraphicsObject::reset_cache_mode(float scale_param_height){
    //获取需要生成的模块矩形的长和宽
    int h=this->boundingRect().height();
//    int w=this->boundingRect().width();
//    float h=scale_param_height;
    std::cout<<"\nheight: "<<h<<std::endl;
    std::cout<<"\nscale_param_height: "<<scale_param_height<<std::endl;
    std::cout<<"\ntime: "<<std::time(0)<<std::endl;


  //  scene.views().at(0)
    //根据模块尺寸选择显示模式
    //添加放大倍数判断
    if((h>1000) || (scale_param_height>1)){
        //  无缓存模式，显示完全且清晰，不做缓存，然后每次都会重新绘制
          QGraphicsItem::setCacheMode(QGraphicsItem::NoCache);
          std::cout<<"\nmode_no_cache"<<std::endl;
    }
    else{
        //高质量模式，但是会有过长的模块显示不全的问题
        QGraphicsItem::setCacheMode( QGraphicsItem::DeviceCoordinateCache );
        std::cout<<"\nmode_device_coordinate_cache"<<std::endl;
    }


    ////  低质量模式，像素很渣，但是能显示全從
    ////   设置模块显示的固定像素大小
    //  auto size = QSize(NodeGraphicsObject::boundingRect().width()*2,
    //                    NodeGraphicsObject::boundingRect().height()*2);
    //  std::cout<<"\nQsize"<<size.rheight()<<" "<<size.rwidth()<<std::endl;
    //  setCacheMode( QGraphicsItem::ItemCoordinateCache, size);
  }


NodeGraphicsObject::
~NodeGraphicsObject()
{
  _scene.removeItem(this);
}


Node&
NodeGraphicsObject::
node()
{
  return _node;
}


Node const&
NodeGraphicsObject::
node() const
{
  return _node;
}


void
NodeGraphicsObject::
embedQWidget()
{
  NodeGeometry & geom = _node.nodeGeometry();

  if (auto w = _node.nodeDataModel()->embeddedWidget())
  {
    _proxyWidget = new QGraphicsProxyWidget(this);

    _proxyWidget->setWidget(w);

    _proxyWidget->setPreferredWidth(5);

    geom.recalculateSize();

    if (w->sizePolicy().verticalPolicy() & QSizePolicy::ExpandFlag)
    {
      // If the widget wants to use as much vertical space as possible, set it to have the geom's equivalentWidgetHeight.
      _proxyWidget->setMinimumHeight(geom.equivalentWidgetHeight());
    }

    _proxyWidget->setPos(geom.widgetPosition());

    update();

    _proxyWidget->setOpacity(1.0);
    _proxyWidget->setFlag(QGraphicsItem::ItemIgnoresParentOpacity);
  }
}


QRectF
NodeGraphicsObject::
boundingRect() const
{
  return _node.nodeGeometry().boundingRect();
}


void
NodeGraphicsObject::
setGeometryChanged()
{
  prepareGeometryChange();
}


void
NodeGraphicsObject::
moveConnections() const
{
  NodeState const & nodeState = _node.nodeState();

  for (PortType portType: {PortType::In, PortType::Out})
  {
    auto const & connectionEntries =
      nodeState.getEntries(portType);

    for (auto const & connections : connectionEntries)
    {
      for (auto & con : connections)
        con.second->getConnectionGraphicsObject().move();
    }
  }
}


void
NodeGraphicsObject::
lock(bool locked)
{
  _locked = locked;

  setFlag(QGraphicsItem::ItemIsMovable, !locked);
  setFlag(QGraphicsItem::ItemIsFocusable, !locked);
  setFlag(QGraphicsItem::ItemIsSelectable, !locked);
}


void
NodeGraphicsObject::
paint(QPainter * painter,
      QStyleOptionGraphicsItem const* option,
      QWidget* )
{
  painter->setClipRect(option->exposedRect);

  NodePainter::paint(painter, _node, _scene);
}


QVariant
NodeGraphicsObject::
itemChange(GraphicsItemChange change, const QVariant &value)
{
  if (change == ItemPositionChange && scene())
  {
    moveConnections();
  }

  return QGraphicsItem::itemChange(change, value);
}


void
NodeGraphicsObject::
mousePressEvent(QGraphicsSceneMouseEvent * event)
{
  if (_locked)
    return;

  // deselect all other items after this one is selected
  if (!isSelected() &&
      !(event->modifiers() & Qt::ControlModifier))
  {
    _scene.clearSelection();
    this->setSelected(true);
  }


  for (PortType portToCheck: {PortType::In, PortType::Out})
  {
    NodeGeometry const & nodeGeometry = _node.nodeGeometry();

    // TODO do not pass sceneTransform
    int const portIndex = nodeGeometry.checkHitScenePoint(portToCheck,
                                                    event->scenePos(),
                                                    sceneTransform());

    if (portIndex != INVALID)
    {
      NodeState const & nodeState = _node.nodeState();

      std::unordered_map<QUuid, Connection*> connections =
        nodeState.connections(portToCheck, portIndex);

      // start dragging existing connection
      if (!connections.empty() && portToCheck == PortType::In)
      {
        auto con = connections.begin()->second;

        NodeConnectionInteraction interaction(_node, *con, _scene);

        interaction.disconnect(portToCheck);
      }
      else // initialize new Connection
      {
        if (portToCheck == PortType::Out)
        {
          auto const outPolicy = _node.nodeDataModel()->portOutConnectionPolicy(portIndex);
          if (!connections.empty() &&
              outPolicy == NodeDataModel::ConnectionPolicy::One)
          {
            _scene.deleteConnection( *connections.begin()->second );
          }
        }

        // todo add to FlowScene
        auto connection = _scene.createConnection(portToCheck,
                                                  _node,
                                                  portIndex);

        _node.nodeState().setConnection(portToCheck,
                                        portIndex,
                                        *connection);

        connection->getConnectionGraphicsObject().grabMouse();
      }
    }
  }

  auto pos     = event->pos();
  auto & geom  = _node.nodeGeometry();
  auto & state = _node.nodeState();

  if (_node.nodeDataModel()->resizable() &&
      geom.resizeRect().contains(QPoint(pos.x(),
                                        pos.y())))
  {
    state.setResizing(true);
  }
}


void
NodeGraphicsObject::
mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
  auto & geom  = _node.nodeGeometry();
  auto & state = _node.nodeState();

  if (state.resizing())
  {
    auto diff = event->pos() - event->lastPos();

    if (auto w = _node.nodeDataModel()->embeddedWidget())
    {
      prepareGeometryChange();

      auto oldSize = w->size();

      oldSize += QSize(diff.x(), diff.y());

      w->setFixedSize(oldSize);

      _proxyWidget->setMinimumSize(oldSize);
      _proxyWidget->setMaximumSize(oldSize);
      _proxyWidget->setPos(geom.widgetPosition());

      geom.recalculateSize();
      update();

      moveConnections();

      event->accept();
    }
  }
  else
  {
    QGraphicsObject::mouseMoveEvent(event);

    if (event->lastPos() != event->pos())
      moveConnections();

    event->ignore();
  }

  QRectF r = scene()->sceneRect();

  r = r.united(mapToScene(boundingRect()).boundingRect());

  scene()->setSceneRect(r);
}


void
NodeGraphicsObject::
mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  auto & state = _node.nodeState();

  state.setResizing(false);

  QGraphicsObject::mouseReleaseEvent(event);

  // position connections precisely after fast node move
  moveConnections();
}


void
NodeGraphicsObject::
hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
  // bring all the colliding nodes to background
  QList<QGraphicsItem *> overlapItems = collidingItems();

  for (QGraphicsItem *item : overlapItems)
  {
    if (item->zValue() > 0.0)
    {
      item->setZValue(0.0);
    }
  }

  // bring this node forward
  setZValue(1.0);

  _node.nodeGeometry().setHovered(true);
  update();
  _scene.nodeHovered(node(), event->screenPos());
  event->accept();
}


void
NodeGraphicsObject::
hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
  _node.nodeGeometry().setHovered(false);
  update();
  _scene.nodeHoverLeft(node());
  event->accept();
}


void
NodeGraphicsObject::
hoverMoveEvent(QGraphicsSceneHoverEvent * event)
{
  auto pos    = event->pos();
  auto & geom = _node.nodeGeometry();

  if (_node.nodeDataModel()->resizable() &&
      geom.resizeRect().contains(QPoint(pos.x(), pos.y())))
  {
    setCursor(QCursor(Qt::SizeFDiagCursor));
  }
  else
  {
    setCursor(QCursor());
  }

  event->accept();
}


void
NodeGraphicsObject::
mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsItem::mouseDoubleClickEvent(event);

  _scene.nodeDoubleClicked(node());
}


void
NodeGraphicsObject::
contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
  _scene.nodeContextMenu(node(), mapToScene(event->pos()));
}
