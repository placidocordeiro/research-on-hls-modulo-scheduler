#include "mii.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Verifica se o subgrafo com arestas distance == 0 contém ciclo (inviável).
// Usa ordenação topológica de Kahn.
static bool has_zero_distance_cycle(const Instance& inst) {
    int n = inst.nodes.size();
    std::vector<std::vector<int>> adj(n);
    std::vector<int> indeg(n, 0);

    for (const auto& e : inst.edges) {
        if (e.distance == 0) {
            adj[e.from].push_back(e.to);
            indeg[e.to]++;
        }
    }

    std::queue<int> q;
    for (int i = 0; i < n; ++i) {
        if (indeg[i] == 0) q.push(i);
    }

    int visited = 0;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        visited++;
        for (int v : adj[u]) {
            if (--indeg[v] == 0) q.push(v);
        }
    }

    return visited != n;
}

static int compute_resmii(const Instance& inst) {
    std::unordered_map<std::string, int> count_per_op;
    for (const auto& node : inst.nodes) {
        count_per_op[node.op]++;
    }

    int resmii = 0;
    for (const auto& res : inst.resources) {
        if (res.count <= 0) continue; // ilimitado ou inválido
        auto it = count_per_op.find(res.type);
        if (it == count_per_op.end()) continue;
        int needed = (it->second + res.count - 1) / res.count;
        resmii = std::max(resmii, needed);
    }

    // Avisa sobre ops sem recurso declarado (exceto phi/addr, comuns).
    std::unordered_set<std::string> known;
    for (const auto& res : inst.resources) known.insert(res.type);
    for (const auto& kv : count_per_op) {
        if (!known.count(kv.first) && kv.first != "phi" && kv.first != "addr") {
            std::cerr << "warning: no resource declared for op '" << kv.first << "'\n";
        }
    }

    return resmii;
}

// Detecta ciclo negativo em grafo com n nós e arestas edges (cost = weight).
// Usa Bellman-Ford com super-source.
static bool has_negative_cycle(int n, const std::vector<std::tuple<int,int,long long>>& wedges) {
    const long long INF = (1LL << 60);
    std::vector<long long> dist(n + 1, INF);
    dist[n] = 0; // super-source

    // super-source conectada a todos
    for (int i = 0; i < n; ++i) {
        // relaxa uma vez na inicialização
        if (dist[i] > dist[n]) dist[i] = dist[n];
    }

    // V iterações
    for (int i = 0; i < n; ++i) {
        bool changed = false;
        for (const auto& e : wedges) {
            int u, v; long long w;
            std::tie(u, v, w) = e;
            if (dist[u] != INF && dist[v] > dist[u] + w) {
                dist[v] = dist[u] + w;
                changed = true;
            }
        }
        // também relaxa da super-source
        for (int v = 0; v < n; ++v) {
            if (dist[v] > dist[n]) {
                dist[v] = dist[n];
                changed = true;
            }
        }
        if (!changed) break;
    }

    // (n+1)-ésima iteração: se relaxar, há ciclo negativo
    for (const auto& e : wedges) {
        int u, v; long long w;
        std::tie(u, v, w) = e;
        if (dist[u] != INF && dist[v] > dist[u] + w) {
            return true;
        }
    }
    return false;
}

// RecMII via busca paramétrica:
// Para um candidato II, existe ciclo com sum(lat)/sum(dist) > II  sse
// existe ciclo com sum(II*dist - lat) < 0  (ciclo negativo).
static int compute_recmii(const Instance& inst) {
    int n = inst.nodes.size();
    long long total_lat = 0;
    for (const auto& node : inst.nodes) total_lat += node.latency;

    long long lo = 0;
    long long hi = std::max(total_lat, 1LL); // RecMII não pode ultrapassar isso

    int best = 0;
    while (lo <= hi) {
        long long mid = lo + (hi - lo) / 2;

        std::vector<std::tuple<int,int,long long>> wedges;
        for (const auto& e : inst.edges) {
            long long cost = mid * (long long)e.distance - inst.nodes[e.from].latency;
            wedges.emplace_back(e.from, e.to, cost);
        }

        if (has_negative_cycle(n, wedges)) {
            // RecMII > mid
            best = (int)mid + 1;
            lo = mid + 1;
        } else {
            // RecMII <= mid
            hi = mid - 1;
        }
    }

    return best;
}

MIIResult compute_mii(const Instance& inst) {
    MIIResult r;
    r.feasible = true;

    if (has_zero_distance_cycle(inst)) {
        r.feasible = false;
        r.message = "zero-distance cycle detected (infeasible)";
        r.resmii = 0;
        r.recmii = 0;
        r.mii = 0;
        return r;
    }

    r.resmii = compute_resmii(inst);
    r.recmii = compute_recmii(inst);
    r.mii = std::max(r.resmii, r.recmii);
    r.message = "ok";
    return r;
}
