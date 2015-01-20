/*
 * nsWebLock.h
 * Copyright (C) 2015 KuoE0 <kuoe0.tw@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef nsWebLock_h__
#define nsWebLock_h__

#include "nsString.h"
#include "nsIWebLock.h"
#include <vector>

using namespace std;

#define NS_WEBLOCK_CID                                                         \
  {                                                                            \
    0xea54eee4, 0x9548, 0x4b63,                                                \
    {                                                                          \
      0xb9, 0x4d, 0xc5, 0x19, 0xff, 0xc9, 0x1d, 0x9                            \
    }                                                                          \
  }

#define NS_WEBLOCK_CONTRACTID "@mozilla.org/weblock;1"

class nsWebLock MOZ_FINAL : public nsIWebLock
{
public:
  nsWebLock();
  NS_DECL_ISUPPORTS
  NS_DECL_NSIWEBLOCK
private:
  ~nsWebLock();
  PRBool mLocked;
  vector<nsString> mSites;
};

#endif /* !nsWebLock_h__ */
