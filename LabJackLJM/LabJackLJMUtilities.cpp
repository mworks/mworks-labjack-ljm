//
//  LabJackLJMUtilities.cpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 11/26/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "LabJackLJMUtilities.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


std::string formatError(int error, const char *description) {
    std::array<char, LJM_MAX_NAME_SIZE> errorName;
    LJM_ErrorToString(error, errorName.data());
    return (boost::format("%s; error = %d (%s)") % description % error % errorName.data()).str();
}


END_NAMESPACE_MW_LABJACK_LJM
