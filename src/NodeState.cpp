#include "NodeState.hpp"

#include "Node.hpp"

#include "NodeDataModel.hpp"

#include "Connection.hpp"

using QtNodes::NodeState;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::Connection;
using QtNodes::Node;

NodeState::
NodeState(std::unique_ptr<NodeDataModel> const &model)
    : _inConnections(model->nPorts(PortType::In))
    , _outConnections(model->nPorts(PortType::Out))
    , _reaction(NOT_REACTING)
    , _reactingPortType(PortType::None)
    , _resizing(false)
{}


std::vector<NodeState::ConnectionPtrSet> const &
NodeState::
getEntries(PortType portType) const
{
    if (portType == PortType::In)
        return _inConnections;
    else
        return _outConnections;
}


std::vector<NodeState::ConnectionPtrSet> &
NodeState::
getEntries(PortType portType)
{
    if (portType == PortType::In)
        return _inConnections;
    else
        return _outConnections;
}


NodeState::ConnectionPtrSet
NodeState::
connections(PortType portType, PortIndex portIndex) const
{
    auto const &connections = getEntries(portType);

    return connections[portIndex];
}


void
NodeState::
setConnection(PortType portType,
              PortIndex portIndex,
              Connection& connection)
{
    auto &connections = getEntries(portType);

    connections.at(portIndex).insert(std::make_pair(connection.id(),
                                                    &connection));
}


void
NodeState::
eraseConnection(PortType portType,
                PortIndex portIndex,
                QUuid id)
{
    getEntries(portType)[portIndex].erase(id);
}


NodeState::ReactToConnectionState
NodeState::
reaction() const
{
    return _reaction;
}


PortType
NodeState::
reactingPortType() const
{
    return _reactingPortType;
}


NodeDataType
NodeState::
reactingDataType() const
{
    return _reactingDataType;
}


void
NodeState::
setReaction(ReactToConnectionState reaction,
            PortType reactingPortType,
            NodeDataType reactingDataType)
{
    _reaction = reaction;

    _reactingPortType = reactingPortType;

    _reactingDataType = std::move(reactingDataType);
}


bool
NodeState::
isReacting() const
{
    return _reaction == REACTING;
}


void
NodeState::
setResizing(bool resizing)
{
    _resizing = resizing;
}


bool
NodeState::
resizing() const
{
    return _resizing;
}

/**
 * @brief NodeState::layoutConnections  当一个模块得输入port新增一条线时，对这条线进行位置布局规划
 * @param con
 */
void NodeState::layoutConnections( Connection &con, const QPointF &outp,const QPointF &inp){
    std::vector<Connection *> currentNodeConnections;

    //将当前Node的输出端点汇总到一起
    std::vector<ConnectionPtrSet> vcpsin = getEntries(PortType::In);
    std::vector<ConnectionPtrSet> vcpsout = getEntries(PortType::Out);
    std::vector<ConnectionPtrSet> vcps;
    PortType portType;
    if(vcpsout.size()>=vcpsin.size()){
        vcps = vcpsout;
        portType = PortType::Out;
    }else{
        vcps = vcpsin;
        portType = PortType::In;
    }

    for(ConnectionPtrSet cps:vcps){
        for(std::pair<QUuid,Connection *> kv:cps)
        {
            currentNodeConnections.push_back(kv.second);
        }
    }

    ConnectionGeometry &cg = con.connectionGeometry();

    //计算横向可放置线段得空间
    double horizontalSpace = std::abs((inp.x() - cg.TURNING_LINE_PADDING)
            - (outp.x() + cg.TURNING_LINE_PADDING));
    //计算每个位置的间隔
    double splitSpace = horizontalSpace/(currentNodeConnections.size()+2);

    QList<QPointF> points;
    QList<QPointF> conPoints = con.connectionGeometry().getPoints();
    if(outp.x()<inp.x()){
        points.append(QPointF(outp.x()+cg.TURNING_LINE_PADDING+splitSpace*(con.getPortIndex(portType)+1),outp.y()));
        points.append(QPointF(outp.x()+cg.TURNING_LINE_PADDING+splitSpace*(con.getPortIndex(portType)+1),inp.y()));
        points.append(QPointF(0,0));
        points.append(QPointF(0,0));
//        qDebug() << "outp1:" << outp << "  inp1:" << inp ;

    } else {

        points.append(QPointF(outp.x()+cg.TURNING_LINE_PADDING+splitSpace*(con.getPortIndex(portType)+1),outp.y()));

        float midposy = std::abs(cg.getEndPoint(PortType::Out).y()-cg.getEndPoint(PortType::In).y())/2;
        points.append(QPointF(outp.x()+cg.TURNING_LINE_PADDING+splitSpace*(con.getPortIndex(portType)+1),outp.y()+midposy));
        points.append(QPointF(inp.x()-cg.TURNING_LINE_PADDING-splitSpace*(con.getPortIndex(portType)+1),outp.y()+midposy));

        points.append(QPointF(inp.x()-cg.TURNING_LINE_PADDING-splitSpace*(con.getPortIndex(portType)+1),inp.y()));

//        qDebug() << "outp1:" << outp << "  inp1:" << inp << "--------------------3线段:" << points;

    }
    con.connectionGeometry().setPoints(points);
}
/**
 * @brief NodeState::connectionIndex    获得当前connection的索引
 * @param con
 * @return
 */
int NodeState::connectionIndex(Connection &con){
    std::vector<Connection *> currentNodeConnections;
    //将当前Node的输出端点汇总到一起
    std::vector<ConnectionPtrSet> vcpsin = getEntries(PortType::In);
    std::vector<ConnectionPtrSet> vcpsout = getEntries(PortType::Out);
    std::vector<ConnectionPtrSet> vcps;
    if(vcpsout.size()>=vcpsin.size()){
        vcps = vcpsout;
    }else{
        vcps = vcpsin;
    }

    for(ConnectionPtrSet cps:vcps){
        for(std::pair<QUuid,Connection *> kv:cps)
        {
            currentNodeConnections.push_back(kv.second);
        }
    }

    if(currentNodeConnections.size()<=0) return 0;
    for(int i=1;i<=currentNodeConnections.size();i++){
        if(currentNodeConnections.at(i-1)->id()==con.id())
            return i;
    }

    return 0;
}
