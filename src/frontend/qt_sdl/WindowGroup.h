/*
    Copyright 2016-2026 melonDS team

    This file is part of melonDS.

    melonDS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    melonDS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with melonDS. If not, see http://www.gnu.org/licenses/.
*/

#ifndef WINDOWGROUP_H
#define WINDOWGROUP_H

class MainWindow;

namespace WindowGroup
{

void arrangeSideBySide();
void arrangeGrid2x2();
void applyEvenAll();
void applyTopOnlyAll();
void setLocked(bool locked);
bool isLocked();

void setHideMenuBar(bool hide);
bool isHideMenuBar();
void syncHideMenuBarActions();

// 2 -> side + even + lock; 4 -> grid + top + lock; other N: arrange/lock, Even if N<=2 else TopOnly
void applyAutoLayout(int n);

// Call when a primary MainWindow is created so lock filters stay attached
void attachMainWindow(MainWindow* win);

void syncLockMenuActions();

// When any primary closes with 2+ instances, close the rest (re-entrant safe).
void closeGroupFrom(MainWindow* source);

// Reset every active local-MP instance (each keeps its own .sav / .sav.N).
void resetAll();

}

#endif
