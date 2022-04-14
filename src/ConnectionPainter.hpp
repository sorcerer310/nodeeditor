#pragma once

#include <QtGui/QPainter>
#include "nodes/internal/Node.hpp"
#include <QThread>


namespace QtNodes
{

class ConnectionGeometry;
class ConnectionState;
class Connection;

class ConnectionPainter
{
public:

  static
  void
  paint(QPainter* painter,
        Connection const& connection);

  static
  QPainterPath
  getPainterStroke(ConnectionGeometry const& geom);

};
}
