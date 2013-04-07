/********************************************************************************
 Copyright (C) 2012 Hugh Bailey <obs.jim@gmail.com>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
********************************************************************************/


#include "OBSAPI.h"


APIInterface *API = NULL;


void LocalizeWindow(HWND hwnd, LocaleStringLookup *lookup)
{
    if(!lookup) lookup = locale;

    int textLen = (int)SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0);
    String strText;
    strText.SetLength(textLen);
    GetWindowText(hwnd, strText, textLen+1);

    if(strText.IsValid() && lookup->HasLookup(strText))
        SetWindowText(hwnd, lookup->LookupString(strText));

    //-------------------------------

    HWND hwndChild = GetWindow(hwnd, GW_CHILD);
    while(hwndChild)
    {
        int textLen = (int)SendMessage(hwndChild, WM_GETTEXTLENGTH, 0, 0);
        strText.SetLength(textLen);
        GetWindowText(hwndChild, strText, textLen+1);

        if(strText.IsValid())
        {
            if(strText[0] == '.')
                SetWindowText(hwndChild, strText.Array()+1);
            else
            {
                if(strText.IsValid() && lookup->HasLookup(strText))
                    SetWindowText(hwndChild, lookup->LookupString(strText));
            }
        }

        hwndChild = GetNextWindow(hwndChild, GW_HWNDNEXT);
    }
};

void LocalizeMenu(HMENU hMenu, LocaleStringLookup *lookup)
{
    if(!lookup) lookup = locale;

    int itemCount = GetMenuItemCount(hMenu);
    if(itemCount == -1)
        return;

    for(int i=0; i<itemCount; i++)
    {
        MENUITEMINFO mii;
        zero(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_SUBMENU|MIIM_STRING|MIIM_FTYPE;
        GetMenuItemInfo(hMenu, i, TRUE, &mii);

        if(mii.fType & MFT_SEPARATOR || mii.cch < 2)
            continue;

        HMENU hSubMenu = mii.hSubMenu;

        String strLookup;
        strLookup.SetLength(mii.cch);

        mii.fMask = MIIM_STRING;
        mii.dwTypeData = strLookup.Array();
        mii.cch = strLookup.Length()+1;
        GetMenuItemInfo(hMenu, i, TRUE, &mii);

        String strName;
        if(strLookup[0] == '.')
            strName = strLookup.Array()+1;
        else if(!lookup->HasLookup(strLookup))
            strName = strLookup;
        else
            strName = lookup->LookupString(strLookup);

        mii.fMask = MIIM_STRING;
        mii.dwTypeData = strName.Array();
        SetMenuItemInfo(hMenu, i, TRUE, &mii);

        if(hSubMenu)
            LocalizeMenu(hSubMenu);
    }
}

String GetLBText(HWND hwndList, UINT id)
{
    UINT curSel = (id != LB_ERR) ? id : (UINT)SendMessage(hwndList, LB_GETCURSEL, 0, 0);
    if(curSel == LB_ERR)
        return String();

    String strText;
    strText.SetLength((UINT)SendMessage(hwndList, LB_GETTEXTLEN, curSel, 0));
    if(strText.Length())
        SendMessage(hwndList, LB_GETTEXT, curSel, (LPARAM)strText.Array());

    return strText;
}

String GetLVText(HWND hwndList, UINT id)
{
    String strText;
    strText.SetLength(256);
    ListView_GetItemText(hwndList, id, 0, (LPWSTR)strText.Array(), 256);

    return strText;
}

String GetCBText(HWND hwndCombo, UINT id)
{
    UINT curSel = (id != CB_ERR) ? id : (UINT)SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
    if(curSel == CB_ERR)
        return String();

    String strText;
    strText.SetLength((UINT)SendMessage(hwndCombo, CB_GETLBTEXTLEN, curSel, 0));
    if(strText.Length())
        SendMessage(hwndCombo, CB_GETLBTEXT, curSel, (LPARAM)strText.Array());

    return strText;
}

String GetEditText(HWND hwndEdit)
{
    String strText;
    strText.SetLength((UINT)SendMessage(hwndEdit, WM_GETTEXTLENGTH, 0, 0));
    if(strText.Length())
        SendMessage(hwndEdit, WM_GETTEXT, strText.Length()+1, (LPARAM)strText.Array());

    return strText;
}

static LPBYTE GetBitmapData(HBITMAP hBmp, BITMAP &bmp)
{
    if (!hBmp)
        return NULL;

    if (GetObject(hBmp, sizeof(bmp), &bmp) != 0) {
        UINT bitmapDataSize = bmp.bmHeight*bmp.bmWidth*bmp.bmBitsPixel;
        bitmapDataSize >>= 3;

        LPBYTE lpBitmapData = (LPBYTE)Allocate(bitmapDataSize);
        GetBitmapBits(hBmp, bitmapDataSize, lpBitmapData);

        return lpBitmapData;
    }

    return NULL;
}

static inline BYTE BitToAlpha(LPBYTE lp1BitTex, int pixel, bool bInvert)
{
    BYTE pixelByte = lp1BitTex[pixel/8];
    BYTE pixelVal = pixelByte >> (7-(pixel%8)) & 1;

    if (bInvert)
        return pixelVal ? 0xFF : 0;
    else
        return pixelVal ? 0 : 0xFF;
}

LPBYTE GetCursorData(HICON hIcon, ICONINFO &ii, UINT &width, UINT &height)
{
    BITMAP bmpColor, bmpMask;
    LPBYTE lpBitmapData = NULL, lpMaskData = NULL;

    if (lpBitmapData = GetBitmapData(ii.hbmColor, bmpColor)) {
        if (bmpColor.bmBitsPixel < 32) {
            Free(lpBitmapData);
            return NULL;
        }

        if (lpMaskData = GetBitmapData(ii.hbmMask, bmpMask)) {
            int pixels = bmpColor.bmHeight*bmpColor.bmWidth;
            bool bHasAlpha = false;

            //god-awful horrible hack to detect 24bit cursor
            for (int i=0; i<pixels; i++) {
                if (lpBitmapData[i*4 + 3] != 0) {
                    bHasAlpha = true;
                    break;
                }
            }

            if (!bHasAlpha) {
                for (int i=0; i<pixels; i++) {
                    lpBitmapData[i*4 + 3] = BitToAlpha(lpMaskData, i, false);
                }
            }

            Free(lpMaskData);
        }

        width  = bmpColor.bmWidth;
        height = bmpColor.bmHeight;
    } else if (lpMaskData = GetBitmapData(ii.hbmMask, bmpMask)) {
        bmpMask.bmHeight /= 2;

        int pixels = bmpMask.bmHeight*bmpMask.bmWidth;
        lpBitmapData = (LPBYTE)Allocate(pixels*4);
        zero(lpBitmapData, pixels*4);

        for (int i=0; i<pixels; i++)
            lpBitmapData[i*4 + 3] = BitToAlpha(lpMaskData+(pixels>>3), i, true);

        Free(lpMaskData);

        width  = bmpMask.bmWidth;
        height = bmpMask.bmHeight;
    }

    return lpBitmapData;
}

extern LARGE_INTEGER clockFreq;

QWORD GetQPCTime100NS()
{
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);

    double timeVal = double(currentTime.QuadPart);
    timeVal *= 10000000.0;
    timeVal /= double(clockFreq.QuadPart);

    return QWORD(timeVal);
}

QWORD GetQPCTimeMS()
{
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);

    QWORD timeVal = currentTime.QuadPart;
    timeVal *= 1000;
    timeVal /= clockFreq.QuadPart;

    return timeVal;
}
