/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "HDMIDisplayProvider.h"
#include "mozilla/Logging.h"
#include "mozilla/Preferences.h"
#include "mozilla/Services.h"
#include "nsIObserverService.h"
#include "nsIServiceManager.h"
#include "nsQueryObject.h"
#include "nsIWindowWatcher.h"

inline static PRLogModuleInfo*
GetHDMIProviderLog()
{
  static PRLogModuleInfo* log = PR_NewLogModule("HDMIDisplayProvider");
  return log;
}
#undef LOG_I
#define LOG_I(...) MOZ_LOG(GetHDMIProviderLog(), mozilla::LogLevel::Debug, (__VA_ARGS__))
#undef LOG_E
#define LOG_E(...) MOZ_LOG(GetHDMIProviderLog(), mozilla::LogLevel::Error, (__VA_ARGS__))

#define DISPLAY_CHANGED_EVENT "display-changed"
#define CLOSE_TOP_LEVEL_WINDOW_EVENT "close-top-level-window"

namespace mozilla {
namespace dom {
namespace presentation {

NS_IMPL_ISUPPORTS(HDMIDisplayProvider::HDMIDisplayDevice,
                  nsIPresentationDevice)

// nsIPresentationDevice
NS_IMETHODIMP
HDMIDisplayProvider::HDMIDisplayDevice::GetId(nsACString& aId)
{
  // add "hdmi:" as prefix
  aId = Move(nsCString("hdmi:"));
  aId.AppendInt(mScreenId);
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
HDMIDisplayProvider::HDMIDisplayDevice::EstablishControlChannel(const nsAString& aUrl,
                                                                const nsAString& aPresentationId,
                                                                nsIPresentationControlChannel** aChannel)
{
  nsresult rv = OpenTopLevelWindow();
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

nsresult
HDMIDisplayProvider::HDMIDisplayDevice::OpenTopLevelWindow()
{
  printf_stderr("<kuoe0> HDMIDisplayDevice::OpenTopLevelWindow");
  if (isTopLevelWindowOpened) {
    printf_stderr("Top level window for HDMI has been opened.");
    return NS_OK;
  }

  nsresult rv;
  char* prefValue = nullptr;
  nsCOMPtr<nsIPrefBranch> pref = do_GetService(NS_PREFSERVICE_CONTRACTID);

  rv = pref->GetCharPref("toolkit.defaultChromeFeatures", &prefValue);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCString flags(prefValue);
  flags.Append(",mozDisplayId=");
  flags.AppendInt(mScreenId);

  printf_stderr("<kuoe0> flags: %s", flags.Data());

  rv = pref->GetCharPref("b2g.multiscreen.chrome_remote_url", &prefValue);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCString remoteShellURL(prefValue);
  remoteShellURL.Append('#');
  remoteShellURL.AppendInt(mScreenId);

  printf_stderr("<kuoe0> prefs: %s", remoteShellURL.Data());

  nsCString windowName(mName);
  windowName.Append("TopWindow");

  // TODO: 這邊需要把 window keep 住，不然就不能 close 啦！
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
  isTopLevelWindowOpened = false;
  // TODO: close before check window exist
  mWindow->Close();
  mWindow = nullptr;
  return NS_OK;
}

NS_IMPL_ISUPPORTS(HDMIDisplayProvider,
                  nsIPresentationDeviceProvider,
                  nsIObserver)

HDMIDisplayProvider::~HDMIDisplayProvider()
{
  Uninit();
}

nsresult
HDMIDisplayProvider::Init()
{
  nsCOMPtr<nsIObserverService> obs = mozilla::services::GetObserverService();
  obs->AddObserver(this, DISPLAY_CHANGED_EVENT, false);
  obs->AddObserver(this, CLOSE_TOP_LEVEL_WINDOW_EVENT, false);
  // initial HDMIDisplayDevice
  mDevice = new HDMIDisplayDevice();
  return NS_OK;
}

nsresult
HDMIDisplayProvider::Uninit()
{
  return NS_OK;
}

nsresult
HDMIDisplayProvider::AddScreen()
{

  nsresult rv;
  nsCOMPtr<nsIPresentationDeviceListener> listener;
  rv = this->GetListener(getter_AddRefs(listener));
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
  rv = this->GetListener(getter_AddRefs(listener));
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }

  mDevice->CloseTopLevelWindow();

  nsCOMPtr<nsIPresentationDevice> device(do_QueryObject(mDevice));
  rv = listener->RemoveDevice(device);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }

  return NS_OK;
}

// nsIPresentationDeviceProvider
NS_IMETHODIMP
HDMIDisplayProvider::GetListener(nsIPresentationDeviceListener** aListener)
{
  if (NS_WARN_IF(!aListener)) {
    return NS_ERROR_INVALID_POINTER;
  }

  // XXX: why use nsCOMPtr and do_QueryReferent
  nsCOMPtr<nsIPresentationDeviceListener> listener = do_QueryReferent(mDeviceListener);
  listener.forget(aListener);

  return NS_OK;
}

NS_IMETHODIMP
HDMIDisplayProvider::SetListener(nsIPresentationDeviceListener* aListener)
{
  // XXX: why use do_GetWeakReference
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
  printf_stderr("<kuoe0> HDMIDisplayProvider::Observe - Topic: %s\n", aTopic);

  if (!strcmp(aTopic, CLOSE_TOP_LEVEL_WINDOW_EVENT)) {
    mDevice->CloseTopLevelWindow();
  }
  else if (!strcmp(aTopic, DISPLAY_CHANGED_EVENT)) {
    // TODO: add check aTopic
    nsCOMPtr<nsIDisplayInfo> displayInfo = do_QueryInterface(aSubject);
    // TODO: check do_QueryInterface success

    int32_t type;
    bool isConnected;
    displayInfo->GetConnected(&isConnected);
    displayInfo->GetId(&type);

    if (type == DisplayType::DISPLAY_EXTERNAL) {
      if (isConnected) {
        AddScreen();
      }
      else {
        RemoveScreen();
      }
    }
  }

  return NS_OK;
}

} // namespace presentation
} // namespace dom
} // namespace mozilla
