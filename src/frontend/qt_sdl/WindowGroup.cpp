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

#include "WindowGroup.h"

#include <cmath>
#include <vector>

#include <QEvent>
#include <QGuiApplication>
#include <QHash>
#include <QObject>
#include <QPoint>
#include <QRect>
#include <QScreen>
#include <QSize>
#include <QTimer>

#include "Config.h"
#include "EmuInstance.h"
#include "ScreenLayout.h"
#include "Window.h"
#include "main.h"

namespace WindowGroup
{

namespace
{

constexpr int kGapPx = 4;

std::vector<MainWindow*> collectPrimaryWindows()
{
    std::vector<MainWindow*> wins;
    for (int i = 0; i < 16; i++)
    {
        EmuInstance* inst = getEmuInstance(i);
        if (!inst)
            continue;
        MainWindow* win = inst->getMainWindow();
        if (win)
            wins.push_back(win);
    }
    return wins;
}

// Native 1x panel size for current sizing/layout (defines content aspect).
QSize contentPanelMinSize(MainWindow* win)
{
    if (!win || !win->panel)
        return QSize(256, 192);
    return win->panel->screenGetMinSize(1);
}

QSize chromeDiff(MainWindow* win)
{
    if (!win || !win->panel)
        return QSize(0, 0);
    return win->size() - win->panel->size();
}

// Snap a free panel size onto the content aspect; dominantAxis: 0=width, 1=height, -1=auto.
QSize snapPanelToAspect(QSize freePanel, QSize aspectRef, int dominantAxis)
{
    if (aspectRef.width() <= 0 || aspectRef.height() <= 0)
        return freePanel;

    const double aspect = double(aspectRef.width()) / double(aspectRef.height());
    int w = qMax(aspectRef.width(), freePanel.width());
    int h = qMax(aspectRef.height(), freePanel.height());

    int axis = dominantAxis;
    if (axis < 0)
    {
        const double dw = double(w) / aspectRef.width();
        const double dh = double(h) / aspectRef.height();
        axis = (dw >= dh) ? 0 : 1;
    }

    if (axis == 0)
        h = int(std::lround(w / aspect));
    else
        w = int(std::lround(h * aspect));

    if (w < aspectRef.width() || h < aspectRef.height())
    {
        w = aspectRef.width();
        h = aspectRef.height();
    }
    return QSize(w, h);
}

void setWindowPanelSize(MainWindow* win, QSize panelSize)
{
    if (!win || !win->panel)
        return;
    win->resize(panelSize + chromeDiff(win));
}

void disableIntegerScalingAll()
{
    for (MainWindow* win : collectPrimaryWindows())
        win->setIntegerScaling(false);
}

QSize savedPanelSize()
{
    auto g = Config::GetGlobalTable();
    return QSize(g.GetInt("WindowGroup.PanelWidth"), g.GetInt("WindowGroup.PanelHeight"));
}

void persistPanelSize(QSize panel, bool writeDisk)
{
    if (panel.width() < 1 || panel.height() < 1)
        return;

    auto g = Config::GetGlobalTable();
    g.SetInt("WindowGroup.PanelWidth", panel.width());
    g.SetInt("WindowGroup.PanelHeight", panel.height());

    if (!writeDisk)
        return;

    static QTimer* debounce = nullptr;
    if (!debounce)
    {
        debounce = new QTimer();
        debounce->setSingleShot(true);
        debounce->setInterval(400);
        QObject::connect(debounce, &QTimer::timeout, []() { Config::Save(); });
    }
    debounce->start();
}

void resizeToSavedAspect(MainWindow* win)
{
    if (!win || !win->panel)
        return;

    const QSize aspectRef = contentPanelMinSize(win);
    QSize free = savedPanelSize();
    if (free.width() < 1 || free.height() < 1)
    {
        free = win->panel->size();
        // Prefer at least 2x native if the panel is still at/near 1x.
        if (free.width() <= aspectRef.width() + 8 && free.height() <= aspectRef.height() + 8)
            free = QSize(aspectRef.width() * 2, aspectRef.height() * 2);
    }

    setWindowPanelSize(win, snapPanelToAspect(free, aspectRef, -1));
}

void rememberGeometry(QHash<MainWindow*, QPoint>& lastPos, QHash<MainWindow*, QSize>& lastSize)
{
    lastPos.clear();
    lastSize.clear();
    for (MainWindow* win : collectPrimaryWindows())
    {
        lastPos[win] = win->pos();
        lastSize[win] = win->size();
    }
}

void placeWindows(const std::vector<MainWindow*>& wins, int cols, bool keepOrigin)
{
    if (wins.empty())
        return;

    const int colsClamped = qMax(1, cols);
    const int rows = (static_cast<int>(wins.size()) + colsClamped - 1) / colsClamped;

    int cellW = 0;
    int cellH = 0;
    for (MainWindow* win : wins)
    {
        cellW = qMax(cellW, win->frameGeometry().width());
        cellH = qMax(cellH, win->frameGeometry().height());
    }
    if (cellW <= 0 || cellH <= 0)
        return;

    int originX = 0;
    int originY = 0;
    if (keepOrigin)
    {
        originX = wins[0]->frameGeometry().x();
        originY = wins[0]->frameGeometry().y();
        for (MainWindow* win : wins)
        {
            originX = qMin(originX, win->frameGeometry().x());
            originY = qMin(originY, win->frameGeometry().y());
        }
    }
    else
    {
        MainWindow* anchor = wins[0];
        QScreen* screen = anchor->screen();
        if (!screen)
            screen = QGuiApplication::primaryScreen();
        if (!screen)
            return;

        const QRect avail = screen->availableGeometry();
        const int totalW = colsClamped * cellW + (colsClamped - 1) * kGapPx;
        const int totalH = rows * cellH + (rows - 1) * kGapPx;
        originX = avail.x() + qMax(0, (avail.width() - totalW) / 2);
        originY = avail.y() + qMax(0, (avail.height() - totalH) / 2);
    }

    for (size_t i = 0; i < wins.size(); i++)
    {
        const int col = static_cast<int>(i) % colsClamped;
        const int row = static_cast<int>(i) / colsClamped;
        const int x = originX + col * (cellW + kGapPx);
        const int y = originY + row * (cellH + kGapPx);
        wins[i]->move(x, y);
        wins[i]->raise();
    }
}

class MoveSyncFilter : public QObject
{
public:
    bool eventFilter(QObject* obj, QEvent* event) override
    {
        if (!locked || syncing)
            return QObject::eventFilter(obj, event);

        MainWindow* leader = qobject_cast<MainWindow*>(obj);
        if (!leader || leader->getWindowID() != 0)
            return QObject::eventFilter(obj, event);

        if (leader->isMaximized() || leader->isFullScreen())
            return QObject::eventFilter(obj, event);

        if (event->type() == QEvent::Move)
            return handleMove(leader);

        if (event->type() == QEvent::Resize)
            return handleResize(leader);

        return QObject::eventFilter(obj, event);
    }

    bool handleMove(MainWindow* leader)
    {
        const QPoint newPos = leader->pos();
        auto it = lastPos.find(leader);
        if (it == lastPos.end())
        {
            lastPos[leader] = newPos;
            return false;
        }

        const QPoint delta = newPos - it.value();
        it.value() = newPos;
        if (delta.isNull())
            return false;

        syncing = true;
        for (MainWindow* win : collectPrimaryWindows())
        {
            if (win == leader)
                continue;
            if (win->isMaximized() || win->isFullScreen())
                continue;
            win->move(win->pos() + delta);
            lastPos[win] = win->pos();
        }
        syncing = false;
        return false;
    }

    bool handleResize(MainWindow* leader)
    {
        if (!leader->panel)
            return false;

        const QSize newWinSize = leader->size();
        auto it = lastSize.find(leader);
        QSize oldWinSize = it == lastSize.end() ? newWinSize : it.value();

        const QSize aspectRef = contentPanelMinSize(leader);
        const QSize chrome = chromeDiff(leader);
        QSize freePanel = newWinSize - chrome;
        if (freePanel.width() < 1 || freePanel.height() < 1)
            freePanel = aspectRef;

        const int dW = newWinSize.width() - oldWinSize.width();
        const int dH = newWinSize.height() - oldWinSize.height();
        int dominant = -1;
        if (std::abs(dW) > std::abs(dH))
            dominant = 0;
        else if (std::abs(dH) > std::abs(dW))
            dominant = 1;

        const QSize snappedPanel = snapPanelToAspect(freePanel, aspectRef, dominant);
        const QSize snappedWin = snappedPanel + chrome;

        syncing = true;

        if (snappedWin != newWinSize)
            leader->resize(snappedWin);

        for (MainWindow* win : collectPrimaryWindows())
        {
            if (win == leader)
                continue;
            if (win->isMaximized() || win->isFullScreen())
                continue;
            win->resize(snappedWin);
        }

        auto wins = collectPrimaryWindows();
        if (wins.size() >= 2)
            placeWindows(wins, arrangeCols, true);

        persistPanelSize(snappedPanel, true);
        rememberGeometry(lastPos, lastSize);
        syncing = false;
        return false;
    }

    void rememberAll()
    {
        rememberGeometry(lastPos, lastSize);
    }

    bool locked = false;
    bool syncing = false;
    int arrangeCols = 2;
    QHash<MainWindow*, QPoint> lastPos;
    QHash<MainWindow*, QSize> lastSize;
};

MoveSyncFilter* filter()
{
    static MoveSyncFilter* f = new MoveSyncFilter();
    return f;
}

void applySizingToAll(int sizing)
{
    for (MainWindow* win : collectPrimaryWindows())
        win->applyScreenSizing(sizing);
}

void prepareNoBarSizing()
{
    disableIntegerScalingAll();
}

void applySavedSizeToAll()
{
    for (MainWindow* win : collectPrimaryWindows())
        resizeToSavedAspect(win);
}

} // namespace

void arrangeSideBySide()
{
    auto wins = collectPrimaryWindows();
    if (wins.size() < 2)
        return;
    if (wins.size() > 2)
        wins.resize(2);
    filter()->arrangeCols = 2;
    filter()->syncing = true;
    placeWindows(wins, 2, false);
    filter()->rememberAll();
    filter()->syncing = false;
}

void arrangeGrid2x2()
{
    auto wins = collectPrimaryWindows();
    if (wins.empty())
        return;
    if (wins.size() > 4)
        wins.resize(4);
    filter()->arrangeCols = 2;
    filter()->syncing = true;
    placeWindows(wins, 2, false);
    filter()->rememberAll();
    filter()->syncing = false;
}

void applyEvenAll()
{
    prepareNoBarSizing();
    applySizingToAll(screenSizing_Even);
    filter()->syncing = true;
    applySavedSizeToAll();
    filter()->rememberAll();
    filter()->syncing = false;
}

void applyTopOnlyAll()
{
    prepareNoBarSizing();
    applySizingToAll(screenSizing_TopOnly);
    filter()->syncing = true;
    applySavedSizeToAll();
    filter()->rememberAll();
    filter()->syncing = false;
}

void setLocked(bool locked)
{
    filter()->locked = locked;
    Config::GetGlobalTable().SetBool("WindowGroup.LockPositions", locked);

    auto wins = collectPrimaryWindows();
    for (MainWindow* win : wins)
    {
        win->removeEventFilter(filter());
        if (locked)
            win->installEventFilter(filter());
    }

    if (locked)
    {
        prepareNoBarSizing();
        filter()->syncing = true;
        applySavedSizeToAll();
        if (wins.size() >= 2)
            placeWindows(wins, filter()->arrangeCols, true);
        filter()->rememberAll();
        filter()->syncing = false;
    }
    else
    {
        filter()->rememberAll();
    }

    syncLockMenuActions();
}

bool isLocked()
{
    return filter()->locked;
}

void setHideMenuBar(bool hide)
{
    Config::GetGlobalTable().SetBool("WindowGroup.HideMenuBar", hide);

    auto wins = collectPrimaryWindows();
    filter()->syncing = true;
    for (MainWindow* win : wins)
    {
        if (win->actHideMenuBar)
            win->actHideMenuBar->setChecked(hide);
        win->applyMenuBarVisibility();
    }

    // Chrome height changed — re-snap panel sizes so aspect stays bar-free.
    applySavedSizeToAll();
    if (wins.size() >= 2 && filter()->locked)
        placeWindows(wins, filter()->arrangeCols, true);
    filter()->rememberAll();
    filter()->syncing = false;

    syncHideMenuBarActions();
}

bool isHideMenuBar()
{
    return Config::GetGlobalTable().GetBool("WindowGroup.HideMenuBar");
}

void syncHideMenuBarActions()
{
    const bool hide = isHideMenuBar();
    for (MainWindow* win : collectPrimaryWindows())
    {
        if (win->actHideMenuBar)
            win->actHideMenuBar->setChecked(hide);
        win->applyMenuBarVisibility();
    }
}

void applyAutoLayout(int n)
{
    if (n < 2)
        return;

    if (n <= 2)
    {
        applyEvenAll();
        setHideMenuBar(true);
        arrangeSideBySide();
    }
    else
    {
        applyTopOnlyAll();
        setHideMenuBar(true);
        arrangeGrid2x2();
    }

    setLocked(true);
}

void attachMainWindow(MainWindow* win)
{
    if (!win || win->getWindowID() != 0)
        return;

    static bool configLoaded = false;
    if (!configLoaded)
    {
        configLoaded = true;
        filter()->locked = Config::GetGlobalTable().GetBool("WindowGroup.LockPositions");
    }

    if (filter()->locked)
    {
        win->installEventFilter(filter());
        filter()->lastPos[win] = win->pos();
        filter()->lastSize[win] = win->size();
    }

    win->applyMenuBarVisibility();
    syncLockMenuActions();
    syncHideMenuBarActions();
}

void syncLockMenuActions()
{
    const bool locked = filter()->locked;
    for (MainWindow* win : collectPrimaryWindows())
    {
        if (win->actMPLockPositions)
            win->actMPLockPositions->setChecked(locked);
    }
}

void closeGroupFrom(MainWindow* source)
{
    static bool closing = false;
    if (closing)
        return;
    if (!source || source->getWindowID() != 0)
        return;
    if (numEmuInstances() < 2)
        return;

    closing = true;
    for (MainWindow* win : collectPrimaryWindows())
    {
        if (win != source)
            win->close();
    }
    closing = false;
}

void resetAll()
{
    for (int i = 0; i < 16; i++)
    {
        EmuInstance* inst = getEmuInstance(i);
        if (!inst)
            continue;
        EmuThread* thread = inst->getEmuThread();
        if (thread && thread->emuIsActive())
            thread->emuReset();
    }
}

} // namespace WindowGroup

