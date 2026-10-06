#!/usr/bin/env python3
"""Converte DFG .dot do Bambu (OP_DFG / OP_FDFG) para JSON interno."""

import argparse
import json
import re
import sys

# Mapeia nome de operador do Bambu para (tipo de recurso, latencia).
OP_MAP = {
    "add_node": ("add", 1),
    "ternary_add_node": ("add", 1),
    "eq_node": ("add", 1),
    "sub_node": ("add", 1),
    "shl_node": ("shift", 1),
    "shr_node": ("shift", 1),
    "fshl_node": ("shift", 1),
    "and_node": ("bitwise", 1),
    "or_node": ("bitwise", 1),
    "xor_node": ("bitwise", 1),
    "not_node": ("bitwise", 1),
    "concat_bit_node": ("bitwise", 1),
    "mul_node": ("mul", 3),
    "div_node": ("div", 4),
    "LOAD": ("load", 2),
    "STORE": ("store", 1),
    "addr_node": ("addr", 0),
    "gep_node": ("addr", 0),
    "phi_stmt": ("phi", 0),
}

IGNORE_OPS = {"nop_stmt", "return_stmt", "MULTI_READ_COND", "ENTRY", "EXIT"}

NODE_RE = re.compile(r"^\s*(\d+)\s*\[([^\]]*)\];\s*$")
EDGE_RE = re.compile(r"^\s*(\d+)\s*->\s*(\d+)\s*\[([^\]]*)\];\s*$")
LABEL_RE = re.compile(r'\blabel\s*=\s*"([^"]*)"')
COLOR_RE = re.compile(r'\bcolor\s*=\s*([A-Za-z0-9_]+)')


def extract_label(attr: str) -> str:
    m = LABEL_RE.search(attr)
    return m.group(1) if m else ""


def extract_color(attr: str) -> str:
    m = COLOR_RE.search(attr)
    return m.group(1) if m else ""


def parse_op(label: str):
    """Retorna (resource_type, latency) a partir do primeiro campo do label."""
    first = label.split("\\n")[0]
    m = re.search(r"-\s+(\S+)", first)
    op = m.group(1) if m else first.strip()

    if op in IGNORE_OPS:
        return None
    if op in OP_MAP:
        return OP_MAP[op]
    # Tenta casar sem sufixo _node, p.ex. LOAD/STORE ja estao no mapa.
    return (op.lower(), 1)


def parse_dot(text: str, name: str = "instance", source: str = ""):
    nodes = {}
    edges = []

    for line in text.splitlines():
        nm = NODE_RE.match(line)
        if nm:
            nid = int(nm.group(1))
            label = extract_label(nm.group(2))
            parsed = parse_op(label)
            if parsed is None:
                continue
            op, latency = parsed
            nodes[nid] = {"id": nid, "op": op, "latency": latency}
            continue

        em = EDGE_RE.match(line)
        if em:
            src = int(em.group(1))
            dst = int(em.group(2))
            attr = em.group(3)
            color = extract_color(attr)
            distance = 1 if color == "lightblue" else 0
            label = extract_label(attr)
            edges.append({"from": src, "to": dst, "distance": distance, "label": label})

    # Filtra arestas cujos extremos foram descartados.
    valid = set(nodes)
    edges = [e for e in edges if e["from"] in valid and e["to"] in valid]

    resources = {}
    for n in nodes.values():
        resources.setdefault(n["op"], {"type": n["op"], "count": None})

    return {
        "name": name,
        "source": source,
        "resources": list(resources.values()),
        "nodes": list(nodes.values()),
        "edges": edges,
    }


def main():
    parser = argparse.ArgumentParser(description="Converte .dot do Bambu para JSON interno.")
    parser.add_argument("dot", nargs="?", help="Arquivo .dot (default: stdin)")
    parser.add_argument("--name", default="instance", help="Nome da instancia")
    parser.add_argument("--source", default="", help="Origem do grafo")
    args = parser.parse_args()

    if args.dot and args.dot != "-":
        with open(args.dot, "r", encoding="utf-8") as f:
            text = f.read()
        source = args.source or args.dot
        name = args.name or args.dot
    else:
        text = sys.stdin.read()
        source = args.source or "stdin"
        name = args.name or "instance"

    inst = parse_dot(text, name=name, source=source)
    json.dump(inst, sys.stdout, indent=2)
    sys.stdout.write("\n")


if __name__ == "__main__":
    main()
