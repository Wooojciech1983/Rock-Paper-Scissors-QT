#include <iostream>
#include "AppSession.h"

int main()
{
    AppSession session(std::cin, std::cout);
    session.Run();
    return 0;
}
