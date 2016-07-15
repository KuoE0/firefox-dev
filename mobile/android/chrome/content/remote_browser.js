// -*- Mode: js2; tab-width: 2; indent-tabs-mode: nil; js2-basic-offset: 2; js2-skip-preprocessor-directives: t; -*-
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
"use strict";

var Cc = Components.classes;
var Ci = Components.interfaces;
var Cu = Components.utils;
var Cr = Components.results;

Cu.import("resource://gre/modules/AppConstants.jsm");
Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");
Cu.import("resource://gre/modules/AddonManager.jsm");
Cu.import("resource://gre/modules/AsyncPrefs.jsm");

XPCOMUtils.defineLazyModuleGetter(this, "Messaging",
                                  "resource://gre/modules/Messaging.jsm");

function InitLater(fn, object, name) {
  return DelayedInit.schedule(fn, object, name, 15000 /* 15s max wait */);
}

var RemoteBrowserApp = {

  frame: null,

  startup: function startup() {
    debug("remote browser chrome startup started.");

    this.frame = document.getElementById("remoteFrame");
    RemoteBrowserApp.loadURI("about:blank");

  },

  loadURI: function loadURI(aURI) {
    if (!this.frame) {
      return;
    }

    if (!aURI) {
      aURI = "about:blank";
    }

    debug("Load URI: " + aURI);
    this.frame.setAttribute('src', aURI);
  },

};
