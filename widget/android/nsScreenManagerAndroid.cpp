/* -*- Mode: C++; tab-width: 40; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim:set ts=4 sw=4 sts=4 et cin: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#define MOZ_FATAL_ASSERTIONS_FOR_THREAD_SAFETY

#include "nsScreenManagerAndroid.h"
#include "AndroidBridge.h"
#include "AndroidRect.h"
#include "GeneratedJNINatives.h"
#include "GeneratedJNIWrappers.h"
#include "nsAppShell.h"

#include <android/log.h>
#include <mozilla/jni/Refs.h>

#define ALOG(args...) __android_log_print(ANDROID_LOG_INFO, "nsScreenManagerAndroid", ## args)

using namespace mozilla;
using namespace mozilla::java;

nsScreenAndroid::nsScreenAndroid(int aDisplayType, nsIntRect aRect)
    : mId(nsScreenAndroid::GetIdFromType(aDisplayType))
    , mRect(aRect)
    , mDisplayType(aDisplayType)
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
    if (mDisplayType != java::GeckoView::DISPLAY_PRIMARY()) {
        *outLeft   = mRect.x;
        *outTop    = mRect.y;
        *outWidth  = mRect.width;
        *outHeight = mRect.height;

        printf_stderr("<kuoe0> nsScreenAndroid::%s width=%d height=%d", __func__, mRect.width, mRect.height);
        return NS_OK;
    }

    if (!mozilla::jni::IsAvailable()) {
      // xpcshell most likely
      *outLeft = *outTop = *outWidth = *outHeight = 0;
      return NS_ERROR_FAILURE;
    }

    java::sdk::Rect::LocalRef rect = java::GeckoAppShell::GetScreenSize();
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

    *aPixelDepth = java::GeckoAppShell::GetScreenDepthWrapper();
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
    if (mDisplayType != java::GeckoView::DISPLAY_PRIMARY()) {
        return;
    }

    if (mozilla::jni::IsAvailable()) {
      java::GeckoAppShell::SetKeepScreenOn(aBrightness == BRIGHTNESS_FULL);
    }
}

class nsScreenManagerAndroid::ScreenManagerHelperSupport final
    : public ScreenManagerHelper::Natives<ScreenManagerHelperSupport>
    , public UsesGeckoThreadProxy // Call AddDisplay / RemoveDisplay in Gecko thread
{
public:
    typedef ScreenManagerHelper::Natives<ScreenManagerHelperSupport> Base;

    static void AddDisplay(int32_t aDisplayType, int32_t aWidth,
                           int32_t aHeight, float aDensity) {
        printf_stderr("<kuoe0> %s: width=%d height=%d density=%f", __func__, aWidth, aHeight, aDensity);
        nsCOMPtr<nsIScreenManager> screenMgr =
            do_GetService("@mozilla.org/gfx/screenmanager;1");
        MOZ_ASSERT(screenMgr, "Failed to get nsIScreenManager");

        RefPtr<nsScreenManagerAndroid> screenMgrAndroid =
            (nsScreenManagerAndroid*) screenMgr.get();
        RefPtr<nsScreenAndroid> screen =
            screenMgrAndroid->AddScreen(aDisplayType,
                                        nsIntRect(0, 0, aWidth, aHeight));
        MOZ_ASSERT(screen);
        screen->SetDensity(aDensity);
    }

    static void RemoveDisplay(int32_t aDisplayType) {
        printf_stderr("<kuoe0> %s", __func__);
        nsCOMPtr<nsIScreenManager> screenMgr =
            do_GetService("@mozilla.org/gfx/screenmanager;1");
        MOZ_ASSERT(screenMgr, "Failed to get nsIScreenManager");
        RefPtr<nsScreenManagerAndroid> screenMgrAndroid =
            (nsScreenManagerAndroid*) screenMgr.get();
        screenMgrAndroid->RemoveScreen(aDisplayType);
    }
};

NS_IMPL_ISUPPORTS(nsScreenManagerAndroid, nsIScreenManager)

nsScreenManagerAndroid::nsScreenManagerAndroid()
{
    ScreenManagerHelperSupport::Base::Init();
    nsCOMPtr<nsIScreen> screen = AddScreen(java::GeckoView::DISPLAY_PRIMARY());
    MOZ_ASSERT(screen);
}

nsScreenManagerAndroid::~nsScreenManagerAndroid()
{
}

NS_IMETHODIMP
nsScreenManagerAndroid::GetPrimaryScreen(nsIScreen **outScreen)
{
    ScreenForId(nsScreenAndroid::GetIdFromType(java::GeckoView::DISPLAY_PRIMARY()), outScreen);
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
    printf_stderr("<kuoe0> at nsScreenManagerAndroid::%s", __func__);
    *aDefaultScale = 1.0f;
    return NS_OK;
}

already_AddRefed<nsScreenAndroid>
nsScreenManagerAndroid::AddScreen(int aDisplayType, nsIntRect aRect)
{
    ALOG("nsScreenManagerAndroid: add %s screen",
        (aDisplayType == java::GeckoView::DISPLAY_PRIMARY()  ? "PRIMARY"  :
        (aDisplayType == java::GeckoView::DISPLAY_EXTERNAL() ? "EXTERNAL" :
                                                               "VIRTUAL")));

    nsCOMPtr<nsIScreen> screen;
    ScreenForId(nsScreenAndroid::GetIdFromType(aDisplayType),
                getter_AddRefs(screen));
    // There is only one nsScreen for each display type.
    MOZ_ASSERT(!screen, "nsScreenAndroid with this type already exists.");

    RefPtr<nsScreenAndroid> newScreen = new nsScreenAndroid(aDisplayType, aRect);
    mScreens.AppendElement(newScreen);
    return newScreen.forget();
}

void
nsScreenManagerAndroid::RemoveScreen(int aDisplayType)
{
    ALOG("nsScreenManagerAndroid: remove %s screen",
        (aDisplayType == java::GeckoView::DISPLAY_PRIMARY()  ? "PRIMARY"  :
        (aDisplayType == java::GeckoView::DISPLAY_EXTERNAL() ? "EXTERNAL" :
                                                               "VIRTUAL")));

    for (size_t i = 0; i < mScreens.Length(); i++) {
        if (aDisplayType == mScreens[i]->GetDisplayType()) {
            mScreens.RemoveElementAt(i);
        }
    }
}
