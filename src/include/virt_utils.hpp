#ifndef __VIRT_UTILS_HPP__
#define __VIRT_UTILS_HPP__

#include <libvirt/libvirt.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

class VirtConnection {
private:
    virConnectPtr con;

public:
    VirtConnection(const char* path) {
        con = virConnectOpen(path);

        if(con == nullptr) {
            fprintf(stderr, "Failed to connect to %s\n", path);
            exit(1);
        }
    }

    ~VirtConnection() {
        virConnectClose(con);
    }

    virConnectPtr getConnection() {
        return con;
    }

    // MUST BE FREED
    int getDomains(virDomainPtr** domains, unsigned int flags = 0) {
        int ret = virConnectListAllDomains(con, domains, flags);
        if(ret < 0) {
            fprintf(stderr, "Failed to Get the List of Domains\n");
            exit(1);
        }

        return ret;
    }

};

namespace VirtUtils {
    static bool VirtualMachineOn(VirtConnection& con, const char* VM) {
        virDomainPtr* domains;
        int ret = con.getDomains(&domains);

        for(int i = 0; i < ret; i++) {
            virDomainInfo domainInfo;
            if(virDomainGetInfo(domains[i], &domainInfo) < 0) {
                fprintf(stderr, "Failed to Get the Domain Info\n");
                continue;
            }

            if(strcmp(virDomainGetName(domains[i]), VM) == 0) {
                switch(domainInfo.state) {
                case VIR_DOMAIN_RUNNING: return true;
                default: return false;
                }
            }

            virDomainFree(domains[i]);
        }

        free(domains);

        return false;
    }

};

#endif
