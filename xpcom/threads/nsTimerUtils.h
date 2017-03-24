/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef nsTimerUtils_h__
#define nsTimerUtils_h__

#include "nsINamed.h"
#include "nsITimer.h"

// A base class for GenericNamedTimerCallback<Function>.
// This is necessary because NS_IMPL_ISUPPORTS doesn't work for a class
// template.
class GenericNamedTimerCallbackBase : public nsITimerCallback,
                                      public nsINamed
{
public:
  NS_DECL_THREADSAFE_ISUPPORTS

protected:
  virtual ~GenericNamedTimerCallbackBase() {}
};

// An nsITimerCallback implementation with nsINamed that can be used with any
// function object that's callable with no arguments.
template <typename Function>
class GenericNamedTimerCallback final : public GenericNamedTimerCallbackBase
{
public:
  explicit GenericNamedTimerCallback(const Function& aFunction,
                                     const char* aName)
    : mFunction(aFunction)
    , mName(aName)
  {
  }

  NS_IMETHOD Notify(nsITimer*) override
  {
    mFunction();
    return NS_OK;
  }

  NS_IMETHOD GetName(nsACString& aName) override
  {
    aName = mName;
    return NS_OK;
  }

  NS_IMETHOD SetName(const char * aName) override
  {
    mName.Assign(aName);
    return NS_OK;
  }

private:
  Function mFunction;
  nsCString mName;
};

// Convenience function for constructing a GenericNamedTimerCallback.
// Returns a raw pointer, suitable for passing directly as an argument to
// nsITimer::InitWithCallback(). The intention is to enable the following
// terse inline usage:
//    timer->InitWithCallback(NewNamedTimerCallback([](){ ... }, name), delay);
template <typename Function>
GenericNamedTimerCallback<Function>*
  NewNamedTimerCallback(const Function& aFunction,
                        const char* aName)
{
  return new GenericNamedTimerCallback<Function>(aFunction, aName);
}

#endif /* !nsTimerUtils_h__ */
