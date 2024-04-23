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
        virDomainPtr domain = virDomainLookupByName(con.getConnection(), VM);

        virDomainInfo info;
        virDomainGetInfo(domain, &info);

        bool returnValue;
        switch(info.state) {
        case VIR_DOMAIN_RUNNING: returnValue = true; break;
        default: returnValue = false; break;
        }

        virDomainFree(domain);
        return returnValue;
    }

    static void toggleVM(VirtConnection& con, const char* VM) {
        virDomainPtr domain = virDomainLookupByName(con.getConnection(), VM);

        virDomainInfo info;
        virDomainGetInfo(domain, &info);

        printf("%u\n", info.state);
        switch(info.state) {
        case VIR_DOMAIN_RUNNING: virDomainShutdown(domain); break;
        case VIR_DOMAIN_PAUSED: virDomainResume(domain); break;
        case VIR_DOMAIN_SHUTDOWN: case VIR_DOMAIN_SHUTOFF: virDomainCreate(domain); break;
        default: break;
        }

        virDomainFree(domain);
    }

};

#endif
