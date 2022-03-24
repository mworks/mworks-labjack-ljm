//
//  LabJackLJMCounterChannel.cpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 3/24/22.
//  Copyright © 2022 The MWorks Project. All rights reserved.
//

#include "LabJackLJMCounterChannel.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


void CounterChannel::describeComponent(ComponentInfo &info) {
    SingleLineChannel::describeComponent(info);
    info.setSignature("iochannel/labjack_ljm_counter");
}


void CounterChannel::resolveLine(DeviceInfo &deviceInfo) {
    SingleLineChannel::resolveLine(deviceInfo);
    if (!(deviceInfo.isHighSpeedCounter(getLine()))) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                              boost::format("%s is not a high-speed counter input line") % lineName);
    }
}


END_NAMESPACE_MW_LABJACK_LJM
