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
#include "nsIWindowWatcher.h"
#include "nsPIDOMWindow.h"
#include "nsQueryObject.h"
#include "nsThreadUtils.h"

static LazyLogModule gSHistoryLog("HDMIDisplayProvider");

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
  nsresult rv = OpenTopLevelWindow();
  NS_ENSURE_SUCCESS(rv, rv);
  MOZ_ASSERT(mProvider);
  return mProvider->RequestSession(this, aUrl, aPresentationId, aControlChannel);
}

NS_IMETHODIMP
HDMIDisplayProvider::HDMIDisplayDevice::Disconnect()
{
  CloseTopLevelWindow();
  return NS_OK;;
}

nsresult
HDMIDisplayProvider::HDMIDisplayDevice::OpenTopLevelWindow()
{
  if (isTopLevelWindowOpened) {
    return NS_OK;
  }

  nsresult rv;
  char* prefValue = nullptr;
  nsCOMPtr<nsIPrefBranch> pref = do_GetService(NS_PREFSERVICE_CONTRACTID);

  rv = pref->GetCharPref("toolkit.defaultChromeFeatures", &prefValue);
  NS_ENSURE_SUCCESS(rv, rv);

  // Due to the limitation of nsWinodw, screen ID should be an integer.
  nsCString flags(prefValue);
  flags.Append(",mozDisplayId=");
  flags.AppendInt(mScreenId);

  rv = pref->GetCharPref("b2g.multiscreen.chrome_remote_url", &prefValue);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCString remoteShellURL(prefValue);
  remoteShellURL.Append('#');
  remoteShellURL.Append(mId);

  nsCString windowName(mName);
  windowName.Append("TopWindow");

  nsCOMPtr<nsIWindowWatcher> ww = do_GetService(NS_WINDOWWATCHER_CONTRACTID);
  rv = ww->OpenWindow(nullptr, remoteShellURL.Data(), windowName.Data(),
                      flags.Data(), nullptr, getter_AddRefs(mWindow));
  MOZ_ASSERT(mWindow);

  isTopLevelWindowOpened = true;
  return NS_OK;
}

nsresult
HDMIDisplayProvider::HDMIDisplayDevice::CloseTopLevelWindow()
{
  if (!mWindow) {
    return NS_OK;
  }

  auto* piWindow = nsPIDOMWindowOuter::From(mWindow);
  piWindow->Close();
  piWindow = nullptr;
  isTopLevelWindowOpened = false;
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
  nsresult rv;

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
  mInitialized = false;
  return NS_OK;
}

nsresult
HDMIDisplayProvider::AddScreen()
{

  nsresult rv;
  nsCOMPtr<nsIPresentationDeviceListener> listener;
  rv = GetListener(getter_AddRefs(listener));
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }

  nsCOMPtr<nsIPresentationDevice> device(do_QueryObject(mDevice));
  rv = listener->AddDevice(device);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }

  return NS_OK;
}

nsresult
HDMIDisplayProvider::RemoveScreen()
{
  nsresult rv;
  nsCOMPtr<nsIPresentationDeviceListener> listener;
  rv = GetListener(getter_AddRefs(listener));
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }

  nsCOMPtr<nsIPresentationDevice> device(do_QueryObject(mDevice));
  rv = listener->RemoveDevice(device);
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

  nsCOMPtr<nsIPresentationDeviceListener> listener = do_QueryReferent(mDeviceListener);
  listener.forget(aListener);

  return NS_OK;
}

NS_IMETHODIMP
HDMIDisplayProvider::SetListener(nsIPresentationDeviceListener* aListener)
{
  mDeviceListener = do_GetWeakReference(aListener);

  nsresult rv;
  if (mDeviceListener) {
    if (NS_WARN_IF(NS_FAILED(rv = Init()))) {
      return rv;
    }
  } else {
    if (NS_WARN_IF(NS_FAILED(rv = Uninit()))) {
      return rv;
    }
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
    if (!displayInfo) {
      return NS_ERROR_NO_INTERFACE;
    }

    int32_t type;
    bool isConnected;
    displayInfo->GetConnected(&isConnected);
    displayInfo->GetId(&type);

    if (type == DisplayType::DISPLAY_EXTERNAL) {
      if (isConnected) {
        AddScreen();
      } else {
        RemoveScreen();
      }
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
