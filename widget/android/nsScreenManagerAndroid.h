/* -*- Mode: C++; tab-width: 40; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef nsScreenManagerAndroid_h___
#define nsScreenManagerAndroid_h___

#include "nsCOMPtr.h"

#include "nsBaseScreen.h"
#include "nsIScreenManager.h"
#include "nsRect.h"
#include "mozilla/WidgetUtils.h"

enum class DisplayType : uint32_t
{
    DISPLAY_PRIMARY  = 0,
    DISPLAY_EXTERNAL = 1,
    DISPLAY_VIRTUAL  = 2
};

class nsScreenAndroid final : public nsBaseScreen
{
public:
    nsScreenAndroid(DisplayType aType, nsIntRect aRect);
    ~nsScreenAndroid();

    NS_IMETHOD GetId(uint32_t* aId) override;
    NS_IMETHOD GetRect(int32_t* aLeft, int32_t* aTop, int32_t* aWidth, int32_t* aHeight) override;
    NS_IMETHOD GetAvailRect(int32_t* aLeft, int32_t* aTop, int32_t* aWidth, int32_t* aHeight) override;
    NS_IMETHOD GetPixelDepth(int32_t* aPixelDepth) override;
    NS_IMETHOD GetColorDepth(int32_t* aColorDepth) override;

    static uint32_t GetIdFromType(DisplayType aType) { return (uint32_t) aType; }

    uint32_t GetId() const { return mId; };
    DisplayType GetDisplayType() const { return mDisplayType; }

protected:
    virtual void ApplyMinimumBrightness(uint32_t aBrightness) override;

private:
    uint32_t mId;
    nsIntRect mRect;
    DisplayType mDisplayType;
};

class nsScreenManagerAndroid final : public nsIScreenManager
{
private:
    ~nsScreenManagerAndroid();

public:
    nsScreenManagerAndroid();

    NS_DECL_ISUPPORTS
    NS_DECL_NSISCREENMANAGER

    void AddScreen(DisplayType aType, nsIntRect aRect);
    void RemoveScreen(DisplayType aType);

protected:
    nsTArray<RefPtr<nsScreenAndroid>> mScreens;
};

#endif /* nsScreenManagerAndroid_h___ */
