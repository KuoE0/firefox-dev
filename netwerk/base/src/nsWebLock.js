/*
 * nsWebLock.js
 * Copyright (C) 2015 KuoE0 <kuoe0.tw@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

const Cc = Components.classes;
const Ci = Components.interfaces;
const Cu = Components.utils;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");

function nsWebLock() {};

nsWebLock.prototype = {
	mLock: false,
	mSites: [],
	classDescription: "WebLock",
	classID:          Components.ID("{EA54EEE4-9548-4B63-B94D-C519FFC91D09}"),
	contractID:       "@mozill.org/jsweblock;1",

	QueryInterface: XPCOMUtils.generateQI([Ci.nsIWebLock]),
	
	lock: function() { this.mLock = true; },
	unlock: function() { this.mLock = false; },
	addSite: function(url) {
		this.mSites[this.mSites.length] = url;
	},
	removeSite: function(url) {
		let idx = this.mSites.indexOf(url);
		this.mSites.splice(idx, 1);
	},
	get sites() {
		let retSites = Cc["@mozilla.org/array;1"].createInstance(Ci.nsIMutableArray);

		for (let i = 0; i < this.mSites.length; ++i) {
			let url = this.mSites[i];
			let iUrl = Cc["@mozilla.org/supports-string;1"].createInstance(Ci.nsISupportsString);
			iUrl.data = url;
			retSites.appendElement(iUrl, false);
		}
		return retSites;
	},
	get stat() { return this.mLock; },
};

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([nsWebLock]);

