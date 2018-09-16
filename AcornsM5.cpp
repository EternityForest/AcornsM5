#include "Arduino.h"
#include "acorns.h"
#include <M5Stack.h>
#include <M5ez.h>

#include <dirent.h>

//Like strcat, but with realloc
void strcatra(char *dest, char *src)
{
    realloc(dest, strlen(dest) + strlen(src) + 1);
    strcat(dest, src);
}

void doNothing()
{
}

static String printOut = ""; 

static void printfunc(loadedProgram * p, const char * s)
{

    if(printOut.length()> 2048)
    {
        printOut = printOut.substring(strlen(s));
    }
    printOut+=s;
}
static void errorfunc(loadedProgram * p, const char * s)
{
    ez.textBox(String("Errror in ")+ p->programID, s);
}

static void uiOneLoop()
{
    ezMenu * myMenu = 0;

 folder:
        myMenu = new ezMenu("Select Folder");

        myMenu->addItem("/spiffs/sqapps", doNothing);
        myMenu->addItem("/sd/sqapps", doNothing);
        myMenu->runOnce();
        const char * dir = myMenu->pickName().c_str();

        DIR *d = opendir(dir);
        if(d==0)
        {
            ez.msgBox("Error","Nonexistant directory.");
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
            buffer[strlen(dir)+1] = 0;
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

            s+=de->d_name;
            if(s.endsWith("/"))
            {
                s+="a.jpg";
            }
            else
            {
                s+="/a.jpg";
            }
            Serial.print(s);
            Serial.print(n);
            jpgs.push_back(s);
            names.push_back(n);
            //myMenu->addItem(SD,s.c_str(),n);
            myMenu->addItem(n);
            de = readdir(d);
        }

        myMenu->runOnce();
        closedir(d);
        jpgs.clear();
        names.clear();
   
        
      
        if (strcmp(myMenu->pickName().c_str(), "..")==0)
        {
            delete myMenu;
            goto folder;
        }
        String s = String(buffer)+myMenu->pickName();
        
            if(s.endsWith("/"))
            {
                s+="a.nut";
            }
            else
            {
                s+="/a.nut";
            }
        GIL_LOCK;
        const char * fn = s.c_str();
        FILE *f = fopen(fn, "r");
        if (f)
        {
            fseek(f, 0L, SEEK_END);
            int sz = ftell(f);
            rewind(f);

            char *buf = (char *)malloc(sz+2);
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
            ez.textBox(myMenu->pickName(),buf);
            M5.Lcd.fillScreen(TFT_WHITE);
            Acorns.runProgram(buf, myMenu->pickName().c_str(), errorfunc, printfunc);
            free(buf);
            Acorns.closeProgram(myMenu->pickName().c_str());
            ez.textBox("Program Output", printOut);
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
    ez.textBox("Welcome!", String(Acorns.getQuote())+ "\n\nPowered by Acorns and Squirrel\nMemory:" + String(ESP.getFreeHeap()));
    while (1)
    {
        uiOneLoop();
    }
}