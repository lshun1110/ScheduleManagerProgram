#ifndef CALENDAR_LOOP_H
#define CALENDAR_LOOP_H

#include "app_scene.h"

/**
 * @brief Entry point for the calendar scene.  This function implements
 *        a simple month?view calendar with basic mouse interactions.  It
 *        returns the next scene to transition to based on user actions.
 *
 * The calendar loop is responsible for drawing the calendar UI and
 * handling mouse/keyboard events.  When the user chooses to log out
 * the function returns SCENE_LOGIN.  When the program should
 * terminate it returns SCENE_EXIT.  Otherwise the calendar loop
 * continues to run until explicitly exited.
 */
SceneState Calendar_Loop(void);

#endif // CALENDAR_LOOP_H