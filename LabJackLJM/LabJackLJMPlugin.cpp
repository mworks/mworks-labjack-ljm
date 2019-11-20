//
//  LabJackLJMPlugin.cpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 11/20/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "LabJackLJMDevice.hpp"


BEGIN_NAMESPACE_MW


class LabJackLJMPlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardComponentFactory, LabJackLJMDevice>();
    }
};


extern "C" Plugin * getPlugin() {
    return new LabJackLJMPlugin();
}


END_NAMESPACE_MW
