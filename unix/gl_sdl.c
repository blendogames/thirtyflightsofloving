#include <stdio.h>
#include "../renderer/r_local.h"
#include "../client/client.h"
#include "../ui/ui_local.h"

#include <SDL.h>
#include "glw_unix.h"

glwstate_t glw_state;

int		mx, my;
//qboolean mouse_active = false;
float	controller_leftx, controller_lefty, controller_rightx, controller_righty;
extern	cvar_t *in_joystick;

int GLimp_Init (void *hinstance, void *wndproc)
{
	/* No-op */
	return 1;
}

void GLimp_Shutdown (void)
{
//	SDL_SetRelativeMouseMode(SDL_FALSE);
//	mouse_active = false;
	/* No-op */
}

void GLimp_BeginFrame(float camera_seperation)
{
	/* No-op */
}

void GLimp_EndFrame (void)
{
	SDL_GL_SwapWindow(glw_state.glWindow);
}

int GLimp_SetMode (int *pwidth, int *pheight, int mode, dispType_t fullscreen)
{
	int			width, height;
	const char	*steam_tenfoot = NULL;

	if (!VID_GetModeInfo(&width, &height, mode))
	{
		Com_Printf(" invalid mode\n");
		return rserr_invalid_mode;
	}

	/* For Big Picture/SteamOS, unconditionally use fullscreen */
	steam_tenfoot = SDL_getenv("SteamTenfoot");
	if (steam_tenfoot != NULL && SDL_strcmp(steam_tenfoot, "1") == 0)
	{
		fullscreen = dt_fullscreen;
	}

	if (fullscreen == dt_fullscreen)
	{
		/* Override the vidmode with the desktop resolution.
		 * If we wanted to be fancy we could use a faux-backbuffer to
		 * simulate lower resolutions, but the engine is smart and lets
		 * us override, so in 2021 let's just assume your PC can run
		 * Quake 2.
		 * -flibit
		 */
		SDL_DisplayMode mode;
		SDL_SetWindowFullscreen(
			glw_state.glWindow,
			SDL_WINDOW_FULLSCREEN_DESKTOP
		);
		SDL_GetCurrentDisplayMode(
			SDL_GetWindowDisplayIndex(glw_state.glWindow),
			&mode
		);
		*pwidth = mode.w;
		*pheight = mode.h;
	}
	else
	{
		SDL_SetWindowSize(glw_state.glWindow, width, height);
		SDL_SetWindowBordered(
			glw_state.glWindow,
			fullscreen != dt_borderless
		);
		*pwidth = width;
		*pheight = height;
	}

	SDL_ShowWindow(glw_state.glWindow);
	VID_NewWindow(*pwidth, *pheight);
	return rserr_ok;
}

void UpdateGammaRamp (void)
{
	/* Unsupported in 2021 */
}

char *Sys_GetClipboardData (void)
{
	return SDL_GetClipboardText();
}

void IN_Activate (qboolean active)
{
/*	if (active)
	{
		if (!mouse_active)
		{
			mx = my = 0;
			SDL_SetRelativeMouseMode(SDL_TRUE);
			mouse_active = true;
		}
	}
	else
	{
		if (mouse_active)
		{
			SDL_SetRelativeMouseMode(SDL_FALSE);
			mouse_active = false;
		}
	} */
	/* No-op */
}

/* IN_Translate functions taken from yquake2 */

/*
 * This creepy function translates SDL keycodes into
 * the id Tech 2 engines interal representation.
 */
static int
IN_TranslateSDLtoQ2Key (unsigned int keysym)
{
	int key = 0;

	/* These must be translated */
	switch (keysym)
	{
		case SDLK_TAB:
			key = K_TAB;
			break;
		case SDLK_RETURN:
			key = K_ENTER;
			break;
		case SDLK_ESCAPE:
			key = K_ESCAPE;
			break;
		case SDLK_BACKSPACE:
			key = K_BACKSPACE;
			break;
		case SDLK_CAPSLOCK:
			key = K_CAPSLOCK;
			break;
		case SDLK_PAUSE:
			key = K_PAUSE;
			break;

		case SDLK_UP:
			key = K_UPARROW;
			break;
		case SDLK_DOWN:
			key = K_DOWNARROW;
			break;
		case SDLK_LEFT:
			key = K_LEFTARROW;
			break;
		case SDLK_RIGHT:
			key = K_RIGHTARROW;
			break;

		case SDLK_RALT:
		case SDLK_LALT:
			key = K_ALT;
			break;
		case SDLK_LCTRL:
		case SDLK_RCTRL:
			key = K_CTRL;
			break;
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			key = K_SHIFT;
			break;
		case SDLK_INSERT:
			key = K_INS;
			break;
		case SDLK_DELETE:
			key = K_DEL;
			break;
		case SDLK_PAGEDOWN:
			key = K_PGDN;
			break;
		case SDLK_PAGEUP:
			key = K_PGUP;
			break;
		case SDLK_HOME:
			key = K_HOME;
			break;
		case SDLK_END:
			key = K_END;
			break;

		case SDLK_F1:
			key = K_F1;
			break;
		case SDLK_F2:
			key = K_F2;
			break;
		case SDLK_F3:
			key = K_F3;
			break;
		case SDLK_F4:
			key = K_F4;
			break;
		case SDLK_F5:
			key = K_F5;
			break;
		case SDLK_F6:
			key = K_F6;
			break;
		case SDLK_F7:
			key = K_F7;
			break;
		case SDLK_F8:
			key = K_F8;
			break;
		case SDLK_F9:
			key = K_F9;
			break;
		case SDLK_F10:
			key = K_F10;
			break;
		case SDLK_F11:
			key = K_F11;
			break;
		case SDLK_F12:
			key = K_F12;
			break;

		case SDLK_KP_7:
			key = K_KP_HOME;
			break;
		case SDLK_KP_8:
			key = K_KP_UPARROW;
			break;
		case SDLK_KP_9:
			key = K_KP_PGUP;
			break;
		case SDLK_KP_4:
			key = K_KP_LEFTARROW;
			break;
		case SDLK_KP_5:
			key = K_KP_5;
			break;
		case SDLK_KP_6:
			key = K_KP_RIGHTARROW;
			break;
		case SDLK_KP_1:
			key = K_KP_END;
			break;
		case SDLK_KP_2:
			key = K_KP_DOWNARROW;
			break;
		case SDLK_KP_3:
			key = K_KP_PGDN;
			break;
		case SDLK_KP_ENTER:
			key = K_KP_ENTER;
			break;
		case SDLK_KP_0:
			key = K_KP_INS;
			break;
		case SDLK_KP_PERIOD:
			key = K_KP_DEL;
			break;
		case SDLK_KP_DIVIDE:
			key = K_KP_SLASH;
			break;
		case SDLK_KP_MINUS:
			key = K_KP_MINUS;
			break;
		case SDLK_KP_PLUS:
			key = K_KP_PLUS;
			break;
		case SDLK_KP_MULTIPLY:
			key = K_KP_MULT;
			break;

		default:
			break;
	}

	return key;
}

/* This however was devised by flibit to match the old TFOL binding layout */
static int
IN_TranslateSDLtoQ2Button(SDL_GameControllerButton button)
{
	int key = 0;

	switch (button)
	{
		case SDL_CONTROLLER_BUTTON_A:
			key = K_JOY1;
			break;
		case SDL_CONTROLLER_BUTTON_B:
			key = K_JOY2;
			break;
		case SDL_CONTROLLER_BUTTON_X:
			key = K_JOY3;
			break;
		case SDL_CONTROLLER_BUTTON_Y:
			key = K_JOY4;
			break;
		case SDL_CONTROLLER_BUTTON_BACK:
			key = K_AUX7;
			break;
		case SDL_CONTROLLER_BUTTON_START:
			key = K_AUX8;
			break;
		case SDL_CONTROLLER_BUTTON_LEFTSTICK:
			key = K_AUX9;
			break;
		case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
			key = K_AUX10;
			break;
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
			key = K_AUX5;
			break;
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
			key = K_AUX6;
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
			key = K_AUX29;
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			key = K_AUX31;
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
			key = K_AUX32;
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			key = K_AUX30;
			break;
		default:
			break;

	}
	return key;
}

void HandleEvents (void)
{
	int multiclicktime = 750;
	SDL_Event evt;
	while (SDL_PollEvent(&evt) == 1)
	{
		if (evt.type == SDL_QUIT)
		{
			Cbuf_ExecuteText(EXEC_NOW, "quit");
		}
		else if (evt.type == SDL_KEYDOWN || evt.type == SDL_KEYUP)
		{
			/* KEYDOWN/KEYUP events taken from yquake2 */
			qboolean down = (evt.type == SDL_KEYDOWN);

			/* workaround for AZERTY-keyboards, which don't have 1, 2, ..., 9, 0 in first row:
			 * always map those physical keys (scancodes) to those keycodes anyway
			 * see also https://bugzilla.libsdl.org/show_bug.cgi?id=3188 */
			SDL_Scancode sc = evt.key.keysym.scancode;

			if (sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_0)
			{
				/* Note that the SDL_SCANCODEs are SDL_SCANCODE_1, _2, ..., _9, SDL_SCANCODE_0
				 * while in ASCII it's '0', '1', ..., '9' => handle 0 and 1-9 separately
				 * (quake2 uses the ASCII values for those keys) */
				int key = '0'; /* implicitly handles SDL_SCANCODE_0 */

				if (sc <= SDL_SCANCODE_9)
				{
					key = '1' + (sc - SDL_SCANCODE_1);
				}

				Key_Event(key, down, Sys_Milliseconds());
			}
			else
			{
				SDL_Keycode kc = evt.key.keysym.sym;
				if(sc == SDL_SCANCODE_GRAVE && kc != '\'' && kc != '"')
				{
					// special case/hack: open the console with the "console key"
					// (beneath Esc, left of 1, above Tab)
					// but not if the keycode for this is a quote (like on Brazilian
					// keyboards) - otherwise you couldn't type them in the console
					if((evt.key.keysym.mod & (KMOD_CAPS|KMOD_SHIFT|KMOD_ALT|KMOD_CTRL|KMOD_GUI)) == 0)
					{
						// also, only do this if no modifiers like shift or AltGr or whatever are pressed
						// so kc will most likely be the ascii char generated by this and can be ignored
						// in case SDL_TEXTINPUT above (so we don't get ^ or whatever as text in console)
						// (can't just check for mod == 0 because numlock is a KMOD too)
						Key_Event('`', down, Sys_Milliseconds());
					}
				}
				else if ((kc >= SDLK_SPACE) && (kc < SDLK_DELETE))
				{
					Key_Event(kc, down, false);
				}
				else
				{
					int key = IN_TranslateSDLtoQ2Key(kc);
					/* yquake2 has this, km does not
					if(key == 0)
					{
						// fallback to scancodes if we don't know the keycode
						key = IN_TranslateScancodeToQ2Key(sc);
					}
					*/
					if(key != 0)
					{
						Key_Event(key, down, Sys_Milliseconds());
					}
					else
					{
						Com_DPrintf("Pressed unknown key with SDL_Keycode %d, SDL_Scancode %d.\n", kc, (int)sc);
					}
				}
			}
		}
		else if (evt.type == SDL_MOUSEMOTION)
		{
		/*	if (mouse_active)
			{
				// Relative should be on here
				mx += evt.motion.xrel;
				my += evt.motion.yrel;
			} */
			mx += evt.motion.xrel;
			my += evt.motion.yrel;
		}
		else if (evt.type == SDL_MOUSEBUTTONDOWN)
		{
			int mouse_button = evt.button.button - 1;
			if (mouse_button == 1)
			{
				mouse_button = 2;
			}
			else if (mouse_button == 2)
			{
				mouse_button = 1;
			}
			if (Sys_Milliseconds() - ui_mousecursor.buttontime[mouse_button] < multiclicktime)
			{
				ui_mousecursor.buttonclicks[mouse_button] += 1;
			}
			else
			{
				ui_mousecursor.buttonclicks[mouse_button] = 1;
			}

			if (ui_mousecursor.buttonclicks[mouse_button] > 3)
			{
				ui_mousecursor.buttonclicks[mouse_button] = 3;
			}

			ui_mousecursor.buttontime[mouse_button] = Sys_Milliseconds();

			ui_mousecursor.buttondown[mouse_button] = true;
			ui_mousecursor.buttonused[mouse_button] = false;
			ui_mousecursor.mouseaction = true;

			Key_Event(K_MOUSE1 + mouse_button, 1, Sys_Milliseconds());
		}
		else if (evt.type == SDL_MOUSEBUTTONUP)
		{
			int mouse_button = evt.button.button - 1;
			if (mouse_button == 1)
			{
				mouse_button = 2;
			}
			else if (mouse_button == 2)
			{
				mouse_button = 1;
			}

			ui_mousecursor.buttondown[mouse_button] = false;
			ui_mousecursor.buttonused[mouse_button] = false;
			ui_mousecursor.mouseaction = true;

			Key_Event(K_MOUSE1 + mouse_button, 0, Sys_Milliseconds());
		}
		else if (evt.type == SDL_MOUSEWHEEL)
		{
			int dir;
			if (evt.wheel.y > 0)
			{
				dir = K_MWHEELUP;
			}
			else
			{
				dir = K_MWHEELDOWN;
			}

			/* We only get one event per wheel change, so instantly press/release */
			Key_Event(dir, 1, Sys_Milliseconds());
			Key_Event(dir, 0, Sys_Milliseconds());
		}
		// flibitijibibo added
		else if (evt.type == SDL_CONTROLLERDEVICEADDED)
		{
			SDL_GameController *c = SDL_GameControllerOpen(evt.cdevice.which);
			Com_DPrintf ("Connected %s\n", SDL_GameControllerName(c));
		}
		else if (evt.type == SDL_CONTROLLERDEVICEREMOVED)
		{
			SDL_GameController *c = SDL_GameControllerFromInstanceID(evt.cdevice.which);
			Com_DPrintf ("Disconnected %s\n", SDL_GameControllerName(c));
			SDL_GameControllerClose(c);
		}
		else if (evt.type == SDL_CONTROLLERBUTTONDOWN)
		{
			if (in_joystick->value)
			{
				Key_Event (
					IN_TranslateSDLtoQ2Button(evt.cbutton.button),
					1,
					Sys_Milliseconds()
				);
			}
		}
		else if (evt.type == SDL_CONTROLLERBUTTONUP)
		{
			Key_Event (
				IN_TranslateSDLtoQ2Button(evt.cbutton.button),
				0,
				Sys_Milliseconds()
			);
		}
		else if (evt.type == SDL_CONTROLLERAXISMOTION)
		{
			if (evt.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
			{
				if (!in_joystick->value)
				{
					evt.caxis.value = 0;
				}
				Key_Event (
					K_AUX27,
					evt.caxis.value > 3855,
					Sys_Milliseconds()
				);
			}
			else if (evt.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
			{
				if (!in_joystick->value)
				{
					evt.caxis.value = 0;
				}
				Key_Event (
					K_AUX28,
					evt.caxis.value > 3855,
					Sys_Milliseconds()
				);
			}
			else if (evt.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
			{
				if ( !in_joystick->value || (SDL_abs(evt.caxis.value) <= 7849) )
				{
					evt.caxis.value = 0;
				}
				controller_leftx = evt.caxis.value / 32768.0f;
			}
			else if (evt.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
			{
				if ( !in_joystick->value || (SDL_abs(evt.caxis.value) <= 7849) )
				{
					evt.caxis.value = 0;
				}
				controller_lefty = evt.caxis.value / 32768.0f;
			}
			else if (evt.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX)
			{
				if ( !in_joystick->value || (SDL_abs(evt.caxis.value) <= 8689) )
				{
					evt.caxis.value = 0;
				}
				controller_rightx = evt.caxis.value / 32768.0f;
			}
			else if (evt.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY)
			{
				if ( !in_joystick->value || (SDL_abs(evt.caxis.value) <= 8689) )
				{
					evt.caxis.value = 0;
				}
				controller_righty = evt.caxis.value / 32768.0f;
			}
		}
		// end flibitijibibo
	}
}
