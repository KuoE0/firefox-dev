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
import org.mozilla.gecko.GeckoAppShell;
import org.mozilla.gecko.GeckoView;
import org.mozilla.gecko.R;
import org.mozilla.gecko.annotation.JNITarget;
import org.mozilla.gecko.annotation.ReflectionTarget;
import org.mozilla.gecko.annotation.WrapForJNI;
import org.mozilla.gecko.gfx.LayerView;
import org.mozilla.gecko.util.EventCallback;
import org.mozilla.gecko.util.NativeEventListener;
import org.mozilla.gecko.util.NativeJSObject;

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
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.ViewGroup.LayoutParams;
import android.view.WindowManager;
import android.widget.RelativeLayout;

import java.util.HashMap;
import java.util.Map;

@WrapForJNI
class ScreenManagerHelper {
    public native static void addDisplay(int displayType, int width, int height, float density);
    public native static void removeDisplay(int displayType);
}

/*
 * Service to keep the remote display running even when the app goes into the background
 */
public class RemotePresentationService extends CastRemoteDisplayLocalService {

    private static final String TAG = "RemotePresentationService";
    private CastPresentation mPresentation;
    private Display mDisplay;

    @Override
    public void onCreatePresentation(Display display) {
        mDisplay = display;
        createPresentation();
    }

    @Override
    public void onDismissPresentation() {
        dismissPresentation();
    }

    private void dismissPresentation() {
        if (mPresentation != null) {
            ScreenManagerHelper.removeDisplay(GeckoView.DISPLAY_VIRTUAL);
            mPresentation.dismiss();
            mPresentation = null;
        }
    }

    private void createPresentation() {
        dismissPresentation();

        DisplayMetrics metrics = new DisplayMetrics();
        mDisplay.getMetrics(metrics);
        ScreenManagerHelper.addDisplay(GeckoView.DISPLAY_VIRTUAL,
                                       metrics.widthPixels,
                                       metrics.heightPixels,
                                       metrics.density);

        mPresentation = new VirtualPresentation(this, mDisplay);

        try {
            mPresentation.show();
        } catch (WindowManager.InvalidDisplayException ex) {
            Log.e(TAG, "Unable to show presentation, display was removed.", ex);
            dismissPresentation();
        }
    }

}

/**
 * The presentation to show on the first screen (the TV).
 * <p>
 * Note that this display may have different metrics from the display on
 * which the main activity is showing so we must be careful to use the
 * presentation's own {@link Context} whenever we load resources.
 * </p>
 */
class VirtualPresentation extends CastPresentation {
    private final String TAG = "VirtualPresentation";
    private RelativeLayout mLayuot;
    private GeckoView mView;

    public VirtualPresentation(Context context, Display display) {
        super(context, display);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        /*
         * NOTICE: The context get from getContext() is different to the context
         * of the application. Presentaion has its own context to get correct
         * resources.
         */

        // Create new GeckoView
        mView = new GeckoView(getContext());
        mView.setLayoutParams(new LayoutParams(LayoutParams.MATCH_PARENT,
                                               LayoutParams.MATCH_PARENT));
        mView.setDisplayType(GeckoView.DISPLAY_VIRTUAL);

        // Create new layout to put the GeckoView
        mLayuot = new RelativeLayout(getContext());
        mLayuot.setLayoutParams(new LayoutParams(LayoutParams.MATCH_PARENT,
                                                 LayoutParams.MATCH_PARENT));
        mLayuot.addView(mView);

        setContentView(mLayuot);
    }
}
