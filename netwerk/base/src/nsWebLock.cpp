/*
 * nsWebLock.cpp
 * Copyright (C) 2015 KuoE0 <kuoe0.tw@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "nsWebLock.h"
#include "nsCOMPtr.h"
#include "nsIMutableArray.h"
#include "nsComponentManagerUtils.h"
#include "nsISupportsPrimitives.h"

NS_IMPL_ISUPPORTS(nsWebLock, nsIWebLock)

nsWebLock::nsWebLock() :mLocked(false) {}

nsWebLock::~nsWebLock() {}

NS_IMETHODIMP
nsWebLock::Lock()
{
  mLocked = PR_TRUE;
  return NS_OK;
}

NS_IMETHODIMP
nsWebLock::Unlock()
{
  mLocked = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
nsWebLock::AddSite(const nsAString &url)
{
  for (auto x : mSites)
    if (url.Equals(x))
      return NS_ERROR_FAILURE;
  mSites.push_back(nsString(url));
  return NS_OK;
}

NS_IMETHODIMP
nsWebLock::RemoveSite(const nsAString &url)
{
  vector<nsString>::iterator iter;
  for (iter = mSites.begin(); iter != mSites.end(); ++iter) {
    if (url.Equals(*iter))
      break;
  }

  if (iter == mSites.end())
    return NS_ERROR_FAILURE;

  mSites.erase(iter);

  return NS_OK;
}

NS_IMETHODIMP
nsWebLock::GetSites(nsIMutableArray **aSites)
{
  nsCOMPtr<nsIMutableArray> array = do_CreateInstance(NS_ARRAY_CONTRACTID);
  for (auto url : mSites) {
    nsCOMPtr<nsISupportsString> iUrl =
      do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID);
    iUrl->SetData(url);
    array->AppendElement(iUrl, PR_FALSE);
  }

  *aSites = array;
  NS_ADDREF(*aSites);
  return NS_OK;
}

NS_IMETHODIMP
nsWebLock::GetStat(bool *aStat)
{
	*aStat = (bool)mLocked;
	return NS_OK;
}

