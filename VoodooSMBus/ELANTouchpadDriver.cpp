
#include "ELANTouchpadDriver.hpp"


#define super IOService

OSDefineMetaClassAndStructors(ELANTouchpadDriver, VoodooSMBusSlaveDeviceDriver);


bool ELANTouchpadDriver::init(OSDictionary *dict) {
    bool result = super::init(dict);
   
    data = reinterpret_cast<elan_tp_data*>(IOMalloc(sizeof(elan_tp_data)));
    return result;
}

void ELANTouchpadDriver::free(void) {
   
    IOFree(data, sizeof(elan_tp_data));
    super::free();
}

void ELANTouchpadDriver::releaseResources() {
    OSSafeReleaseNULL(device_nub);
}

bool ELANTouchpadDriver::start(IOService* provider) {
    if (!super::start(provider)) {
        return false;
    }

    if(tryInitialize()) {
        goto exit;
    }

    return true;
exit:
    releaseResources();
    return false;
}




void ELANTouchpadDriver::stop(IOService* provider) {
    releaseResources();
    super::stop(provider);
}

ELANTouchpadDriver* ELANTouchpadDriver::probe(IOService* provider, SInt32* score) {
    IOLog("Touchpad probe\n");
    if (!super::probe(provider, score)) {
        return NULL;
    }
    
    device_nub = OSDynamicCast(VoodooSMBusDeviceNub, provider);
    if (!device_nub) {
        IOLog("%s Could not get VoodooSMBus device nub instance\n", getName());
        return NULL;
    }
    return this;
}


int ELANTouchpadDriver::tryInitialize() {
    int repeat = ETP_RETRY_COUNT;
    int error;
    do {
        error = initialize();
        if (!error)
            return 0;
        
        IOSleep(30);
    } while (--repeat > 0);
    return error;
}

void ELANTouchpadDriver::handleHostNotify() {
    thread_t new_thread;
    kern_return_t ret = kernel_thread_start(OSMemberFunctionCast(thread_continue_t, this, &ELANTouchpadDriver::handleHostNotifyThreaded), this, &new_thread);
    
    if (ret != KERN_SUCCESS) {
        IOLogDebug(" Thread error while attemping to get input report\n");
    } else {
        thread_deallocate(new_thread);
    }
}

void ELANTouchpadDriver::handleHostNotifyThreaded() {
    int error;
    u8 report[ETP_MAX_REPORT_LEN];
    error = getReport(report);
    if (error) {
        return;
    }
    
    switch (report[ETP_REPORT_ID_OFFSET]) {
        case ETP_REPORT_ID:
            reportAbsolute(report);
            break;
        case ETP_TP_REPORT_ID:
            reportTrackpoint(report);
            break;
        default:
            IOLogError("invalid report id data (%x)\n",
                    report[ETP_REPORT_ID_OFFSET]);
    }
}



// elan_smbus_initialize
int ELANTouchpadDriver::initialize() {
    UInt8 check[ETP_SMBUS_HELLOPACKET_LEN] = { 0x55, 0x55, 0x55, 0x55, 0x55 };
    UInt8 values[I2C_SMBUS_BLOCK_MAX] = {0};
    int len, error;
    
    /* Get hello packet */
    len = device_nub->ReadBlockData(ETP_SMBUS_HELLOPACKET_CMD, values);
    
    if (len != ETP_SMBUS_HELLOPACKET_LEN) {
        IOLog("hello packet length fail: %d\n", len);
        error = len < 0 ? len : -EIO;
        return error;
    }
    
    /* compare hello packet */
    if (memcmp(values, check, ETP_SMBUS_HELLOPACKET_LEN)) {
        IOLog("hello packet fail [%*ph]\n",
                ETP_SMBUS_HELLOPACKET_LEN, values);
        return -ENXIO;
    }
    
    /* enable tp */
    error = device_nub->WriteByte(ETP_SMBUS_ENABLE_TP);
    if (error) {
        IOLog("failed to enable touchpad: %d\n", error);
        return error;
    }
    
    u8 mode = ETP_ENABLE_ABS;
    error = setMode(mode);
    if (error) {
        IOLogDebug("failed to switch to absolute mode: %d\n", error);
        return error;
    }

    
    return 0;
}

int ELANTouchpadDriver::setMode(u8 mode) {
    u8 cmd[4] = { 0x00, 0x07, 0x00, mode };
    
    return device_nub->writeBlockData(ETP_SMBUS_IAP_CMD, sizeof(cmd), cmd);
}

// TODO lets query stuff
bool ELANTouchpadDriver::setDeviceParameters() {
    
    u8 hw_x_res = 1, hw_y_res = 1;
    unsigned int x_traces = 1, y_traces = 1;
    
    data->max_x = 3052;
    data->max_y = 1888;
    data->width_x = data->max_x / x_traces;
    data->width_y = data->max_x / y_traces;
    
    data->pressure_adjustment = 25;
    
    data->x_res = convertResolution(hw_x_res);
    data->y_res = convertResolution(hw_y_res);
    
    return true;
}

// elan_convert_resolution
unsigned int ELANTouchpadDriver::convertResolution(u8 val) {
    /*
     * (value from firmware) * 10 + 790 = dpi
     *
     * We also have to convert dpi to dots/mm (*10/254 to avoid floating
     * point).
     */
    
    return ((int)(char)val * 10 + 790) * 10 / 254;
}


// elan_smbus_get_report
int ELANTouchpadDriver::getReport(u8 *report)
{
    int len;
    
    len = device_nub->ReadBlockData(ETP_SMBUS_PACKET_QUERY,
                                    &report[ETP_SMBUS_REPORT_OFFSET]);
    if (len < 0) {
        IOLogError("failed to read report data: %d\n", len);
        return len;
    }
    
    if (len != ETP_SMBUS_REPORT_LEN) {
        IOLogError("wrong report length (%d vs %d expected)\n", len,
                   ETP_SMBUS_REPORT_LEN);
        return -EIO;
    }
    
    return 0;
}

void ELANTouchpadDriver::reportTrackpoint(u8 *report) {
    IOLogDebug("reporting trackpoint\n");
}

// elan_report_contact
void ELANTouchpadDriver::reportContact(int contact_num, bool contact_valid, u8 *finger_data) {
    unsigned int pos_x, pos_y;
    unsigned int pressure, mk_x, mk_y;
    unsigned int area_x, area_y, major, minor;
    unsigned int scaled_pressure;
    
    if (contact_valid) {
        pos_x = ((finger_data[0] & 0xf0) << 4) |
        finger_data[1];
        pos_y = ((finger_data[0] & 0x0f) << 8) |
        finger_data[2];
        mk_x = (finger_data[3] & 0x0f);
        mk_y = (finger_data[3] >> 4);
        pressure = finger_data[4];
        
        if (pos_x > data->max_x || pos_y > data->max_y) {
            IOLogDebug("[%d] x=%d y=%d over max (%d, %d)",
                    contact_num, pos_x, pos_y,
                    data->max_x, data->max_y);
            return;
        }
        
        /*
         * To avoid treating large finger as palm, let's reduce the
         * width x and y per trace.
         */
        area_x = mk_x * (data->width_x - ETP_FWIDTH_REDUCE);
        area_y = mk_y * (data->width_y - ETP_FWIDTH_REDUCE);
        
        major = max(area_x, area_y);
        minor = min(area_x, area_y);
        
        scaled_pressure = pressure + data->pressure_adjustment;
        
        if (scaled_pressure > ETP_MAX_PRESSURE)
            scaled_pressure = ETP_MAX_PRESSURE;
        
        IOLogDebug("contact_valid_finger: %d", contact_num);
//        input_mt_slot(input, contact_num);
//        input_mt_report_slot_state(input, MT_TOOL_FINGER, true);
        IOLogDebug(" ABS_MT_POSITION_X %du", pos_x);
        IOLogDebug(" ABS_MT_POSITION_Y %du", data->max_y - pos_y);
        IOLogDebug(" ABS_MT_PRESSURE %du", scaled_pressure);
        IOLogDebug(" ABS_TOOL_WIDTH %du", mk_x);
        IOLogDebug(" ABS_MT_TOUCH_MAJOR %du", major);
        IOLogDebug(" ABS_MT_TOUCH_MINOR %du", minor);
    } else {
        IOLogDebug("no_contact_finger: %d", contact_num);
//        input_mt_report_slot_state(input, MT_TOOL_FINGER, false);
    }
}

// elan_report_absolute
void ELANTouchpadDriver::reportAbsolute(u8 *packet) {
    u8 *finger_data = &packet[ETP_FINGER_DATA_OFFSET];
    int i;
    u8 tp_info = packet[ETP_TOUCH_INFO_OFFSET];
    u8 hover_info = packet[ETP_HOVER_INFO_OFFSET];
    bool contact_valid, hover_event;
    
    hover_event = hover_info & 0x40;
    for (i = 0; i < ETP_MAX_FINGERS; i++) {
        contact_valid = tp_info & (1U << (3 + i));
        reportContact(i, contact_valid, finger_data);
        
        if (contact_valid)
            finger_data += ETP_FINGER_DATA_LEN;
    }
    
    IOLogDebug("BTN_LEFT %lu",   tp_info & BIT(0));
    IOLogDebug("BTN_MIDDLE %lu", tp_info & BIT(2));
    IOLogDebug("BTN_RIGHT %lu",  tp_info & BIT(1));
    IOLogDebug("ABS_DISTANCE %d", hover_event != 0);
//    input_mt_report_pointer_emulation(input, true);
//    input_sync(input);
}

