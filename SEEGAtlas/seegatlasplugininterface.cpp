#include "seegatlasplugininterface.h"
#include "SEEGAtlasWidget.h"
#include <QtPlugin>

//Q_EXPORT_STATIC_PLUGIN2( SEEGAtlas, SEEGAtlasPluginInterface );

SEEGAtlasPluginInterface::SEEGAtlasPluginInterface()
{
    // The constructor is called when IBIS starts - so keep it simple
    // and write the initialization for the plugin in CreateTab()
}

SEEGAtlasPluginInterface::~SEEGAtlasPluginInterface()
{
    //Only called when IBIS is closed (not when the plugin is closed)
}

bool SEEGAtlasPluginInterface::CanRun()
{
    //check status of application elements
    // e.g. it could check whether  the acquisition is on
    return true;
}

QString SEEGAtlasPluginInterface::GetPluginDescription()
{
    return QString("<html><h1>SEEG Atlas plugin</h1><p>Author: Rina Zelmann</p><p>Description</p><p>This plugin is used to semi-automatically model depth electrodes and to estimate the anatomical location of each contact and channel.</p></html>");
}

QWidget * SEEGAtlasPluginInterface::CreateTab()
{
    //called when the pluggin is activated (from the menu)
    SEEGAtlasWidget * widget = new SEEGAtlasWidget;
    widget->SetPluginInterface(this);
    return widget;
}

