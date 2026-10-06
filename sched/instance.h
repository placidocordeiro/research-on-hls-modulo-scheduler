#ifndef INSTANCE_H
#define INSTANCE_H

#include <string>
#include <unordered_map>
#include <vector>

struct Node {
    int id;
    std::string op;
    int latency;
};

struct Edge {
    int from;     // índice do nó de origem
    int to;       // índice do nó de destino
    int distance; // iterações de distância
};

struct Resource {
    std::string type;
    int count;    // -1 representa ilimitado (null no JSON)
};

struct Instance {
    std::string name;
    std::string source;
    std::vector<Node> nodes;
    std::vector<Edge> edges;
    std::vector<Resource> resources;
    std::unordered_map<int, int> id2idx; // mapeia id externo -> índice interno

    static Instance from_json_file(const std::string& path);
};

#endif
