/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_presentation_provider_HDMIDisplayProvider_h
#define mozilla_dom_presentation_provider_HDMIDisplayProvider_h

#include "mozilla/RefPtr.h"
#include "nsCOMPtr.h"
#include "nsIDisplayInfo.h"
#include "nsIDOMWindow.h"
#include "nsIObserver.h"
#include "nsIPresentationDevice.h"
#include "nsIPresentationDeviceProvider.h"
#include "nsIWindowWatcher.h"
#include "nsString.h"
#include "nsTArray.h"
#include "nsWeakPtr.h"

namespace mozilla {
namespace dom {
namespace presentation {

// Consistent definition with the definition in  widget/gonk/libdisplay/GonkDisplay.h.
enum DisplayType {
    DISPLAY_PRIMARY,
    DISPLAY_EXTERNAL,
    DISPLAY_VIRTUAL,
    NUM_DISPLAY_TYPES
};

class HDMIDisplayProvider final
  : public nsIObserver
  , public nsIPresentationDeviceProvider
{
private:
  class HDMIDisplayDevice final : public nsIPresentationDevice
  {
  public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSIPRESENTATIONDEVICE

    // The action menu in Gaia can return only integer after some item seleted.
    // Due to the limitation, we use an integer means "external display" as the
    // ID in HDMIDisplayDevice.

    explicit HDMIDisplayDevice(HDMIDisplayProvider* aProvider)
      : mScreenId(DisplayType::DISPLAY_EXTERNAL)
      , mName("HDMI")
      , mType("external")
      , mId("hdmi")
      , mProvider(aProvider)
      , isTopLevelWindowOpened(false)
    {}

    nsresult OpenTopLevelWindow();
    nsresult CloseTopLevelWindow();

    const nsCString Id() const { return mId; }

  private:
    ~HDMIDisplayDevice() = default;

    uint32_t mScreenId;
    nsCString mName;
    nsCString mType;
    nsCString mId;

    nsCOMPtr<mozIDOMWindowProxy> mWindow;
    HDMIDisplayProvider* mProvider;
    bool isTopLevelWindowOpened;
  };

public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIOBSERVER
  NS_DECL_NSIPRESENTATIONDEVICEPROVIDER

  nsresult Init();
  nsresult Uninit();

  nsresult RequestSession(HDMIDisplayDevice* aDevice,
                          const nsAString& aUrl,
                          const nsAString& aPresentationId,
                          nsIPresentationControlChannel** aControlChannel);
private:
  virtual ~HDMIDisplayProvider();

  nsresult AddScreen();
  nsresult RemoveScreen();

  // There should be only one HDMI display.
  RefPtr<HDMIDisplayDevice> mDevice;
  nsWeakPtr mDeviceListener;

  bool mInitialized = false;
};

} // mozilla
} // dom
} // presentation

#endif // mozilla_dom_presentation_provider_HDMIDisplayProvider_h

