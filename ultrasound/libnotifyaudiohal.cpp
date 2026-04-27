/*
 * Copyright (C) 2023 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "libnotifyaudiohal"

#include <android/hardware/audio/6.0/IDevicesFactory.h>
#include <log/log.h>
#include <string.h>

#include <string>

using android::sp;
using android::hardware::audio::V6_0::IDevicesFactory;
using android::hardware::audio::V6_0::IPrimaryDevice;
using android::hardware::audio::V6_0::Result;

static sp<IDevicesFactory> gFactory;
static sp<IPrimaryDevice> gPrimaryDevice;

static sp<IPrimaryDevice> getPrimaryDevice() {
    if (gPrimaryDevice != nullptr) {
        return gPrimaryDevice;
    }
    gFactory = IDevicesFactory::getService();
    if (gFactory == nullptr) {
        ALOGE("Failed to get audio devices factory");
        return nullptr;
    }
    auto ret = gFactory->openPrimaryDevice(
            [&](Result retval, const sp<IPrimaryDevice>& result) {
                if (retval == Result::OK) {
                    gPrimaryDevice = result;
                } else {
                    ALOGE("Failed to open primary audio device, retval=%d",
                          static_cast<int>(retval));
                }
            });
    if (!ret.isOk()) {
        ALOGE("openPrimaryDevice transaction failed: %s",
              ret.description().c_str());
        gFactory = nullptr;
        gPrimaryDevice = nullptr;
    }
    return gPrimaryDevice;
}

void ultrasound_enable(int enable) {
    ALOGD("ultrasound_enable: %d", enable);
    sp<IPrimaryDevice> device = getPrimaryDevice();
    if (device == nullptr) {
        ALOGE("No primary audio device");
        return;
    }
    auto ret = device->setParameters(
            {} /* context */,
            {
                    {"ultrasound_enable", std::to_string(enable)},
            });
    if (!ret.isOk()) {
        ALOGE("setParameters transaction failed: %s",
              ret.description().c_str());
        gFactory = nullptr;
        gPrimaryDevice = nullptr;
    }
}

extern "C" void elliptic_notify_audio_hal(char* param) {
    ALOGD("elliptic_notify_audio_hal: %s", param);
    if (!strcmp(param, "ultrasound_enable=1")) {
        ultrasound_enable(1);
    } else if (!strcmp(param, "ultrasound_enable=0")) {
        ultrasound_enable(0);
    }
}

extern "C" void elliptic_ultrasound_supported() {
    return;
}
