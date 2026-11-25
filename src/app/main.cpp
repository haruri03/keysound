#include <windows.h>
#include <thread>
#include "app/icon.h"
#include "audio/pack_player.h"
#include "audio/trace.h"
#include "core/paths.h"
#include "input/hooks.h"
#pragma comment(linker,"/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

int main()
{
    packPlayer.start();
    keysound::trace::start(runPathW());
    keyboardReady = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!keyboardReady)
    {
        packPlayer.shutdown();
        return 1;
    }
    std::thread t2(key_hook);
    WaitForSingleObject(keyboardReady, INFINITE);
    CloseHandle(keyboardReady);
    keyboardReady = NULL;
    SetIco();
    PostThreadMessage(keyboardThreadId, WM_QUIT, 0, 0);
    t2.join();
    keysound::trace::stop();
    packPlayer.shutdown();
    return 0;
}
