/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


/**
 *
 * A sample of XPConnect. This file contains an implementation nsSample
 * of the interface nsISample.
 *
 */
#include <stdio.h>

#include "nsSample2.h"
#include "nsMemory.h"

#include "nsIClassInfoImpl.h"
////////////////////////////////////////////////////////////////////////

nsSample2Impl::nsSample2Impl() : mValue(nullptr)
{
  mValue = (char*)nsMemory::Clone("initial value", 14);
}

nsSample2Impl::~nsSample2Impl()
{
  if (mValue) {
    nsMemory::Free(mValue);
  }
}

/**
 * NS_IMPL_ISUPPORTS expands to a simple implementation of the nsISupports
 * interface.  This includes a proper implementation of AddRef, Release,
 * and QueryInterface.  If this class supported more interfaces than just
 * nsISupports,
 * you could use NS_IMPL_ADDREF() and NS_IMPL_RELEASE() to take care of the
 * simple stuff, but you would have to create QueryInterface on your own.
 * nsSampleFactory.cpp is an example of this approach.
 * Notice that the second parameter to the macro is name of the interface, and
 * NOT the #defined IID.
 *
 * The _CI variant adds support for nsIClassInfo, which permits introspection
 * and interface flattening.
 */
NS_IMPL_CLASSINFO(nsSample2Impl, nullptr, 0, NS_SAMPLE_CID)
NS_IMPL_ISUPPORTS_CI(nsSample2Impl, nsISample2)
/**
 * Notice that in the protoype for this function, the NS_IMETHOD macro was
 * used to declare the return type.  For the implementation, the return
 * type is declared by NS_IMETHODIMP
 */
NS_IMETHODIMP
nsSample2Impl::GetValue(char** aValue)
{
  NS_PRECONDITION(aValue != nullptr, "null ptr");
  if (!aValue) {
    return NS_ERROR_NULL_POINTER;
  }

  if (mValue) {
    /**
     * GetValue's job is to return data known by an instance of
     * nsSampleImpl to the outside world.  If we  were to simply return
     * a pointer to data owned by this instance, and the client were to
     * free it, bad things would surely follow.
     * On the other hand, if we create a new copy of the data for our
     * client, and it turns out that client is implemented in JavaScript,
     * there would be no way to free the buffer.  The solution to the
     * buffer ownership problem is the nsMemory singleton.  Any buffer
     * returned by an XPCOM method should be allocated by the nsMemory.
     * This convention lets things like JavaScript reflection do their
     * job, and simplifies the way C++ clients deal with returned buffers.
     */
    *aValue = (char*)nsMemory::Clone(mValue, strlen(mValue) + 1);
    if (!*aValue) {
      return NS_ERROR_NULL_POINTER;
    }
  } else {
    *aValue = nullptr;
  }
  return NS_OK;
}

NS_IMETHODIMP
nsSample2Impl::SetValue(const char* aValue)
{
  NS_PRECONDITION(aValue != nullptr, "null ptr");
  if (!aValue) {
    return NS_ERROR_NULL_POINTER;
  }

  if (mValue) {
    nsMemory::Free(mValue);
  }

  /**
   * Another buffer passing convention is that buffers passed INTO your
   * object ARE NOT YOURS.  Keep your hands off them, unless they are
   * declared "inout".  If you want to keep the value for posterity,
   * you will have to make a copy of it.
   */
  mValue = (char*)nsMemory::Clone(aValue, strlen(aValue) + 1);
  return NS_OK;
}

NS_IMETHODIMP
nsSample2Impl::Poke(const char* aValue)
{
  return SetValue((char*)aValue);
}



NS_IMETHODIMP
nsSample2Impl::WriteValue(const char* aPrefix)
{
  NS_PRECONDITION(aPrefix != nullptr, "null ptr");
  if (!aPrefix) {
    return NS_ERROR_NULL_POINTER;
  }

  printf("%s %s\n", aPrefix, mValue);

  return NS_OK;
}
