#pragma once

#include <stdbool.h>

#include <SDL.h>

bool absinput_dispatch_event(SDL_Event ev);

bool absinput_controllerdevice_event(SDL_Event ev);