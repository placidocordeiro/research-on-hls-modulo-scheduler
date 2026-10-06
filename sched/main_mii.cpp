#include "instance.h"
#include "mii.h"

#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <instance.json>\n";
        return 1;
    }

    try {
        Instance inst = Instance::from_json_file(argv[1]);
        MIIResult mii = compute_mii(inst);

        std::cout << "name: " << inst.name << "\n";
        std::cout << "nodes: " << inst.nodes.size() << "  edges: " << inst.edges.size() << "\n";

        if (!mii.feasible) {
            std::cerr << "error: " << mii.message << "\n";
            return 2;
        }

        std::cout << "ResMII: " << mii.resmii << "\n";
        std::cout << "RecMII: " << mii.recmii << "\n";
        std::cout << "MII: " << mii.mii << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}
