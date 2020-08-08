#include "Server/Server.h"

int main() {
    Server svr;
    svr.init();

    svr.listen("localhost", 1234);

    return 0;
}
