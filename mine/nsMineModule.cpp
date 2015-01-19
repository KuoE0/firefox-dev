/*
 * nsMineModule.cpp
 * Copyright (C) 2015 KuoE0 <kuoe0.tw@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "mozilla/ModuleUtils.h"
#include "nsWebLock.h"

NS_DEFINE_NAMED_CID(NS_WEBLOCK_CID);
NS_GENERIC_FACTORY_CONSTRUCTOR(nsWebLock)

static const mozilla::Module::CIDEntry kNeckoCIDs[] = {
  { &kNS_WEBLOCK_CID, false, nullptr, nsWebLockConstructor }, { nullptr }
};

static const mozilla::Module::ContractIDEntry kNeckoContracts[] = {
  { NS_WEBLOCK_CONTRACTID, &kNS_WEBLOCK_CID }, { nullptr }
};

static const mozilla::Module kNeckoModule = {
  mozilla::Module::kVersion, kNeckoCIDs, kNeckoContracts, nullptr,
  nullptr,                   nullptr,    nullptr
};

NSMODULE_DEFN(mine) = &kNeckoModule;
