/*
 * TestWebLock.cpp
 * Copyright (C) 2015 KuoE0 <kuoe0.tw@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#define MOZILLA_INTERNAL_API

#include "TestHarness.h"
#include "nsIWebLock.h"
#include "prerror.h"
#include "nsString.h"

int
main() {
  ScopedXPCOM xpcom("nsWebLock");

  if (xpcom.failed()) {
    fail("Unable to initialize XPCOM");
    return -1;
  }

  nsresult rv;

  nsCOMPtr<nsIWebLock> weblock =
    do_CreateInstance("@mozilla.org/weblock;1", &rv);
  if (NS_FAILED(rv)) {
    fail("Failed to create weblcok.");
    printf("Error code: 0x%X\n", rv);
    return -1;
  }


  passed("WebLock Works");
	return 0;
}



