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

#ifndef WINDOW_H
#define WINDOW_H

#include "glad/glad.h"
#include "ScreenLayout.h"
#include "duckstation/gl/context.h"

#include <QWidget>
#include <QWindow>
#include <QMainWindow>
#include <QImage>
#include <QActionGroup>
#include <QTimer>
#include <QMutex>
#include <QScreen>
#include <QCloseEvent>
#include <QLineEdit>

#include "Screen.h"
#include "Config.h"
#include "MPInterface.h"


class EmuInstance;
class EmuThread;

const int kMaxRecentROMs = 10;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(int id, EmuInstance* inst, QWidget* parent = nullptr);
    ~MainWindow();

    EmuInstance* getEmuInstance() { return emuInstance; }
    Config::Table& getWindowConfig() { return windowCfg; }
    int getWindowID() { return windowID; }

    bool winHasMenu() { return hasMenu; }

    void saveEnabled(bool enabled);

    void toggleFullscreen();

    bool hasOpenGL() { return hasOGL; }
    GL::Context* getOGLContext();
    void initOpenGL();
    void deinitOpenGL();
    void setGLSwapInterval(int intv);
    void makeCurrentGL();
    void releaseGL();

    void drawScreen();

    bool preloadROMs(QStringList file, QStringList gbafile, bool boot);
    QStringList splitArchivePath(const QString& filename, bool useMemberSyntax);

    void onAppStateChanged(Qt::ApplicationState state);

    void onFocusIn();
    void onFocusOut();
    bool isFocused() { return focused; }

    void osdAddMessage(unsigned int color, const char* msg);

    // Apply ScreenSizing (Even / Top only / …) and refresh layout — used by WindowGroup
    void applyScreenSizing(int sizing);
    void setIntegerScaling(bool enabled);

    void applyMenuBarVisibility();
    void showCompactMenu(const QPoint& globalPos);

    // Forward a key to this instance (app-level rescue when ScreenPanel lacks focus).
    void routeKeyEvent(QKeyEvent* event, bool press);

    // Soft/remote keyboards (RustDesk Android) often need an IME-capable widget
    // focused — OpenGL panels do not count. Click the game to focus this sink.
    void focusKeySink();
    bool isKeySink(QWidget* w) const;

    // called when the MP interface is changed
    void updateMPInterface(melonDS::MPInterfaceType type);

    void loadRecentFilesMenu(bool loadcfg);
    //void updateVideoSettings(bool glchange);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

signals:
    void screenLayoutChange();

private slots:
    void onOpenFile();
    void onClickRecentFile();
    void onClearRecentFiles();
    void onBootFirmware();
    void onInsertCart();
    void onEjectCart();
    void onInsertGBACart();
    void onInsertGBAAddon();
    void onEjectGBACart();
    void onSaveState();
    void onLoadState();
    void onUndoStateLoad();
    void onImportSavefile();
    void onQuit();

    void onPause(bool checked);
    void onReset();
    void onResetAll();
    void onStop();
    void onFrameStep();
    void onOpenPowerManagement();
    void onOpenDateTime();
    void onDateTimeDialogFinished(int res);
    void onEnableCheats(bool checked);
    void onSetupCheats();
    void onCheatsDialogFinished(int res);
    void onROMInfo();
    void onRAMInfo();
    void onOpenTitleManager();
    void onMPNewInstance();
    void onMPArrangeSideBySide();
    void onMPArrangeGrid2x2();
    void onMPDualScreenAll();
    void onMPTopOnlyAll();
    void onMPLockPositions(bool checked);
    void onMPLinkedInput(bool checked);
    void onLANStartHost();
    void onLANStartClient();
    void onNPStartHost();
    void onNPStartClient();
    void onNPTest();

    void onOpenEmuSettings();
    void onEmuSettingsDialogFinished(int res);
    void onOpenInputConfig();
    void onInputConfigFinished(int res);
    void onOpenVideoSettings();
    void onOpenCameraSettings();
    void onCameraSettingsFinished(int res);
    void onOpenAudioSettings();
    void onUpdateAudioVolume(int vol, int dsisync);
    void onUpdateAudioSettings();
    void onAudioSettingsFinished(int res);
    void onOpenMPSettings();
    void onMPSettingsFinished(int res);
    void onOpenWifiSettings();
    void onWifiSettingsFinished(int res);
    void onOpenFirmwareSettings();
    void onFirmwareSettingsFinished(int res);
    void onOpenPathSettings();
    void onPathSettingsFinished(int res);
    void onOpenInterfaceSettings();
    void onInterfaceSettingsFinished(int res);
    void onUpdateInterfaceSettings();
    void onChangeScreenSize();
    void onChangeScreenRotation(QAction* act);
    void onChangeScreenGap(QAction* act);
    void onChangeScreenLayout(QAction* act);
    void onChangeScreenSwap(bool checked);
    void onChangeScreenSizing(QAction* act);
    void onChangeScreenAspect(QAction* act);
    void onChangeIntegerScaling(bool checked);
    void onOpenNewWindow();
    void onChangeScreenFiltering(bool checked);
    void onChangeShowOSD(bool checked);
    void onChangeHideMenuBar(bool checked);
    void onChangeLimitFramerate(bool checked);
    void onChangeAudioSync(bool checked);

    void onTitleUpdate(QString title);

    void onEmuStart();
    void onEmuStop();
    void onEmuPause(bool pause);
    void onEmuReset();

    void onUpdateVideoSettings(bool glchange);

    void onFullscreenToggled();
    void onScreenEmphasisToggled();

private:
    virtual void closeEvent(QCloseEvent* event) override;

    QStringList currentROM;
    QStringList currentGBAROM;
    QList<QString> recentFileList;
    QMenu *recentMenu;
    void updateRecentFilesMenu();

    bool verifySetup();
    QString pickFileFromArchive(QString archiveFileName);
    QStringList pickROM(bool gba);
    void updateCartInserted(bool gba);

    void createScreenPanel();
    void ensureKeySink();

    bool lanWarning(bool host);

    bool showOSD;

    bool hasOGL;

    bool pauseOnLostFocus;
    bool pausedManually;

    int windowID;
    bool enabledSaved;

    bool focused;

    EmuInstance* emuInstance;
    EmuThread* emuThread;

    Config::Table& globalCfg;
    Config::Table& localCfg;
    Config::Table windowCfg;

public:
    ScreenPanel* panel;
    QLineEdit* keySink = nullptr;

    bool hasMenu;

    QAction* actOpenROM;
    QAction* actBootFirmware;
    QAction* actCurrentCart;
    QAction* actInsertCart;
    QAction* actEjectCart;
    QAction* actCurrentGBACart;
    QAction* actInsertGBACart;
    QList<QAction*> actInsertGBAAddon;
    QAction* actEjectGBACart;
    QAction* actImportSavefile;
    QAction* actSaveState[9];
    QAction* actLoadState[9];
    QAction* actUndoStateLoad;
    QAction* actOpenConfig;
    QAction* actQuit;

    QAction* actPause;
    QAction* actReset;
    QAction* actResetAll;
    QAction* actStop;
    QAction* actFrameStep;
    QAction* actPowerManagement;
    QAction* actDateTime;
    QAction* actEnableCheats;
    QAction* actSetupCheats;
    QAction* actROMInfo;
    QAction* actRAMInfo;
    QAction* actTitleManager;
    QAction* actMPNewInstance;
    QAction* actMPArrangeSideBySide;
    QAction* actMPArrangeGrid2x2;
    QAction* actMPDualScreenAll;
    QAction* actMPTopOnlyAll;
    QAction* actMPLockPositions;
    QAction* actMPLinkedInput;
    QAction* actLANStartHost;
    QAction* actLANStartClient;
    QAction* actNPStartHost;
    QAction* actNPStartClient;
    QAction* actNPTest;

    QAction* actEmuSettings;
#ifdef __APPLE__
    QAction* actPreferences;
#endif
    QAction* actInputConfig;
    QAction* actVideoSettings;
    QAction* actCameraSettings;
    QAction* actAudioSettings;
    QAction* actMPSettings;
    QAction* actWifiSettings;
    QAction* actFirmwareSettings;
    QAction* actPathSettings;
    QAction* actInterfaceSettings;
    QAction* actScreenSize[4];
    QActionGroup* grpScreenRotation;
    QAction* actScreenRotation[screenRot_MAX];
    QActionGroup* grpScreenGap;
    QAction* actScreenGap[6];
    QActionGroup* grpScreenLayout;
    QAction* actScreenLayout[screenLayout_MAX];
    QAction* actScreenSwap;
    QActionGroup* grpScreenSizing;
    QAction* actScreenSizing[screenSizing_MAX];
    QAction* actIntegerScaling;
    QActionGroup* grpScreenAspectTop;
    QAction** actScreenAspectTop;
    QActionGroup* grpScreenAspectBot;
    QAction** actScreenAspectBot;
    QAction* actNewWindow;
    QAction* actScreenFiltering;
    QAction* actShowOSD;
    QAction* actHideMenuBar;
    QAction* actLimitFramerate;
    QAction* actAudioSync;

    QAction* actAbout;
};

#endif // WINDOW_H
