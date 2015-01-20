/*
 * TestWebLock.js
 * Copyright (C) 2015 KuoE0 <kuoe0.tw@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */


function run_test() {

	do_print("Test WebLock...");
	var urls = [ "http://google.com/", "https://foo.com/", "ftp://bar.net/", "http://mozilla.org/" ];

	try {
		var weblock = Cc['@mozilla.org/weblock;1'].createInstance(Ci.nsIWebLock);
		// Cc = Components.classes
		// Ci = Components.interfaces

		do_print("testing constructor");
		equal(weblock.stat, false);

		do_print("testing lock()");
		weblock.lock();
		equal(weblock.stat, true);

		do_print("testing unlock()");
		weblock.unlock();
		equal(weblock.stat, false);

		do_print("testing addSite() and sites");
		for (let i = 0; i < urls.length; ++i) {
			weblock.addSite(urls[i]);
			for (let j = 0; j <= i; ++j) {
				let url = weblock.sites.queryElementAt(j, Ci.nsISupportsString);
				equal(urls[j], url.data);
			}
		}

		do_print("testing removeSite()");
		weblock.removeSite(urls[2]);
		for (let i = 0; i < urls.length - 1; ++i) {
			let t = i < 2 ? i : i + 1;
			let url = weblock.sites.queryElementAt(i, Ci.nsISupportsString);
			equal(urls[t], url.data);
		}

	}
	catch (e) {
		dump(e);
		ok(false);
	}

	ok(true);
}

