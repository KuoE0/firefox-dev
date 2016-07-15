/* -*- Mode: C++; tab-width: 40; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim:set ts=4 sw=4 sts=4 et cin: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#define MOZ_FATAL_ASSERTIONS_FOR_THREAD_SAFETY

#include "nsScreenManagerAndroid.h"
#include "AndroidBridge.h"
#include "GeneratedJNIWrappers.h"
#include "AndroidRect.h"

#include <android/log.h>
#include <mozilla/jni/Refs.h>

#define ALOG(args...) __android_log_print(ANDROID_LOG_INFO, "nsScreenManagerAndroid", ## args)

using namespace mozilla;

nsScreenAndroid::nsScreenAndroid(DisplayType aType, nsIntRect aRect)
    : mId(nsScreenAndroid::GetIdFromType(aType))
    , mRect(aRect)
    , mDisplayType(aType)
{
}

nsScreenAndroid::~nsScreenAndroid()
{
}

NS_IMETHODIMP
nsScreenAndroid::GetId(uint32_t *outId)
{
    *outId = mId;
    return NS_OK;
}

NS_IMETHODIMP
nsScreenAndroid::GetRect(int32_t *outLeft, int32_t *outTop, int32_t *outWidth, int32_t *outHeight)
{
    if (mDisplayType != DisplayType::DISPLAY_PRIMARY) {
        *outLeft   = mRect.x;
        *outTop    = mRect.y;
        *outWidth  = mRect.width;
        *outHeight = mRect.height;

        return NS_OK;
    }

    if (!mozilla::jni::IsAvailable()) {
      // xpcshell most likely
      *outLeft = *outTop = *outWidth = *outHeight = 0;
      return NS_ERROR_FAILURE;
    }

    widget::sdk::Rect::LocalRef rect = widget::GeckoAppShell::GetScreenSize();
    rect->Left(outLeft);
    rect->Top(outTop);
    rect->Width(outWidth);
    rect->Height(outHeight);

    return NS_OK;
}


NS_IMETHODIMP
nsScreenAndroid::GetAvailRect(int32_t *outLeft, int32_t *outTop, int32_t *outWidth, int32_t *outHeight)
{
    return GetRect(outLeft, outTop, outWidth, outHeight);
}



NS_IMETHODIMP
nsScreenAndroid::GetPixelDepth(int32_t *aPixelDepth)
{
    if (!mozilla::jni::IsAvailable()) {
      // xpcshell most likely
      *aPixelDepth = 16;
      return NS_ERROR_FAILURE;
    }

    *aPixelDepth = widget::GeckoAppShell::GetScreenDepthWrapper();
    return NS_OK;
}


NS_IMETHODIMP
nsScreenAndroid::GetColorDepth(int32_t *aColorDepth)
{
    return GetPixelDepth(aColorDepth);
}


void
nsScreenAndroid::ApplyMinimumBrightness(uint32_t aBrightness)
{
    if (mDisplayType != DisplayType::DISPLAY_PRIMARY) {
        return;
    }

    if (mozilla::jni::IsAvailable()) {
      widget::GeckoAppShell::SetKeepScreenOn(aBrightness == BRIGHTNESS_FULL);
    }
}

NS_IMPL_ISUPPORTS(nsScreenManagerAndroid, nsIScreenManager)

nsScreenManagerAndroid::nsScreenManagerAndroid()
{
}

nsScreenManagerAndroid::~nsScreenManagerAndroid()
{
}

NS_IMETHODIMP
nsScreenManagerAndroid::GetPrimaryScreen(nsIScreen **outScreen)
{
    ScreenForId(nsScreenAndroid::GetIdFromType(DisplayType::DISPLAY_PRIMARY), outScreen);
    return NS_OK;
}

NS_IMETHODIMP
nsScreenManagerAndroid::ScreenForId(uint32_t aId,
                                    nsIScreen **outScreen)
{
    for (size_t i = 0; i < mScreens.Length(); ++i) {
        if (aId == mScreens[i]->GetId()) {
            NS_IF_ADDREF(*outScreen = mScreens[i].get());
            return NS_OK;
        }
    }

    *outScreen = nullptr;
    return NS_OK;
}

NS_IMETHODIMP
nsScreenManagerAndroid::ScreenForRect(int32_t inLeft,
                                      int32_t inTop,
                                      int32_t inWidth,
                                      int32_t inHeight,
                                      nsIScreen **outScreen)
{
    // Not support to query non-primary screen with rect.
    return GetPrimaryScreen(outScreen);
}

NS_IMETHODIMP
nsScreenManagerAndroid::ScreenForNativeWidget(void *aWidget, nsIScreen **outScreen)
{
    // Not support to query non-primary screen with native widget.
    return GetPrimaryScreen(outScreen);
}

NS_IMETHODIMP
nsScreenManagerAndroid::GetNumberOfScreens(uint32_t *aNumberOfScreens)
{
    *aNumberOfScreens = mScreens.Length();
    return NS_OK;
}

NS_IMETHODIMP
nsScreenManagerAndroid::GetSystemDefaultScale(float *aDefaultScale)
{
    *aDefaultScale = 1.0f;
    return NS_OK;
}

void
nsScreenManagerAndroid::AddScreen(DisplayType aType, nsIntRect aRect)
{
    ALOG("nsScreenManagerAndroid: add %s screen",
        (aType == DisplayType::DISPLAY_PRIMARY ? "PRIMARY" :
        (aType == DisplayType::DISPLAY_EXTERNAL ? "EXTERNAL" : "VIRTUAL")));

    RefPtr<nsScreenAndroid> screen = new nsScreenAndroid(aType, aRect);
    mScreens.AppendElement(screen);
}

void
nsScreenManagerAndroid::RemoveScreen(DisplayType aType)
{
    ALOG("nsScreenManagerAndroid: remove %s screen",
        (aType == DisplayType::DISPLAY_PRIMARY ? "PRIMARY" :
        (aType == DisplayType::DISPLAY_EXTERNAL ? "EXTERNAL" : "VIRTUAL")));

    for (size_t i = 0; i < mScreens.Length(); i++) {
        if (aType == mScreens[i]->GetDisplayType()) {
            mScreens.RemoveElementAt(i);
        }
    }
}
