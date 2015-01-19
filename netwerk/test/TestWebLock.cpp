/*
 * TestWebLock.cpp
 * Copyright (C) 2015 KuoE0 <kuoe0.tw@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "TestHarness.h"
#include "nsIWebLock.h"
#include "nsIMutableArray.h"
#include "nsISupportsPrimitives.h"
#include "nsStringAPI.h"
#include "prerror.h"

int
main()
{
  ScopedXPCOM xpcom("WebLock");

  if (xpcom.failed()) {
    fail("Unable to initialize XPCOM");
    return -1;
  }

  nsresult rv;
  /*
   * URL candidates
   */
  nsString url[] = { NS_LITERAL_STRING("http://google.com/"),
                     NS_LITERAL_STRING("https://foo.com/"),
                     NS_LITERAL_STRING("ftp://bar.net/"),
                     NS_LITERAL_STRING("http://mozilla.org/") };

  nsCOMPtr<nsIWebLock> weblock =
    do_CreateInstance("@mozilla.org/weblock;1", &rv);
  if (NS_FAILED(rv)) {
    fail("Creating Instance: 0x%X.\n", rv);
    return -1;
  }

  bool lockStat;
  nsCOMPtr<nsIMutableArray> whiteUrl;
  nsCOMPtr<nsISupportsString> iUrl;
  char16_t *pUrl;

  rv = weblock->GetStat(&lockStat);

  if (NS_FAILED(rv)) {
    fail("GetStat(): 0x%X.\n", rv);
    return -1;
  }
  passed("GetStat()");

  if (lockStat == true) {
    fail("Initial Value Error");
    return -1;
  }

  rv = weblock->Lock();
  if (NS_FAILED(rv)) {
    fail("Lock(): 0x%X.\n", rv);
    return -1;
  }

  rv = weblock->GetStat(&lockStat);

  if (NS_FAILED(rv)) {
    fail("GetStat(): 0x%X.\n", rv);
    return -1;
  }
  if (lockStat == false) {
    fail("Lock() Not Work.");
    return -1;
  }
  passed("Lock()");

  rv = weblock->Unlock();
  if (NS_FAILED(rv)) {
    fail("Unlock(): 0x%X.\n", rv);
    return -1;
  }

  rv = weblock->GetStat(&lockStat);

  if (NS_FAILED(rv)) {
    fail("GetStat(): 0x%X.\n", rv);
    return -1;
  }
  if (lockStat == true) {
    fail("Unlock() Not Work.");
    return -1;
  }
  passed("Unlock()");

  for (int i = 0; i < 4; ++i) {
    rv = weblock->AddSite(url[i]);
    if (NS_FAILED(rv)) {
      fail("AddSite(): 0x%X.\n", rv);
      return -1;
    }

    rv = weblock->GetSites(getter_AddRefs(whiteUrl));
    if (NS_FAILED(rv)) {
      fail("GetSites(): 0x%X.\n", rv);
      return -1;
    }
    for (int j = 0; j <= i; ++j) {
      rv = whiteUrl->QueryElementAt(j, NS_GET_IID(nsISupportsString),
                                    getter_AddRefs(iUrl));
      if (NS_FAILED(rv)) {
        fail("QeuryElementAt(): 0x%X.\n", rv);
        return -1;
      }

      rv = iUrl->ToString(&pUrl);
      if (!url[j].Equals(pUrl)) {
        fail("Urls Not Equal!\n");
        return -1;
      }
    }
  }
  passed("AddSite() & GetSite()");

  rv = weblock->RemoveSite(url[2]);
  if (NS_FAILED(rv)) {
    fail("RemoveSite(): 0x%X.\n", rv);
    return -1;
  }

  rv = weblock->GetSites(getter_AddRefs(whiteUrl));
  if (NS_FAILED(rv)) {
    fail("GetSites(): 0x%X.\n", rv);
    return -1;
  }

  for (int i = 0; i < 3; ++i) {
    int t = i < 2 ? i : i + 1;

    rv = whiteUrl->QueryElementAt(i, NS_GET_IID(nsISupportsString),
                                  getter_AddRefs(iUrl));

    if (NS_FAILED(rv)) {
      fail("QeuryElementAt(): 0x%X.\n", rv);
      return -1;
    }

    rv = iUrl->ToString(&pUrl);

    if (!url[t].Equals(pUrl)) {
      fail("RemoveSite()");
      return -1;
    }
  }
  passed("RemoveSite()");

  passed("WebLock Works");
  return 0;
}
