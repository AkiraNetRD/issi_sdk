#ifndef IXXI_H
#define IXXI_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    STATUS_CODE_PARAMETER_ERROR = -6,
    STATUS_CODE_INVALID_MAC_ADDRESS = -5,
    STATUS_CODE_ISSI_API_RESULT_UNEXPECTED = -4,
	STATUS_CODE_RESULT_IS_NULL_OR_SIZE_ERROR = -3,
	STATUS_CODE_ISSI_API_FAILE = -2,
    STATUS_CODE_UNKNOWN_ERROR = -1,
	STATUS_CODE_OK = 0
};

#define GHN_ATTR_GHN_MAC "GhnMACAddress"
#define GHN_ATTR_DEVICE_NAME "Device Name"
#define GHN_ATTR_NODE_TYPE "Node Type"
#define GHN_ATTR_DOMAIN_NAME "DomainName"
#define GHN_ATTR_ENC_STATE "Encryption Status"
#define GHN_ATTR_ENC_PWD "Encryption Password"
#define GHN_ATTR_UPTIME "UpTime"
#define GHN_ATTR_FIRMWARE_VERSION "FirmwareVersion"
#define GHN_ATTR_BANDWIDTH "Bandwidth"
#define GHN_ATTR_NODE_TYPE_CONFIGURATION "NodeTypeConfiguration"


#define INIT_SDK_VARIABLE \
	sGet_stations_Information	getstation; \
	bool						bHasAdapterIP = FALSE; \
	bool						bHasDeviceMac = FALSE; \
	bool						bHasDeviceIP = FALSE; \
	char						strAdapterIP[GHN_LIB_IP_ADDRESS_LEN]; \
	char						strDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN]; \
	char						strDeviceIP[GHN_LIB_IP_ADDRESS_LEN];

#pragma pack(push, 1)
typedef struct _GhnDeviceInfo {
    char ghnMac[32];
    char devName[64];
    char nodeType;  // 1: DM, 0: RN
    char domainName[64];
    char encStatus; // 1: on, 0: off
    char encPwd[64];
    char upTime[32];
    char isMIMO;    // 1: MIMO 0: SISO
    char plcBandwidth[8];
    char plcFwVersion[16];
} GhnDeviceInfo;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _PhyRateData {
    char saMac[18];
    char daMac[15][18];
    int rate[15];
    char daCount;
} PhyRateData;
#pragma pack(pop)

int isDM(void);
int grabNeighborCount(void);
int grabLocalDeviceInfo(struct _GhnDeviceInfo *deviceInfo);
int grabAllDeviceInfo(struct _GhnDeviceInfo devInfoAry[], int arraySize);
int resetDevice(char *target);
int restoreFactoryDefault(char *target);
int getPhyRateTable(struct _PhyRateData *prData);
int firmwareUpdate(void);
int getNetworkEncryptionMode(char *target);
int setNetworkEncryptionMode(char *target, int mode);
int getNetworkDevicePassword(char *target, char *password);
int setNetworkDevicePassword(char *target, char *password);
int getNetworkDomainName(char *target, char *domainName);
int setNetworkDomainName(char *target, char *domainName);
int getDeviceName(char *target, char *name);
int setDeviceName(char *target, char *name);

#ifdef __cplusplus
}
#endif

#endif