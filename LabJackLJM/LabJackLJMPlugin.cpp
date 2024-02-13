//
//  LabJackLJMPlugin.cpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 11/20/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "LabJackLJMAction.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


class Plugin : public mw::Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardComponentFactory, Device>();
        registry->registerFactory<StandardComponentFactory, AnalogInputChannel>();
        registry->registerFactory<StandardComponentFactory, AnalogOutputChannel>();
        registry->registerFactory<StandardComponentFactory, AnalogWaveformChannel>();
        registry->registerFactory<StandardComponentFactory, DigitalInputChannel>();
        registry->registerFactory<StandardComponentFactory, DigitalOutputChannel>();
        registry->registerFactory<StandardComponentFactory, WordInputChannel>();
        registry->registerFactory<StandardComponentFactory, WordOutputChannel>();
        registry->registerFactory<StandardComponentFactory, CounterChannel>();
        registry->registerFactory<StandardComponentFactory, QuadratureInputChannel>();
        registry->registerFactory<StandardComponentFactory, StartAnalogWaveformOutputAction>();
        registry->registerFactory<StandardComponentFactory, StopAnalogWaveformOutputAction>();
    }
};


extern "C" mw::Plugin * getPlugin() {
    return new Plugin();
}


END_NAMESPACE_MW_LABJACK_LJM
