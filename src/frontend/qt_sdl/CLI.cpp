/*
    Copyright 2021-2023 melonDS team

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <QApplication>
#include <QCommandLineParser>
#include <QStringList>

#include "CLI.h"
#include "Platform.h"

using melonDS::Platform::Log;
using melonDS::Platform::LogLevel;

namespace CLI
{

CommandLineOptions* ManageArgs(QApplication& melon)
{
    QCommandLineParser parser;
    parser.addHelpOption();

    parser.addPositionalArgument("nds", "Nintendo DS ROM (or an archive file which contains it) to load into Slot-1");
    parser.addPositionalArgument("gba", "GBA ROM (or an archive file which contains it) to load into Slot-2");

    parser.addOption(QCommandLineOption({"b", "boot"}, "Whether to boot firmware on startup. Defaults to \"auto\" (boot if NDS rom given)", "auto/always/never", "auto"));
    parser.addOption(QCommandLineOption({"f", "fullscreen"}, "Start melonDS in fullscreen mode"));
    // Matches kMaxEmuInstances in main.cpp (local MP in-process slots).
    parser.addOption(QCommandLineOption({"n", "instances"}, "Number of local multiplayer instances to start (1-16). Same ROM/saves as File > Multiplayer > Launch new instance (.sav, .sav.2, ...)", "count", "1"));
    parser.addOption(QCommandLineOption(QString("mp-layout"), "Local MP window layout after spawn: none (default) or auto (2=side-by-side dual-screen+lock; 4=2x2 top-only+lock)", "mode", "none"));

#ifdef ARCHIVE_SUPPORT_ENABLED
    parser.addOption(QCommandLineOption({"a", "archive-file"}, "Specify file to load inside an archive given (NDS)", "rom"));
    parser.addOption(QCommandLineOption({"A", "archive-file-gba"}, "Specify file to load inside an archive given (GBA)", "rom"));
#endif

    parser.process(melon);

    CommandLineOptions* options = new CommandLineOptions;

    options->fullscreen = parser.isSet("fullscreen");
    options->instances = 1;

    QStringList posargs = parser.positionalArguments();
    switch (posargs.size())
    {
        default:
            Log(LogLevel::Warn, "Too many positional arguments; ignoring 3 onwards\n");
        case 2:
            options->gbaRomPath = posargs[1];
        case 1:
            options->dsRomPath = posargs[0];
        case 0:
            break;
    }

    QString bootMode = parser.value("boot");
    if (bootMode == "auto")
    {
        options->boot = !posargs.empty();
    }
    else if (bootMode == "always")
    {
        options->boot = true;
    }
    else if (bootMode == "never")
    {
        options->boot = false;
    }
    else
    {
        Log(LogLevel::Error, "ERROR: -b/--boot only accepts auto/always/never as arguments\n");
        exit(1);
    }

    {
        bool ok = false;
        const int instances = parser.value("instances").toInt(&ok);
        // Keep in sync with kMaxEmuInstances in main.cpp.
        constexpr int kMaxInstances = 16;
        if (!ok || instances < 1 || instances > kMaxInstances)
        {
            Log(LogLevel::Error, "ERROR: -n/--instances must be an integer from 1 to %d\n", kMaxInstances);
            exit(1);
        }
        if (instances > 1 && !options->dsRomPath.has_value())
        {
            Log(LogLevel::Error, "ERROR: -n/--instances > 1 requires an NDS ROM path\n");
            exit(1);
        }
        options->instances = instances;
    }

    {
        const QString layout = parser.value("mp-layout").toLower();
        if (layout != "none" && layout != "auto")
        {
            Log(LogLevel::Error, "ERROR: --mp-layout only accepts none/auto\n");
            exit(1);
        }
        if (layout == "auto" && options->instances < 2)
        {
            Log(LogLevel::Error, "ERROR: --mp-layout auto requires -n/--instances >= 2\n");
            exit(1);
        }
        options->mpLayout = layout;
    }

#ifdef ARCHIVE_SUPPORT_ENABLED
    if (parser.isSet("archive-file"))
    {
        if (options->dsRomPath.has_value())
        {
            options->dsRomArchivePath = parser.value("archive-file");
        }
        else
        {
            Log(LogLevel::Error, "Option -a/--archive-file given, but no archive specified!");
        }
    }

    if (parser.isSet("archive-file-gba"))
    {
        if (options->gbaRomPath.has_value())
        {
            options->gbaRomArchivePath = parser.value("archive-file-gba");
        }
        else
        {
            Log(LogLevel::Error, "Option -A/--archive-file-gba given, but no archive specified!");
        }
    }
#endif

    return options;
}

}
