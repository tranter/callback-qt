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

#include "phonegap.h"
#include "pluginregistry.h"
#include "pgwebpage.h"

#include <QDebug>
#include <QXmlStreamReader>
#include <QCoreApplication>

PhoneGap::PhoneGap(QWebView *webView) : QObject(webView) {
    m_webView = webView;
    // Configure web view
    m_webView->settings()->enablePersistentStorage();

    // Listen to load finished signal
    QObject::connect( m_webView, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)) );

    // Set our own WebPage class
    m_webView->setPage( new PGWebPage() );

    // Read the phonegap.xml options config file
    readConfigFile();

    if (QCoreApplication::instance()->arguments().size() == 2) {
        // If user specified a URL on the command line, use that
        m_webView->load( QUrl::fromUserInput(QCoreApplication::instance()->arguments()[1]));
    } else {
        // Otherwise load www/index.html relative to current working directory

        // Determine index file path
        m_workingDir = QDir::current();
        QDir wwwDir( m_workingDir );
        wwwDir.cd( "www" );

        // Load the correct startup file
        m_webView->load( QUrl::fromUserInput(wwwDir.absoluteFilePath("index.html")) );
        //m_webView->load( QUrl::fromUserInput("http://html5test.com/") );
    }
}

/**
 * Called when the webview finished loading a new page
 */
void PhoneGap::loadFinished( bool ok ) {
    Q_UNUSED(ok)

    // Change into the xml-directory
    QDir xmlDir( m_workingDir );
    xmlDir.cd( "xml" );

    // Try to open the plugins configuration
    QFile pluginsXml( xmlDir.filePath("plugins.xml") );
    if( !pluginsXml.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
        qDebug() << "Error loading plugins config!";
        return;
    }

    // Start reading the file as a stream
    QXmlStreamReader plugins;
    plugins.setDevice( &pluginsXml );

    // Get a reference to the current main-frame
    QWebFrame *webFrame = m_webView->page()->mainFrame();

    // Iterate over plugins-configuration and load all according plugins
    while(!plugins.atEnd()) {
        if( plugins.readNext() == QXmlStreamReader::StartElement ) {
            // Check if we have a plugin element
            if( plugins.name() == "plugin" ) {
                QXmlStreamAttributes attribs = plugins.attributes();
                // Check for name & value attributes
                if( attribs.hasAttribute("name") && attribs.hasAttribute("value") ) {
                    // Construct object & attribute names
                    QString attribName = attribs.value( "name" ).toString();
                    QString attribValue = attribs.value( "value" ).toString();
                    QString objectName = attribName + "_native";

                    qDebug() << "Adding Plugin " << attribName << " with " << attribValue;
                    // Check for such a plugin
                    PGPlugin *currPlugin = PluginRegistry::getRegistry()->getPlugin( attribValue );
                    if( currPlugin != NULL ) {
                        currPlugin->setWebFrame( webFrame );
                        webFrame->addToJavaScriptWindowObject( objectName, currPlugin );

                        webFrame->evaluateJavaScript( "PhoneGap.Qt.registerObject( '" + attribValue + "', " + objectName + " )" );
                        webFrame->evaluateJavaScript( "PhoneGap.enablePlugin( '" + attribValue + "' )" );
                    }
                    else {
                        qDebug() << "Unknown Plugin " << attribName;
                    }
                }
            }
        }
    }

    // Device is now ready to rumble
    webFrame->evaluateJavaScript( "PhoneGap.deviceready();" );
}

void PhoneGap::readConfigFile() {
    // Change into the xml-directory
    QDir xmlDir( m_workingDir );
    xmlDir.cd( "xml" );

    // Try to open the PhoneGap configuration file
    QFile phoneGapXml( xmlDir.filePath("phonegap.xml") );
    if( !phoneGapXml.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
        qDebug() << "Error loading phonegap config!";
    } else {

        // Start reading the file as a stream
        QXmlStreamReader config;
        config.setDevice( &phoneGapXml );

        // Iterate over config file and set options according to values
        //qDebug() << "Reading config options from phonegap.xml";
        while (!config.atEnd()) {
            if (config.readNext() == QXmlStreamReader::StartElement) {
                if( config.name() == "option" ) {

                    if( config.attributes().value("name") == "developerextras" ) {
                        // Add Inspect to context menu. Useful for debug. May want to remove for production code.
                        //qDebug() << "  Developer extras:" << config.attributes().value("enabled");
                        m_webView->settings()->setAttribute( QWebSettings::DeveloperExtrasEnabled, config.attributes().value("enabled") == "true");

                    } else if(config.attributes().value("name") == "localstorage" ) {
                        //qDebug() << "  Local storage:" << config.attributes().value("enabled");
                        m_webView->settings()->setAttribute( QWebSettings::LocalStorageEnabled, config.attributes().value("enabled") == "true");

                    } else if(config.attributes().value("name") == "offlinestoragedatabase" ) {
                        //qDebug() << "  Offline storage database:" << config.attributes().value("enabled");
                        m_webView->settings()->setAttribute( QWebSettings::OfflineStorageDatabaseEnabled, config.attributes().value("enabled") == "true");

                    } else if(config.attributes().value("name") == "localcontentcanaccessremoteurls" ) {
                        //qDebug() << "  Local content can access remote urls:" << config.attributes().value("enabled");
                        m_webView->settings()->setAttribute( QWebSettings::LocalContentCanAccessRemoteUrls, config.attributes().value("enabled") == "true");

                    } else if(config.attributes().value("name") == "offlinewebapplicationcache" ) {
                        //qDebug() << "  Offline web application cache:" << config.attributes().value("enabled");
                        m_webView->settings()->setAttribute( QWebSettings::OfflineWebApplicationCacheEnabled, config.attributes().value("enabled") == "true");

                    } else if(config.attributes().value("name") == "plugins" ) {
                        //qDebug() << "  Plugins:" << config.attributes().value("enabled");
                        m_webView->settings()->setAttribute( QWebSettings::PluginsEnabled, config.attributes().value("enabled") == "true");

                    } else {
                        qDebug() << "Unknown config option" << config.attributes().value("name");
                    }
                }
            }
        }
    }
}
