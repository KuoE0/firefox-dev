/**
 *
 * A sample of XPCOM. This file contains an implementation nsSample2
 * of the interface nsISample2.
 *
 */
#include <stdio.h>
#include "nsSample2.h"
#include "nsMemory.h"

#include "nsIClassInfoImpl.h"
////////////////////////////////////////////////////////////////////////

nsSample2Impl::nsSample2Impl() : mValue(NS_LITERAL_CSTRING("initial value")) {}

nsSample2Impl::~nsSample2Impl() {}

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
NS_IMPL_CLASSINFO(nsSample2Impl, nullptr, 0, NS_SAMPLE2_CID)
NS_IMPL_ISUPPORTS_CI(nsSample2Impl, nsISample2)

/**
 * Notice that in the protoype for this function, the NS_IMETHOD macro was
 * used to declare the return type.  For the implementation, the return
 * type is declared by NS_IMETHODIMP
 */
NS_IMETHODIMP
nsSample2Impl::GetValue(nsACString &aValue)
{
  aValue = mValue;
  return NS_OK;
}

NS_IMETHODIMP
nsSample2Impl::SetValue(const nsACString &aValue)
{
  mValue = aValue;
  return NS_OK;
}

NS_IMETHODIMP
nsSample2Impl::Poke(const nsACString &aValue) { return SetValue(aValue); }

NS_IMETHODIMP
nsSample2Impl::WriteValue(const nsACString &aPrefix)
{
  printf("%s %s\n", ToNewCString(aPrefix), ToNewCString(mValue));
  return NS_OK;
}

NS_IMETHODIMP
nsSample2Impl::Strlen(size_t *_retval)
{
	*_retval = mValue.Length();
	return NS_OK;
}
