/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _plm_uid_prefix_h_
#define _plm_uid_prefix_h_

#define PLM_UID_PREFIX "1.2.826.0.1.3680043.8.274.1.1"
#define MONDOSHOT_UID_PREFIX "1.2.826.0.1.3680043.8.274.1.1.200"

#include <string>
#include <memory>

// Singleton class to store the UID prefix
class PlmUidPrefix {
private:
    std::string value;
    PlmUidPrefix() : value(PLM_UID_PREFIX) {}

public:
    PlmUidPrefix(const PlmUidPrefix&) = delete;
    PlmUidPrefix& operator=(const PlmUidPrefix&) = delete;

    static PlmUidPrefix& getInstance() {
      static PlmUidPrefix instance;
      return instance;
    }

    void set(const std::string& newValue) {
        value = newValue;
    }

    std::string get() const {
        return value;
    }
};

#endif
