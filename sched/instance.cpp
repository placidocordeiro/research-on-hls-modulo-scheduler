#include "instance.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>

using json = nlohmann::json;

Instance Instance::from_json_file(const std::string& path) {
    std::ifstream f(path);
    if (!f) {
        throw std::runtime_error("cannot open file: " + path);
    }

    json j;
    f >> j;

    Instance inst;
    inst.name = j.value("name", "");
    inst.source = j.value("source", "");

    // Recursos
    if (j.contains("resources") && j["resources"].is_array()) {
        for (const auto& r : j["resources"]) {
            Resource res;
            res.type = r.value("type", "");
            if (r["count"].is_null()) {
                res.count = -1;
            } else {
                res.count = r.value("count", -1);
            }
            inst.resources.push_back(res);
        }
    }

    // Nós
    if (!j.contains("nodes") || !j["nodes"].is_array()) {
        throw std::runtime_error("missing 'nodes' array");
    }
    int idx = 0;
    for (const auto& n : j["nodes"]) {
        Node node;
        node.id = n.value("id", -1);
        node.op = n.value("op", "");
        node.latency = n.value("latency", 0);

        if (node.id < 0) {
            throw std::runtime_error("node with invalid id");
        }
        if (node.latency < 0) {
            throw std::runtime_error("negative latency for node id " + std::to_string(node.id));
        }
        if (inst.id2idx.count(node.id)) {
            throw std::runtime_error("duplicate node id: " + std::to_string(node.id));
        }

        inst.id2idx[node.id] = idx++;
        inst.nodes.push_back(node);
    }

    // Arestas
    if (!j.contains("edges") || !j["edges"].is_array()) {
        throw std::runtime_error("missing 'edges' array");
    }
    for (const auto& e : j["edges"]) {
        int from_id = e.value("from", -1);
        int to_id = e.value("to", -1);
        int distance = e.value("distance", 0);

        if (!inst.id2idx.count(from_id)) {
            throw std::runtime_error("edge references unknown node id: " + std::to_string(from_id));
        }
        if (!inst.id2idx.count(to_id)) {
            throw std::runtime_error("edge references unknown node id: " + std::to_string(to_id));
        }
        if (distance < 0) {
            throw std::runtime_error("negative distance for edge");
        }

        Edge edge;
        edge.from = inst.id2idx[from_id];
        edge.to = inst.id2idx[to_id];
        edge.distance = distance;
        inst.edges.push_back(edge);
    }

    return inst;
}
