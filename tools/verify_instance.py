#!/usr/bin/env python3
"""Verificacao minima de uma instancia JSON de modulo scheduling."""

import argparse
import json
import sys
from collections import defaultdict, deque


def load(path):
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def check(inst):
    ok = True
    nodes = {n["id"]: n for n in inst["nodes"]}
    ops = defaultdict(int)
    for n in inst["nodes"]:
        ops[n["op"]] += 1

    indeg = defaultdict(int)
    outdeg = defaultdict(int)
    lc = 0
    for e in inst["edges"]:
        if e["from"] not in nodes:
            print(f"ERRO: aresta referencia node inexistente {e['from']}")
            ok = False
        if e["to"] not in nodes:
            print(f"ERRO: aresta referencia node inexistente {e['to']}")
            ok = False
        if e["distance"] != 0:
            lc += 1
        outdeg[e["from"]] += 1
        indeg[e["to"]] += 1

    print(f"nodes: {len(inst['nodes'])}")
    print(f"edges: {len(inst['edges'])} (loop-carried: {lc})")
    print("ops:")
    for op, c in sorted(ops.items()):
        print(f"  {op}: {c}")

    # Subgrafo intra-iteracao (distance == 0) deve ser aciclico.
    zero_adj = defaultdict(list)
    for e in inst["edges"]:
        if e["distance"] == 0:
            zero_adj[e["from"]].append(e["to"])

    in_zero = {nid: 0 for nid in nodes}
    for u in zero_adj:
        for v in zero_adj[u]:
            in_zero[v] += 1

    q = deque([nid for nid in nodes if in_zero[nid] == 0])
    topo = []
    while q:
        u = q.popleft()
        topo.append(u)
        for v in zero_adj[u]:
            in_zero[v] -= 1
            if in_zero[v] == 0:
                q.append(v)

    if len(topo) != len(nodes):
        print("ERRO: ciclo detectado em arestas distance == 0")
        ok = False
    else:
        print("OK: grafo distance == 0 eh aciclico")

    # Longest path (ASAP) no DAG intra-iteracao.
    asap = {nid: 0 for nid in nodes}
    for u in topo:
        for v in zero_adj[u]:
            asap[v] = max(asap[v], asap[u] + nodes[u]["latency"])
    print(f"ASAP max (caminho critico intra-iteracao): {max(asap.values())}")

    roots = [nid for nid in nodes if indeg[nid] == 0]
    leaves = [nid for nid in nodes if outdeg[nid] == 0]
    print(f"roots: {len(roots)}, leaves: {len(leaves)}")

    return ok


def main():
    parser = argparse.ArgumentParser(description="Verifica instancia JSON.")
    parser.add_argument("json", help="Arquivo JSON")
    args = parser.parse_args()
    inst = load(args.json)
    ok = check(inst)
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
