#include "Arduino.h"
#include "acorns.h"
#include <M5Stack.h>
#include <M5ez.h>

static uint16_t fg_color = 0;
static uint16_t bg_color = 0xffff;

const char *currentApp;
#define m5color(red, green, blue) ((red) << 11 | (green) << 5 | (blue))

static SQInteger sqsetfg(HSQUIRRELVM v)
{
    SQInteger i = sq_gettop(v);
    SQInteger r = 0;
    SQInteger g = 0;
    SQInteger b = 0;

    if (sq_getinteger(v, 2, &r) == SQ_ERROR)
    {
        sq_throwerror(v, "Integer is required");
        return SQ_ERROR;
    }

    if (sq_getinteger(v, 3, &g) == SQ_ERROR)
    {
        sq_throwerror(v, "Integer is required");
        return SQ_ERROR;
    }
    if (sq_getinteger(v, 4, &b) == SQ_ERROR)
    {
        sq_throwerror(v, "Integer is required");
        return SQ_ERROR;
    }
    fg_color = m5color(r, g, b);
}

static SQInteger sqsetbg(HSQUIRRELVM v)
{
    SQInteger i = sq_gettop(v);
    SQInteger r = 0;
    SQInteger g = 0;
    SQInteger b = 0;

    if (sq_getinteger(v, 2, &r) == SQ_ERROR)
    {
        sq_throwerror(v, "Integer is required");
        return SQ_ERROR;
    }

    if (sq_getinteger(v, 3, &g) == SQ_ERROR)
    {
        sq_throwerror(v, "Integer is required");
        return SQ_ERROR;
    }

    if (sq_getinteger(v, 4, &b) == SQ_ERROR)
    {
        sq_throwerror(v, "Integer is required");
        return SQ_ERROR;
    }
    bg_color = m5color(r, g, b);
}

static SQInteger sqfill(HSQUIRRELVM v)
{
    SQInteger i = sq_gettop(v);
    SQInteger r = 0;
    SQInteger g = 0;
    SQInteger b = 0;

    if (sq_getinteger(v, 2, &r) == SQ_ERROR)
    {
        sq_throwerror(v, "Integer is required");
        return SQ_ERROR;
    }

    if (sq_getinteger(v, 3, &g) == SQ_ERROR)
    {
        sq_throwerror(v, "Integer is required");
        return SQ_ERROR;
    }

    if (sq_getinteger(v, 4, &b) == SQ_ERROR)
    {
        sq_throwerror(v, "Integer is required");
        return SQ_ERROR;
    }
    M5.Lcd.fillScreen(m5color(r, g, b));
}

static void doNothing()
{
}
static SQInteger sqMenu(HSQUIRRELVM v)
{
    SQInteger s = 1;
    const char *p;
    const char *n;

    sq_pushnull(v);
    sq_getstring(v, 2, &n);
    ezMenu myMenu(n);

    while (sq_next(v, -2) != SQ_ERROR)
    {
        sq_getstring(v, -1, &p);
        myMenu.addItem(p, doNothing);
        sq_pop(v, 2);
    }
    sq_pop(v, 1);
    GIL_UNLOCK;
    uint16_t sel = myMenu.runOnce();
    GIL_LOCK;
    sq_pushinteger(v, sel);
    return (1);
}

static SQInteger sqtextsize(HSQUIRRELVM v)
{
    SQInteger s = 1;

    if (sq_getinteger(v, 3, &s) == SQ_ERROR)
    {
        sq_throwerror(v, "Integer is required");
        return SQ_ERROR;
    }
    M5.Lcd.setTextSize(s);
}

static SQInteger sqdrawstring(HSQUIRRELVM v)
{
    SQInteger i = sq_gettop(v);
    SQInteger x = 0;
    SQInteger y = 0;
    const char *t = 0;

    if (sq_getinteger(v, 3, &x) == SQ_ERROR)
    {
        sq_throwerror(v, "Integer is required");
        return SQ_ERROR;
    }

    if (sq_getinteger(v, 4, &y) == SQ_ERROR)
    {
        sq_throwerror(v, "Integer is required");
        return SQ_ERROR;
    }
    if (sq_getstring(v, 2, &t) == SQ_ERROR)
    {
        sq_throwerror(v, "String is required");
        return SQ_ERROR;
    }
    M5.Lcd.setTextColor(fg_color, bg_color);
    M5.Lcd.setCursor(x, y);
    M5.Lcd.print(t);
}


static SQInteger sqtextbox(HSQUIRRELVM v)
{
    SQInteger i = sq_gettop(v);
    const char *s = 0;
    const char *t = "";


    if (sq_getstring(v, 3, &t) == SQ_ERROR)
    {
        sq_throwerror(v, "String is required");
        return SQ_ERROR;
    }
    sq_getstring(v,2,&s);

   ez.textBox(t, s);
}

//Returns a button event string of the form xy, where x is a b or c indicating button,
//and y is p, r or l for press, release, or longpress.

//Waits up to the first arg in ms for an event, if present.
static SQInteger sqButtonEvent(HSQUIRRELVM v)
{
    SQInteger i = sq_gettop(v);
    SQInteger x = 0;
    const char *ret;

    static char ap = 0;
    static char cp = 0;
    static char bp = 0;

    static char ar = 1;
    static char cr = 1;
    static char br = 1;

    static char al = 0;
    static char cl = 0;
    static char bl = 0;

    if (sq_getinteger(v, 2, &x) == SQ_ERROR)
    {
        //do nothing, non block
    }

    uint64_t start = millis();

    char first = 1;
    ret = "";
    while (((millis() - start) < x) | first)
    {
        first = 0;
        if (M5.BtnA.pressedFor(15))
        {
            ar = 0;
            if (ap == 0)
            {
                ret = "ap";
                ap = 1;
                break;
            }
        }
        if (M5.BtnB.pressedFor(15))
        {
            cr = 0;

            if (bp == 0)
            {
                bp = 1;
                ret = "bp";
                break;
            }
        }

        if (M5.BtnC.pressedFor(15))
        {
            cr = 0;
            if (cp == 0)
            {
                cp = 1;
                ret = "cp";
                break;
            }
        }

        if (M5.BtnA.releasedFor(10))
        {
            al = 0;
            ap = 0;
            if (ar == 0)
            {
                ar = 1;
                ret = "ar";
                break;
            }
        }
        if (M5.BtnB.releasedFor(10))
        {
            bl = 0;
            bp = 0;
            if (br == 0)
            {
                br = 1;
                ret = "br";
                break;
            }
        }

        if (M5.BtnC.releasedFor(10))
        {
            cl = 0;
            cp = 0;
            if (cr == 0)
            {
                cr = 1;
                ret = "cr";
                break;
            }
        }



        if (M5.BtnA.pressedFor(500))
        {
            if (al == 0)
            {
                al = 1;
                ret = "al";
                break;
            }
        }
        if (M5.BtnB.pressedFor(500))
        {
            if (bl == 0)
            {
                bl = 1;
                ret = "bl";
                break;
            }
        }

        if (M5.BtnC.pressedFor(500))
        {
            if (cl == 0)
            {
                cl = 1;
                ret = "cl";
                break;
            }
        }

        if(x)
        {
            GIL_UNLOCK;
            delay(24);
            GIL_LOCK;
        }
    }
    if (strlen(ret) > 1)
    {
        sq_pushstring(v, ret, -1);
    }
    else
    {
        sq_pushnull(v);
    }
    return (1);
}

static SQInteger sqdrawimage(HSQUIRRELVM v)
{
    SQInteger i = sq_gettop(v);
    SQInteger x = 0;
    SQInteger y = 0;
    const char *t = 0;

    if (sq_getinteger(v, 3, &x) == SQ_ERROR)
    {
        sq_throwerror(v, "Integer is required");
        return SQ_ERROR;
    }

    if (sq_getinteger(v, 4, &y) == SQ_ERROR)
    {
        sq_throwerror(v, "Integer is required");
        return SQ_ERROR;
    }
    if (sq_getstring(v, 2, &t) == SQ_ERROR)
    {
        sq_throwerror(v, "String is required");
        return SQ_ERROR;
    }
    M5.Lcd.drawJpgFile(SD, t, x, y);
}

void sqDoM5Bindings()
{
    Acorns.registerFunction(0, sqfill, "fillScreen");
    Acorns.registerFunction(0, sqdrawimage, "drawImage");
    Acorns.registerFunction(0, sqButtonEvent, "buttonEvent");
    Acorns.registerFunction(0, sqsetbg, "backgroundColor");
    Acorns.registerFunction(0, sqsetfg, "foregroundColor");
    Acorns.registerFunction(0, sqtextsize, "textSize");
    Acorns.registerFunction(0, sqdrawstring, "drawString");
    Acorns.registerFunction(0, sqtextbox, "textBox");
    Acorns.registerFunction(0, sqMenu, "menu");
}