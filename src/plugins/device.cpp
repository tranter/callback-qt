/*
 *  Copyright 2011 Wolfgang Koller - http://www.gofg.at/
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "device.h"

#include "../pluginregistry.h"

#if QT_VERSION < 0x050000
#include <QSystemDeviceInfo>
#include <QSystemInfo>
#else
#include <QDeviceInfo>
#include <QtSystemInfo>
#endif

#include <QDebug>

#define PHONEGAP "1.0.0"

#ifdef QTM_NAMESPACE
QTM_USE_NAMESPACE
#endif

// Create static instance of ourself
Device* Device::m_device = new Device();

/**
 * Constructor - NOTE: Never do anything except registering the plugin
 */
Device::Device() : PGPlugin() {
    PluginRegistry::getRegistry()->registerPlugin( "com.phonegap.Device", this );
}

/**
 * Called by the javascript constructor in order to receive all required device info(s)
 */
void Device::getInfo( int scId, int ecId ) {
    Q_UNUSED(ecId)

#if QT_VERSION < 0x050000
    QSystemDeviceInfo *systemDeviceInfo = new QSystemDeviceInfo(this);
    QSystemInfo *systemInfo = new QSystemInfo(this);
#else
    QDeviceInfo *systemDeviceInfo = new QDeviceInfo(this);
    QDeviceInfo *systemInfo = new QDeviceInfo(this);
#endif

#ifdef Q_OS_SYMBIAN
    QString platform = "Symbian";
#endif
#ifdef Q_OS_WIN
    QString platform = "Windows";
#endif
#ifdef Q_OS_WINCE
    QString platform = "Windows CE";
#endif
#ifdef Q_OS_LINUX
    QString platform = "Linux";
#endif

#if QT_VERSION < 0x050000
    this->callback( scId, "'" + systemDeviceInfo->model() + "', '" + PHONEGAP + "', '" + platform + "', '" + systemDeviceInfo->uniqueDeviceID() + "', '" + systemInfo->version( QSystemInfo::Os ) + "'" );
#else
    this->callback( scId, "'" + systemDeviceInfo->model() + "', '" + PHONEGAP + "', '" + platform + "', '" + systemDeviceInfo->uniqueDeviceID() + "', '" + systemInfo->version( QDeviceInfo::Os ) + "'" );
#endif
}
