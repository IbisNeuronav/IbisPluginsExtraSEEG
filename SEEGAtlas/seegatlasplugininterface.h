#ifndef __SEEGAtlasPluginInterface_h_
#define __SEEGAtlasPluginInterface_h_

#include "toolplugininterface.h"

class SEEGAtlasWidget;

class SEEGAtlasPluginInterface : public ToolPluginInterface
{

    Q_OBJECT
    Q_INTERFACES(IbisPlugin)
    Q_PLUGIN_METADATA(IID "Ibis.SEEGAtlasPluginInterface" )

public:

    SEEGAtlasPluginInterface();
    virtual ~SEEGAtlasPluginInterface();
    virtual QString GetPluginName() { return QString("SEEGAtlas"); }
    virtual QString GetPluginDescription();
    virtual bool CanRun();
    virtual QString GetMenuEntryString() { return QString("SEEG Atlas"); }

    virtual QWidget * CreateTab();

};
#endif
