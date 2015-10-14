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
#include "nsString.h"
#include "nsTArray.h"
#include "nsWeakPtr.h"

#define STR(X) #X

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
  : public nsIPresentationDeviceProvider
  , public nsIObserver
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

    explicit HDMIDisplayDevice()
      : mScreenId(DisplayType::DISPLAY_EXTERNAL)
      , mName("HDMI")
      , mType("hdmi")
      , isTopLevelWindowOpened(false)
    {}

    nsresult OpenTopLevelWindow();
    nsresult CloseTopLevelWindow();

  private:
    uint32_t mScreenId;
    nsCString mName;
    nsCString mType;

    nsCOMPtr<nsIDOMWindow> mWindow;
    bool isTopLevelWindowOpened;
  };

public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPRESENTATIONDEVICEPROVIDER
  NS_DECL_NSIOBSERVER

  nsresult Init();
  nsresult Uninit();

private:
  virtual ~HDMIDisplayProvider();

  nsresult AddScreen();
  nsresult RemoveScreen();

  // There should be only one HDMI display.
  RefPtr<HDMIDisplayDevice> mDevice;
  nsWeakPtr mDeviceListener;
};

} // mozilla
} // dom
} // presentation

#endif // mozilla_dom_presentation_provider_HDMIDisplayProvider_h

