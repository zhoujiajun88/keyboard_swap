#include "App.h"

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, PWSTR commandLine, int) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    App app(instance, commandLine ? commandLine : L"");
    return app.Run();
}
