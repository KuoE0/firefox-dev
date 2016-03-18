/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "HDMIDisplayProvider.h"
#include "mozilla/Logging.h"
#include "mozilla/Preferences.h"
#include "mozilla/Services.h"
#include "mozilla/unused.h"
#include "nsIObserverService.h"
#include "nsIServiceManager.h"
#include "nsIURI.h"
#include "nsIWindowWatcher.h"
#include "nsPIDOMWindow.h"
#include "nsQueryObject.h"
#include "nsSimpleURI.h"
#include "nsThreadUtils.h"

static mozilla::LazyLogModule gSHistoryLog("HDMIDisplayProvider");

#define LOG(format) MOZ_LOG(gSHistoryLog, mozilla::LogLevel::Debug, format)

#define DISPLAY_CHANGED_EVENT "display-changed"

namespace mozilla {
namespace dom {
namespace presentation {

NS_IMPL_ISUPPORTS(HDMIDisplayProvider::HDMIDisplayDevice,
                  nsIPresentationDevice)

// nsIPresentationDevice
NS_IMETHODIMP
HDMIDisplayProvider::HDMIDisplayDevice::GetId(nsACString& aId)
{
  aId = mId;
  return NS_OK;
}

NS_IMETHODIMP
HDMIDisplayProvider::HDMIDisplayDevice::GetName(nsACString& aName)
{
  aName = mName;
  return NS_OK;
}

NS_IMETHODIMP
HDMIDisplayProvider::HDMIDisplayDevice::GetType(nsACString& aType)
{
  aType = mType;
  return NS_OK;
}

NS_IMETHODIMP
HDMIDisplayProvider::HDMIDisplayDevice
                   ::EstablishControlChannel(const nsAString& aUrl,
                                             const nsAString& aPresentationId,
                                             nsIPresentationControlChannel** aControlChannel)
{
  MOZ_ASSERT(mProvider);
  nsresult rv = OpenTopLevelWindow();
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }
  return mProvider->RequestSession(this, aUrl, aPresentationId, aControlChannel);
}

NS_IMETHODIMP
HDMIDisplayProvider::HDMIDisplayDevice::Disconnect()
{
  nsresult rv = CloseTopLevelWindow();
  if (NS_WARN_IF(NS_FAILED(rv))) {
      return rv;
  }
  return NS_OK;;
}

nsresult
HDMIDisplayProvider::HDMIDisplayDevice::OpenTopLevelWindow()
{
  MOZ_ASSERT(!mWindow);

  nsresult rv;
  nsAutoCString prefValue;
  nsCOMPtr<nsIPrefBranch> pref = do_GetService(NS_PREFSERVICE_CONTRACTID);

  rv = pref->GetCharPref("toolkit.defaultChromeFeatures",
                         getter_Copies(prefValue));
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }

  nsAutoCString flags(prefValue);
  flags.AppendLiteral(",mozDisplayId=");
  flags.AppendInt(mScreenId);

  rv = pref->GetCharPref("b2g.multiscreen.chrome_remote_url",
                         getter_Copies(prefValue));
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }

  nsCOMPtr<nsIURI> remoteShellURL = new nsSimpleURI();
  rv = remoteShellURL->SetSpec(prefValue);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }
  rv = remoteShellURL->SetRef(prefValue);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }

  nsAutoCString remoteShellURLString;
  rv = remoteShellURL->GetSpec(remoteShellURLString);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }

  nsAutoCString windowName(mName);
  windowName.AppendLiteral("TopWindow");

  nsCOMPtr<nsIWindowWatcher> ww = do_GetService(NS_WINDOWWATCHER_CONTRACTID);
  rv = ww->OpenWindow(nullptr,
                      remoteShellURLString.get(),
                      windowName.get(),
                      flags.get(),
                      nullptr,
                      getter_AddRefs(mWindow));
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }

  return NS_OK;
}

nsresult
HDMIDisplayProvider::HDMIDisplayDevice::CloseTopLevelWindow()
{
  MOZ_ASSERT(mWindow);

  nsCOMPtr<nsPIDOMWindowOuter> piWindow = nsPIDOMWindowOuter::From(mWindow);
  nsresult rv = piWindow->Close();
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }

  piWindow = nullptr;
  mWindow = nullptr;

  return NS_OK;
}

NS_IMPL_ISUPPORTS(HDMIDisplayProvider,
                  nsIObserver,
                  nsIPresentationDeviceProvider)

HDMIDisplayProvider::~HDMIDisplayProvider()
{
  Uninit();
}

nsresult
HDMIDisplayProvider::Init()
{
  // Provider must be initialized only once.
  if (mInitialized) {
    return NS_OK;
  }

  nsCOMPtr<nsIObserverService> obs = mozilla::services::GetObserverService();
  obs->AddObserver(this, DISPLAY_CHANGED_EVENT, false);
  // initial HDMIDisplayDevice
  mDevice = new HDMIDisplayDevice(this);
  mInitialized = true;
  return NS_OK;
}

nsresult
HDMIDisplayProvider::Uninit()
{
  // Provider must be deleted only once.
  if (!mInitialized) {
    return NS_OK;
  }
  mInitialized = false;
  return NS_OK;
}

nsresult
HDMIDisplayProvider::AddScreen()
{
  MOZ_ASSERT(mDeviceListener);

  nsresult rv;
  nsCOMPtr<nsIPresentationDeviceListener> listener;
  rv = GetListener(getter_AddRefs(listener));
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }

  rv = listener->AddDevice(mDevice);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }

  return NS_OK;
}

nsresult
HDMIDisplayProvider::RemoveScreen()
{
  MOZ_ASSERT(mDeviceListener);

  nsresult rv;
  nsCOMPtr<nsIPresentationDeviceListener> listener;
  rv = GetListener(getter_AddRefs(listener));
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }

  rv = listener->RemoveDevice(mDevice);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }

  mDevice->Disconnect();
  return NS_OK;
}

// nsIPresentationDeviceProvider
NS_IMETHODIMP
HDMIDisplayProvider::GetListener(nsIPresentationDeviceListener** aListener)
{
  if (NS_WARN_IF(!aListener)) {
    return NS_ERROR_INVALID_POINTER;
  }

  nsresult rv;
  nsCOMPtr<nsIPresentationDeviceListener> listener =
    do_QueryReferent(mDeviceListener, &rv);
  if (NS_WARN_IF(NS_FAILED(rv))) {
      return rv;
  }

  listener.forget(aListener);

  return NS_OK;
}

NS_IMETHODIMP
HDMIDisplayProvider::SetListener(nsIPresentationDeviceListener* aListener)
{
  mDeviceListener = do_GetWeakReference(aListener);
  nsresult rv = mDeviceListener ? Init() : Uninit();
  if(NS_WARN_IF(NS_FAILED(rv))) {
      return rv;
  }
  return NS_OK;
}

NS_IMETHODIMP
HDMIDisplayProvider::ForceDiscovery()
{
  return NS_OK;
}

// nsIObserver
NS_IMETHODIMP
HDMIDisplayProvider::Observe(nsISupports* aSubject,
                             const char* aTopic,
                             const char16_t* aData)
{
  if (!strcmp(aTopic, DISPLAY_CHANGED_EVENT)) {
    nsCOMPtr<nsIDisplayInfo> displayInfo = do_QueryInterface(aSubject);
    MOZ_ASSERT(displayInfo);

    int32_t type;
    bool isConnected;
    displayInfo->GetConnected(&isConnected);
    // XXX
    // The ID is as same as the type of display.
    // See Bug 1138287 and nsScreenManagerGonk::AddScreen() for more detail.
    displayInfo->GetId(&type);

    if (type == DisplayType::DISPLAY_EXTERNAL) {
      isConnected ? AddScreen() : RemoveScreen();
    }
  }

  return NS_OK;
}

nsresult
HDMIDisplayProvider::RequestSession(HDMIDisplayDevice* aDevice,
                                    const nsAString& aUrl,
                                    const nsAString& aPresentationId,
                                    nsIPresentationControlChannel** aControlChannel)
{
  return NS_OK;
}

} // namespace presentation
} // namespace dom
} // namespace mozilla
