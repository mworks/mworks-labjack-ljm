//
//  LabJackLJMUtilities.hpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 11/26/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#ifndef LabJackLJMUtilities_hpp
#define LabJackLJMUtilities_hpp

#define BEGIN_NAMESPACE_MW_LABJACK_LJM  BEGIN_NAMESPACE_MW         BEGIN_NAMESPACE(labjack_ljm)
#define END_NAMESPACE_MW_LABJACK_LJM    END_NAMESPACE(labjack_ljm) END_NAMESPACE_MW


BEGIN_NAMESPACE_MW_LABJACK_LJM


std::string formatError(int error, const char *description);


inline bool logError(int error, const char *description) {
    if (LJME_NOERROR != error) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "%s", formatError(error, description).c_str());
        return true;
    }
    return false;
}


inline void throwError(int error, const char *description) {
    if (LJME_NOERROR != error) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, formatError(error, description));
    }
}


END_NAMESPACE_MW_LABJACK_LJM


#endif /* LabJackLJMUtilities_hpp */
