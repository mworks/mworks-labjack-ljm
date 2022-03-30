//
//  LabJackLJMQuadratureChannel.cpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 3/28/22.
//  Copyright © 2022 The MWorks Project. All rights reserved.
//

#include "LabJackLJMQuadratureChannel.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


void QuadratureInputChannel::describeComponent(ComponentInfo &info) {
    MultipleLineChannel::describeComponent(info);
    info.setSignature("iochannel/labjack_ljm_quadrature_input");
}


QuadratureInputChannel::QuadratureInputChannel(const ParameterValueMap &parameters) :
    MultipleLineChannel(parameters)
{ }


void QuadratureInputChannel::resolveLines(DeviceInfo &deviceInfo) {
    MultipleLineChannel::resolveLines(deviceInfo);
    
    if (!deviceInfo.isQuadraturePhaseA(getFirstLine())) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                              boost::format("%s is not a quadrature input A line") % getFirstLineName());
    }
    
    int secondLine = getFirstLine() + 1;  // Quadrature inputs are always adjacent pairs
    if (!deviceInfo.reserveLine(secondLine)) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                              boost::format("Quadrature input requires two consecutive lines, "
                                            "but the line after %s is already in use")
                              % getFirstLineName());
    }
    
    canonicalFirstLineName = deviceInfo.getCanonicalLineName(getFirstLine());
    canonicalSecondLineName = deviceInfo.getCanonicalLineName(secondLine);
}


END_NAMESPACE_MW_LABJACK_LJM
