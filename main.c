#include "common.h"
#include "set_mode.h"
#include "ui_core.h"
#include "login_ui.h"
#include "login_loop.h" 
#include "calendar_loop.h"
#include "app_scene.h"

User g_current_user; // 현재 로그인한 사용자
int running = 1;

int main()
{
    set_console();

    SceneState scene = SCENE_LOGIN;
    
    while (running)
    {
        switch (scene)
        {
        case SCENE_LOGIN:
            scene = Login_Loop();
            break;

        case SCENE_CALENDAR:
            scene = Calendar_Loop();     // 캘린더 메인
            break;

        case SCENE_EXIT:
            running = 0;
            break;
        default:
            running = 0;
            break;
        }
        
    }

    return 0;
}