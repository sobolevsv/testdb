#include <iostream>
#include "Server/Server.h"

///using namespace httplib;

int main() {
    Server svr;
    svr.init();

    svr.listen("localhost", 1234);

    return 0;
}
