#pragma once

#include <QtWidgets/QGraphicsView>

#include "Export.hpp"
//#include <QObject>

namespace QtNodes
{

class FlowScene;

class NODE_EDITOR_PUBLIC FlowView
  : /*public QObject,*/ public QGraphicsView
{
  Q_OBJECT
//    Q_OBJECT
//  public:

public:

  FlowView(QWidget *parent = Q_NULLPTR);
  FlowView(FlowScene *scene, QWidget *parent = Q_NULLPTR);

  FlowView(const FlowView&) = delete;
  FlowView operator=(const FlowView&) = delete;

  QAction* clearSelectionAction() const;

  QAction* deleteSelectionAction() const;

  void setScene(FlowScene *scene);

  //gzl
  float tm11() const;

  //gzl
Q_SIGNALS:
  void signal_scale_param(float scale_param_height);

public Q_SLOTS:

  void scaleUp();

  void scaleDown();

  void deleteSelectedNodes();

protected:

  void contextMenuEvent(QContextMenuEvent *event) override;

  void wheelEvent(QWheelEvent *event) override;

  void keyPressEvent(QKeyEvent *event) override;

  void keyReleaseEvent(QKeyEvent *event) override;

  void mousePressEvent(QMouseEvent *event) override;

  void mouseMoveEvent(QMouseEvent *event) override;

  void drawBackground(QPainter* painter, const QRectF& r) override;

  void showEvent(QShowEvent *event) override;



protected:

  FlowScene * scene();

private:

  QAction* _clearSelectionAction;
  QAction* _deleteSelectionAction;
  //gzl
//  float _scale_param;

  QPointF _clickPos;

  FlowScene* _scene;


};
}
