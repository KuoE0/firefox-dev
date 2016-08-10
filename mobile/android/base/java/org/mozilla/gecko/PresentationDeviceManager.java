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
import org.mozilla.gecko.RemotePresentationService;

import com.google.android.gms.cast.CastDevice;
import com.google.android.gms.cast.CastMediaControlIntent;
import com.google.android.gms.cast.CastPresentation;
import com.google.android.gms.cast.CastRemoteDisplayLocalService;
import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GooglePlayServicesUtil;
import com.google.android.gms.common.api.Status;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v7.media.MediaControlIntent;
import android.support.v7.media.MediaRouteSelector;
import android.support.v7.media.MediaRouter.RouteInfo;
import android.support.v7.media.MediaRouter;
import android.util.Log;
import android.view.Display;
import android.view.WindowManager;

import java.util.HashMap;
import java.util.Map;

class ChromecastRemoteDevice {
    private static final String LOGTAG = "ChromecastRemoteDevice";
    private final String INTENT_EXTRA_CAST_DEVICE = "CastDevice";
    private final Context mContext;
    private final RouteInfo mRoute;
    private CastDevice mCastDevice;

    public ChromecastRemoteDevice(Context context, RouteInfo route) {
        int status =  GooglePlayServicesUtil.isGooglePlayServicesAvailable(context);
        if (status != ConnectionResult.SUCCESS) {
            throw new IllegalStateException("Play services are required for Chromecast support (got status code " + status + ")");
        }

        this.mContext = context;
        this.mRoute = route;
        this.mCastDevice = CastDevice.getFromBundle(mRoute.getExtras());
    }

    public JSONObject toJSON() {
        final JSONObject obj = new JSONObject();
        try {
            if (mCastDevice == null) {
                return null;
            }

            obj.put("uuid", mRoute.getId());
            obj.put("version", mCastDevice.getDeviceVersion());
            obj.put("friendlyName", mCastDevice.getFriendlyName());
            obj.put("location", mCastDevice.getIpAddress().toString());
            obj.put("modelName", mCastDevice.getModelName());
            // For now we just assume all of these are Google devices
            obj.put("manufacturer", "Google Inc.");
        } catch (JSONException ex) {
            Log.d(LOGTAG, "Error building route", ex);
        }

        return obj;
    }

    public void start(final EventCallback callback) {
        // Nothing to be done here
        Log.d(LOGTAG, "<kuoe0> start()");
        if (startPresentation()) {
            sendSuccess(callback, null);
        } else {
            callback.sendError(null);
        }
    }

    public void stop(final EventCallback callback) {
        // Nothing to be done here
        sendSuccess(callback, null);
        Log.d(LOGTAG, "<kuoe0> stop()");
    }

    // EventCallback which is actually a GeckoEventCallback is sometimes being invoked more
    // than once. That causes the IllegalStateException to be thrown. To prevent a crash,
    // catch the exception and report it as an error to the log.
    private static void sendSuccess(final EventCallback callback, final String msg) {
        try {
            callback.sendSuccess(msg);
        } catch (final IllegalStateException e) {
            Log.e(LOGTAG, "Attempting to invoke callback.sendSuccess more than once.", e);
        }
    }

    private boolean startPresentation() {
        Log.d(LOGTAG, "<kuoe0> startPresentation");
        try {
            Intent intent = new Intent(mContext, RemotePresentationService.class);
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
            PendingIntent notificationPendingIntent = PendingIntent.getActivity(mContext, 0, intent, 0);

            CastRemoteDisplayLocalService.NotificationSettings settings =
                new CastRemoteDisplayLocalService.NotificationSettings.Builder()
                .setNotificationPendingIntent(notificationPendingIntent).build();

            CastRemoteDisplayLocalService.startService(
                    mContext,
                    RemotePresentationService.class,
                    mContext.getString(R.string.chromecast_app_id),
                    mCastDevice,
                    settings,
                    new CastRemoteDisplayLocalService.Callbacks() {
                        public void onServiceCreated(CastRemoteDisplayLocalService service) {
                            Log.d(LOGTAG, "<kuoe0> onServiceCreated");
                        }

                        @Override
                        public void onRemoteDisplaySessionStarted(CastRemoteDisplayLocalService service) {
                            Log.d(LOGTAG, "<kuoe0> onServiceStarted");
                        }

                        @Override
                        public void onRemoteDisplaySessionError(Status errorReason) {
                            int code = errorReason.getStatusCode();
                            Log.d(LOGTAG, "<kuoe0> onServiceError: " + errorReason.getStatusCode());

                            mCastDevice = null;
                        }
            });
            return true;
        } catch (final IllegalArgumentException e) {
            Log.e(LOGTAG, "IllegalArgumentException", e);
            return false;
        }
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

    private static final boolean SHOW_DEBUG = true;
    // Simplified debugging interfaces
    private static void debug(String msg, Exception e) {
        if (SHOW_DEBUG) {
            Log.e(LOGTAG, "<kuoe0> " + msg, e);
        }
    }

    private static void debug(String msg) {
        if (SHOW_DEBUG) {
            Log.d(LOGTAG, "<kuoe0> " + msg);
        }
    }

    protected MediaRouter mediaRouter = null;
    protected final Map<String, ChromecastRemoteDevice> devices = new HashMap<String, ChromecastRemoteDevice>();

    @Override
    public void onCreate(Bundle savedInstanceState) {
        debug("onCreate");
        super.onCreate(savedInstanceState);
        EventDispatcher.getInstance().registerGeckoThreadListener(this,
                "PresentationDevice:Start",
                "PresentationDevice:Stop");
    }

    @Override
    @JNITarget
    public void onDestroy() {
        super.onDestroy();
        EventDispatcher.getInstance().unregisterGeckoThreadListener(this,
                "PresentationDevice:Start",
                "PresentationDevice:Stop");
    }

    // GeckoEventListener implementation
    @Override
    public void handleMessage(String event, final NativeJSObject message, final EventCallback callback) {
        debug(event);
        debug("Callback: " + !(callback == null));
        final ChromecastRemoteDevice device = devices.get(message.getString("id"));
        if (device == null) {
            Log.e(LOGTAG, "Couldn't find a device for this id: " + message.getString("id") + " for message: " + event);
            if (callback != null) {
                callback.sendError(null);
            }
            return;
        }

        if ("PresentationDevice:Start".equals(event)) {
            device.start(callback);
        } else if ("PresentationDevice:Stop".equals(event)) {
            device.stop(callback);
        }
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
        debug("onResume");

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
