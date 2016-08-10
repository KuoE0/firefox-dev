// -*- Mode: js2; tab-width: 2; indent-tabs-mode: nil; js2-basic-offset: 2; js2-skip-preprocessor-directives: t; -*-
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

"use strict";

this.EXPORTED_SYMBOLS = ["AndroidPresentationApp"];

const { classes: Cc, interfaces: Ci, utils: Cu } = Components;

Cu.import("resource://gre/modules/Services.jsm");
Cu.import("resource://gre/modules/Messaging.jsm");
var log = Cu.import("resource://gre/modules/AndroidLog.jsm", {}).AndroidLog.d.bind(null, "AndroidPresentationApp");

function debug(str) {
  dump("-*- AndroidPresentationApp -*-: " + "<kuoe0> " + str + "\n");
}

// Helper function for sending commands to Java.
function send(type, data, callback) {
  let msg = {
    type: type
  };

  for (let i in data) {
    msg[i] = data[i];
  }

  Messaging.sendRequestForResult(msg)
    .then(result => callback(result, null),
          error => callback(null, error));
}

/* These apps represent players supported natively by the platform. This class will proxy commands
 * to native controls */
function AndroidPresentationApp(service) {
  debug("AndroidPresentationApp created!");
  this.service = service;
  this.location = service.location;
  this.id = service.uuid;
}

AndroidPresentationApp.prototype = {
  start: function start(callback) {
    send("PresentationDevice:Start", { id: this.id }, (result, err) => {
      if (callback) {
        callback(err == null);
      }
    });
  },

  stop: function stop(callback) {
    send("PresentationDevice:Stop", { id: this.id }, (result, err) => {
      if (callback) {
        callback(err == null);
      }
    });
  },
};
