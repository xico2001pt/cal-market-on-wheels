#include "GUI.h"

using Nd = GraphViewer::Node;
using Ed = GraphViewer::Edge;

GUI::GUI(const Graph<Node> * graph, int centerID, int width, int height): graph(graph), centerID(centerID), width(width), height(height), gv(new GraphViewer()) {

    for (Vertex<Node> *vertex: graph->getVertexSet()) {

        Nd &node = gv->addNode(vertex->getInfo().getNodeId(), sf::Vector2f(vertex->getInfo().getX(), vertex->getInfo().getY()));

        if (vertex->getInfo().getClient() != nullptr) {         // If a vertex is a Client
            node.setLabel(vertex->getInfo().getClient()->getName());
            node.setSize(50);
            node.setColor(sf::Color::Red);
        } else if (vertex->getInfo().getSupplier() != nullptr) {  // If a vertex is a Supplier
            node.setLabel("Supplier");
            node.setSize(50);
            node.setColor(sf::Color::Green);
        } else if (vertex->getInfo().getNodeId() == centerID) {
            node.setLabel("Center");
            node.setSize(50);
            node.setColor(sf::Color::Cyan);
        } else node.setSize(0);
    }

    for (Vertex<Node> *vertex : graph->getVertexSet()) {
        for (Edge<Node> *edge : vertex->getOutgoing())
            gv->addEdge(edge->getId(), gv->getNode(vertex->getInfo().getNodeId()),gv->getNode(edge->getDest()->getInfo().getNodeId()), Ed::EdgeType::DIRECTED).setThickness(2);
    }
}

bool GUI::setCenterID(int centerID) {
    if (gv->getNodes().size() < centerID) return false;
    Nd &node2 = gv->getNode(centerID);
    if (node2.getLabel() != "") return false;

    // Resetting centerNode
    Nd &node1 = gv->getNode(this->centerID);
    node1.setLabel();
    node1.setSize(0);
    node1.setColor();

    // Updating centerNode
    this->centerID = centerID;
    node2.setLabel("Center");
    node2.setSize(50);
    node2.setColor(sf::Color::Cyan);

    return true;
}

void GUI::show() {
    // Center on center node
    gv->setCenter(gv->getNode(centerID).getPosition());
    gv->createWindow(width, height);
    gv->join();
    gv->closeWindow();
}

void GUI::disableNotStrong() {
    for (Vertex<Node> *vertex: graph->getVertexSet()) {
        if (!vertex->getStrong()) {
            gv->getNode(vertex->getInfo().getNodeId()).disable();
            for (Edge<Node> *edge: vertex->getOutgoing()) gv->getEdge(edge->getId()).disable();
            for (Edge<Node> *edge: vertex->getIncoming()) gv->getEdge(edge->getId()).disable();
        }
    }
}

void GUI::enableNotStrong() {
    for (Vertex<Node> *vertex: graph->getVertexSet()) {
        if (!vertex->getStrong()) {
            gv->getNode(vertex->getInfo().getNodeId()).enable();
            for (Edge<Node> *edge: vertex->getOutgoing()) gv->getEdge(edge->getId()).enable();
            for (Edge<Node> *edge: vertex->getIncoming()) gv->getEdge(edge->getId()).enable();
        }
    }
}

void GUI::showStrong() {
    disableNotStrong();
    show();
    enableNotStrong();
}

void GUI::showPaths(const std::vector<Path> &paths) {
    sf::Color colors[] = {sf::Color::Red, sf::Color::Green, sf::Color::Blue, sf::Color::Magenta};

    disableNotStrong();

    gv->setCenter(gv->getNode(centerID).getPosition());
    gv->createWindow(width, height);

    int color = 0;
    for (const Path &path: paths) {
        for (int id: path.getPath()) {
            gv->lock();
            Ed &edge = gv->getEdge(id);
            edge.setColor(colors[color]);
            edge.setThickness(5);
            gv->unlock();
            Sleep(5);
            if (!gv->isWindowOpen()) {
                gv->closeWindow();
                for (const Path &path: paths) {
                    for (int id: path.getPath()) {
                        Ed &edge = gv->getEdge(id);
                        edge.setColor(sf::Color::Black);
                        edge.setThickness(2);
                    }
                }
                return;
            }
        }
        color = (color + 1) % 4;
    }

    gv->join();
    gv->closeWindow();

    enableNotStrong();

    for (const Path &path: paths) {
        for (int id: path.getPath()) {
            Ed &edge = gv->getEdge(id);
            edge.setColor(sf::Color::Black);
            edge.setThickness(2);
        }
    }
}

GUI::~GUI() {
    if (gv != nullptr) {
        if (gv->isWindowOpen()) gv->closeWindow();
        delete(gv);
    }
}
