#include "Arduino.h"
#include "acorns.h"
#include <M5Stack.h>
#include <M5ez.h>
#include "FS.h"
#include "SPIFFS.h"

#include <dirent.h>
#define m5color(red, green, blue) ((red) << 11 | (green) << 5 | (blue))

//Like strcat, but with realloc
void strcatra(char *dest, char *src)
{
    realloc(dest, strlen(dest) + strlen(src) + 1);
    strcat(dest, src);
}

void doNothing()
{
}

static void refreshTheme()
{
        ez.theme->header_bgcolor = strtol(Acorns.getConfig("theme.header_bgcolor","0x7A60").c_str(),0,16);
        ez.theme->header_fgcolor = strtol(Acorns.getConfig("theme.header_fgcolor","0x96CE").c_str(),0,16);
        ez.theme->background =  strtol(Acorns.getConfig("theme.background","0xD634").c_str(),0,16);
        ez.theme->foreground =  strtol(Acorns.getConfig("theme.foreground","0x38A1").c_str(),0,16);

        ez.theme->button_bgcolor_b = strtol(Acorns.getConfig("theme.button_bgcolor_b","0x1B20").c_str(),0,16);
        ez.theme->button_bgcolor_t = strtol(Acorns.getConfig("theme.button_bgcolor_t","0x4A27").c_str(),0,16);
        ez.theme->menu_sel_bgcolor = strtol(Acorns.getConfig("theme.menu_sel_bgcolor","0x7A60").c_str(),0,16);
        ez.theme->menu_sel_fgcolor = strtol(Acorns.getConfig("theme.menu_sel_fgcolor","0xAE90").c_str(),0,16);
        ez.theme->menu_item_color = strtol(Acorns.getConfig("theme.menu_item_color","0x38A1").c_str(),0,16);


}



static String printOut = "";

static void printfunc(loadedProgram *p, const char *s)
{

    if (printOut.length() > 2048)
    {
        printOut = printOut.substring(strlen(s));
    }
    printOut += s;
}
static void errorfunc(loadedProgram *p, const char *s)
{
    ez.textBox(String("Errror in ") + p->programID, s);
}
void m5sq_clearButtonsEvents();

static int exists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}
static void uiOneLoop()
{
    ezMenu *myMenu = 0;

folder:
    myMenu = new ezMenu("Select Folder");

    myMenu->addItem("/spiffs/sqapps", doNothing);
    myMenu->addItem("/sd/sqapps", doNothing);
    myMenu->runOnce();
    const char *dir = myMenu->pickName().c_str();

    DIR *d = opendir(dir);
    if (d == 0)
    {
        ez.msgBox("Error", "Nonexistant directory.");
        delete myMenu;
        goto folder;
    }

    char buffer[256];
    struct dirent *de = readdir(d);
    char *fnpart;

    strcpy(buffer, dir);
    if (buffer[strlen(dir) - 1] == '/')
    {
        fnpart = buffer + strlen(dir);
    }
    else
    {
        buffer[strlen(dir)] = '/';
        buffer[strlen(dir) + 1] = 0;
        fnpart = buffer + strlen(dir) + 1;
    }
    String title = myMenu->pickName();
    delete myMenu;
    myMenu = new ezMenu(title);

    myMenu->addItem("..", doNothing);

    //Iterate over every directory, add the dir name as the menu entry, and the a.jpg if possible as
    //The app icon.

    std::vector<String> jpgs;
    std::vector<String> names;

    while (de)
    {
        String s = String(buffer);
        String n = String(de->d_name);

        s += de->d_name;
        if (s.endsWith("/"))
        {
            s += "a.jpg";
        }
        else
        {
            s += "/a.jpg";
        }
        jpgs.push_back(s);
        names.push_back(n);
        if (String(s).startsWith("/sd/") && exists(s.c_str()))
        {
            myMenu->addItem(SD, s.c_str() + 3, n);
        }
        else if (String(s).startsWith("/spiffs/") && exists(s.c_str()))
        {
            myMenu->addItem(SPIFFS, s.c_str() + 7, n);
        }
        else
        {
            myMenu->addItem(n);
        }

        //myMenu->addItem(n);
        de = readdir(d);
    }

    myMenu->txtFont(&FreeSerif18pt7b);
    myMenu->imgBackground(strtol(Acorns.getConfig("theme.background","0xD634").c_str(),0,16));
    myMenu->imgFromTop(24);
    myMenu->imgCaptionFont(&FreeSerifBoldItalic18pt7b);

    myMenu->imgCaptionLocation(BC_DATUM);
    myMenu->imgCaptionColor(m5color(0, 0, 0));
    myMenu->imgCaptionMargins(5);

    myMenu->runOnce();
    closedir(d);
    jpgs.clear();
    names.clear();

    if (strcmp(myMenu->pickName().c_str(), "..") == 0)
    {
        delete myMenu;
        goto folder;
    }
    String s = String(buffer) + myMenu->pickName();

    if (s.endsWith("/"))
    {
        s += "a.nut";
    }
    else
    {
        s += "/a.nut";
    }
    GIL_LOCK;
    const char *fn = s.c_str();
    FILE *f = fopen(fn, "r");
    if (f)
    {
        fseek(f, 0L, SEEK_END);
        int sz = ftell(f);
        rewind(f);

        char *buf = (char *)malloc(sz + 2);
        int p = 0;

        int chr = fgetc(f);
        while (chr != EOF)
        {
            buf[p] = chr;
            chr = fgetc(f);
            p += 1;
        }
        buf[p] = 0;
        fclose(f);

        GIL_UNLOCK;
        //Do we want to do anything to indicate an app is running?
        // ez.textBox(myMenu->pickName(), buf);
        // M5.Lcd.fillScreen(TFT_WHITE);
        m5sq_clearButtonsEvents();
        Acorns.runProgram(buf, myMenu->pickName().c_str(), errorfunc, printfunc);
        free(buf);
        Acorns.closeProgram(myMenu->pickName().c_str());
        if(printOut.length())
        {
            ez.textBox("Program Output", printOut);
        }
        printOut = "";
        GIL_LOCK;
    }
    else
    {
        GIL_UNLOCK;
        ez.msgBox("File not found", fn);
        GIL_LOCK;
    }
    GIL_UNLOCK;
    delete myMenu;
}


void acorns_UILoop(void *p)
{
    ez.setFont(&FreeSerif18pt7b);
    refreshTheme();

    ez.textBox("Welcome!", String(Acorns.getQuote()) + "\n\nPowered by Acorns and Squirrel\nMemory:" + String(ESP.getFreeHeap()));
    while (1)
    {
        //Every time we run a program is a convenient time to refresh this, for dynamically trying
        //Stuff
        refreshTheme();
        uiOneLoop();
    }
}