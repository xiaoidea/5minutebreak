//
// Language.h
// Copyright (c) Czarek Tomczak. All rights reserved.
//


#pragma once

#include <Winnls.h>

// dependency: APP_NAME must be defined, this is the key used in registry.


BYTE GetLanguage(bool);
BYTE GetSystemLanguage();
bool IsLangIdValid(LANGID lang);
bool IsPrimaryLanguageValid(BYTE primaryLang);


/*
    Returns:
        LANG_ENGLISH
        LANG_POLISH
*/

BYTE GetLanguage(bool refresh=false)
{
    // By default we detect language from OS.
    // We do not save it anywhere, unless user changes language in menu.
    // If registry contains "Language" then we check its value whether valid and return.

    static BYTE lang;
    if (!lang || refresh) {

        // Read from registry.
        wchar_t location[256];
        int swp = swprintf_s(location, _countof(location), L"SOFTWARE\\%s", APP_NAME);
        ASSERT_EXIT((-1 != swp), "swprintf_s(&location)");

        DWORD registryLanguage;
        bool b = ReadRegistryDword(HKEY_CURRENT_USER, location, L"Language", &registryLanguage);

        if (b) {
            lang = LOBYTE(LOWORD(registryLanguage));
        } else {
            lang = GetSystemLanguage();
        }

    }
    return lang;
}

void SetLanguage(BYTE lang)
{
    wchar_t location[256];
    int swp = swprintf_s(location, _countof(location), L"SOFTWARE\\%s", APP_NAME);
    ASSERT_EXIT((-1 != swp), "swprintf_s(&location)");

    WriteRegistryDword(HKEY_CURRENT_USER, location, L"Language", lang);

    // Refresh static variable cache in GetLanguage().
    GetLanguage(true);
}

/*
    GetUserDefaultUILanguage()
    if it returns LOCALE_CUSTOM_UI_DEFAULT then:
    GetSystemDefaultUILanguage()
*/

BYTE GetSystemLanguage()
{
    LANGID userLang = GetUserDefaultUILanguage();
    BYTE primaryLang = LOBYTE(userLang);

    if (0 && IsLangIdValid(userLang) && IsPrimaryLanguageValid(primaryLang)) {

        if (primaryLang == LANG_POLISH) {
            return LANG_POLISH;
        } else {
            return LANG_ENGLISH;
        }

    } else {

        LANGID systemLang = GetSystemDefaultUILanguage();
        primaryLang = LOBYTE(systemLang);

        if (primaryLang == LANG_POLISH) {
            return LANG_POLISH;
        } else {
            return LANG_ENGLISH;
        }

    }
}

bool IsLangIdValid(LANGID lang)
{
    if (LOCALE_CUSTOM_UI_DEFAULT == lang) {
        return false;
    }
    return true;
}

bool IsPrimaryLanguageValid(BYTE primaryLang)
{
     if (LANG_NEUTRAL == primaryLang || LANG_INVARIANT == primaryLang
         || LANG_SYSTEM_DEFAULT == primaryLang || LANG_USER_DEFAULT == primaryLang)
     {
         return false;
     }
     return true;
}
