#include "Application.h"
#include "util/Utils.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>

Application::Application() {
    loadData();
}

const Graph<Node> & Application::getGraph() const {
    return this->graph;
}

int Application::getCenterId() const {
    return this->centerID;
}

bool Application::setCenterId(int centerID) {
    if (graph.getVertexSet().size() < centerID || graph.getVertex(centerID - 1)->getInfo().getClient() != nullptr || graph.getVertex(centerID - 1)->getInfo().getSupplier() != nullptr) return false;
    this->centerID = centerID;
    std::cout << "Analyzing graph connectivity\n";
    graph.analyzeGraphConnectivity(this->centerID);
    return true;
}

bool Application::loadNodes() {
    std::string line;
    int nodeId;
    float x, y;

    std::ifstream file("../maps/" + files.map + "/" + files.nodesFile);
    if (!file.is_open()) return false;

    getline(file, line);
    int nNodes = stoi(line);
    for (int i = 0; i < nNodes; ++i) {
        getline(file, line, '(');
        getline(file, line, ',');
        nodeId = stoi(line);

        getline(file, line, ',');
        x = stof(line);

        getline(file, line, ')');
        y = stof(line);

        Node n = Node(nodeId, x, y);
        graph.addVertex(n);
    }

    file.close();
    return true;
}

bool Application::loadEdges() {
    std::string line;
    int orig, dest;
    double weight;

    std::ifstream file("../maps/" + files.map + "/" + files.edgesFile);
    if (!file.is_open()) return false;

    getline(file, line);
    int nEdges = stoi(line);
    for (int i = 0; i < nEdges; ++i) {
        getline(file, line, '(');
        getline(file, line, ',');
        orig = stoi(line);

        getline(file, line, ')');
        dest = stoi(line);

        Node n1 = graph.getVertex(orig-1)->getInfo();
        Node n2 = graph.getVertex(dest-1)->getInfo();
        weight = distance(n1.getX(), n1.getY(), n2.getX(), n2.getY());
        graph.addEdge(i, n1, n2, weight);
    }

    file.close();
    return true;
}

bool Application::loadClients() {
    std::string line;
    int clientId, nodeId;

    std::ifstream file("../maps/" + files.map + "/" + files.clientsFile);
    if (!file.is_open()) return false;

    getline(file, line);
    int nClients = stoi(line);
    clients = std::vector<Client>(nClients);
    for (int i = 0; i < nClients; ++i) {
        getline(file, line, '(');
        getline(file, line, ',');
        clientId = stoi(line);

        getline(file, line, ',');
        nodeId = stoi(line);

        getline(file, line, ')');
        clients[i] = Client(clientId, nodeId, line);
        Node nd = graph.getVertex(nodeId - 1)->getInfo();
        nd.setClient(&clients[i]);
        graph.getVertex(nodeId - 1)->setInfo(nd);
    }

    file.close();
    return true;
}

bool Application::loadOrders() {
    std::string line;
    int orderId, clientId, productId, quantity;

    std::ifstream file("../maps/" + files.map + "/" + files.ordersFile);
    if (!file.is_open()) return false;

    getline(file, line);
    int nOrders = stoi(line);
    orders = std::vector<Order>(nOrders);
    for (int i = 0; i < nOrders; ++i) {
        getline(file, line, '(');
        getline(file, line, ',');
        orderId = stoi(line);

        getline(file, line, ',');
        clientId = stoi(line);

        getline(file, line, ')');

        orders[i] = Order(orderId, &(clients[clientId - 1]));

        stringstream ss(line);
        while (getline(ss, line, ',')) {
            stringstream ss2(line);
            ss2 >> productId >> quantity;
            orders[i].getProducts().setQuantity(productId, quantity);
        }
    }

    file.close();
    sort(orders.begin(), orders.end(), [&](const Order &left, const Order &right) {return getTotalWeight(left) >
                                                                                          getTotalWeight(right);});
    return true;
}

bool Application::loadProducts() {
    std::string line, name;
    int productId;
    double weight, cost;

    std::ifstream file("../maps/" + files.map + "/" + files.productsFile);
    if (!file.is_open()) return false;

    getline(file, line);
    int nProducts = stoi(line);
    products = std::vector<Product>(nProducts);
    for (int i = 0; i < nProducts; ++i) {
        getline(file, line, '(');

        getline(file, line, ',');
        productId = stoi(line);

        getline(file, line, ',');
        name = line;

        getline(file, line, ',');
        weight = stod(line);

        getline(file, line, ')');
        cost = stod(line);

        products[i] = Product(productId, name, weight, cost);
    }

    file.close();
    return true;
}

bool Application::loadSuppliers() {
    std::string line;
    int supplierId, nodeId, productId, quantity;

    std::ifstream file("../maps/" + files.map + "/" + files.suppliersFile);
    if (!file.is_open()) return false;

    getline(file, line);
    int nSuppliers = stoi(line);
    suppliers = std::vector<Supplier>(nSuppliers);
    for (int i = 0; i < nSuppliers; ++i) {
        getline(file, line, '(');
        getline(file, line, ',');
        supplierId = stoi(line);

        getline(file, line, ',');
        nodeId = stoi(line);

        getline(file, line, ')');

        suppliers[i] = Supplier(supplierId, nodeId);

        stringstream ss(line);
        while (getline(ss, line, ',')) {
            stringstream ss2(line);
            ss2 >> productId >> quantity;
            suppliers[i].getStock().setQuantity(productId, quantity);
        }

        Node nd = graph.getVertex(nodeId - 1)->getInfo();
        nd.setSupplier(&suppliers[i]);
        graph.getVertex(nodeId - 1)->setInfo(nd);
    }

    file.close();
    return true;
}

bool Application::loadVehicles() {
    std::string line;
    int vehicleId;
    double capacity;

    std::ifstream file("../maps/" + files.map + "/" + files.vehiclesFile);
    if (!file.is_open()) return false;

    getline(file, line);
    int nVehicles = stoi(line);
    vehicles = std::vector<Vehicle>(nVehicles);
    for (int i = 0; i < nVehicles; ++i) {
        getline(file, line, '(');
        getline(file, line, ',');
        vehicleId = stoi(line);

        getline(file, line, ',');
        capacity = stod(line);

        vehicles[i] = Vehicle(vehicleId, capacity);
    }

    file.close();
    sort(vehicles.begin(), vehicles.end());
    return true;
}

bool Application::loadData() {
    std::cout << "Loading data...\n";

    graph.clear();

    if (!loadNodes()) {
        std::cout << "Failed to load nodes\n";
        return false;
    } else std::cout << "Nodes loaded successfully\n";

    if (!loadEdges()) {
        std::cout << "Failed to load edges\n";
        return false;
    } else std::cout << "Edges loaded successfully\n";

    setCenterId(1);

    if (!loadClients()) {
        std::cout << "Failed to load clients\n";
        return false;
    } else std::cout << "Clients loaded successfully\n";

    if (!loadProducts()) {
        std::cout << "Failed to load products\n";
        return false;
    } else std::cout << "Products loaded successfully\n";

    if (!loadOrders()) {
        std::cout << "Failed to load orders\n";
        return false;
    } else std::cout << "Orders loaded successfully\n";

    if (!loadSuppliers()) {
        std::cout << "Failed to load suppliers\n";
        return false;
    } else std::cout << "Suppliers loaded successfully\n";

    if (!loadVehicles()) {
        std::cout << "Failed to load vehicles\n";
        return false;
    } else std::cout << "Vehicles loaded successfully\n";

    return true;
}

void Application::setMap(const string &map) {
    files.map = map;
    loadData();
};

std::vector<Order> Application::filterOrders() const {
    Stock allSuppliers;
    for (const Supplier &supplier : suppliers) if (graph.getVertex(supplier.getNodeId() - 1)->getStrong()) allSuppliers += supplier.getStock();
    std::vector<Order> orders;
    for (const Order &order: this->orders) {
        if (graph.getVertex(order.getOwner()->getNodeId() - 1)->getStrong() && allSuppliers.contains(order.getProducts())) {
            orders.push_back(order);
            allSuppliers -= order.getProducts();
        }
    }
    return orders;
}

double Application::getTotalWeight(const Order &order) const {
    double weight = 0;
    Stock &products = order.getProducts();
    for (int id : products.getIds()) weight += products.getQuantity(id) * this->products[id-1].getWeight();
    return weight;
}

std::vector<Path> Application::shortestPathUnlimited(bool displayTime) {
    std::vector<Path> result;

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<Order> orders = filterOrders();
    Path delivery = graph.nearestNeighbor(centerID, orders);
    Path comeback = graph.dijkstra(graph.getEdge(delivery.getPath().back())->getDest()->getInfo().getNodeId(), centerID);
    result.push_back(delivery + comeback);

    auto finish = std::chrono::high_resolution_clock::now();
    if (displayTime) {
        auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
        std::cout << "The algorithm took " << milli << " milliseconds to run.\n";
    }

    loadSuppliers();

    return result;
}

std::vector<Path> Application::shortestPathLimited(bool displayTime) {
    std::vector<Path> result;

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<Order> orders = filterOrders();

    double currentCapacity;

    for (Vehicle v: vehicles) {
        currentCapacity = 0;
        std::vector<Order> vehicleOrders;
        for (std::vector<Order>::iterator itr = orders.begin(); itr != orders.end();) {
            if (v.getMaxCapacity() >= getTotalWeight(*itr) + currentCapacity) {
                vehicleOrders.push_back(*itr);
                currentCapacity += getTotalWeight(*itr);
                itr = orders.erase(itr);
            }
            else itr++;
        }
        Path delivery = graph.nearestNeighbor(centerID, vehicleOrders);
        Path comeback = graph.dijkstra(graph.getEdge(delivery.getPath().back())->getDest()->getInfo().getNodeId(), centerID);
        result.push_back(delivery + comeback);
    }

    auto finish = std::chrono::high_resolution_clock::now();
    if (displayTime) {
        auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
        std::cout << "The algorithm took " << milli << " milliseconds to run.\n";
    }

    loadSuppliers();

    return result;
}