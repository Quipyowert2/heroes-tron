# Microsoft Developer Studio Project File - Name="heroes" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=heroes - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "heroes.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "heroes.mak" CFG="heroes - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "heroes - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "heroes - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "heroes - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "HAVE_CONFIG_H" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MT /W3 /GX /O2 /I "." /I "..\src" /I "..\lib" /I "\sdl\include" /D "HAVE_CONFIG_H" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /subsystem:console /machine:I386 /out:"../../bin/heroes.exe"

!ELSEIF  "$(CFG)" == "heroes - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "HAVE_CONFIG_H" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /Zp1 /MTd /W3 /Gm /GX /ZI /Od /I "." /I "..\src" /I "..\lib" /I "\sdl\include" /D "HAVE_CONFIG_H" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /subsystem:console /debug /machine:I386 /out:"../../bin/heroes.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "heroes - Win32 Release"
# Name "heroes - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\argv.c
# End Source File
# Begin Source File

SOURCE=..\src\argv.h
# End Source File
# Begin Source File

SOURCE=..\src\bonus.c
# End Source File
# Begin Source File

SOURCE=..\src\bonus.h
# End Source File
# Begin Source File

SOURCE=..\src\const.c
# End Source File
# Begin Source File

SOURCE=..\src\const.h
# End Source File
# Begin Source File

SOURCE=..\src\debughash.c
# End Source File
# Begin Source File

SOURCE=..\src\debughash.h
# End Source File
# Begin Source File

SOURCE=..\src\debugmsg.c
# End Source File
# Begin Source File

SOURCE=..\src\debugmsg.h
# End Source File
# Begin Source File

SOURCE=..\src\display.c
# End Source File
# Begin Source File

SOURCE=..\src\display.h
# End Source File
# Begin Source File

SOURCE=..\src\endian.h
# End Source File
# Begin Source File

SOURCE=..\src\endscroll.c
# End Source File
# Begin Source File

SOURCE=..\src\endscroll.h
# End Source File
# Begin Source File

SOURCE=..\src\errors.c
# End Source File
# Begin Source File

SOURCE=..\src\errors.h
# End Source File
# Begin Source File

SOURCE=..\src\explosions.c
# End Source File
# Begin Source File

SOURCE=..\src\explosions.h
# End Source File
# Begin Source File

SOURCE=..\src\extras.c
# End Source File
# Begin Source File

SOURCE=..\src\extras.h
# End Source File
# Begin Source File

SOURCE=..\src\fader.c
# End Source File
# Begin Source File

SOURCE=..\src\fader.h
# End Source File
# Begin Source File

SOURCE=..\src\fastmem.h
# End Source File
# Begin Source File

SOURCE=..\src\font.h
# End Source File
# Begin Source File

SOURCE=..\src\font_help.h
# End Source File
# Begin Source File

SOURCE=..\src\fontdata.c
# End Source File
# Begin Source File

SOURCE=..\src\fontdata.h
# End Source File
# Begin Source File

SOURCE=..\src\generic_list.h
# End Source File
# Begin Source File

SOURCE=..\src\gfx_reader.h
# End Source File
# Begin Source File

SOURCE=..\src\hedlite.c
# End Source File
# Begin Source File

SOURCE=..\src\hedlite.h
# End Source File
# Begin Source File

SOURCE=..\src\hendian.c
# End Source File
# Begin Source File

SOURCE=..\src\hendian.h
# End Source File
# Begin Source File

SOURCE=..\src\heroes.c
# End Source File
# Begin Source File

SOURCE=..\src\heroes.h
# End Source File
# Begin Source File

SOURCE=..\src\intro.c
# End Source File
# Begin Source File

SOURCE=..\src\intro.h
# End Source File
# Begin Source File

SOURCE=..\src\items.c
# End Source File
# Begin Source File

SOURCE=..\src\items.h
# End Source File
# Begin Source File

SOURCE=..\src\joystick.c
# End Source File
# Begin Source File

SOURCE=..\src\joystick.h
# End Source File
# Begin Source File

SOURCE=..\src\keyb.c
# End Source File
# Begin Source File

SOURCE=..\src\keyb.h
# End Source File
# Begin Source File

SOURCE=..\src\keys_heroes.h
# End Source File
# Begin Source File

SOURCE=..\src\keysdef.c
# End Source File
# Begin Source File

SOURCE=..\src\keysdef.h
# End Source File
# Begin Source File

SOURCE=..\src\menus.c
# End Source File
# Begin Source File

SOURCE=..\src\menus.h
# End Source File
# Begin Source File

SOURCE=..\src\misc.c
# End Source File
# Begin Source File

SOURCE=..\src\misc.h
# End Source File
# Begin Source File

SOURCE=..\src\musicfiles.c
# End Source File
# Begin Source File

SOURCE=..\src\musicfiles.h
# End Source File
# Begin Source File

SOURCE=..\src\options.c
# End Source File
# Begin Source File

SOURCE=..\src\options.h
# End Source File
# Begin Source File

SOURCE=..\src\pcx.c
# End Source File
# Begin Source File

SOURCE=..\src\pcx.h
# End Source File
# Begin Source File

SOURCE=..\src\pixelize.c
# End Source File
# Begin Source File

SOURCE=..\src\pixelize.h
# End Source File
# Begin Source File

SOURCE=..\src\render.c
# End Source File
# Begin Source File

SOURCE=..\src\render.h
# End Source File
# Begin Source File

SOURCE=..\src\renderdata.c
# End Source File
# Begin Source File

SOURCE=..\src\renderdata.h
# End Source File
# Begin Source File

SOURCE=..\src\rsc_files.c
# End Source File
# Begin Source File

SOURCE=..\src\rsc_files.h
# End Source File
# Begin Source File

SOURCE=..\src\rsc_files_hash.c
# End Source File
# Begin Source File

SOURCE=..\src\rsc_files_hash.h
# End Source File
# Begin Source File

SOURCE=..\src\savegame.c
# End Source File
# Begin Source File

SOURCE=..\src\savegame.h
# End Source File
# Begin Source File

SOURCE=..\src\scores.c
# End Source File
# Begin Source File

SOURCE=..\src\scores.h
# End Source File
# Begin Source File

SOURCE=..\src\scrtools.c
# End Source File
# Begin Source File

SOURCE=..\src\scrtools.h
# End Source File
# Begin Source File

SOURCE=..\src\sfx.c
# End Source File
# Begin Source File

SOURCE=..\src\sfx.h
# End Source File
# Begin Source File

SOURCE=..\src\sound.c
# End Source File
# Begin Source File

SOURCE=..\src\sound.h
# End Source File
# Begin Source File

SOURCE=..\src\sprglenz.c
# End Source File
# Begin Source File

SOURCE=..\src\sprglenz.h
# End Source File
# Begin Source File

SOURCE=..\src\sprite.c
# End Source File
# Begin Source File

SOURCE=..\src\sprite.h
# End Source File
# Begin Source File

SOURCE=..\src\spropaque.c
# End Source File
# Begin Source File

SOURCE=..\src\spropaque.h
# End Source File
# Begin Source File

SOURCE=..\src\sprprog.c
# End Source File
# Begin Source File

SOURCE=..\src\sprprog.h
# End Source File
# Begin Source File

SOURCE=..\src\sprprogwav.c
# End Source File
# Begin Source File

SOURCE=..\src\sprprogwav.h
# End Source File
# Begin Source File

SOURCE=..\src\sprrle.c
# End Source File
# Begin Source File

SOURCE=..\src\sprrle.h
# End Source File
# Begin Source File

SOURCE=..\src\sprshade.c
# End Source File
# Begin Source File

SOURCE=..\src\sprshade.h
# End Source File
# Begin Source File

SOURCE=..\src\sprtext.c
# End Source File
# Begin Source File

SOURCE=..\src\sprtext.h
# End Source File
# Begin Source File

SOURCE=..\src\sprunish.c
# End Source File
# Begin Source File

SOURCE=..\src\sprunish.h
# End Source File
# Begin Source File

SOURCE=..\src\sprzcol.c
# End Source File
# Begin Source File

SOURCE=..\src\sprzcol.h
# End Source File
# Begin Source File

SOURCE=..\src\structs.h
# End Source File
# Begin Source File

SOURCE=..\src\system.h
# End Source File
# Begin Source File

SOURCE=..\src\timer.c
# End Source File
# Begin Source File

SOURCE=..\src\timer.h
# End Source File
# Begin Source File

SOURCE=..\src\txts.c
# End Source File
# Begin Source File

SOURCE=..\src\txts.h
# End Source File
# Begin Source File

SOURCE=..\src\userconf.c
# End Source File
# Begin Source File

SOURCE=..\src\userconf.h
# End Source File
# Begin Source File

SOURCE=..\src\userdir.c
# End Source File
# Begin Source File

SOURCE=..\src\userdir.h
# End Source File
# Begin Source File

SOURCE=..\src\visuals.c
# End Source File
# Begin Source File

SOURCE=..\src\visuals.h
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=".\keysdef-inc.h"
# End Source File
# Begin Source File

SOURCE=.\mikmod.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Lib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\lib\error.c
# End Source File
# Begin Source File

SOURCE=..\lib\fstrcmp.c
# End Source File
# Begin Source File

SOURCE=..\src\..\lib\getopt.c
# End Source File
# Begin Source File

SOURCE=..\src\..\lib\getopt.h
# End Source File
# Begin Source File

SOURCE=..\src\..\lib\getopt1.c
# End Source File
# Begin Source File

SOURCE=..\lib\getshline.c
# End Source File
# Begin Source File

SOURCE=..\lib\hash.c
# End Source File
# Begin Source File

SOURCE=..\src\..\lib\strcasecmp.c
# End Source File
# Begin Source File

SOURCE=..\src\..\lib\w_dirent.c
# End Source File
# Begin Source File

SOURCE=..\src\..\lib\w_dirent.h
# End Source File
# Begin Source File

SOURCE=..\lib\xmalloc.c
# End Source File
# Begin Source File

SOURCE=..\lib\xstrdup.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\sdl\lib\SDL.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\sdl\lib\SDLmain.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\sdl\lib\SDL_mixer.lib
# End Source File
# End Group
# End Target
# End Project
