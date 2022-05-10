#pragma once

#include "PortType.hpp"

#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtCore/qdebug.h>


#include <iostream>


namespace QtNodes
{

class ConnectionGeometry
{
public:

  ConnectionGeometry();

public:

  QPointF const&
  getEndPoint(PortType portType) const;

  void
  setEndPoint(PortType portType, QPointF const& point);

  void
  moveEndPoint(PortType portType, QPointF const &offset);

  QRectF
  boundingRect() const;

  std::pair<QPointF, QPointF>
  pointsC1C2() const;


  std::pair<double,double> getOutPort2NodeSizeDistance();
  std::pair<double,double> getInPort2NodeSizeDistance();

  QList<QPointF> connectionPoints();
//  QList<QPointF> connectionPoints(std::pair<double,double> portInDistancePair,std::pair<double,double> portOutDistancePair);
  QList<QPointF> connectionPoints(QPointF pos,std::pair<double,double> portInDistancePair,std::pair<double,double> portOutDistancePair);
  QList<QPointF> getPoints() const {return _points;}
  void setPoints(QList<QPointF> p)  {_points = p;}
//  QList<QPointF>

  QPointF
  source() const { return _out; }
  QPointF
  sink() const { return _in; }

  double
  lineWidth() const { return _lineWidth; }

  bool
  hovered() const { return _hovered; }
  void
  setHovered(bool hovered) { _hovered = hovered; }

  bool selected() const {return _selected;}
  void setSelected(bool selected) {_selected = selected;}
//  void setConnection(Connection c) {_connection = c;}
  void setPortInDistancePair(std::pair<double,double> pair){_portInDistancePair = pair;}
  void setPortOutDistancePair(std::pair<double,double> pair){_portOutDistancePair = pair;}

public:
  int TURNING_LINE_PADDING = 20;

private:
  // local object coordinates
  QPointF _in;
  QPointF _out;
  QList<QPointF> _points;

  std::pair<double,double> _portInDistancePair;
  std::pair<double,double> _portOutDistancePair;

  //int _animationPhase;

  double _lineWidth;

  bool _hovered;
  bool _selected;
};
}
