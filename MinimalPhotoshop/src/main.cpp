#include "application.h"

#if defined(_WIN32) && !defined(_DEBUG)
    #define main WinMain
#else 
    #define main main
#endif

int main()
{
    Application app;
    app.Run();
}
