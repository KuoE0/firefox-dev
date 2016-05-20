/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 *
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko;

import org.json.JSONObject;
import org.json.JSONException;

import org.json.JSONObject;
import org.mozilla.gecko.AppConstants.Versions;
import org.mozilla.gecko.R;
import org.mozilla.gecko.annotation.JNITarget;
import org.mozilla.gecko.annotation.ReflectionTarget;
import org.mozilla.gecko.util.EventCallback;
import org.mozilla.gecko.util.NativeEventListener;
import org.mozilla.gecko.util.NativeJSObject;

import com.google.android.gms.cast.CastDevice;
import com.google.android.gms.cast.CastMediaControlIntent;
import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GooglePlayServicesUtil;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v7.media.MediaControlIntent;
import android.support.v7.media.MediaRouteSelector;
import android.support.v7.media.MediaRouter.RouteInfo;
import android.support.v7.media.MediaRouter;
import android.util.Log;

import java.util.HashMap;
import java.util.Map;

class ChromecastRemoteDevice {
    private static final String LOGTAG = "ChromecastRemoteDevice";
    private final Context mContext;
    private final RouteInfo mRoute;

    public ChromecastRemoteDevice(Context context, RouteInfo route) {
        int status =  GooglePlayServicesUtil.isGooglePlayServicesAvailable(context);
        if (status != ConnectionResult.SUCCESS) {
            throw new IllegalStateException("Play services are required for Chromecast support (got status code " + status + ")");
        }

        this.mContext = context;
        this.mRoute = route;
    }

    public JSONObject toJSON() {
        final JSONObject obj = new JSONObject();
        try {
            final CastDevice device = CastDevice.getFromBundle(mRoute.getExtras());
            if (device == null) {
                return null;
            }

            obj.put("uuid", mRoute.getId());
            obj.put("version", device.getDeviceVersion());
            obj.put("friendlyName", device.getFriendlyName());
            obj.put("location", device.getIpAddress().toString());
            obj.put("modelName", device.getModelName());
            // For now we just assume all of these are Google devices
            obj.put("manufacturer", "Google Inc.");
        } catch (JSONException ex) {
            Log.d(LOGTAG, "Error building route", ex);
        }

        return obj;
    }
}

/**
 * Manages a list of GeckoPresentationDevice methods (i.e. Chromecast/Miracast). Routes messages
 * from Gecko to the correct caster based on the id of the device
 */
public class PresentationDeviceManager extends Fragment implements NativeEventListener {
    /**
     * Create a new instance of DetailsFragment, initialized to
     * show the text at 'index'.
     */
    @ReflectionTarget
    public static PresentationDeviceManager newInstance() {
        return new PresentationDeviceManager();
    }

    private static final String LOGTAG = "GeckoPresentationDeviceManager";

    @ReflectionTarget
    public static final String PRESENTATION_DEVICE_TAG = "MPManagerFragment";

    private static final boolean SHOW_DEBUG = false;
    // Simplified debugging interfaces
    private static void debug(String msg, Exception e) {
        if (SHOW_DEBUG) {
            Log.e(LOGTAG, msg, e);
        }
    }

    private static void debug(String msg) {
        if (SHOW_DEBUG) {
            Log.d(LOGTAG, msg);
        }
    }

    protected MediaRouter mediaRouter = null;
    protected final Map<String, ChromecastRemoteDevice> devices = new HashMap<String, ChromecastRemoteDevice>();

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    @JNITarget
    public void onDestroy() {
        super.onDestroy();
    }

    // GeckoEventListener implementation
    @Override
    public void handleMessage(String event, final NativeJSObject message, final EventCallback callback) {
        debug(event);
    }

    private final MediaRouter.Callback callback =
        new MediaRouter.Callback() {
            @Override
            public void onRouteRemoved(MediaRouter router, RouteInfo route) {
                debug("onRouteRemoved: route=" + route);
                devices.remove(route.getId());
                GeckoAppShell.notifyObservers("PresentationDevice:Removed", route.getId());
                updatePresentation();
            }

            @SuppressWarnings("unused")
            public void onRouteSelected(MediaRouter router, int type, MediaRouter.RouteInfo route) {
                updatePresentation();
            }

            // These methods aren't used by the support version Media Router
            @SuppressWarnings("unused")
            public void onRouteUnselected(MediaRouter router, int type, RouteInfo route) {
                updatePresentation();
            }

            @Override
            public void onRoutePresentationDisplayChanged(MediaRouter router, RouteInfo route) {
                updatePresentation();
            }

            @Override
            public void onRouteVolumeChanged(MediaRouter router, RouteInfo route) {
            }

            @Override
            public void onRouteAdded(MediaRouter router, MediaRouter.RouteInfo route) {
                debug("onRouteAdded: route=" + route);
                final ChromecastRemoteDevice device = getPresentationDeviceForRoute(route);
                saveAndNotifyOfDevice("PresentationDevice:Added", route, device);
                updatePresentation();
            }

            @Override
            public void onRouteChanged(MediaRouter router, MediaRouter.RouteInfo route) {
                debug("onRouteChanged: route=" + route);
                final ChromecastRemoteDevice device = devices.get(route.getId());
                saveAndNotifyOfDevice("PresentationDevice:Changed", route, device);
                updatePresentation();
            }

            private void saveAndNotifyOfDevice(final String eventName,
                    MediaRouter.RouteInfo route, final ChromecastRemoteDevice device) {
                if (device == null) {
                    return;
                }

                final JSONObject json = device.toJSON();
                if (json == null) {
                    return;
                }

                devices.put(route.getId(), device);
                GeckoAppShell.notifyObservers(eventName, json.toString());
            }
        };

    private ChromecastRemoteDevice getPresentationDeviceForRoute(MediaRouter.RouteInfo route) {
        try {
            return new ChromecastRemoteDevice(getActivity(), route);
        } catch (Exception ex) {
            debug("Error handling presentation", ex);
        }

        return null;
    }

    @Override
    public void onPause() {
        super.onPause();
        mediaRouter.removeCallback(callback);
        mediaRouter = null;
    }

    @Override
    public void onResume() {
        super.onResume();

        // The mediaRouter shouldn't exist here, but this is a nice safety check.
        if (mediaRouter != null) {
            return;
        }

        mediaRouter = MediaRouter.getInstance(getActivity());
        final MediaRouteSelector selectorBuilder = new MediaRouteSelector.Builder()
            .addControlCategory(CastMediaControlIntent.categoryForCast(getActivity().getString(R.string.chromecast_app_id)))
            .build();
        mediaRouter.addCallback(selectorBuilder, callback, MediaRouter.CALLBACK_FLAG_REQUEST_DISCOVERY);
    }

    protected void updatePresentation() { /* Overridden in sub-classes. */ }
}
