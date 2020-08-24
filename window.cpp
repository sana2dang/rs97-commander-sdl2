#include <iostream>
#include "window.h"
#include "def.h"
#include "sdlutils.h"

#define KEYHOLD_TIMER_FIRST   6
#define KEYHOLD_TIMER         2

extern SDL_Surface *ScreenSurface;
extern SDL_GameController *sdl_cntrl;

CWindow::CWindow(void):
    m_timer(0),
    m_lastPressed(SDLK_0),
    m_retVal(0)
{
    // Add window to the lists for render
    Globals::g_windows.push_back(this);
}

CWindow::~CWindow(void)
{
    // Remove last window
    Globals::g_windows.pop_back();
}

const int CWindow::execute(void)
{
    m_retVal = 0;
    Uint32 l_time(0);
    SDL_Event l_event;
    bool l_loop(true);
    bool l_render(true);
    // Main loop
    while (l_loop)
    {
        l_time = SDL_GetTicks();
        // Handle key press
        while (SDL_PollEvent(&l_event))
        {
            if (l_event.type == SDL_KEYDOWN)
            {
                l_render = this->keyPress(l_event);
                if (m_retVal)
                    l_loop = false;
            }
            else if (l_event.type == SDL_QUIT) 
            {
                return m_retVal;
            }
#if 0
			else if (l_event.type == SDL_JOYAXISMOTION)
			{
                SDL_Event key_event;
				
				if (l_event.jaxis.value != 0)
				{
					if (l_event.jaxis.axis == 0)	// left/right
					{
						if (l_event.jaxis.value < 0)
							key_event.key.keysym.sym = MYKEY_LEFT;
						else if (l_event.jaxis.value > 0)		
							key_event.key.keysym.sym = MYKEY_RIGHT;						
					}
					else if (l_event.jaxis.axis == 1) // up/down
					{
						if (l_event.jaxis.value < 0)
							key_event.key.keysym.sym = MYKEY_UP;
						else if (l_event.jaxis.value > 0)
							key_event.key.keysym.sym = MYKEY_DOWN;
					}
						
					l_render = this->keyPress(key_event);
					if (m_retVal)
						l_loop = false;		
				}
				
			}
			else if (l_event.type == SDL_JOYHATMOTION)
			{
                //printf("hat:%d, %d\n",l_event.jhat.hat, l_event.jhat.value);
                SDL_Event key_event;
				
				if (l_event.jhat.value != 0)
				{
					switch (l_event.jhat.value)
					{
					case SDL_HAT_UP: //up
						key_event.key.keysym.sym = MYKEY_UP;
						break;
					case SDL_HAT_DOWN: //down
						key_event.key.keysym.sym = MYKEY_DOWN;
						break;
					case SDL_HAT_LEFT: //left
						key_event.key.keysym.sym = MYKEY_LEFT;
						break;
					case SDL_HAT_RIGHT: //right
						key_event.key.keysym.sym = MYKEY_RIGHT;
						break;
	 
					default:
						break;
					}
					
					l_render = this->keyPress(key_event);
					if (m_retVal)
						l_loop = false;		
				}				
			}
            else if(l_event.type == SDL_JOYBUTTONDOWN)
            {
#if  0 //def ODROID_GO_ADVANCE
                // printf("key:%d\n",l_event.jbutton.button);
                SDL_Event key_event;
                switch (l_event.jbutton.button)
                {
                case 6: //up
                    key_event.key.keysym.sym = MYKEY_UP;
                    break;
                case 7: //down
                    key_event.key.keysym.sym = MYKEY_DOWN;
                    break;
                case 8: //left
                    key_event.key.keysym.sym = MYKEY_LEFT;
                    break;
                case 9: //right
                    key_event.key.keysym.sym = MYKEY_RIGHT;
                    break;
                case 0: //a
                    key_event.key.keysym.sym = MYKEY_PARENT;
                    break;
                case 1: //b
                    key_event.key.keysym.sym = MYKEY_OPEN;
                    break;
                case 2: //x
                    key_event.key.keysym.sym = MYKEY_OPERATION;
                    break;
                case 3: //y
                    key_event.key.keysym.sym = MYKEY_SYSTEM;
                    break;
                case 4: //l
                    key_event.key.keysym.sym = MYKEY_PAGEUP;
                    break;
                case 5: //r
                    key_event.key.keysym.sym = MYKEY_PAGEDOWN;
                    break;
                case 14://select
                    key_event.key.keysym.sym = MYKEY_SELECT;
                    break;
                case 15://start
                    key_event.key.keysym.sym = MYKEY_TRANSFER;
                    break; 
                default:
                    break;
                }
                l_render = this->keyPress(key_event);
                if (m_retVal)
                    l_loop = false;
#endif


#if 1
                // printf("key:%d\n",l_event.jbutton.button);
                SDL_Event key_event;
                switch (l_event.jbutton.button)
                {
                case 2: //b
                    key_event.key.keysym.sym = MYKEY_PARENT;
                    break;
                case 1: //a
                    key_event.key.keysym.sym = MYKEY_OPEN;
                    break;
                case 0: //x
                    key_event.key.keysym.sym = MYKEY_OPERATION;
                    break;
                case 3: //y
                    key_event.key.keysym.sym = MYKEY_SYSTEM;
                    break;
                case 4: //l
                    key_event.key.keysym.sym = MYKEY_PAGEUP;
                    break;
                case 5: //r
                    key_event.key.keysym.sym = MYKEY_PAGEDOWN;
                    break;
                case 8://select
                    key_event.key.keysym.sym = MYKEY_SELECT;
                    break;
                case 9://start
                    key_event.key.keysym.sym = MYKEY_TRANSFER;
                    break; 
                default:
                    break;
                }
                l_render = this->keyPress(key_event);
                if (m_retVal)
                    l_loop = false;
#endif
            }
#endif
			else if (l_event.type == SDL_CONTROLLERBUTTONDOWN)
			{
				//printf("[trngaje] SDL_CONTROLLERBUTTONDOWN\n");
                SDL_Event key_event;
                switch (l_event.cbutton.button)
                {
                case SDL_CONTROLLER_BUTTON_DPAD_UP: //up
                    key_event.key.keysym.sym = MYKEY_UP;
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_DOWN: //down
                    key_event.key.keysym.sym = MYKEY_DOWN;
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_LEFT: //left
                    key_event.key.keysym.sym = MYKEY_LEFT;
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: //right
                    key_event.key.keysym.sym = MYKEY_RIGHT;
                    break;
                case SDL_CONTROLLER_BUTTON_B: //b
                    key_event.key.keysym.sym = MYKEY_PARENT;
                    break;
                case SDL_CONTROLLER_BUTTON_A: //a
                    key_event.key.keysym.sym = MYKEY_OPEN;
                    break;
                case SDL_CONTROLLER_BUTTON_X: //x
                    key_event.key.keysym.sym = MYKEY_OPERATION;
                    break;
                case SDL_CONTROLLER_BUTTON_Y: //y
                    key_event.key.keysym.sym = MYKEY_SYSTEM;
                    break;
                case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: //l
                    key_event.key.keysym.sym = MYKEY_PAGEUP;
                    break;
                case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: //r
                    key_event.key.keysym.sym = MYKEY_PAGEDOWN;
                    break;
                case SDL_CONTROLLER_BUTTON_BACK://select
                    key_event.key.keysym.sym = MYKEY_SELECT;
                    break;
                case SDL_CONTROLLER_BUTTON_START://start
                    key_event.key.keysym.sym = MYKEY_TRANSFER;
                    break; 
                default:
                    break;
                }
                l_render = this->keyPress(key_event);
                if (m_retVal)
                    l_loop = false;
				
			}
			else if (l_event.type == SDL_CONTROLLERAXISMOTION)
			{
				//printf("[trngaje] SDL_CONTROLLERAXISMOTION\n");
                SDL_Event key_event;
				
				if (l_event.caxis.value != 0)
				{
					if (l_event.caxis.axis == 0)	// left/right
					{
						if (l_event.caxis.value < 0)
							key_event.key.keysym.sym = MYKEY_LEFT;
						else if (l_event.caxis.value > 0)		
							key_event.key.keysym.sym = MYKEY_RIGHT;						
					}
					else if (l_event.caxis.axis == 1) // up/down
					{
						if (l_event.caxis.value < 0)
							key_event.key.keysym.sym = MYKEY_UP;
						else if (l_event.caxis.value > 0)
							key_event.key.keysym.sym = MYKEY_DOWN;
					}
						
					l_render = this->keyPress(key_event);
					if (m_retVal)
						l_loop = false;		
				}
			}
        }
        // Handle key hold
        if (l_loop)
            l_render = this->keyHold() || l_render;
        // Render if necessary
        if (l_render && l_loop)
        {
            SDL_utils::renderAll();
            // Flip twice to avoid graphical glitch on Dingoo
            //SDL_Flip(Globals::g_screen);
            //SDL_Flip(Globals::g_screen);
            // SDL_Flip(ScreenSurface);
            SDL_UpdateWindowSurface(Globals::g_sdlwindow);
            SDL_UpdateWindowSurface(Globals::g_sdlwindow);

            l_render = false;
            INHIBIT(std::cout << "Render time: " << SDL_GetTicks() - l_time << "ms"<< std::endl;)
        }
        // Cap the framerate
        l_time = MS_PER_FRAME - (SDL_GetTicks() - l_time);
        if (l_time <= MS_PER_FRAME) SDL_Delay(l_time);
    }
    return m_retVal;
}

const bool CWindow::keyPress(const SDL_Event &p_event)
{
    // Reset timer if running
    if (m_timer)
        m_timer = 0;
    m_lastPressed = p_event.key.keysym.sym;
    return false;
}

const bool CWindow::keyHold(void)
{
    // Default behavior
    return false;
}

const bool CWindow::tick(const Uint8 p_held)
{
    bool l_ret(false);
    if (p_held)
    {
        if (m_timer)
        {
            --m_timer;
            if (!m_timer)
            {
                // Trigger!
                l_ret = true;
                // Timer continues
                m_timer = KEYHOLD_TIMER;
            }
        }
        else
        {
            // Start timer
            m_timer = KEYHOLD_TIMER_FIRST;
        }
    }
    else
    {
        // Stop timer if running
        if (m_timer)
            m_timer = 0;
    }
    return l_ret;
}

const int CWindow::getReturnValue(void) const
{
    return m_retVal;
}

bool CWindow::isFullScreen(void) const
{
    // Default behavior
    return false;
}
