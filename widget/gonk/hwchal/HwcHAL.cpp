/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim:set ts=4 sw=4 sts=4 et: */
/*
 * Copyright (c) 2015 The Linux Foundation. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "HwcHAL.h"
#include "libdisplay/GonkDisplay.h"
#include "mozilla/Assertions.h"

#include <ui/GraphicBuffer.h>
#include "cutils/properties.h"

namespace mozilla {

HwcHAL::HwcHAL()
    : HwcHALBase()
{
    // Some HALs don't want to open hwc twice.
    // If GetDisplay already load hwc module, we don't need to load again
    mHwc = (HwcDevice*)GetGonkDisplay()->GetHWCDevice();
    if (!mHwc) {
        printf_stderr("HwcHAL Error: Cannot load hwcomposer");
        return;
    }
}

HwcHAL::~HwcHAL()
{
    mHwc = nullptr;
}

bool
HwcHAL::Query(QueryType aType)
{
    if (!mHwc || !mHwc->query) {
        return false;
    }

    bool value = false;
    int supported = 0;
    if (mHwc->query(mHwc, static_cast<int>(aType), &supported) == 0/*android::NO_ERROR*/) {
        value = !!supported;
    }
    return value;
}

int
HwcHAL::Set(HwcList *aList,
            uint32_t aDisp)
{
    MOZ_ASSERT(mHwc);
    if (!mHwc) {
        return -1;
    }

    HwcList *displays[HWC_NUM_DISPLAY_TYPES] = { nullptr };
    displays[aDisp] = aList;

    if (aDisp == 1) {
        printf_stderr("--- <kuoe0> HwcHal::Set Begin ---");
        printf_stderr("<kuoe0> Layer Num: %d", aList->numHwLayers);
        for (size_t i = 0; i < aList->numHwLayers; ++i) {
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | hints: %d", i, aDisp, aList->hwLayers[i].hints);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | flags: %d", i, aDisp, aList->hwLayers[i].flags);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | transform: %d", i, aDisp, aList->hwLayers[i].transform);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | handle: %p", i, aDisp, aList->hwLayers[i].handle);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | blending: %d", i, aDisp, aList->hwLayers[i].blending);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | compositionType: %d", i, aDisp, aList->hwLayers[i].compositionType);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | acquireFenceFd: %d", i, aDisp, aList->hwLayers[i].acquireFenceFd);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | releaseFenceFd: %d", i, aDisp, aList->hwLayers[i].releaseFenceFd);

            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | displayFrame.left: %d", i, aDisp, aList->hwLayers[i].displayFrame.left);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | displayFrame.right: %d", i, aDisp, aList->hwLayers[i].displayFrame.right);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | displayFrame.top: %d", i, aDisp, aList->hwLayers[i].displayFrame.top);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | displayFrame.bottom: %d", i, aDisp, aList->hwLayers[i].displayFrame.bottom);

            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | sourceCrop.left: %d", i, aDisp, aList->hwLayers[i].sourceCrop.left);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | sourceCrop.right: %d", i, aDisp, aList->hwLayers[i].sourceCrop.right);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | sourceCrop.top: %d", i, aDisp, aList->hwLayers[i].sourceCrop.top);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | sourceCrop.bottom): %d", i, aDisp, aList->hwLayers[i].sourceCrop.bottom);

            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | # rect in visibleRegionScreen: %d", i, aDisp, aList->hwLayers[i].visibleRegionScreen.numRects);
            printf_stderr("-----");
        }
    }

    return mHwc->set(mHwc, HWC_NUM_DISPLAY_TYPES, displays);
}

int
HwcHAL::ResetHwc()
{
    return Set(nullptr, HWC_DISPLAY_PRIMARY);
}

int
HwcHAL::Prepare(HwcList *aList,
                uint32_t aDisp,
                hwc_rect_t aDispRect,
                buffer_handle_t aHandle,
                int aFenceFd)
{
    MOZ_ASSERT(mHwc);
    if (!mHwc) {
        printf_stderr("HwcHAL Error: HwcDevice doesn't exist. A fence might be leaked.");
        return -1;
    }

    HwcList *displays[HWC_NUM_DISPLAY_TYPES] = { nullptr };
    displays[aDisp] = aList;

#if ANDROID_VERSION >= 18
    aList->outbufAcquireFenceFd = -1;
    aList->outbuf = nullptr;
#endif
    aList->retireFenceFd = -1;

    const auto idx = aList->numHwLayers - 1;
    aList->hwLayers[idx].hints = 0;
    aList->hwLayers[idx].flags = 0;
    aList->hwLayers[idx].transform = 0;
    aList->hwLayers[idx].handle = aHandle;
    aList->hwLayers[idx].blending = HWC_BLENDING_PREMULT;
    aList->hwLayers[idx].compositionType = HWC_FRAMEBUFFER_TARGET;
    SetCrop(aList->hwLayers[idx], aDispRect);
    aList->hwLayers[idx].displayFrame = aDispRect;
    aList->hwLayers[idx].visibleRegionScreen.numRects = 1;
    aList->hwLayers[idx].visibleRegionScreen.rects = &aList->hwLayers[idx].displayFrame;
    aList->hwLayers[idx].acquireFenceFd = aFenceFd;
    aList->hwLayers[idx].releaseFenceFd = -1;
#if ANDROID_VERSION >= 18
    aList->hwLayers[idx].planeAlpha = 0xFF;
#endif

    if (aDisp == 1) {
        printf_stderr("--- <kuoe0> HwcHal::Prepare Begin ---");
        printf_stderr("Layer Num: %d", aList->numHwLayers);
        for (size_t i = 0; i < aList->numHwLayers; ++i) {
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | hints: %d", i, aDisp, aList->hwLayers[i].hints);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | flags: %d", i, aDisp, aList->hwLayers[i].flags);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | transform: %d", i, aDisp, aList->hwLayers[i].transform);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | handle: %p", i, aDisp, aList->hwLayers[i].handle);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | blending: %d", i, aDisp, aList->hwLayers[i].blending);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | compositionType: %d", i, aDisp, aList->hwLayers[i].compositionType);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | acquireFenceFd: %d", i, aDisp, aList->hwLayers[i].acquireFenceFd);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | releaseFenceFd: %d", i, aDisp, aList->hwLayers[i].releaseFenceFd);

            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | displayFrame.left: %d", i, aDisp, aList->hwLayers[i].displayFrame.left);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | displayFrame.right: %d", i, aDisp, aList->hwLayers[i].displayFrame.right);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | displayFrame.top: %d", i, aDisp, aList->hwLayers[i].displayFrame.top);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | displayFrame.bottom: %d", i, aDisp, aList->hwLayers[i].displayFrame.bottom);

            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | sourceCrop.left: %d", i, aDisp, aList->hwLayers[i].sourceCrop.left);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | sourceCrop.right: %d", i, aDisp, aList->hwLayers[i].sourceCrop.right);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | sourceCrop.top: %d", i, aDisp, aList->hwLayers[i].sourceCrop.top);
            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | sourceCrop.bottom): %d", i, aDisp, aList->hwLayers[i].sourceCrop.bottom);

            printf_stderr("<kuoe0> Layer #%d | DisplayType: %d | # rect in visibleRegionScreen: %d", i, aDisp, aList->hwLayers[i].visibleRegionScreen.numRects);

            printf_stderr("-----");
            /* for (int j = 0; j < aList->hwLayers[i].visibleRegionScreen.numRects; ++j) { */
            /*     printf_stderr("Layer #%d | DisplayType: %d | visibleRect #%d (left): %d", i, aDisp, j, aList->hwLayers[i].visibleRegionScreen.rects[j].left); */
            /*     printf_stderr("Layer #%d | DisplayType: %d | visibleRect #%d (right): %d", i, aDisp, j, aList->hwLayers[i].visibleRegionScreen.rects[j].right); */
            /*     printf_stderr("Layer #%d | DisplayType: %d | visibleRect #%d (top): %d", i, aDisp, j, aList->hwLayers[i].visibleRegionScreen.rects[j].top); */
            /*     printf_stderr("Layer #%d | DisplayType: %d | visibleRect #%d (bottom): %d", i, aDisp, j, aList->hwLayers[i].visibleRegionScreen.rects[j].bottom); */
            /* } */
        }

        /* if (aDisp == 1 && aList->numHwLayers == 2) { */

        /*     android::sp<android::GraphicBuffer> mBuffer = new android::GraphicBuffer(aList->hwLayers[1].handle); */

        /*     void *locked_buffer=nullptr; */
        /*     uint8_t *byte_buffer=nullptr; */
        /*     uint32_t *pixel_buffer=nullptr; */

        /*     //lock buffer */
        /*     if(mBuffer->lock(android::GraphicBuffer::USAGE_SW_WRITE_NEVER | */
        /*                    android::GraphicBuffer::USAGE_SW_READ_OFTEN, &locked_buffer)!=0){ */
        /*         printf_stderr("bignose lock failed"); */
        /*         return false; */
        /*     } */

        /*     /1* char propValue[PROPERTY_VALUE_MAX]; *1/ */
        /*     /1* property_get("debug.screen", propValue, "0"); *1/ */
        /*     /1* bool value = (atoi(propValue) == 1) ? true : false; *1/ */

        /*     /1* if (value) { *1/ */
        /*     /1*     // bignose dump to file *1/ */
        /*     /1*     static int count=0; *1/ */
        /*     /1*     ++count; *1/ */

        /*     /1*     std::stringstream sstream; *1/ */
        /*     /1*     sstream << "/data/local/tmp/hwc_path_" << count << ".data"; *1/ */

        /*     /1*     FILE *pWritingFile = fopen(sstream.str().c_str(), "wb+"); *1/ */
        /*     /1*     if (pWritingFile) { *1/ */
        /*     /1*         printf_stderr("bignose write hwc(%d,%d) to file",1024,768); *1/ */
        /*     /1*         fwrite(locked_buffer, 1024*768*4, 1, pWritingFile); *1/ */
        /*     /1*         fclose(pWritingFile); *1/ */
        /*     /1*     } *1/ */
        /*     /1* } *1/ */

        /*     //unlock buffer */
        /*     if(mBuffer->unlock()!=0){ */
        /*         printf_stderr("bignose unlock failed"); */
        /*         return false; */
        /*     } */
        /* } */
    }

    return mHwc->prepare(mHwc, HWC_NUM_DISPLAY_TYPES, displays);
}

bool
HwcHAL::SupportTransparency() const
{
#if ANDROID_VERSION >= 18
    return true;
#endif
    return false;
}

uint32_t
HwcHAL::GetGeometryChangedFlag(bool aGeometryChanged) const
{
#if ANDROID_VERSION >= 19
    return aGeometryChanged ? HWC_GEOMETRY_CHANGED : 0;
#else
    return HWC_GEOMETRY_CHANGED;
#endif
}

void
HwcHAL::SetCrop(HwcLayer &aLayer,
                const hwc_rect_t &aSrcCrop) const
{
    if (GetAPIVersion() >= HwcAPIVersion(1, 3)) {
#if ANDROID_VERSION >= 19
        aLayer.sourceCropf.left = aSrcCrop.left;
        aLayer.sourceCropf.top = aSrcCrop.top;
        aLayer.sourceCropf.right = aSrcCrop.right;
        aLayer.sourceCropf.bottom = aSrcCrop.bottom;
#endif
    } else {
        aLayer.sourceCrop = aSrcCrop;
    }
}

bool
HwcHAL::EnableVsync(bool aEnable)
{
    // Only support hardware vsync on kitkat, L and up due to inaccurate timings
    // with JellyBean.
#if (ANDROID_VERSION == 19 || ANDROID_VERSION >= 21)
    if (!mHwc) {
        return false;
    }
    return !mHwc->eventControl(mHwc,
                               HWC_DISPLAY_PRIMARY,
                               HWC_EVENT_VSYNC,
                               aEnable);
#else
    return false;
#endif
}

bool
HwcHAL::RegisterHwcEventCallback(const HwcHALProcs_t &aProcs)
{
    if (!mHwc || !mHwc->registerProcs) {
        printf_stderr("Failed to get hwc\n");
        return false;
    }

    // Disable Vsync first, and then register callback functions.
    mHwc->eventControl(mHwc,
                       HWC_DISPLAY_PRIMARY,
                       HWC_EVENT_VSYNC,
                       false);
    static const hwc_procs_t sHwcJBProcs = {aProcs.invalidate,
                                            aProcs.vsync,
                                            aProcs.hotplug};
    mHwc->registerProcs(mHwc, &sHwcJBProcs);

    // Only support hardware vsync on kitkat, L and up due to inaccurate timings
    // with JellyBean.
#if (ANDROID_VERSION == 19 || ANDROID_VERSION >= 21)
    return true;
#else
    return false;
#endif
}

uint32_t
HwcHAL::GetAPIVersion() const
{
    if (!mHwc) {
        // default value: HWC_MODULE_API_VERSION_0_1
        return 1;
    }
    return mHwc->common.version;
}

// Create HwcHAL
UniquePtr<HwcHALBase>
HwcHALBase::CreateHwcHAL()
{
    return Move(MakeUnique<HwcHAL>());
}

} // namespace mozilla
