#include <boost/program_options.hpp>

#include "Server/Server.h"

int main(int argc, char *argv[]) {
    boost::program_options::options_description desc{"Options"};
    desc.add_options()
            ("help,h", "Help screen")
            ("port", boost::program_options::value<int>()->default_value(8080) );

    boost::program_options::variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    int port= 8080;

    if (vm.count("help")) {
        std::cout << desc << '\n';
        return 0;
    }

    if (vm.count("port")) {
        port = vm["port"].as<int>();
    }

    Server svr;
    svr.init();

    std::cout << "start listening on port: " << port << std::endl;

    svr.listen("0.0.0.0", port);

    return 0;
}
