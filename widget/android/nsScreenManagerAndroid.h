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

class nsScreenAndroid final : public nsBaseScreen
{
public:
    nsScreenAndroid(int32_t aDisplayType, nsIntRect aRect);
    ~nsScreenAndroid();

    NS_IMETHOD GetId(uint32_t* aId) override;
    NS_IMETHOD GetRect(int32_t* aLeft, int32_t* aTop, int32_t* aWidth, int32_t* aHeight) override;
    NS_IMETHOD GetAvailRect(int32_t* aLeft, int32_t* aTop, int32_t* aWidth, int32_t* aHeight) override;
    NS_IMETHOD GetPixelDepth(int32_t* aPixelDepth) override;
    NS_IMETHOD GetColorDepth(int32_t* aColorDepth) override;

    static uint32_t GetIdFromType(int32_t aType) { return (uint32_t)aType; }

    uint32_t GetId() const { return mId; };
    int32_t GetDisplayType() const { return mDisplayType; }

    void SetDensity(double aDensity) { mDensity = aDensity; }
    double GetDensity() const { return mDensity; }

protected:
    virtual void ApplyMinimumBrightness(uint32_t aBrightness) override;

private:
    uint32_t mId;
    nsIntRect mRect;
    int32_t mDisplayType;
    double mDensity;
};

class nsScreenManagerAndroid final : public nsIScreenManager
{
private:
    ~nsScreenManagerAndroid();

public:
    nsScreenManagerAndroid();

    NS_DECL_ISUPPORTS
    NS_DECL_NSISCREENMANAGER

    already_AddRefed<nsScreenAndroid> AddScreen(int32_t aDisplayType,
                                                nsIntRect aRect = nsIntRect());
    void RemoveScreen(int32_t aDisplayType);

protected:
    nsTArray<RefPtr<nsScreenAndroid>> mScreens;
};

#endif /* nsScreenManagerAndroid_h___ */
