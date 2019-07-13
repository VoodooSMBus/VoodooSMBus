#include "VoodooSMBusControllerDriver.hpp"
#include "VoodooSMBusDeviceNub.hpp"

OSDefineMetaClassAndStructors(VoodooSMBusControllerDriver, IOService)

#define super IOService

bool VoodooSMBusControllerDriver::init(OSDictionary *dict) {
    bool result = super::init(dict);
    IOLog("Initializing\n");
    
    // For now, we support only one slave device
    device_nubs = OSArray::withCapacity(1);

    physical_device = reinterpret_cast<VoodooSMBusPhysicalDevice*>(IOMalloc(sizeof(VoodooSMBusPhysicalDevice)));
    
    physical_device->awake = true;
    return result;
}

void VoodooSMBusControllerDriver::free(void) {
    IOLog("Freeing\n");
    IOFree(physical_device, sizeof(VoodooSMBusPhysicalDevice));
    OSSafeReleaseNULL(device_nubs);
    super::free();
}

IOService *VoodooSMBusControllerDriver::probe(IOService *provider, SInt32 *score) {
    IOService *result = super::probe(provider, score);
    IOLog("Probing\n");
    
    return result;
}

bool VoodooSMBusControllerDriver::start(IOService *provider) {
    bool result = super::start(provider);
    IOLog("Starting\n");
    
    pci_device = OSDynamicCast(IOPCIDevice, provider);
    
    if (!(pci_device = OSDynamicCast(IOPCIDevice, provider))) {
        IOLog("Failed to cast provider\n");
        return false;
    }
   
    physical_device->provider = provider;
    physical_device->name = getMatchedName(physical_device->provider);
    
    pci_device->retain();
    if (!pci_device->open(this)) {
        IOLog("%s::%s Could not open provider\n", getName(), pci_device->getName());
        return false;
    }
    
    uint32_t host_config = pci_device->configRead8(SMBHSTCFG);
    if ((host_config & SMBHSTCFG_HST_EN) == 0) {
        IOLog("SMBus disabled\n");
        return false;
    }
    
    // TODO why 0xfffe
    physical_device->smba = pci_device->configRead16(SMBBAR) & 0xFFFE;
    if (host_config & SMBHSTCFG_SMB_SMI_EN) {
        IOLog("No PCI IRQ. Poll mode is not implemented. Unloading.\n");
        return false;
    }
    pci_device->setIOEnable(true);
    
    physical_device->original_slvcmd = pci_device->ioRead8(SMBSLVCMD(physical_device));
    
    enableHostNotify();
    
    publishNub();
    
    IOLog("Everything went well: %s", physical_device->name);
    return result;
}

void VoodooSMBusControllerDriver::stop(IOService *provider) {
    IOLog("Stopping\n");
    
    if (device_nubs) {
        while (device_nubs->getCount() > 0) {
            VoodooSMBusDeviceNub *device_nub = reinterpret_cast<VoodooSMBusDeviceNub*>(device_nubs->getLastObject());
            device_nub->detach(this);
            device_nubs->removeObject(device_nubs->getCount() - 1);
            OSSafeReleaseNULL(device_nub);
        }
    }
    
    disableHostNotify();
    
    pci_device->close(this);
    pci_device->release();
    
    super::stop(provider);
}


IOReturn VoodooSMBusControllerDriver::publishNub() {
    IOLog("%s::%s Publishing nub\n", getName(), physical_device->name);
    
    VoodooSMBusDeviceNub* device_nub = OSTypeAlloc(VoodooSMBusDeviceNub);
    
    
    if (!device_nub || !device_nub->init()) {
        IOLog("%s::%s Could not initialise nub", getName(), physical_device->name);
        goto exit;
    }
    
    if (!device_nub->attach(this)) {
        IOLog("%s::%s Could not attach nub", getName(), physical_device->name);
        goto exit;
    }
    
    if (!device_nub->start(this)) {
        IOLog("%s::%s Could not start nub", getName(), physical_device->name);
        goto exit;
    }
    
    device_nubs->setObject(device_nub);
    
    return kIOReturnSuccess;
    
exit:
    OSSafeReleaseNULL(device_nub);
    
    return kIOReturnError;
}


void VoodooSMBusControllerDriver::enableHostNotify() {
    
    if(!(physical_device->original_slvcmd & SMBSLVCMD_HST_NTFY_INTREN)) {
        pci_device->ioWrite8(SMBSLVCMD(physical_device), SMBSLVCMD_HST_NTFY_INTREN | physical_device->original_slvcmd);
    }

    /* clear Host Notify bit to allow a new notification */
    pci_device->ioWrite8(SMBSLVSTS(physical_device), SMBSLVSTS_HST_NTFY_STS);
}

void VoodooSMBusControllerDriver::disableHostNotify() {
    pci_device->ioWrite8(SMBSLVCMD(physical_device), physical_device->original_slvcmd);
}

