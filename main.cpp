#include "iostream"
#include "fstream"
#include "cxxopts.hpp"
#include "nlohmann/json.hpp"
#include "elzip/elzip.hpp"
// #include <HPatch/patch.h>

int main(int argc, char * argv[]) {
    cxxopts::Options parser("GenshinPatcher", "Auto patch GenshinImpact game's diff file");
    parser.add_options()
        ("game", "Path of GenshinImpact")
        ("diff", "hdiff package")
        ;
    parser.parse_positional("game");
    auto args = parser.parse(argc, argv);

    nlohmann::json data = R"({"remoteName": "/abc/def/efg/fgh/audio.pak"})"_json;

    std::cout << "Hello, World!\n" << data["remoteName"].get<std::string>() << std::endl;
    return 0;
}
