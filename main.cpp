/***************************************************************************
 *   Copyright (C) 2010, 2011, 2012, 2013, 2014 by Terraneo Federico       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#include "mxgui/entry.h"
#include "mxgui/display.h"
#include "mxgui/misc_inst.h"
#include "mxgui/level2/input.h"
#include <cstdio>
#include <cstring>
#include <vector>

using namespace std;
using namespace mxgui;

struct ColorArea {
    Color color;
    unsigned short x1, y1, x2, y2; 
};

class SimonGame {
private:
    Display& display;
    InputHandler& inputHandler;
    std::vector<Color> sequence;
    std::vector<ColorArea> areas;    
    int baseDelay = 3000;
    int minDelay = 100;
    Color colorMap[4] = {red, green, blue, white}; 
    
    void fillRectangle(Point p1, Point p2, Color color) {
        DrawingContext dc(display);
        for (int y = p1.y(); y <= p2.y(); ++y) {
            dc.line(Point(p1.x(), y), Point(p2.x(), y), color);
        }
    }

    void drawAllAreas() {
        {
            DrawingContext dc(display);
            for (int i = 0; i < 4; ++i) {
                ColorArea area = areas[i];
                fillRectangle(Point(area.x1, area.y1), Point(area.x2, area.y2), colorMap[i]);
            }
        }
    }

    void displayGameOver() {
        {
            DrawingContext dc(display);
            dc.clear(black);
            dc.setTextColor(white, black); 
            dc.setFont(tahoma);
            dc.write(Point(display.getWidth() / 2 - 30, display.getHeight() / 2 - 10), "Game Over"); 
        }
        miosix::delayMs(2000);
    }

    void initialMenu() {
        for (;;) {            
            Event e = inputHandler.popEvent(); 
            if (e.getEvent() == EventType::None) {
                break;
            }
        }
        drawAllAreas();        
        {
            DrawingContext dc(display);
            dc.setTextColor(white, black);
            dc.setFont(tahoma); 
            
            dc.write(Point(display.getWidth() / 2 - 47, display.getHeight() / 2 - 10), "SIMON SAYS GAME");
            dc.write(Point(display.getWidth() / 2 - 32, display.getHeight() / 2 + 10), "Scroll to start");
        }
        
        for (;;) {
            Event e = inputHandler.popEvent();
            if (e.getEvent() == EventType::TouchMove) {
                DrawingContext dc(display);
                miosix::delayMs(1000);
                dc.clear(black);
                break;
            }
        }
    }

    void displayColor(Color color) {
        Color flashColor = colorMap[static_cast<int>(color)];
        ColorArea area = areas[static_cast<int>(color)];

        fillRectangle(Point(area.x1, area.y1), Point(area.x2, area.y2), flashColor);
    }
    
    void displaySequence() {
        int delay = max(minDelay, baseDelay - static_cast<int>(sequence.size() * 200));

        miosix::delayMs(200);
        for (Color color : sequence) {
            DrawingContext dc(display);
            dc.clear(black);
            miosix::delayMs(100);
            displayColor(color);
            miosix::delayMs(delay);
        }
    }

    Point handleTouch(){
        for (;;) {            
            Event e = inputHandler.popEvent(); 
            if (e.getEvent() == EventType::None) {
                break;
            }
        }  
        for(;;)
        {
            Event e = inputHandler.popEvent();
            switch(e.getEvent())
            { 
                case EventType::TouchDown:
                {
                    return e.getPoint();
                }
                default:
                break;
            }
        }
    }    

    Color getColorFromTouch(Point p) {
        for (const auto& area : areas) {
            if (p.x() >= area.x1 && p.x() <= area.x2 && p.y() >= area.y1 && p.y() <= area.y2)
                return area.color;
        }
        return red;
    }

    bool getUserInput() {
        for (Color expectedColor : sequence) {
            Point touchedPoint = handleTouch();
            ColorArea area = areas[static_cast<int>(expectedColor)];
            if (!(touchedPoint.x() >= area.x1 && touchedPoint.x() <= area.x2 && touchedPoint.y() >= area.y1 && touchedPoint.y() <= area.y2))
                return false;
        }
        sequence.push_back(static_cast<Color>(rand() % 4));
        return true;
    }

public:
    SimonGame(Display& disp, InputHandler& handler)
        : display(disp), inputHandler(handler) {
        srand(static_cast<unsigned int>(time(0)));        
        unsigned short maxX = display.getWidth() - 1;
        unsigned short maxY = display.getHeight() - 1;
        areas.push_back({red, 0, 0, static_cast<unsigned short>(maxX / 2), static_cast<unsigned short>(maxY / 2)});
        areas.push_back({white, static_cast<unsigned short>(maxX / 2), 0, maxX, static_cast<unsigned short>(maxY / 2)});
        areas.push_back({green, 0, static_cast<unsigned short>(maxY / 2), static_cast<unsigned short>(maxX / 2), maxY});
        areas.push_back({blue, static_cast<unsigned short>(maxX / 2), static_cast<unsigned short>(maxY / 2), maxX, maxY});
    }

   void run() {
        for (;;) {            
            initialMenu();
            sequence.clear();
            sequence.push_back(static_cast<Color>(rand() % 4));
            
            for (;;) {
                displaySequence();
                drawAllAreas();
                if (!getUserInput()) {
                    displayGameOver(); 
                    break;
                }
            }
        }
    }
};

ENTRY()
{
    Display& display = DisplayManager::instance().getDisplay();
    InputHandler& inputHandler = InputHandler::instance();

    SimonGame game(display, inputHandler);
    game.run();

    return 0;
}
